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

#ifndef SOLAR_FBOW_API_H
#define SOLAR_FBOW_API_H

#if _WIN32
#if defined(SolARModuleFBOW_API_DLLEXPORT) || defined(SolARModuleFBOWCuda_API_DLLEXPORT)
#define SOLARFBOW_EXPORT_API __declspec(dllexport)
#else //SOLARFBOW_API_DLLEXPORT
#define SOLARFBOW_EXPORT_API __declspec(dllimport)
#endif //SOLARFBOW_API_DLLEXPORT
#else //_WIN32
#define SOLARFBOW_EXPORT_API
#endif //_WIN32
#include "SolARModuleFBOW_traits.h"
#endif //SOLAR_FBOW_API
