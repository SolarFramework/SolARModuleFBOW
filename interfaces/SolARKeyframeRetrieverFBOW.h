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
#include <core/SerializationDefinitions.h>
#include "fbow.h"

namespace SolAR {
namespace MODULES {
namespace FBOW {

/**
 * @class SolARKeyframeRetrieverFBOW
 * @brief <B>Retrieves keyframes closest to an input frame based on a Fast Bag Of Words implementaton.</B>
 * <TT>UUID: 9d1b1afa-bdbc-11e8-a355-529269fb1459</TT>
 *
 * @SolARComponentPropertiesBegin
 * @SolARComponentProperty{ VOCpath,
 *                          path to the vocabulary file,
 *                          @SolARComponentPropertyDescString{ "" }}
 * @SolARComponentProperty{ threshold,
 *                          the threshold above which keyframes are considered valid,
 *                          @SolARComponentPropertyDescNum{ float, [0..MAX FLOAT], 0.f }}
 * @SolARComponentProperty{ matchingDistanceRatio,
 *                          distance ratio used to keep good matches,
 *                          @SolARComponentPropertyDescNum{ float, [0..MAX FLOAT], 0.7f }}
 * @SolARComponentProperty{ matchingDistanceMax,
 *                          distance max used to keep good matches,
 *                          @SolARComponentPropertyDescNum{ float, [0..MAX FLOAT], 100.f }}
 * @SolARComponentPropertiesEnd
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
	/// @param[in] useMatchedDescriptor: if true bow feature will be computed merely from descriptors which are matched to other frames, by default is set to false meaning that all descriptors will be used
	/// @return FrameworkReturnCode::_SUCCESS if the keyfram adding succeed, else FrameworkReturnCode::_ERROR_
    FrameworkReturnCode addKeyframe(const SRef<datastructure::Keyframe> keyframe, bool useMatchedDescriptor=false) override;

	/// @brief Suppress a keyframe from the retrieval model
	/// @param[in] keyframe_id: the keyframe to supress from the retrieval model
	/// @return FrameworkReturnCode::_SUCCESS if the keyfram adding succeed, else FrameworkReturnCode::_ERROR_
	FrameworkReturnCode suppressKeyframe(uint32_t keyframe_id) override;


	/// @brief Retrieve a set of keyframes close to the frame pass in input.
	/// @param[in] frame: the frame for which we want to retrieve close keyframes.
	/// @param[out] retKeyframes_id: a set of keyframe ids which are close to the frame pass in input
	/// @return FrameworkReturnCode::_SUCCESS if the retrieve succeed, else FrameworkReturnCode::_ERROR_
    FrameworkReturnCode retrieve(const SRef<datastructure::Frame> frame, std::vector<uint32_t> &retKeyframes_id) override;

	/// @brief Retrieve a set of keyframes close to the frame pass in input.
	/// @param[in] frame: the frame for which we want to retrieve close keyframes.
	/// @param[in] canKeyframes_id: a set includes id of keyframe candidates
	/// @param[out] retKeyframes_id: a set of keyframe ids which are close to the frame pass in input
	/// @return FrameworkReturnCode::_SUCCESS if the retrieve succeed, else FrameworkReturnCode::_ERROR_
    FrameworkReturnCode retrieve(const SRef<datastructure::Frame> frame, const std::set<unsigned int> & canKeyframes_id, std::vector<uint32_t> & retKeyframes_id) override;

	/// @brief This method allows to save the keyframe feature to the external file
	/// @param[in] the file name
	/// @return FrameworkReturnCode::_SUCCESS_ if the suppression succeed, else FrameworkReturnCode::_ERROR.
    FrameworkReturnCode saveToFile(const std::string& file) const override;

	/// @brief This method allows to load the keyframe feature from the external file
	/// @param[in] the file name
	/// @return FrameworkReturnCode::_SUCCESS_ if the suppression succeed, else FrameworkReturnCode::_ERROR.
    FrameworkReturnCode loadFromFile(const std::string & file) override;

	/// @brief Match a frame with a keyframe
	/// @param[in] frame: the frame to match
	/// @param[in] keyframe: keyframe to match
	/// @param[out] matches: a set of matches between frame and keyframe
	/// @return FrameworkReturnCode::_SUCCESS if the retrieve succeed, else FrameworkReturnCode::_ERROR_
    FrameworkReturnCode match(const SRef<datastructure::Frame> frame, const SRef<datastructure::Keyframe> keyframe, std::vector<datastructure::DescriptorMatch> &matches) override;

	/// @brief Match a set of descriptors with a keyframe
	/// @param[in] indexDescriptors: index of descriptors to match.
	/// @param[in] descriptors: a descriptor buffer contains all descriptors
	/// @param[in] keyframe: keyframe to match
	/// @param[out] matches: a set of matches between frame and keyframe
	/// @return FrameworkReturnCode::_SUCCESS if the retrieve succeed, else FrameworkReturnCode::_ERROR_
    FrameworkReturnCode match(const std::vector<int> &indexDescriptors, const SRef<datastructure::DescriptorBuffer> descriptors, const SRef<datastructure::Keyframe> keyframe, std::vector<datastructure::DescriptorMatch> &matches) override;

	/// @brief This method returns the keyframe retrieval
	/// @return the keyframe retrieval
	const SRef<datastructure::KeyframeRetrieval> & getConstKeyframeRetrieval() const override;

	/// @brief This method returns the keyframe retrieval
	/// @param[out] keyframeRetrieval the keyframe retrieval of map
	/// @return the keyframe retrieval
	std::unique_lock<std::mutex> getKeyframeRetrieval(SRef<datastructure::KeyframeRetrieval>& keyframeRetrieval) override;

	/// @brief This method is to set the keyframe retrieval
	/// @param[in] keyframeRetrieval the keyframe retrieval of map
	void setKeyframeRetrieval(const SRef<datastructure::KeyframeRetrieval> keyframeRetrieval) override;

    /// @brief This method is to reset keyframe retrieval contents 
    void resetKeyframeRetrieval() override;

private:
	/// @brief Match a feature to a set of features
	/// @param[in] feature1: a feature
	/// @param[in] features2: a set of features
	/// @param[in] idx: a set of indices of used features2
	/// @param[out] bestIdx: the best found index of features2 matched to feature1. (-1: not found)
	/// @param[out] bestDist: the best corresponding distance
	void findBestMatches(const cv::Mat &feature1, const cv::Mat &features2, std::vector<uint32_t> &idx, int &bestIdx, float &bestDist);

private:
	SRef<datastructure::KeyframeRetrieval> m_keyframeRetrieval;

    /// @brief path to the vocabulary file
    std::string m_VOCPath   = "";

    /// @brief the threshold above which keyframes are considered valid
    float m_threshold       = 0;

    /// @brief a vocabulary of visual words
    fbow::Vocabulary        m_VOC;	

	/// @brief level stored for BoW2
	int	m_level				= 3;

	/// @brief distance ratio used to keep good matches.
    float m_distanceRatio = 0.7f;

	/// @brief distance max used to keep good matches.
    float m_distanceMax = 100.f;

    /// @brief distance metric
    int m_distanceMetricId = 0;
};

}
}
}

#endif // SOLARKEYFRAMERETRIEVERFBOW_H
