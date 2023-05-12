# FBoW Creator Tool

[![License](https://img.shields.io/github/license/SolARFramework/Sample-Slam?style=flat-square&label=License)](https://www.apache.org/licenses/LICENSE-2.0)


This project is a tool that allows to create a fbow vocabulary from a set of images

## How to run

### Data installation

You can run installData.bat (under windows) or installData.sh (under linux) to download a small example dataset of about 1000 images, as well as two camera calibration files necessary to run the code. camera_calibration_1920x1080.json corresponds to the original image resolution. If you use camera_calibration_640x480.json, the input images will be resized to 640x480 before the FBOW generation. Note that in both cases, only the width and height is used. Actually, without such a .json calibration file, **SolARImagesAsCameraOpencv** will not work. That is why we need these two calibration files. 

### Configuration

Go to the binary directory and open the configuration file SolARTool_FBOWCreator_conf.xml. You can set the **imagesDirectoryPath** value of the **SolARImagesAsCameraOpencv** to your path to images. By default, it has already been to the downloaded dataset folder. 
You can also configure the type of feature (e.g. AKAZE2, ORB, SIFT, PopSift, ...) that you want to use to create your fbow vocabulary.

### More Datasets

A larger database is available at b-com filer. You can download the 7-Scene-Dataset (43 000 images) from <pre><code>\\shr_filer.b-com.local\filer\projects\ARCAD\Data\Bcom\datafbow\datafbow_7scenedataset.zip</code></pre>. But, for this, you may need to ask for access right. Once downloaded, please remember to modify **imagesDirectoryPath** in SolARTool_FBOWCreator_conf.xml in order to generate the FBOW dictionary corresponding to this dataset. 


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