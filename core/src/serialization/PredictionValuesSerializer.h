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

#ifndef GRF_PREDICTIONVALUESSERIALIZER_H
#define GRF_PREDICTIONVALUESSERIALIZER_H

#include "prediction/PredictionValues.h"

class PredictionValuesSerializer {
public:
  void serialize(std::ostream& stream, const PredictionValues& prediction_values);
  PredictionValues deserialize(std::istream& stream);
};


#endif //GRF_PREDICTIONVALUESSERIALIZER_H
