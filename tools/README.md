# FBoW Creator Tool

[![License](https://img.shields.io/github/license/SolARFramework/Sample-Slam?style=flat-square&label=License)](https://www.apache.org/licenses/LICENSE-2.0)


This project is a tool that allows to create a fbow vocabulary from a set of images.

## How to run

### Data installation

You can run installData.bat (under windows) or installData.sh (under linux) to download a small example dataset of about 1000 images, as well as two camera calibration files necessary to run the code. The file camera_calibration_1344x756.json corresponds to the original image resolution. If you use the other file camera_calibration_640x480.json, the input images will be resized to 640x480 before the FBOW generation. Note that in both cases, only the width and height are used. In fact, without such a calibration file, **SolARImagesAsCameraOpencv** will not work. That is why we need these two calibration files. 

### Configuration

Go to the binary directory and open the configuration file SolARTool_FBOWCreator_conf.xml. You can set the **imagesDirectoryPath** value of the **SolARImagesAsCameraOpencv** to your path to images. By default, it has already been set to the downloaded dataset folder (after running installData.bat or installData.sh). 
You can also configure the type of feature (e.g. AKAZE2, ORB, SIFT, PopSift, ...) that you want to use to create your fbow vocabulary.

### More Datasets

It is recommanded to use a larger database in order to generate better vocabularies. You can download public image databases or use your own datasets. For example, you can download the 7-Scenes dataset (https://www.microsoft.com/en-us/research/project/rgb-d-dataset-7-scenes/). Please remember to modify **imagesDirectoryPath** in SolARTool_FBOWCreator_conf.xml beforing running the code. 


### Run samples

* In the binary directory, you run the following command. When finishing, you archive a vocabulary **voc.fbow** by default:
<pre><code>SolARTool_FBOWCreator.exe</code></pre>

* To see more details run:
<pre><code>SolARTool_FBOWCreator.exe -h</code></pre>

* For example, if you want to generate fbow dictionary for PopSift:
<pre><code>.\SolARTool_FBOWCreator.exe --config=.\SolARTool_FBOWCreator_PopSift_conf.xml --out=popsift_uint8.fbow --v=1</code></pre>

## Contact 
Website https://solarframework.github.io/

Contact framework.solar@b-com.com