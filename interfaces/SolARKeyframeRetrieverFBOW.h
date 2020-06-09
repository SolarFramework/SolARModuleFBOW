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
#include <fstream>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/set.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/base_object.hpp>

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

	/// @brief Add a keyframe to the retrieval model
	/// @param[in] keyframe: the keyframe to add to the retrieval model
	/// @return FrameworkReturnCode::_SUCCESS if the keyfram adding succeed, else FrameworkReturnCode::_ERROR_
	FrameworkReturnCode addKeyframe(const SRef<Keyframe>& keyframe) override;

	/// @brief Suppress a keyframe from the retrieval model
	/// @param[in] keyframe_id: the keyframe to supress from the retrieval model
	/// @return FrameworkReturnCode::_SUCCESS if the keyfram adding succeed, else FrameworkReturnCode::_ERROR_
	FrameworkReturnCode suppressKeyframe(uint32_t keyframe_id) override;


	/// @brief Retrieve a set of keyframes close to the frame pass in input.
	/// @param[in] frame: the frame for which we want to retrieve close keyframes.
	/// @param[out] retKeyframes_id: a set of keyframe ids which are close to the frame pass in input
	/// @return FrameworkReturnCode::_SUCCESS if the retrieve succeed, else FrameworkReturnCode::_ERROR_
	FrameworkReturnCode retrieve(const SRef<Frame>& frame, std::vector<uint32_t> &retKeyframes_id) override;

	/// @brief Retrieve a set of keyframes close to the frame pass in input.
	/// @param[in] frame: the frame for which we want to retrieve close keyframes.
	/// @param[in] canKeyframes_id: a set includes id of keyframe candidates
	/// @param[out] retKeyframes_id: a set of keyframe ids which are close to the frame pass in input
	/// @return FrameworkReturnCode::_SUCCESS if the retrieve succeed, else FrameworkReturnCode::_ERROR_
	FrameworkReturnCode retrieve(const SRef<Frame>& frame, std::set<unsigned int> &canKeyframes_id, std::vector<uint32_t> & retKeyframes_id) override;

	/// @brief This method allows to save the keyframe feature to the external file
	/// @param[out] the file name
	/// @return FrameworkReturnCode::_SUCCESS_ if the suppression succeed, else FrameworkReturnCode::_ERROR.
	FrameworkReturnCode saveToFile(std::string file) override;

	/// @brief This method allows to load the keyframe feature from the external file
	/// @param[in] the file name
	/// @return FrameworkReturnCode::_SUCCESS_ if the suppression succeed, else FrameworkReturnCode::_ERROR.
	FrameworkReturnCode loadFromFile(std::string file) override;

	/// @brief Match a frame with a keyframe
	/// @param[in] frame: the frame to match
	/// @param[in] keyframe: keyframe to match
	/// @param[out] matches: a set of matches between frame and keyframe
	/// @return FrameworkReturnCode::_SUCCESS if the retrieve succeed, else FrameworkReturnCode::_ERROR_
	virtual FrameworkReturnCode match(const SRef<Frame>& frame, const SRef<Keyframe>& keyframe, std::vector<DescriptorMatch> &matches) override;

	/// @brief Match a set of descriptors with a keyframe
	/// @param[in] indexDescriptors: index of descriptors to match.
	/// @param[in] descriptors: a descriptor buffer contains all descriptors
	/// @param[in] keyframe: keyframe to match
	/// @param[out] matches: a set of matches between frame and keyframe
	/// @return FrameworkReturnCode::_SUCCESS if the retrieve succeed, else FrameworkReturnCode::_ERROR_
	virtual FrameworkReturnCode match(const std::vector<int> &indexDescriptors, const SRef<DescriptorBuffer> &descriptors, const SRef<Keyframe> &keyframe, std::vector<DescriptorMatch> &matches) override;
private:
	/// @brief Match a feature to a set of features
	/// @param[in] feature1: a feature
	/// @param[in] features2: a set of features
	/// @param[in] idx: a set of indices of used features2
	/// @param[out] bestIdx: the best found index of features2 matched to feature1. (-1: not found)
	/// @param[out] bestDist: the best corresponding distance
	void findBestMatches(const cv::Mat &feature1, const cv::Mat &features2, std::vector<uint32_t> &idx, int &bestIdx, float &bestDist);

private:

    /// @brief path to the vocabulary file
    std::string m_VOCPath   = "";

    /// @brief the threshold above which keyframes are considered valid
    float m_threshold       = 0;

    /// @brief a vocabulary of visual words
    fbow::Vocabulary        m_VOC;	

	/// @brief a map BoW descriptor of keyframes
	std::map<uint32_t, fbow::fBow> m_list_KFBoW;

	/// @brief level stored for BoW2
	int	m_level				= 1;

	/// @brief a map BoW2 descriptor of keyframes which save index of feature at nodes of the expected level
	std::map<uint32_t, fbow::fBow2> m_list_KFBoW2;

	/// @brief For each node at level m_level stores a set of frames that contain it
	std::map<uint32_t, std::set<uint32_t>> m_invertedIndexKfs;

	/// @brief distance ratio used to keep good matches.
	float m_distanceRatio = 0.7;

	/// @brief distance max used to keep good matches.
	float m_distanceMax = 100;
};

}
}
}

#endif // SOLARKEYFRAMERETRIEVERFBOW_H
