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

#include "SolARKeyframeRetrieverFBOW.h"

namespace xpcf = org::bcom::xpcf;


XPCF_DEFINE_FACTORY_CREATE_INSTANCE(SolAR::MODULES::FBOW::SolARKeyframeretrieverFBOW)

namespace SolAR {
using namespace datastructure;
namespace MODULES {
namespace FBOW {

SolARKeyframeRetrieverFBOW::SolARKeyframRretrieverFBOW():ConfigurableBase(xpcf::toUUID<SolARKeyframeRetrieverFBOW>())
{
    addInterface<api::solver::map::IKeyframeRetriever>(this);

    SRef<xpcf::IPropertyMap> params = getPropertyRootNode();

    params->wrapString("VOCpath",m_VOCpath);
    params->wrapFloat("threshold", m_threshold);

   LOG_DEBUG("SolARKeyframeRetrieverFBOW constructor");

}

SolARKeyframeRetrieverFBOW::~SolARKeyframeRetrieverFBOW()
{
    LOG_DEBUG(" SolARKeyframeRetrieverFBOW destructor")
}

xpcf::XPCFErrorCode SolARKeyframeRetrieverFBOW::onConfigured()
{
    LOG_DEBUG(" SolARKeyframeRetrieverFBOW onConfigured");

    // HERE, all you need for your initialization

    return xpcf::_SUCCESS;
}

FrameworkReturnCode SolARKeyframeRetrieverFBOW::addKeyframe(SRef<Keyframe> keyframe)
{

    return FrameworkReturnCode::_SUCCESS;
}

FrameworkReturnCode SolARKeyframeRetrieverFBOW::retrieve(const SRef<Frame> frame, std::vector<SRef<Keyframe>>& keyframes)
{

    return FrameworkReturnCode::_SUCCESS;
}


}
}
}
