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

#include <iostream>
#include <string>
#include <vector>

#include <boost/log/core.hpp>

// ADD MODULES TRAITS HEADERS HERE

// ADD XPCF HEADERS HERE
#include "xpcf/xpcf.h"

// ADD COMPONENTS HEADERS HERE

#include "api/image/IImageLoader.h"
#include "api/features/IKeypointDetector.h"
#include "api/features/IDescriptorsExtractor.h"
#include "api/reloc/IKeyframeRetriever.h"
#include "core/Log.h"


using namespace SolAR;
using namespace SolAR::datastructure;
using namespace SolAR::api;

namespace xpcf = org::bcom::xpcf;

int main(int argc, char **argv) {

#if NDEBUG
    boost::log::core::get()->set_logging_enabled(false);
#endif

    LOG_ADD_LOG_TO_CONSOLE();
    try {
        /* instantiate component manager*/
        /* this is needed in dynamic mode */
        SRef<xpcf::IComponentManager> xpcfComponentManager = xpcf::getComponentManagerInstance();

        if(xpcfComponentManager->load("SolARTest_ModuleFBOW_ClosestKeyframeRetrieval_conf.xml")!=org::bcom::xpcf::_SUCCESS)
        {
            LOG_ERROR("Failed to load the configuration file SolARTest_ModuleFBOW_ClosestKeyframeRetrieval_conf.xml")
            return -1;
        }

        // declare and create components
        LOG_INFO("<<<<<<<<<<<<<<<<<<  Start creating components");

        //load images used to create Reference keyFrames
        auto imageLoader1 =xpcfComponentManager->resolve<image::IImageLoader>("frame_0001");
        auto imageLoader2 =xpcfComponentManager->resolve<image::IImageLoader>("frame_0002");
        auto imageLoader3 =xpcfComponentManager->resolve<image::IImageLoader>("frame_0003");

         //load images used to retrieve the closest Reference keyframe
        auto imageLoader4 =xpcfComponentManager->resolve<image::IImageLoader>("frame_0004");
        auto imageLoader5 =xpcfComponentManager->resolve<image::IImageLoader>("frame_0005");

        // keypoints detector and descriptor extractor
        auto keypointsDetector = xpcfComponentManager->resolve<features::IKeypointDetector>();
        auto descriptorExtractor = xpcfComponentManager->resolve<features::IDescriptorsExtractor>();


        // KeyframeRetriever component to relocalize
        auto kfRetriever = xpcfComponentManager->resolve<reloc::IKeyframeRetriever>();

        SRef<Image> image1,image2,image3,image4,image5;
		std::vector<Keypoint> keypoints1, keypoints2, keypoints3, keypoints4, keypoints5;
		SRef<DescriptorBuffer>      descriptors1, descriptors2, descriptors3, descriptors4, descriptors5;

		// create a covisibility graph
		std::string fileName = "keyframes_retriever.txt";
		LOG_INFO("Load the keyframes feature from {}", fileName);
		if (kfRetriever->loadFromFile(fileName) == FrameworkReturnCode::_ERROR_) {
			LOG_INFO("This file doesn't exist. Create a new keyframe database");
			if (imageLoader1->getImage(image1) != FrameworkReturnCode::_SUCCESS)
			{
				LOG_ERROR("Cannot load image 1 with path {}", imageLoader1->bindTo<xpcf::IConfigurable>()->getProperty("pathFile")->getStringValue());
				return -1;
			}
			if (imageLoader2->getImage(image2) != FrameworkReturnCode::_SUCCESS)
			{
				LOG_ERROR("Cannot load image 2 with path {}", imageLoader2->bindTo<xpcf::IConfigurable>()->getProperty("pathFile")->getStringValue());
				return -1;
			}
			if (imageLoader3->getImage(image3) != FrameworkReturnCode::_SUCCESS)
			{
				LOG_ERROR("Cannot load image 1 with path {}", imageLoader3->bindTo<xpcf::IConfigurable>()->getProperty("pathFile")->getStringValue());
				return -1;
			}

			keypointsDetector->detect(image1, keypoints1);
			keypointsDetector->detect(image2, keypoints2);
			keypointsDetector->detect(image3, keypoints3);

			descriptorExtractor->extract(image1, keypoints1, descriptors1);
			descriptorExtractor->extract(image2, keypoints2, descriptors2);
			descriptorExtractor->extract(image3, keypoints3, descriptors3);

			// the tree first images are converted to keyframes
			SRef<Keyframe> keyframe1, keyframe2, keyframe3;

			keyframe1 = xpcf::utils::make_shared<Keyframe>(keypoints1, descriptors1, image1);
			keyframe2 = xpcf::utils::make_shared<Keyframe>(keypoints2, descriptors2, image2);
			keyframe3 = xpcf::utils::make_shared<Keyframe>(keypoints3, descriptors3, image3);

			keyframe1->setId(0);
			keyframe2->setId(1);
			keyframe3->setId(2);

			// these keyframes are registred in the keyframe retriever for relocalisation
			kfRetriever->addKeyframe(keyframe1);
			kfRetriever->addKeyframe(keyframe2);
			kfRetriever->addKeyframe(keyframe3);

			kfRetriever->saveToFile(fileName);
		}
		else {
			LOG_INFO("Load done");
		}

        
		// using image 4, 5 for testing
        if (imageLoader4->getImage(image4) != FrameworkReturnCode::_SUCCESS)
        {
            LOG_ERROR("Cannot load image 4 with path {}", imageLoader4->bindTo<xpcf::IConfigurable>()->getProperty("pathFile")->getStringValue());
            return -1;
        }
        if (imageLoader5->getImage(image5) != FrameworkReturnCode::_SUCCESS)
        {
            LOG_ERROR("Cannot load image 5 with path {}", imageLoader5->bindTo<xpcf::IConfigurable>()->getProperty("pathFile")->getStringValue());
            return -1;
        }

        keypointsDetector->detect(image4, keypoints4);
        keypointsDetector->detect(image5, keypoints5);
     
        descriptorExtractor->extract(image4, keypoints4, descriptors4);
        descriptorExtractor->extract(image5, keypoints5, descriptors5);

        // with test image 4 the retriever should return keyFrame3 in top retrieved keyframes
        std::vector <uint32_t> ret_keyframes;
        SRef<Frame> frame4 = xpcf::utils::make_shared<Frame>(keypoints4, descriptors4, image4);
		if (kfRetriever->retrieve(frame4, ret_keyframes) == FrameworkReturnCode::_SUCCESS) {
			LOG_INFO("Retrieval Success");
			if (ret_keyframes[0] == 2) {
				LOG_INFO("image 4 test is OK ")
			}
			else {
				LOG_INFO("image 4 test is KO ")
			}
        }
        else
            LOG_INFO("image 4 test is KO ")


        // with test image 5 the retriever should not return any keyframe
        SRef<Frame> frame5 = xpcf::utils::make_shared<Frame>(keypoints5, descriptors5, image5);
        ret_keyframes.clear();
        if (kfRetriever->retrieve(frame5, ret_keyframes) != FrameworkReturnCode::_SUCCESS) {
            LOG_INFO("image 5 test is OK ")
        }
        else
            LOG_INFO("image 5 test is KO ")
    }
    catch (xpcf::Exception e)
    {
        LOG_ERROR ("The following exception has been catched: {}", e.what());
        return -1;
    }

    return 0;
}



