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

using namespace grf;

// [[Rcpp::export]]
Rcpp::List causal_train(Rcpp::NumericMatrix train_matrix,
                        Eigen::SparseMatrix<double> sparse_train_matrix,
                        size_t outcome_index,
                        size_t treatment_index,
                        size_t sample_weight_index,
                        bool use_sample_weights,
                        unsigned int mtry,
                        unsigned int num_trees,
                        unsigned int min_node_size,
                        double sample_fraction,
                        bool honesty,
                        double honesty_fraction,
                        bool honesty_prune_leaves,
                        size_t ci_group_size,
                        double reduced_form_weight,
                        double alpha,
                        double imbalance_penalty,
                        bool stabilize_splits,
                        std::vector<size_t> clusters,
                        unsigned int samples_per_cluster,
                        bool compute_oob_predictions,
                        unsigned int num_threads,
                        unsigned int seed) {
  ForestTrainer trainer = instrumental_trainer(reduced_form_weight, stabilize_splits);

  std::unique_ptr<Data> data = RcppUtilities::convert_data(train_matrix, sparse_train_matrix);
  data->set_outcome_index(outcome_index - 1);
  data->set_treatment_index(treatment_index - 1);
  data->set_instrument_index(treatment_index - 1);
  if(use_sample_weights) {
      data->set_weight_index(sample_weight_index - 1);
  }

  ForestOptions options(num_trees, ci_group_size, sample_fraction, mtry, min_node_size, honesty,
                        honesty_fraction, honesty_prune_leaves, alpha, imbalance_penalty, num_threads, seed, clusters, samples_per_cluster);
  Forest forest = trainer.train(*data, options);

  std::vector<Prediction> predictions;
  if (compute_oob_predictions) {
    ForestPredictor predictor = instrumental_predictor(num_threads);
    predictions = predictor.predict_oob(forest, *data, false);
  }

  return RcppUtilities::create_forest_object(forest, predictions);
}


// [[Rcpp::export]]
Rcpp::List causal_predict(SEXP forest_xptr,
                          Rcpp::NumericMatrix train_matrix,
                          Eigen::SparseMatrix<double> sparse_train_matrix,
                          size_t outcome_index,
                          size_t treatment_index,
                          Rcpp::NumericMatrix test_matrix,
                          Eigen::SparseMatrix<double> sparse_test_matrix,
                          unsigned int num_threads,
                          bool estimate_variance) {
  std::unique_ptr<Data> train_data = RcppUtilities::convert_data(train_matrix, sparse_train_matrix);
  train_data->set_outcome_index(outcome_index - 1);
  train_data->set_treatment_index(treatment_index - 1);
  train_data->set_instrument_index(treatment_index - 1);
  std::unique_ptr<Data> data = RcppUtilities::convert_data(test_matrix, sparse_test_matrix);

  Rcpp::XPtr<Forest> forest(forest_xptr);

  ForestPredictor predictor = instrumental_predictor(num_threads);
  std::vector<Prediction> predictions = predictor.predict(*forest, *train_data, *data, estimate_variance);
  Rcpp::List result = RcppUtilities::create_prediction_object(predictions);

  return result;
}

// [[Rcpp::export]]
Rcpp::List causal_predict_oob(SEXP forest_xptr,
                              Rcpp::NumericMatrix train_matrix,
                              Eigen::SparseMatrix<double> sparse_train_matrix,
                              size_t outcome_index,
                              size_t treatment_index,
                              unsigned int num_threads,
                              bool estimate_variance) {
  std::unique_ptr<Data> data = RcppUtilities::convert_data(train_matrix, sparse_train_matrix);
  data->set_outcome_index(outcome_index - 1);
  data->set_treatment_index(treatment_index - 1);
  data->set_instrument_index(treatment_index - 1);

  Rcpp::XPtr<Forest> forest(forest_xptr);

  ForestPredictor predictor = instrumental_predictor(num_threads);
  std::vector<Prediction> predictions = predictor.predict_oob(*forest, *data, estimate_variance);
  Rcpp::List result = RcppUtilities::create_prediction_object(predictions);

  return result;
}

// [[Rcpp::export]]
Rcpp::List ll_causal_predict(SEXP forest_xptr,
                             Rcpp::NumericMatrix input_data,
                             Rcpp::NumericMatrix training_data,
                             Eigen::SparseMatrix<double> sparse_input_data,
                             Eigen::SparseMatrix<double> sparse_training_data,
                             size_t outcome_index,
                             size_t treatment_index,
                             std::vector<double> lambdas,
                             bool use_weighted_penalty,
                             std::vector<size_t> linear_correction_variables,
                             unsigned int num_threads,
                             bool estimate_variance) {
  std::unique_ptr<Data> data = RcppUtilities::convert_data(input_data, sparse_input_data);
  std::unique_ptr<Data> train_data = RcppUtilities::convert_data(training_data, sparse_training_data);

  train_data->set_outcome_index(outcome_index - 1);
  train_data->set_treatment_index(treatment_index - 1);
  train_data->set_instrument_index(treatment_index - 1);

  Rcpp::XPtr<Forest> forest(forest_xptr);

  ForestPredictor predictor = ll_causal_predictor(num_threads, lambdas, use_weighted_penalty,
                                                                 linear_correction_variables);
  std::vector<Prediction> predictions = predictor.predict(*forest, *train_data, *data, estimate_variance);
  Rcpp::List result = RcppUtilities::create_prediction_object(predictions);

  return result;
}

// [[Rcpp::export]]
Rcpp::List ll_causal_predict_oob(SEXP forest_xptr,
                                     Rcpp::NumericMatrix input_data,
                                     Eigen::SparseMatrix<double> sparse_input_data,
                                     size_t outcome_index,
                                     size_t treatment_index,
                                     std::vector<double> lambdas,
                                     bool use_weighted_penalty,
                                     std::vector<size_t> linear_correction_variables,
                                     unsigned int num_threads,
                                     bool estimate_variance) {
  std::unique_ptr<Data> data = RcppUtilities::convert_data(input_data, sparse_input_data);

  data->set_outcome_index(outcome_index - 1);
  data->set_treatment_index(treatment_index - 1);
  data->set_instrument_index(treatment_index - 1);

  Rcpp::XPtr<Forest> forest(forest_xptr);

  ForestPredictor predictor = ll_causal_predictor(num_threads, lambdas, use_weighted_penalty,
                                                                 linear_correction_variables);
  std::vector<Prediction> predictions = predictor.predict_oob(*forest, *data, estimate_variance);
  Rcpp::List result = RcppUtilities::create_prediction_object(predictions);

  return result;
}
