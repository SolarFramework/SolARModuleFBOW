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
#include "api/features/IDescriptorsExtractorFromImage.h"
#include "api/reloc/IKeyframeRetriever.h"
#include "api/display/IMatchesOverlay.h"
#include "api/display/IImageViewer.h"
#include "api/features/IMatchesFilter.h"
#include "api/features/IDescriptorMatcher.h"
#include "core/Log.h"


using namespace SolAR;
using namespace SolAR::datastructure;
using namespace SolAR::api;

namespace xpcf = org::bcom::xpcf;

// rotate image before matching keypoints  
//#define ROTATE_IMAGE_BEFORE_MATCHING

// rotate image after matching keypoints 
//#define ROTATE_IMAGE_AFTER_MATCHING

int main(int argc, char **argv) {

#if NDEBUG
    boost::log::core::get()->set_logging_enabled(false);
#endif

    LOG_ADD_LOG_TO_CONSOLE();
    try {
        /* instantiate component manager*/
        /* this is needed in dynamic mode */
        SRef<xpcf::IComponentManager> xpcfComponentManager = xpcf::getComponentManagerInstance();

		std::string filenameXmlConfig = "SolARTest_ModuleFBOW_FeatureMatching_conf.xml";
		if (argc == 2) {
			filenameXmlConfig = argv[1];
			LOG_INFO("Loading config file {}", filenameXmlConfig);
		}

        if(xpcfComponentManager->load(filenameXmlConfig.c_str())!=org::bcom::xpcf::_SUCCESS)
        {
            LOG_ERROR("Failed to load the configuration file SolARTest_ModuleFBOW_FeatureMatching_conf.xml")
            return -1;
        }

        //load images used to create Reference keyFrames
        auto imageLoader1 =xpcfComponentManager->resolve<image::IImageLoader>("image1");
        auto imageLoader2 =xpcfComponentManager->resolve<image::IImageLoader>("image2");

        // keypoints detector and descriptor extractor
        auto extractor = xpcfComponentManager->resolve<features::IDescriptorsExtractorFromImage>();

        // descriptor matcher 
        auto matcher = xpcfComponentManager->resolve<features::IDescriptorMatcher>();

        // KeyframeRetriever component to relocalize
        auto kfRetriever = xpcfComponentManager->resolve<reloc::IKeyframeRetriever>();

        SRef<Image>				image1, image2;
		std::vector<Keypoint>	keypoints1, keypoints2;
		SRef<DescriptorBuffer>  descriptors1, descriptors2;

		// create a covisibility graph
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

#ifdef ROTATE_IMAGE_BEFORE_MATCHING
        image1->rotate90();
#endif
		extractor->extract(image1, keypoints1, descriptors1);
		extractor->extract(image2, keypoints2, descriptors2);
		LOG_INFO("Nb keypoints of image 1: {}", keypoints1.size());
		LOG_INFO("Nb keypoints of image 2: {}", keypoints2.size());

		// the tree first images are converted to keyframes
		SRef<Keyframe> keyframe1;
		keyframe1 = xpcf::utils::make_shared<Keyframe>(keypoints1, descriptors1, image1);
		keyframe1->setId(0);

		// these keyframes are registred in the keyframe retriever for relocalisation
		kfRetriever->addKeyframe(keyframe1);

		// create frame 2
		SRef<Frame> frame2 = xpcf::utils::make_shared<Frame>(keypoints2, descriptors2, image2);

        // match frame 2 to keyframe1
		std::vector<DescriptorMatch> matches;
		matcher->match(frame2->getDescriptors(), keyframe1->getDescriptors(), matches);
		LOG_INFO("Nb matches: {}", matches.size());

		// matches filter
		auto matchesFilter = xpcfComponentManager->resolve<features::IMatchesFilter>();
		matchesFilter->filter(matches, matches, keypoints2, keypoints1);
		LOG_INFO("Nb matches filter: {}", matches.size());

#ifdef ROTATE_IMAGE_AFTER_MATCHING
        image1->rotate90();
        for (auto& kpt : keypoints1) {
        Keypoint newPt(kpt.getId(), image1->getWidth() - kpt.getY(), kpt.getX(), kpt.getR(), kpt.getG(), kpt.getB(), -kpt.getSize(), kpt.getAngle(), kpt.getResponse(), kpt.getOctave(), kpt.getClassId());
        kpt = newPt;
        }
#endif
		// Draw the matches in a dedicated image
		auto overlay = xpcfComponentManager->resolve<display::IMatchesOverlay>();
		SRef<Image> imageMatches;
		LOG_INFO("Draw matches");
		overlay->draw(image2, image1, imageMatches, keypoints2, keypoints1, matches);

		auto viewer = xpcfComponentManager->resolve<display::IImageViewer>();
		LOG_INFO("Display");
		while (true)
		{
			// Display the image with matches in a viewer. If escape key is pressed, exit the loop.
			if (viewer->display(imageMatches) == FrameworkReturnCode::_STOP)
			{
				LOG_INFO("End of DescriptorMatcherUsingFBOW test");
				break;
			}
		}
    }
    catch (xpcf::Exception e)
    {
        LOG_ERROR ("The following exception has been catched: {}", e.what());
        return -1;
    }

    return 0;
}



