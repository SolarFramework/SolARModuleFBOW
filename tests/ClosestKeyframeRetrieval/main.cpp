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

#include "SolARModuleOpencv_traits.h"
#include "SolARModuleFBOW_traits.h"


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

using namespace SolAR::MODULES::OPENCV;
using namespace SolAR::MODULES::FBOW;


namespace xpcf = org::bcom::xpcf;

int main(int argc, char **argv) {

#if NDEBUG
    boost::log::core::get()->set_logging_enabled(false);
#endif

    LOG_ADD_LOG_TO_CONSOLE();

    /* instantiate component manager*/
    /* this is needed in dynamic mode */
    SRef<xpcf::IComponentManager> xpcfComponentManager = xpcf::getComponentManagerInstance();

    std::string configxml = std::string("conf_ClosestKeyframeRetrieval.xml");
    if (argc == 2)
        configxml = std::string(argv[1]);
    if (xpcfComponentManager->load(configxml.c_str()) != org::bcom::xpcf::_SUCCESS)
    {
        LOG_ERROR("Failed to load the configuration file {}", configxml.c_str())
            return -1;
    }

    // declare and create components
    LOG_INFO("<<<<<<<<<<<<<<<<<<  Start creating components");

    //load images used to create Reference keyFrames
    auto imageLoader1 =xpcfComponentManager->create<SolARImageLoaderOpencv>("frame_0001")->bindTo<image::IImageLoader>();
    LOG_INFO("frame_0001 loaded");
    auto imageLoader2 =xpcfComponentManager->create<SolARImageLoaderOpencv>("frame_0002")->bindTo<image::IImageLoader>();
    LOG_INFO("frame_0002 loaded");
    auto imageLoader3 =xpcfComponentManager->create<SolARImageLoaderOpencv>("frame_0003")->bindTo<image::IImageLoader>();
    LOG_INFO("frame_0003 loaded");

     //load images used to retrieve the closest Reference keyframe
    auto imageLoader4 =xpcfComponentManager->create<SolARImageLoaderOpencv>("frame_0004")->bindTo<image::IImageLoader>();
    LOG_INFO("frame_0004 loaded");
    auto imageLoader5 =xpcfComponentManager->create<SolARImageLoaderOpencv>("frame_0005")->bindTo<image::IImageLoader>();
    LOG_INFO("frame_0005 loaded");

    // keypoints detector and descriptor extractor
    auto keypointsDetector = xpcfComponentManager->create<SolARKeypointDetectorOpencv>()->bindTo<features::IKeypointDetector>();
    auto descriptorExtractor = xpcfComponentManager->create<SolARDescriptorsExtractorAKAZE2Opencv>()->bindTo<features::IDescriptorsExtractor>();


    // KeyframeRetriever component to relocalize
    auto kfRetriever = xpcfComponentManager->create<SolARKeyframeRetrieverFBOW>()->bindTo<reloc::IKeyframeRetriever>();


    SRef<Image> image1,image2,image3,image4,image5;

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


    //keypoints and corresponding descriptors are extracted
    std::vector<SRef<Keypoint>> keypoints1,keypoints2,keypoints3,keypoints4,keypoints5;

    keypointsDetector->detect(image1, keypoints1);
    keypointsDetector->detect(image2, keypoints2);
    keypointsDetector->detect(image3, keypoints3);
    keypointsDetector->detect(image4, keypoints4);
    keypointsDetector->detect(image5, keypoints5);


    SRef<DescriptorBuffer>      descriptors1,descriptors2,descriptors3,descriptors4,descriptors5;

    descriptorExtractor->extract(image1, keypoints1, descriptors1);
    descriptorExtractor->extract(image2, keypoints2, descriptors2);
    descriptorExtractor->extract(image3, keypoints3, descriptors3);
    descriptorExtractor->extract(image4, keypoints4, descriptors4);
    descriptorExtractor->extract(image5, keypoints5, descriptors5);


    // the tree first images are converted to keyframes
    SRef<Keyframe> keyframe1,keyframe2,keyframe3;

    keyframe1 = xpcf::utils::make_shared<Keyframe>(keypoints1, descriptors1, image1);
    keyframe2 = xpcf::utils::make_shared<Keyframe>(keypoints2, descriptors2, image2);
    keyframe3 = xpcf::utils::make_shared<Keyframe>(keypoints3, descriptors3, image3);

    // these keyframes are registred in the keyframe retriever for relocalisation
    kfRetriever->addKeyframe(keyframe1);
    kfRetriever->addKeyframe(keyframe2);
    kfRetriever->addKeyframe(keyframe3);

    // image 4 and 5 are used to test the kf retriever

    // with test image 4 the retriever should return keyFrame3 in top retrieved keyframes

    std::vector < SRef <Keyframe>> ret_keyframes;

    SRef<Frame> frame4=xpcf::utils::make_shared<Frame>(keypoints4, descriptors4, image4);
    if (kfRetriever->retrieve(frame4, ret_keyframes) == FrameworkReturnCode::_SUCCESS) {
        LOG_INFO("Retrieval Success");
        if(ret_keyframes[0]->m_idx==2){
            LOG_INFO("image 4 test is OK ")
        }
        else{
            LOG_INFO("image 4 test is KO ")
        }
    }
    else
        LOG_INFO("image 4 test is KO ")


    // with test image 5 the retriever should not return any keyframe

    SRef<Frame> frame5=xpcf::utils::make_shared<Frame>(keypoints5, descriptors5, image5);
    ret_keyframes.clear();
    if (kfRetriever->retrieve(frame5, ret_keyframes) != FrameworkReturnCode::_SUCCESS) {
        LOG_INFO("image 5 test is OK ")
    }
    else
        LOG_INFO("image 5 test is KO ")



    return 0;
}



