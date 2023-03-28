/**
 * @copyright Copyright (c) 2017 B-com http://www.b-com.com/
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef SOLARFBOWHELPER_H
#define SOLARFBOWHELPER_H

#include "SolARFBOWAPI.h"
#include "fbow.h"
#include "datastructure/KeyframeRetrieval.h"

namespace SolAR {
namespace MODULES {
namespace FBOW {

class SOLARFBOW_EXPORT_API SolARFBOWHelper
{
public:
    static datastructure::BoWFeature fbow2Solar(const fbow::fBow& fbow);
    static datastructure::BoWLevelFeature fbow2Solar(const fbow::fBow2& fbow2);
    static double distanceBoW(const datastructure::BoWFeature& bow1, const datastructure::BoWFeature& bow2);
	static double distanceL1BoW(const datastructure::BoWFeature& bow1, const datastructure::BoWFeature& bow2);
	static double distanceChiSquareBoW(const datastructure::BoWFeature& bow1, const datastructure::BoWFeature& bow2);
	static double distanceKLSBoW(const datastructure::BoWFeature& bow1, const datastructure::BoWFeature& bow2);
	static double distanceBhattacharyyaBoW(const datastructure::BoWFeature& bow1, const datastructure::BoWFeature& bow2);
	static double distanceDotProductBoW(const datastructure::BoWFeature& bow1, const datastructure::BoWFeature& bow2);
};

}
}
}

#endif // SOLARFBOWHELPER_H
