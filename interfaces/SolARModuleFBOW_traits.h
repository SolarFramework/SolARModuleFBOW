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

#ifndef SOLARMODULEFBOW_TRAITS_H
#define SOLARMODULEFBOW_TRAITS_H

#include "xpcf/api/IComponentManager.h"

namespace SolAR {
namespace MODULES {
namespace FBOW {
class SolARKeyframeRetrieverFBOW;
}
}
}

XPCF_DEFINE_COMPONENT_TRAITS(SolAR::MODULES::FBOW::SolARKeyframeRetrieverFBOW,
                             "9d1b1afa-bdbc-11e8-a355-529269fb1459",
                             "SolARKeyframeRetrieverFBOW",
                             "Retrieves keyframes closest to an input frame based on a Fast Bag Of Words implementaton.")

#endif // SOLARMODULEFBOW_TRAITS_H
