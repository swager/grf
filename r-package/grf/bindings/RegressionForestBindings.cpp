/*-------------------------------------------------------------------------------
  This file is part of generalized random forest (grf).

  grf is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  grf is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with grf. If not, see <http://www.gnu.org/licenses/>.
 #-------------------------------------------------------------------------------*/

#include <map>
#include <Rcpp.h>
#include <sstream>
#include <vector>

#include "commons/globals.h"
#include "Eigen/Sparse"
#include "forest/ForestPredictors.h"
#include "forest/ForestTrainers.h"
#include "RcppUtilities.h"

// [[Rcpp::export]]
Rcpp::List regression_train(Rcpp::NumericMatrix train_matrix,
                            Eigen::SparseMatrix<double> sparse_train_matrix,
                            size_t outcome_index,
                            size_t sample_weight_index,
                            unsigned int mtry,
                            unsigned int num_trees,
                            unsigned int num_threads,
                            unsigned int min_node_size,
                            double sample_fraction,
                            unsigned int seed,
                            bool honesty,
                            double honesty_fraction,
                            size_t ci_group_size,
                            double alpha,
                            double imbalance_penalty,
                            std::vector<size_t> clusters,
                            unsigned int samples_per_cluster) {
  ForestTrainer trainer = ForestTrainers::regression_trainer();

  Data* data = RcppUtilities::convert_data(train_matrix, sparse_train_matrix);
  data->set_outcome_index(outcome_index - 1);
  if(sample_weight_index - 1 != data->get_num_cols()) {
      data->set_weight_index(sample_weight_index - 1);
  }
  data->sort();

  ForestOptions options(num_trees, ci_group_size, sample_fraction, mtry, min_node_size, honesty,
      honesty_fraction, alpha, imbalance_penalty, num_threads, seed, clusters, samples_per_cluster);

  Forest forest = trainer.train(data, options);
  Rcpp::List result = RcppUtilities::create_forest_object(forest);
  result.push_back(options.get_tree_options().get_min_node_size(), "min.node.size");

  delete data;
  return result;
}

// [[Rcpp::export]]
Rcpp::List regression_predict(Rcpp::List forest_object,
                              Rcpp::NumericMatrix train_matrix,
                              Eigen::SparseMatrix<double> sparse_train_matrix,
                              size_t outcome_index,
                              Rcpp::NumericMatrix test_matrix,
                              Eigen::SparseMatrix<double> sparse_test_matrix,
                              unsigned int num_threads,
                              unsigned int estimate_variance) {
  Data* train_data = RcppUtilities::convert_data(train_matrix, sparse_train_matrix);
  train_data->set_outcome_index(outcome_index - 1);

  Data* data = RcppUtilities::convert_data(test_matrix, sparse_test_matrix);
  Forest forest = RcppUtilities::deserialize_forest(
          forest_object[RcppUtilities::SERIALIZED_FOREST_KEY]);

  ForestPredictor predictor = ForestPredictors::regression_predictor(num_threads);
  std::vector<Prediction> predictions = predictor.predict(forest, train_data, data, estimate_variance);

  Rcpp::List result = RcppUtilities::create_prediction_object(predictions);
  delete train_data;
  delete data;
  return result;
}

// [[Rcpp::export]]
Rcpp::List regression_predict_oob(Rcpp::List forest_object,
                                  Rcpp::NumericMatrix train_matrix,
                                  Eigen::SparseMatrix<double> sparse_train_matrix,
                                  size_t outcome_index,
                                  unsigned int num_threads,
                                  bool estimate_variance) {
  Data* data = RcppUtilities::convert_data(train_matrix, sparse_train_matrix);
  data->set_outcome_index(outcome_index - 1);

  Forest forest = RcppUtilities::deserialize_forest(
          forest_object[RcppUtilities::SERIALIZED_FOREST_KEY]);

  ForestPredictor predictor = ForestPredictors::regression_predictor(num_threads);
  std::vector<Prediction> predictions = predictor.predict_oob(forest, data, estimate_variance);

  Rcpp::List result = RcppUtilities::create_prediction_object(predictions);
  delete data;
  return result;
}

// [[Rcpp::export]]
Rcpp::List local_linear_predict(Rcpp::List forest,
                                Rcpp::NumericMatrix train_matrix,
                                Eigen::SparseMatrix<double> sparse_train_matrix,
                                size_t outcome_index,
                                Rcpp::NumericMatrix test_matrix,
                                Eigen::SparseMatrix<double> sparse_test_matrix,
                                std::vector<double> lambdas,
                                bool weight_penalty,
                                std::vector<size_t> linear_correction_variables,
                                unsigned int num_threads,
                                bool estimate_variance) {
  Data* train_data = RcppUtilities::convert_data(train_matrix, sparse_train_matrix);
  train_data->set_outcome_index(outcome_index - 1);
  Data* data = RcppUtilities::convert_data(test_matrix, sparse_test_matrix);
  
  Forest deserialized_forest = RcppUtilities::deserialize_forest(forest[RcppUtilities::SERIALIZED_FOREST_KEY]);

  ForestPredictor predictor = ForestPredictors::local_linear_predictor(num_threads,
      lambdas, weight_penalty, linear_correction_variables);
  std::vector<Prediction> predictions = predictor.predict(deserialized_forest, train_data, data, estimate_variance);
  Rcpp::List result = RcppUtilities::create_prediction_object(predictions);

  delete train_data;
  delete data;
  return result;
}

// [[Rcpp::export]]
Rcpp::List local_linear_predict_oob(Rcpp::List forest,
                                    Rcpp::NumericMatrix train_matrix,
                                    Eigen::SparseMatrix<double> sparse_train_matrix,
                                    size_t outcome_index,
                                    std::vector<double> lambdas,
                                    bool weight_penalty,
                                    std::vector<size_t> linear_correction_variables,
                                    unsigned int num_threads,
                                    bool estimate_variance) {
  Data* data = RcppUtilities::convert_data(train_matrix, sparse_train_matrix);
  data->set_outcome_index(outcome_index - 1);

  Forest deserialized_forest = RcppUtilities::deserialize_forest(forest[RcppUtilities::SERIALIZED_FOREST_KEY]);

  ForestPredictor predictor = ForestPredictors::local_linear_predictor(num_threads,
      lambdas, weight_penalty, linear_correction_variables);
  std::vector<Prediction> predictions = predictor.predict_oob(deserialized_forest, data, estimate_variance);
  Rcpp::List result = RcppUtilities::create_prediction_object(predictions);

  delete data;
  return result;
}

