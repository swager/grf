/*-------------------------------------------------------------------------------
  This file is part of generalized-random-forest (grf).

  generalized-random-forest is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  generalized-random-forest is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with generalized-random-forest. If not, see <http://www.gnu.org/licenses/>.
 #-------------------------------------------------------------------------------*/

#ifndef GRF_PREDICTIONCOLLECTOR_H
#define GRF_PREDICTIONCOLLECTOR_H

#include "forest/Forest.h"

class PredictionCollector {
public:
  virtual std::vector<Prediction> collect_predictions(const Forest& forest,
                                                      Data* prediction_data,
                                                      std::vector<std::vector<size_t>> leaf_nodes_by_tree,
                                                      std::vector<std::vector<bool>> trees_by_sample) = 0;
};

#endif //GRF_PREDICTIONCOLLECTOR_H
