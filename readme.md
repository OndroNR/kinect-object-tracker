DP 2015 - Galbavy - Kinect object tracker
===============================================================================
**Author:** Ing. Ondrej Galbavý  
**Master thesis:** Visual analysis of a robot movement using methods of 3D stereo reconstruction  
**Supervisor:** Ing. Vanda Benešová, PhD.  
**Keywords:** computer vision, 3D movement tracking, stereo reconstruction   
2015, May  

Thesis Description
===============================================================================
Processing spatial data and tracking the movement in 3D space are current research topics of computer vision. This thesis focuses on development of methods of automatic visual analysis of the moving objects in 3D space. The developed method uses a background modeling, key point tracking in the foreground masks, stereo depth reconstruction of key points through local descriptors and stereo pair clustering. Moreover, the simplified method for generating reference data, which uses Microsoft Kinect for Xbox to determine the depth, was developed. The methods are verified on the dataset in the domain of robotics to provide information about the movement of the robot for further processing. An accuracy and time requirement of the method is examined.

------------------------------------------------------------------------------

Coded in C++.  
Microsoft Visual Studio 2012 + MSVC 11  
Kinect for Windows SDK v1.8  
OpenCV 2.4.10 path (d:\data\c3\sdk\OpenCV2410\build\include\opencv) currently hardcoded in OpenCV_*.props for MS VS.