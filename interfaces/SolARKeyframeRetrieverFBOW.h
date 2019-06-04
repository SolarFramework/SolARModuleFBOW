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
#ifndef SOLARKEYFRAMERETRIEVERFBOW_H
#define SOLARKEYFRAMERETRIEVERFBOW_H

#include "api/reloc/IKeyframeRetriever.h"
#include "SolARFBOWAPI.h"
#include "xpcf/component/ConfigurableBase.h"
#include <vector>

#include "fbow.h"

namespace SolAR {
using namespace datastructure;
namespace MODULES {
namespace FBOW {

/**
 * @class SolARKeyframeRetrieverFBOW
 * @brief <B>Retrieves keyframes closest to an input frame based on a Fast Bag Of Words implementaton.</B>
 * <TT>UUID: 9d1b1afa-bdbc-11e8-a355-529269fb1459</TT>
 *
 */

class SOLARFBOW_EXPORT_API SolARKeyframeRetrieverFBOW : public org::bcom::xpcf::ConfigurableBase,
    public api::reloc::IKeyframeRetriever
{
public:
    SolARKeyframeRetrieverFBOW();
    ~SolARKeyframeRetrieverFBOW();

    org::bcom::xpcf::XPCFErrorCode onConfigured() override final;
    void unloadComponent () override final;

    /// @brief Add a keyframe to the bag of words
    /// @param[in] keyframe: the keyframe to add to the bag of words
    /// @return FrameworkReturnCode::_SUCCESS if the keyfram adding succeed, else FrameworkReturnCode::_ERROR_
    FrameworkReturnCode addKeyframe(SRef<Keyframe> keyframe) override;


    /// @brief Retrieve a set of keyframes close to the frame pass in input.
    /// @param[in] frame: the frame for which we want to retrieve close keyframes.
    /// @param[out] keyframes: a set of keyframe which are close to the frame pass in input
    /// @return FrameworkReturnCode::_SUCCESS if the retrieve succeed, else FrameworkReturnCode::_ERROR_
    FrameworkReturnCode retrieve(const SRef<Frame> frame, std::vector<SRef<Keyframe>>& keyframes) override;

private:

    /// @brief path to the vocabulary file
    std::string m_VOCPath   = "";

    /// @brief the threshold above which keyframes are considered valid
    float m_threshold       = 0;

    /// @brief a vocabulary of visual words
    fbow::Vocabulary        m_VOC;

	/// @brief a list BoW descriptor of keyframes
	std::vector<fbow::fBow> m_list_KFBoW;

	/// @brief a list of keyframes
	std::vector<SRef<Keyframe>> m_list_keyframes;
};

}
}
}

#endif // SOLARKEYFRAMERETRIEVERFBOW_H
