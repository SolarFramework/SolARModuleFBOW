# FBoW Creator Tool

[![License](https://img.shields.io/github/license/SolARFramework/Sample-Slam?style=flat-square&label=License)](https://www.apache.org/licenses/LICENSE-2.0)


This project is a tool that allows to create a fbow vocabulary from a set of images

## How to run

### Configuration

Open the configuration file and set the **imagesDirectoryPath** value of the **SolARImagesAsCameraOpencv** to your path to images.
You can also configure the type of feature (e.g. AKAZE2, ORB, SIFT,...) that you want to use to create your fbow vocabulary.

### Data

You can run installData.bat (under windows) or installData.sh (under linux) to download a small example dataset of about 1000 images. If you want to use a larger database, you can download the 7-Scene-Dataset (43 000 images) from the b-com filer (\\shr_filer.b-com.local\filer\projects\ARCAD\Data\Bcom\datafbow\datafbow_7scenedataset.zip). For this, you may need to ask for access right.   

### Run samples

* In the binary directory, you run the following command. When finishing, you archive a vocabulary **voc.fbow** by default:
<pre><code>SolARTool_FBOWCreator.exe</code></pre>

* To see more details run:
<pre><code>SolARTool_FBOWCreator.exe -h</code></pre>

## Contact 
Website https://solarframework.github.io/

Contact framework.solar@b-com.com