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
	v_bow = m_VOC.transform(desc_OpenCV);

	// add bow desc and keyfram to the lists
	m_list_KFBoW.push_back(v_bow);
	m_list_keyframes.push_back(keyframe);

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


}
}
}
