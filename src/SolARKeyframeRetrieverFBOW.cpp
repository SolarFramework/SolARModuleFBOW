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
#include "SolARFBOWHelper.h"
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
	m_keyframeRetrieval = xpcf::utils::make_shared<KeyframeRetrieval>();
    declareProperty("VOCpath",m_VOCPath);
    declareProperty("threshold", m_threshold);
    declareProperty("level", m_level);
	declareProperty("matchingDistanceRatio", m_distanceRatio);
	declareProperty("matchingDistanceMax", m_distanceMax);
    declareProperty("distanceMetricId", m_distanceMetricId);

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
	std::ifstream file(m_VOCPath);
	if (!file.is_open())
		LOG_ERROR(" SolARKeyframeRetrieverFBOW onConfigured: Cannot load the vocabulary from file");
	file.close();
    m_VOC.readFromFile(m_VOCPath);
    if (!m_VOC.isValid())
        return xpcf::XPCFErrorCode::_ERROR_INVALID_ARGUMENT;
	LOG_DEBUG("Descriptor name: {}", m_VOC.getDescName());
	LOG_DEBUG("Descriptor type: {}", m_VOC.getDescType());
	LOG_DEBUG("Descriptor size: {}", m_VOC.getDescSize());	
	LOG_DEBUG("Nb of cluster per node: {}", m_VOC.getK());

    return xpcf::XPCFErrorCode::_SUCCESS;
}

FrameworkReturnCode SolARKeyframeRetrieverFBOW::addKeyframe(const SRef<Keyframe> keyframe, bool useMatchedDescriptor)
{
	// Convert desc of keyframe to Mat opencv
	SRef<DescriptorBuffer> desc_Solar = keyframe->getDescriptors();
	SRef<DescriptorBuffer> descToComputeBow;
	if (!useMatchedDescriptor) {
		descToComputeBow = desc_Solar; // by default, use all descriptors 
	}
	else {
		const auto& isMatched = keyframe->getIsKeypointMatched();
		if (!isMatched.empty()) {
			descToComputeBow = xpcf::utils::make_shared<DescriptorBuffer>(desc_Solar->getDescriptorType(), desc_Solar->getDescriptorDataType(), desc_Solar->getNbElements(), 0);
            for (auto i = 0; i < desc_Solar->getNbDescriptors(); i++)
                if (isMatched[i])
                    descToComputeBow->append(desc_Solar->getDescriptor(i));
		}
		else {
            // when keyframe's matched keypoint map is empty, use all descriptors also
			descToComputeBow = desc_Solar;
		}
	}
	cv::Mat desc_OpenCV(descToComputeBow->getNbDescriptors(), descToComputeBow->getNbElements(), m_VOC.getDescType(), descToComputeBow->data());

	// Get bow desc corresponding to keyframe desc
	fbow::fBow v_bow;
	fbow::fBow2 v_bow2;
	m_VOC.transform(desc_OpenCV, m_level, v_bow, v_bow2);

    // convertir bow to solar
    datastructure::BoWFeature v_bowFeature = SolARFBOWHelper::fbow2Solar(v_bow);
    datastructure::BoWLevelFeature v_bowLevelFeature = SolARFBOWHelper::fbow2Solar(v_bow2);

	// Add bow desc to the database
	m_keyframeRetrieval->acquireLock();
    return m_keyframeRetrieval->addDescriptor(keyframe->getId(), v_bowFeature, v_bowLevelFeature);
}

FrameworkReturnCode SolARKeyframeRetrieverFBOW::suppressKeyframe(uint32_t keyframe_id)
{
	m_keyframeRetrieval->acquireLock();
	return m_keyframeRetrieval->removeDescriptor(keyframe_id);	
}

FrameworkReturnCode SolARKeyframeRetrieverFBOW::retrieve(const SRef<Frame> frame, std::vector<uint32_t> &retKeyframes_id)
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

    // convertir bow to solar
    datastructure::BoWFeature v_bowFeature = SolARFBOWHelper::fbow2Solar(v_bow);
    datastructure::BoWLevelFeature v_bowLevelFeature = SolARFBOWHelper::fbow2Solar(v_bow2);

	// get candidates that have at least 1 common word with the query frame
	std::map<uint32_t, int> scoreCandidates;
    for (auto const &it : v_bowLevelFeature) {
		std::set<uint32_t> kfs_id; 		
		if (m_keyframeRetrieval->getInvertedIndex(it.first, kfs_id) != FrameworkReturnCode::_SUCCESS)
			continue;
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
    std::vector<std::pair<int, double>> distKeyframes;
	for (auto const &it : bestCandidates) {
        datastructure::BoWFeature kfBoW;
        if (m_keyframeRetrieval->getBoWFeature(it, kfBoW) != FrameworkReturnCode::_SUCCESS)
			continue;
        double score = 0.;
        ScoringType scoreMethod = static_cast<ScoringType>(m_distanceMetricId);
        if (scoreMethod == ScoringType::L2_NORM)
            score = SolARFBOWHelper::distanceBoW(kfBoW, v_bowFeature);
        else if (scoreMethod == ScoringType::L1_NORM)
            score = SolARFBOWHelper::distanceL1BoW(kfBoW, v_bowFeature);
        else if (scoreMethod == ScoringType::BHATTACHARYYA)
            score = SolARFBOWHelper::distanceBhattacharyyaBoW(kfBoW, v_bowFeature);
        else if (scoreMethod == ScoringType::CHI_SQUARE)
            score = SolARFBOWHelper::distanceChiSquareBoW(kfBoW, v_bowFeature);
        else if (scoreMethod == ScoringType::DOT_PRODUCT)
            score = SolARFBOWHelper::distanceDotProductBoW(kfBoW, v_bowFeature);
        else if (scoreMethod == ScoringType::KLS)
            score = SolARFBOWHelper::distanceKLSBoW(kfBoW, v_bowFeature);
        else {
            LOG_WARNING("Invalid BoW metric ID {}, use default L2", m_distanceMetricId);
            score = SolARFBOWHelper::distanceBoW(kfBoW, v_bowFeature);
        }
		if (score > m_threshold)
            distKeyframes.push_back(std::pair<int, double>(it, score));
	}

    if (distKeyframes.size() == 0)
		return FrameworkReturnCode::_ERROR_;

    // sort candidate keyframes according to score
    std::sort(distKeyframes.begin(), distKeyframes.end(),
            [](const std::pair<int, double>& v1, const std::pair<int, double>& v2) { return v1.second > v2.second; });

    for (auto const &it : distKeyframes) {
        retKeyframes_id.push_back(it.first);
	}	

    return FrameworkReturnCode::_SUCCESS;
}

FrameworkReturnCode SolARKeyframeRetrieverFBOW::retrieve(const SRef<Frame> frame, const std::set<unsigned int> & canKeyframes_id, std::vector<uint32_t> & retKeyframes_id)
{
	// convert frame desc to Mat opencv
	SRef<DescriptorBuffer> desc_Solar = frame->getDescriptors();
	if (desc_Solar->getNbDescriptors() == 0)
		return FrameworkReturnCode::_ERROR_;
	cv::Mat desc_OpenCV(desc_Solar->getNbDescriptors(), desc_Solar->getNbElements(), m_VOC.getDescType(), desc_Solar->data());

	// calculate bow desc corresponding to the query frame
	fbow::fBow v_bow;
	v_bow = m_VOC.transform(desc_OpenCV);

    // convertir bow to solar
    datastructure::BoWFeature v_bowFeature = SolARFBOWHelper::fbow2Solar(v_bow);

	// find nearest keyframes
    std::vector<std::pair<int, double>> distKeyframes;
	for (auto const &it : canKeyframes_id) {
        datastructure::BoWFeature kfBoW;
        if (m_keyframeRetrieval->getBoWFeature(it, kfBoW) != FrameworkReturnCode::_SUCCESS)
			continue;
        double score = SolARFBOWHelper::distanceBoW(kfBoW, v_bowFeature);
        if (score > m_threshold)
            distKeyframes.push_back(std::pair<int, double>(it, score));
	}

    if (distKeyframes.size() == 0)
        return FrameworkReturnCode::_ERROR_;

    // sort candidate keyframes according to score
    std::sort(distKeyframes.begin(), distKeyframes.end(),
            [](const std::pair<int, double>& v1, const std::pair<int, double>& v2) { return v1.second > v2.second; });

    for (auto const &it : distKeyframes) {
        retKeyframes_id.push_back(it.first);
    }

	return FrameworkReturnCode::_SUCCESS;
}

FrameworkReturnCode SolARKeyframeRetrieverFBOW::saveToFile(const std::string& file) const
{    
	std::ofstream ofs(file, std::ios::binary);
	OutputArchive oa(ofs);
	oa << m_level;
	oa << m_keyframeRetrieval;
	ofs.close();
	return FrameworkReturnCode::_SUCCESS;
}

FrameworkReturnCode SolARKeyframeRetrieverFBOW::loadFromFile(const std::string& file)
{
	std::ifstream ifs(file, std::ios::binary);
	if (!ifs.is_open())
		return FrameworkReturnCode::_ERROR_;
    InputArchive ia(ifs);
	ia >> m_level;
	ia >> m_keyframeRetrieval;
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

FrameworkReturnCode SolARKeyframeRetrieverFBOW::match(const SRef<Frame> frame, const SRef<Keyframe> keyframe, std::vector<DescriptorMatch> &matches)
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

    // get bow level desc of keyframe
    datastructure::BoWLevelFeature bowLevelFeature;
    if (m_keyframeRetrieval->getBoWLevelFeature(keyframe->getId(), bowLevelFeature) != FrameworkReturnCode::_SUCCESS)
		return FrameworkReturnCode::_ERROR_;

	for (int i = 0; i < cvDescriptors.rows; i++) {
		const cv::Mat cvDescriptor = cvDescriptors.row(i);
		int node = m_VOC.transform(cvDescriptor, m_level);
		std::vector<uint32_t> candidates;
        auto it = bowLevelFeature.find(node);
        if (it != bowLevelFeature.end())
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

FrameworkReturnCode SolARKeyframeRetrieverFBOW::match(const std::vector<int> &indexDescriptors, const SRef<DescriptorBuffer> descriptors, const SRef<Keyframe> keyframe, std::vector<DescriptorMatch> &matches)
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

    // get bow level desc of keyframe
    datastructure::BoWLevelFeature bowLevelFeature;
    if (m_keyframeRetrieval->getBoWLevelFeature(keyframe->getId(), bowLevelFeature) != FrameworkReturnCode::_SUCCESS)
        return FrameworkReturnCode::_ERROR_;

	std::vector<bool> checkMatches(keyframe->getKeypoints().size(), true);
	for (auto &it_des: indexDescriptors) {
		const cv::Mat cvDescriptor = cvDescriptors.row(it_des);
		int node = m_VOC.transform(cvDescriptor, m_level);
		std::vector<uint32_t> candidates;
        auto it = bowLevelFeature.find(node);
        if (it != bowLevelFeature.end())
			candidates = it->second;

		// find the best match
		int bestIdx;
		float bestDist;
		findBestMatches(cvDescriptor, cvDescriptors_kf, candidates, bestIdx, bestDist);
		if ((bestIdx != -1) && checkMatches[bestIdx]) {
			matches.push_back(DescriptorMatch(it_des, bestIdx, bestDist));
			checkMatches[bestIdx] = false;
		}
	}
	return FrameworkReturnCode::_SUCCESS;
}

const SRef<datastructure::KeyframeRetrieval>& SolARKeyframeRetrieverFBOW::getConstKeyframeRetrieval() const
{
	return m_keyframeRetrieval;
}

std::unique_lock<std::mutex> SolARKeyframeRetrieverFBOW::getKeyframeRetrieval(SRef<datastructure::KeyframeRetrieval>& keyframeRetrieval)
{
	keyframeRetrieval = m_keyframeRetrieval;
	return m_keyframeRetrieval->acquireLock();
}

void SolARKeyframeRetrieverFBOW::setKeyframeRetrieval(const SRef<datastructure::KeyframeRetrieval> keyframeRetrieval)
{
	m_keyframeRetrieval = keyframeRetrieval;
}


}
}
}
