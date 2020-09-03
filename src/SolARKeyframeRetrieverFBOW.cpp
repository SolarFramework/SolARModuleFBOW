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
	declareProperty("matchingDistanceMax", m_distanceMax);

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

FrameworkReturnCode SolARKeyframeRetrieverFBOW::addKeyframe(const SRef<Keyframe>& keyframe)
{
	// Convert desc of keyframe to Mat opencv
	SRef<DescriptorBuffer> desc_Solar = keyframe->getDescriptors();
	cv::Mat desc_OpenCV(desc_Solar->getNbDescriptors(), desc_Solar->getNbElements(), m_VOC.getDescType(), desc_Solar->data());

	// Get bow desc corresponding to keyframe desc
	fbow::fBow v_bow;
	fbow::fBow2 v_bow2;
	m_VOC.transform(desc_OpenCV, m_level, v_bow, v_bow2);

	// Add bow desc to the database
	m_list_KFBoW[keyframe->getId()] = v_bow;
	m_list_KFBoW2[keyframe->getId()] = v_bow2;

	// Add inverted index to the database
	for (auto const &it : v_bow2)
		m_invertedIndexKfs[it.first].insert(keyframe->getId());

    return FrameworkReturnCode::_SUCCESS;
}

FrameworkReturnCode SolARKeyframeRetrieverFBOW::suppressKeyframe(uint32_t keyframe_id)
{
	auto it_KFBoW2 = m_list_KFBoW2.find(keyframe_id);
	if (it_KFBoW2 == m_list_KFBoW2.end())
		return FrameworkReturnCode::_ERROR_;

	// remove inverted index
	const fbow::fBow2 &v_bow2 = it_KFBoW2->second;
	for (auto const &it : v_bow2)
		m_invertedIndexKfs[it.first].erase(keyframe_id);

	// remove keyframe descriptors
	m_list_KFBoW.erase(keyframe_id);
	m_list_KFBoW2.erase(keyframe_id);	

	return FrameworkReturnCode::_SUCCESS;
}

FrameworkReturnCode SolARKeyframeRetrieverFBOW::retrieve(const SRef<Frame>& frame, std::vector<uint32_t> &retKeyframes_id)
{
	// convert frame desc to Mat opencv
	SRef<DescriptorBuffer> desc_Solar = frame->getDescriptors();
	if (desc_Solar->getNbDescriptors() == 0)
		return FrameworkReturnCode::_ERROR_;
	cv::Mat desc_OpenCV(desc_Solar->getNbDescriptors(), desc_Solar->getNbElements(), m_VOC.getDescType(), desc_Solar->data());

	// calculate bow desc corresponding to the query frame
	fbow::fBow v_bow;
	fbow::fBow2 v_bow2;
	m_VOC.transform(desc_OpenCV, m_level, v_bow, v_bow2);

	// get candidates that have at least 1 common word with the query frame
	std::map<uint32_t, int> scoreCandidates;
	for (auto const &it : v_bow2) {
		const std::set<uint32_t> &kfs_id = m_invertedIndexKfs[it.first];
		for (auto const &it_kf : kfs_id)
			scoreCandidates[it_kf]++;
	}
	if (scoreCandidates.size() == 0)
		return FrameworkReturnCode::_ERROR_;

	// find max common words
	int maxScore = 0;
	for (auto const &it : scoreCandidates)
		if (it.second > maxScore)
			maxScore = it.second;
	int minScore = 0.5 * maxScore;

	// get best candidates
	std::vector<uint32_t> bestCandidates;
	for (auto const &it : scoreCandidates)
		if (it.second > minScore)
			bestCandidates.push_back(it.first);

	// find nearest keyframes
	std::multimap<double, int> sortDisKeyframes;
	for (auto const &it : bestCandidates) {
		const fbow::fBow & kfBoW = m_list_KFBoW[it];
		double score = kfBoW.score(kfBoW, v_bow);
		if (score > m_threshold)
			sortDisKeyframes.insert(std::pair<double, int>(-score, it));
	}

	if (sortDisKeyframes.size() == 0)
		return FrameworkReturnCode::_ERROR_;

	for (auto const &it : sortDisKeyframes) {
		retKeyframes_id.push_back(it.second);
	}	

    return FrameworkReturnCode::_SUCCESS;
}

FrameworkReturnCode SolARKeyframeRetrieverFBOW::retrieve(const SRef<Frame>& frame, std::set<unsigned int> &canKeyframes_id, std::vector<uint32_t> & retKeyframes_id)
{
	// convert frame desc to Mat opencv
	SRef<DescriptorBuffer> desc_Solar = frame->getDescriptors();
	if (desc_Solar->getNbDescriptors() == 0)
		return FrameworkReturnCode::_ERROR_;
	cv::Mat desc_OpenCV(desc_Solar->getNbDescriptors(), desc_Solar->getNbElements(), m_VOC.getDescType(), desc_Solar->data());

	// calculate bow desc corresponding to the query frame
	fbow::fBow v_bow;
	v_bow = m_VOC.transform(desc_OpenCV);

	// find nearest keyframes
	std::multimap<double, int> sortDisKeyframes;
	for (auto const &it : canKeyframes_id) {
		auto it_keyframe = m_list_KFBoW.find(it);
		if (it_keyframe != m_list_KFBoW.end()) {
			const fbow::fBow& kfBoW = it_keyframe->second;
			double score = kfBoW.score(kfBoW, v_bow);
			if (score > m_threshold)
				sortDisKeyframes.insert(std::pair<double, int>(-score, it));
		}
	}

	if (sortDisKeyframes.size() == 0)
		return FrameworkReturnCode::_ERROR_;

	for (auto const &it : sortDisKeyframes) {
		retKeyframes_id.push_back(it.second);
	}	

	return FrameworkReturnCode::_SUCCESS;
}

FrameworkReturnCode SolARKeyframeRetrieverFBOW::saveToFile(std::string file)
{    
	std::ofstream ofs(file, std::ios::binary);
	OutputArchive oa(ofs);
	oa << m_level;
	oa << m_list_KFBoW;
	oa << m_list_KFBoW2;
	oa << m_invertedIndexKfs;
	ofs.close();
	return FrameworkReturnCode::_SUCCESS;
}

FrameworkReturnCode SolARKeyframeRetrieverFBOW::loadFromFile(std::string file)
{
	std::ifstream ifs(file, std::ios::binary);
	if (!ifs.is_open())
		return FrameworkReturnCode::_ERROR_;
    InputArchive ia(ifs);
	ia >> m_level;
	ia >> m_list_KFBoW;
	ia >> m_list_KFBoW2;
	ia >> m_invertedIndexKfs;
	ifs.close();
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

	if ((bestDist > m_distanceRatio * bestDist2) || (bestDist > m_distanceMax))
		bestIdx = -1;
}

FrameworkReturnCode SolARKeyframeRetrieverFBOW::match(const SRef<Frame>& frame, const SRef<Keyframe>& keyframe, std::vector<DescriptorMatch> &matches)
{
	// convert frame desc to Mat opencv
	SRef<DescriptorBuffer> descriptors = frame->getDescriptors();
	if (descriptors->getNbDescriptors() == 0)
		return FrameworkReturnCode::_ERROR_;
	cv::Mat cvDescriptors(descriptors->getNbDescriptors(), descriptors->getNbElements(), m_VOC.getDescType(), descriptors->data());

	// convert keyframe desc to Mat opencv
	SRef<DescriptorBuffer> descriptors_kf = keyframe->getDescriptors();
	if (descriptors_kf->getNbDescriptors() == 0)
		return FrameworkReturnCode::_ERROR_;
	cv::Mat cvDescriptors_kf(descriptors_kf->getNbDescriptors(), descriptors_kf->getNbElements(), m_VOC.getDescType(), descriptors_kf->data());

	// get fbow2 desc of keyframe
	auto itBoW2 = m_list_KFBoW2.find(keyframe->getId());
	if (itBoW2 == m_list_KFBoW2.end())
		return FrameworkReturnCode::_ERROR_;
	const fbow::fBow2 &kfFBow2 = itBoW2->second;

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
		findBestMatches(cvDescriptor, cvDescriptors_kf, candidates, bestIdx, bestDist);
		if (bestIdx != -1)
			matches.push_back(DescriptorMatch(i, bestIdx, bestDist));
	}

	return FrameworkReturnCode::_SUCCESS;
}

FrameworkReturnCode SolARKeyframeRetrieverFBOW::match(const std::vector<int> &indexDescriptors, const SRef<DescriptorBuffer> &descriptors, const SRef<Keyframe> &keyframe, std::vector<DescriptorMatch> &matches)
{
	// convert frame desc to Mat opencv
	if (descriptors->getNbDescriptors() == 0)
		return FrameworkReturnCode::_ERROR_;
	cv::Mat cvDescriptors(descriptors->getNbDescriptors(), descriptors->getNbElements(), m_VOC.getDescType(), descriptors->data());

	// convert keyframe desc to Mat opencv
	SRef<DescriptorBuffer> descriptors_kf = keyframe->getDescriptors();
	if (descriptors_kf->getNbDescriptors() == 0)
		return FrameworkReturnCode::_ERROR_;
	cv::Mat cvDescriptors_kf(descriptors_kf->getNbDescriptors(), descriptors_kf->getNbElements(), m_VOC.getDescType(), descriptors_kf->data());

	// get fbow2 desc of keyframe
	auto itBoW2 = m_list_KFBoW2.find(keyframe->getId());
	if (itBoW2 == m_list_KFBoW2.end())
		return FrameworkReturnCode::_ERROR_;
	const fbow::fBow2 &kfFBow2 = itBoW2->second;

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
		findBestMatches(cvDescriptor, cvDescriptors_kf, candidates, bestIdx, bestDist);
		if (bestIdx != -1)
			matches.push_back(DescriptorMatch(it_des, bestIdx, bestDist));
	}
	return FrameworkReturnCode::_SUCCESS;
}


}
}
}
