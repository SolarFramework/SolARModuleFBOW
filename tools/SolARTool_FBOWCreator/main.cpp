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

// SolAR header
#include <boost/log/core.hpp>
#include "xpcf/xpcf.h"
#include "core/Log.h"
#include "api/input/devices/ICamera.h"
#include "api/display/IImageViewer.h"
#include "api/display/I2DOverlay.h"
#include "api/features/IDescriptorsExtractorFromImage.h"
#include "api/image/IImageConvertor.h"
// OpenCV header
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/features2d.hpp"
#include "SolAROpenCVHelper.h"
// Fbow header
#include "vocabulary_creator.h"

using namespace SolAR;
using namespace SolAR::datastructure;
using namespace SolAR::api;
namespace xpcf  = org::bcom::xpcf;

const cv::String keys =
"{help h usage ?||}"
"{config|SolARTool_FBOWCreator_conf.xml| xml configuration file}"
"{out|voc.fbow| the name of output file}"
"{k|10| number of cluster at each node}"
"{l|6| number of levels}"
"{t|8| number of threads to accelerate}"
"{maxIters|5000| number of maximal iterations at each node}"
"{v|0| verbose}"
;

int main(int argc, char *argv[])
{
#if NDEBUG
    boost::log::core::get()->set_logging_enabled(false);
#endif
    LOG_ADD_LOG_TO_CONSOLE();

	cv::CommandLineParser parser(argc, argv, keys);
	if (parser.has("help"))
	{
		parser.printMessage();
		return 0;
	}

	// get parameters
	std::string configxml = parser.get<std::string>("config");
	std::string outputName = parser.get<std::string>("out");
	int nbClusters = parser.get<int>("k");
	int nbLevels = parser.get<int>("l");
	int nbThreads = parser.get<int>("t");
	int nbMaxIters = parser.get<int>("maxIters");
	int bVerbose = parser.get<int>("v");

	// components
	SRef<input::devices::ICamera> camera;
	SRef<display::IImageViewer> imageViewer;
	SRef<display::I2DOverlay> overlay2D;
    SRef<features::IDescriptorsExtractorFromImage> descriptorExtractorFromImage;
	SRef<image::IImageConvertor> imageConvertor;

	// load components
    try {
        /* instantiate component manager */
        /* this is needed in dynamic mode */
        SRef<xpcf::IComponentManager> xpcfComponentManager = xpcf::getComponentManagerInstance();
        if(xpcfComponentManager->load(configxml.c_str())!=org::bcom::xpcf::_SUCCESS)
        {
			LOG_ERROR("Failed to load the configuration file {}", configxml.c_str());
            return -1;
        }
        /* Declare and create components */
        LOG_INFO("Start creating components");
		camera = xpcfComponentManager->resolve<input::devices::ICamera>();
		imageViewer = xpcfComponentManager->resolve<display::IImageViewer>();
		overlay2D = xpcfComponentManager->resolve<display::I2DOverlay>();
        descriptorExtractorFromImage = xpcfComponentManager->resolve<features::IDescriptorsExtractorFromImage>();
		imageConvertor = xpcfComponentManager->resolve<image::IImageConvertor>();
		LOG_INFO("Components created!");
	}
	catch (xpcf::Exception e)
	{
		LOG_ERROR("The following exception has been catch : {}", e.what());
		return -1;
	}

	// Start camera capture
	if (camera->start() == FrameworkReturnCode::_ERROR_)
	{
		LOG_ERROR("Cannot start loader");
		return -1;
	}
	LOG_INFO("Data loader started!");

	// Get and set camera intrinsics parameters
	CameraParameters camParams;
	camParams = camera->getParameters();
	uint32_t width = camParams.resolution.width;
	uint32_t height = camParams.resolution.height;
	LOG_INFO("Resolution of image: {} x {}", width, height);
	std::string descName = descriptorExtractorFromImage->getTypeString();
	LOG_INFO("Feature type: {}", descName);

	// Feature extraction
	std::vector<SRef<DescriptorBuffer>> imageDescriptors;
	clock_t start = clock();
	int count(0);
	while (true)
	{
		// get image
		SRef<Image> image;
		if (camera->getNextImage(image) != FrameworkReturnCode::_SUCCESS) {
			LOG_ERROR("Error during capture");
			break;
		}
		// convert to grey image
		SRef<Image> greyImage;
		if (image->getImageLayout() != Image::ImageLayout::LAYOUT_GREY)
			imageConvertor->convert(image, greyImage, datastructure::Image::ImageLayout::LAYOUT_GREY);
		else
			greyImage = image;
		// feature extraction image
		std::vector<Keypoint> keypoints;
        SRef<DescriptorBuffer> descriptors;
		if (descriptorExtractorFromImage->extract(greyImage, keypoints, descriptors) != FrameworkReturnCode::_SUCCESS)
			continue;        
		imageDescriptors.push_back(descriptors);
		//LOG_INFO("Image {} - Number of features: {}\r", count, keypoints.size());
		std::cout << "Process frame " << count << "\r";
		count++;
		// display image
		overlay2D->drawCircles(keypoints, image);
		if (imageViewer->display(image) == SolAR::FrameworkReturnCode::_STOP)
			break;
    }
	// convert to opencv descriptors
	LOG_INFO("\nConverting features to opencv feature...");
	std::vector<cv::Mat> allFeatures;
	for (const auto& it : imageDescriptors) {
		uint32_t type_conversion = SolAR::MODULES::OPENCV::SolAROpenCVHelper::deduceOpenDescriptorCVType(it->getDescriptorDataType());
		cv::Mat cvDescriptorFrame(it->getNbDescriptors(), it->getNbElements(), type_conversion);
		cvDescriptorFrame.data = (uchar*)it->data();
		for (int i = 0; i < cvDescriptorFrame.rows; ++i)
			allFeatures.push_back(cvDescriptorFrame.row(i));
	}
	LOG_INFO("Number of all features: {}", allFeatures.size());

	// train the vocabulary using fbow
	fbow::VocabularyCreator::Params params;
	params.k = nbClusters;
	params.L = nbLevels;
	params.nthreads = nbThreads;
	params.maxIters = nbMaxIters;
	params.verbose = bVerbose;
	srand(0);
	fbow::VocabularyCreator voc_creator;
	fbow::Vocabulary voc;

	LOG_INFO("Creating a {} ^ {} vocabulary of {} descriptor...", params.k, params.L, descName);
	voc_creator.create(voc, allFeatures, descName, params);
	std::cout << "nblocks=" << voc.size() << std::endl;
	voc.saveToFile(outputName);

	std::cout << "Save dict done!!!" << std::endl;
	// display stats on frame rate
	clock_t end = clock();
	double duration = double(end - start) / CLOCKS_PER_SEC;
	printf("\n\nElasped time is %.2lf seconds.\n", duration);
    
    return 0;
}
