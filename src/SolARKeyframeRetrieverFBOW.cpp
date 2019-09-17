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
#include <core/Log.h>

namespace xpcf = org::bcom::xpcf;


XPCF_DEFINE_FACTORY_CREATE_INSTANCE(SolAR::MODULES::FBOW::SolARKeyframeRetrieverFBOW)

namespace SolAR {
using namespace datastructure;
namespace MODULES {
namespace FBOW {

SolARKeyframeRetrieverFBOW::SolARKeyframeRetrieverFBOW():ConfigurableBase(xpcf::toUUID<SolARKeyframeRetrieverFBOW>())
{
    addInterface<api::reloc::IKeyframeRetriever>(this);

    declareProperty("VOCpath",m_VOCPath);
    declareProperty("threshold", m_threshold);
    declareProperty("level", m_level);
	declareProperty("matchingDistanceRatio", m_distanceRatio);
	declareProperty("matchingDistanceMAX", m_distanceMax);

   LOG_DEBUG("SolARKeyframeRetrieverFBOW constructor");

}

SolARKeyframeRetrieverFBOW::~SolARKeyframeRetrieverFBOW()
{
    LOG_DEBUG(" SolARKeyframeRetrieverFBOW destructor")
}

xpcf::XPCFErrorCode SolARKeyframeRetrieverFBOW::onConfigured()
{
    LOG_DEBUG(" SolARKeyframeRetrieverFBOW onConfigured");

    // Load a vocabulary from m_VOCpath
    m_VOC.readFromFile(m_VOCPath);
    if (!m_VOC.isValid()){
        LOG_DEBUG(" SolARKeyframeRetrieverFBOW onConfigured: Cannot load the vocabulary from file");
        return xpcf::_ERROR_INVALID_ARGUMENT;
    }

    return xpcf::_SUCCESS;
}

FrameworkReturnCode SolARKeyframeRetrieverFBOW::addKeyframe(SRef<Keyframe> keyframe)
{
	// convert desc of keyframe to Mat opencv
	SRef<DescriptorBuffer> desc_Solar = keyframe->getDescriptors();
	cv::Mat desc_OpenCV(desc_Solar->getNbDescriptors(), desc_Solar->getNbElements(), m_VOC.getDescType(), desc_Solar->data());

	// get bow desc corresponding to keyframe desc
	fbow::fBow v_bow;
	fbow::fBow2 v_bow2;
	m_VOC.transform(desc_OpenCV, m_level, v_bow, v_bow2);

	// add bow desc and keyfram to the lists
	m_list_KFBoW.push_back(v_bow);
	m_list_KFBoW2.push_back(v_bow2);
	m_list_keyframes.push_back(keyframe);
	m_list_des.push_back(desc_OpenCV);

    return FrameworkReturnCode::_SUCCESS;
}

FrameworkReturnCode SolARKeyframeRetrieverFBOW::retrieve(const SRef<Frame> frame, std::vector<SRef<Keyframe>>& keyframes)
{
	// convert frame desc to Mat opencv
	SRef<DescriptorBuffer> desc_Solar = frame->getDescriptors();
	if (desc_Solar->getNbDescriptors() == 0)
		return FrameworkReturnCode::_ERROR_;
	cv::Mat desc_OpenCV(desc_Solar->getNbDescriptors(), desc_Solar->getNbElements(), m_VOC.getDescType(), desc_Solar->data());

	// get bow desc corresponding to keyframe desc
	fbow::fBow v_bow;
	v_bow = m_VOC.transform(desc_OpenCV);	

	// find nearest keyframes
	std::multimap<double, int> sortDisKeyframes;
	for (int i = 0; i < m_list_KFBoW.size(); ++i) {
		double score = m_list_KFBoW[i].score(m_list_KFBoW[i], v_bow);
		if (score > m_threshold)
			sortDisKeyframes.insert(std::pair<double, int>(-score, i));
	}

	for (auto it = sortDisKeyframes.begin(); it != sortDisKeyframes.end(); it++) {
		keyframes.push_back(m_list_keyframes[it->second]);
		if (keyframes.size() >= 3)
			break;
	}

	if (keyframes.size() == 0)
		return FrameworkReturnCode::_ERROR_;

    return FrameworkReturnCode::_SUCCESS;
}

FrameworkReturnCode SolARKeyframeRetrieverFBOW::retrieve(const SRef<Frame> frame, std::set<unsigned int> &idxKfCandidates, std::vector<SRef<Keyframe>> & keyframes)
{
	// convert frame desc to Mat opencv
	SRef<DescriptorBuffer> desc_Solar = frame->getDescriptors();
	if (desc_Solar->getNbDescriptors() == 0)
		return FrameworkReturnCode::_ERROR_;
	cv::Mat desc_OpenCV(desc_Solar->getNbDescriptors(), desc_Solar->getNbElements(), m_VOC.getDescType(), desc_Solar->data());

	// get bow desc corresponding to keyframe desc
	fbow::fBow v_bow;
	v_bow = m_VOC.transform(desc_OpenCV);

	// find nearest keyframes
	std::multimap<double, int> sortDisKeyframes;
	for (auto it = idxKfCandidates.begin(); it != idxKfCandidates.end(); it++) {
		double score = m_list_KFBoW[*it].score(m_list_KFBoW[*it], v_bow);
		if (score > m_threshold)
			sortDisKeyframes.insert(std::pair<double, int>(-score, *it));
	}

	for (auto it = sortDisKeyframes.begin(); it != sortDisKeyframes.end(); it++) {
		keyframes.push_back(m_list_keyframes[it->second]);
		if (keyframes.size() >= 3)
			break;
	}

	if (keyframes.size() == 0)
		return FrameworkReturnCode::_ERROR_;

	return FrameworkReturnCode::_SUCCESS;
}

void SolARKeyframeRetrieverFBOW::findBestMatches(const cv::Mat &feature1, const cv::Mat &features2, std::vector<uint32_t> &idx, int &bestIdx, float &bestDist) {
	bestIdx = -1;
	if (idx.size() == 0)
		return;
	bestDist = FLT_MAX;
	float bestDist2 = FLT_MAX;

	for (auto &it : idx) {
		float dist = cv::norm(feature1 - features2.row(it));

		if (dist < bestDist)
		{
			bestDist2 = bestDist;
			bestDist = dist;
			bestIdx = it;
		}
		else if (dist < bestDist2)
		{
			bestDist2 = dist;
		}
	}

	if ((bestDist > m_distanceRatio * bestDist2) && (bestDist > m_distanceMax))
		bestIdx = -1;
}

FrameworkReturnCode SolARKeyframeRetrieverFBOW::match(const SRef<Frame> frame, int index, std::vector<DescriptorMatch>& matches)
{
	// convert frame desc to Mat opencv
	SRef<DescriptorBuffer> descriptors = frame->getDescriptors();
	if (descriptors->getNbDescriptors() == 0)
		return FrameworkReturnCode::_ERROR_;
	cv::Mat cvDescriptors(descriptors->getNbDescriptors(), descriptors->getNbElements(), m_VOC.getDescType(), descriptors->data());

	const cv::Mat &kfDescriptors = m_list_des[index];
	const fbow::fBow2 &kfFBow2 = m_list_KFBoW2[index];

	for (int i = 0; i < cvDescriptors.rows; i++) {
		const cv::Mat cvDescriptor = cvDescriptors.row(i);
		int node = m_VOC.transform(cvDescriptor, m_level);
		std::vector<uint32_t> candidates;
		auto it = kfFBow2.find(node);
		if (it != kfFBow2.end())
			candidates = it->second;

		// find the best match
		int bestIdx;
		float bestDist;
		findBestMatches(cvDescriptor, kfDescriptors, candidates, bestIdx, bestDist);
		if (bestIdx != -1)
			matches.push_back(DescriptorMatch(i, bestIdx, bestDist));
	}

	return FrameworkReturnCode::_SUCCESS;
}

FrameworkReturnCode SolARKeyframeRetrieverFBOW::match(const std::vector<int>& indexDescriptors, const SRef<DescriptorBuffer> & descriptors, int indexKeyframe, std::vector<DescriptorMatch>& matches)
{
	// convert frame desc to Mat opencv
	if (descriptors->getNbDescriptors() == 0)
		return FrameworkReturnCode::_ERROR_;
	cv::Mat cvDescriptors(descriptors->getNbDescriptors(), descriptors->getNbElements(), m_VOC.getDescType(), descriptors->data());

	const cv::Mat &kfDescriptors = m_list_des[indexKeyframe];
	const fbow::fBow2 &kfFBow2 = m_list_KFBoW2[indexKeyframe];

	for (auto &it_des: indexDescriptors) {
		const cv::Mat cvDescriptor = cvDescriptors.row(it_des);
		int node = m_VOC.transform(cvDescriptor, m_level);
		std::vector<uint32_t> candidates;
		auto it = kfFBow2.find(node);
		if (it != kfFBow2.end())
			candidates = it->second;

		// find the best match
		int bestIdx;
		float bestDist;
		findBestMatches(cvDescriptor, kfDescriptors, candidates, bestIdx, bestDist);
		if (bestIdx != -1)
			matches.push_back(DescriptorMatch(it_des, bestIdx, bestDist));
	}
	return FrameworkReturnCode::_SUCCESS;
}


}
}
}
