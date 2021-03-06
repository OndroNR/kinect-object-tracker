// based on OpenCV CamShift demo and KinectBridgeWithOpenCVBasics-D2D

#include <opencv2/opencv.hpp>
//#include "MyKinectHelper.h"
#include "KinectHelper.h"
#include "OpenCVHelper.h"
#include <fstream>

using namespace cv;
using namespace Microsoft::KinectBridge;
using namespace std;

Microsoft::KinectBridge::OpenCVFrameHelper m_frameHelper;
OpenCVHelper m_openCVHelper;

NUI_IMAGE_RESOLUTION m_colorResolution;
NUI_IMAGE_RESOLUTION m_depthResolution;

#define DEPT_COLOR_PAIR pair<NUI_DEPTH_IMAGE_POINT, NUI_COLOR_IMAGE_POINT>

/// <summary>
/// Initializes the first available Kinect found
/// </summary>
/// <returns>S_OK if successful, E_FAIL otherwise</returns>
HRESULT CreateFirstConnected()
{
    // If Kinect is already initialized, return
    if (m_frameHelper.IsInitialized()) 
    {
        return S_OK;
    }

    HRESULT hr;

    // Get number of Kinect sensors
    int sensorCount = 0;
    hr = NuiGetSensorCount(&sensorCount);
    if (FAILED(hr)) 
    {
        return hr;
    }

    // If no sensors, update status bar to report failure and return
    if (sensorCount == 0)
    {
        //SetStatusMessage(IDS_ERROR_KINECT_NOKINECT);
        return E_FAIL;
    }

    // Iterate through Kinect sensors until one is successfully initialized
    for (int i = 0; i < sensorCount; ++i) 
    {
        INuiSensor* sensor = NULL;
        hr = NuiCreateSensorByIndex(i, &sensor);
        if (SUCCEEDED(hr))
        {
            hr = m_frameHelper.Initialize(sensor);
            if (SUCCEEDED(hr)) 
            {
                // Report success
                //SetStatusMessage(IDS_STATUS_INITSUCCESS);
                return S_OK;
            }
            else
            {
                // Uninitialize KinectHelper to show that Kinect is not ready
                m_frameHelper.UnInitialize();
            }
        }
    }

    // Report failure
    //SetStatusMessage(IDS_ERROR_KINECT_INIT);
    return E_FAIL;
}


bool selectObject = false;
int trackObject = 0;
Point origin;
Rect selection;

bool manualPick = false;
Point manualPickPoint;
NUI_DEPTH_IMAGE_POINT manualPickDepthPoint;

Mat fgColor;


// selecting initial camshift state
static void onMouse( int event, int x, int y, int, void* )
{
    if( selectObject )
    {
        selection.x = MIN(x, origin.x);
        selection.y = MIN(y, origin.y);
        selection.width = std::abs(x - origin.x);
        selection.height = std::abs(y - origin.y);

        selection &= Rect(0, 0, fgColor.cols, fgColor.rows);
    }

    switch( event )
    {
    case EVENT_LBUTTONDOWN:
        origin = Point(x,y);
        selection = Rect(x,y,0,0);
        selectObject = true;
        break;
    case EVENT_LBUTTONUP:
        selectObject = false;
        if( selection.width > 0 && selection.height > 0 )
            trackObject = -1;
        break;
    }
}

static void onMouseManual( int event, int x, int y, int, void* )
{
    switch( event )
    {
    case EVENT_LBUTTONDOWN:
		manualPick = true;
        manualPickPoint = Point(x,y);
        break;
    case EVENT_LBUTTONUP:
		manualPick = false;
        break;
    }
}

int main( int argc, char** argv )
{
	//if (!KinectHelper::initKinect())
	//{
	//	cerr << "Could not init Kinect!" << endl;
	//	return 1;
	//}

	if (!SUCCEEDED(CreateFirstConnected()))
    {
		cerr << "Could not init Kinect!" << endl;
		return 1;
	}

    // Set color resolution
    m_colorResolution = NUI_IMAGE_RESOLUTION_640x480;
    m_frameHelper.SetColorFrameResolution(m_colorResolution);

    // Set depth resolution
    m_depthResolution = NUI_IMAGE_RESOLUTION_640x480;
    m_frameHelper.SetDepthFrameResolution(m_depthResolution);

	Mat m_colorMat;
	Mat m_depthMat;
	Mat m_depthRgbMat;

	m_colorMat.create(Size(640, 480), m_frameHelper.COLOR_TYPE);
	m_depthMat.create(Size(640, 480), m_frameHelper.COLOR_TYPE);
	m_depthRgbMat.create(Size(640, 480), m_frameHelper.DEPTH_TYPE);

	BackgroundSubtractorMOG2 mog2color;
	BackgroundSubtractorMOG2 mog2depth;
	Mat fgDepth;
	mog2depth.setInt("nmixtures", 3);


	cout << "Hello" << endl;

	auto fs = FileStorage();
	fs.open("world_calibration_kinect.xml", FileStorage::READ);
	Mat worldTransformationMatrix;

	if (!fs.isOpened())
	{
		cerr << "Failed to open world calibration file" << endl;
	}
	else
	{
		fs["transformMatrix"] >> worldTransformationMatrix;
		fs.release();
	}

	Mat colorStaticMask = imread("mask.png");
	cvtColor(colorStaticMask, colorStaticMask, CV_BGR2GRAY);

	int dilation_size = 1;
	Mat morphElement = getStructuringElement( MORPH_ELLIPSE,
                                       Size( 2*dilation_size + 1, 2*dilation_size+1 ),
                                       Point( dilation_size, dilation_size ) );


	Rect trackWindow;



	INuiCoordinateMapper* pCordinateMapper;
	m_frameHelper.GetCoordinateMapper(&pCordinateMapper);
	

    namedWindow( "Fg mask", 0 );
    setMouseCallback( "Fg mask", onMouse, 0 );

    namedWindow( "Color", 0 );
    setMouseCallback( "Color", onMouseManual, 0 );

	bool haveSomething = false;

	ofstream outStream("traj.txt");

	while (true)
	{
		if (SUCCEEDED(m_frameHelper.UpdateColorFrame()))
		{
			//cout << "UpdateColorFrame" << endl;

			HRESULT hr = m_frameHelper.GetColorImage(&m_colorMat);
            if (!FAILED(hr))
            {
                mog2color(m_colorMat, fgColor);
				morphologyEx(fgColor, fgColor, MORPH_OPEN, morphElement, Point(-1,-1), 1);
				haveSomething = true;
            }

		}
		if (SUCCEEDED(m_frameHelper.UpdateDepthFrame()))
		{
			//cout << "UpdateDepthFrame" << endl;

			//HRESULT hr = m_frameHelper.GetDepthImage(&m_depthMat);
			//HRESULT hr2 = m_frameHelper.GetDepthImageAsArgb(&m_depthRgbMat);
   //         if (!FAILED(hr))
   //         {
   //             //mog2depth(m_depthMat, fgDepth);
			//	//haveSomething = true;
   //         }

		}

		if (trackObject)
		{
			if( trackObject < 0 )
			{
				trackWindow = selection;
                trackObject = 1;
			}

			RotatedRect trackBox = CamShift(fgColor, trackWindow,
									TermCriteria( TermCriteria::EPS | TermCriteria::COUNT, 10, 1 ));
            if( trackWindow.area() <= 1 )
            {
                int cols = fgColor.cols, rows = fgColor.rows, r = (MIN(cols, rows) + 5)/6;
                trackWindow = Rect(trackWindow.x - r, trackWindow.y - r,
                                    trackWindow.x + r, trackWindow.y + r) &
                                Rect(0, 0, cols, rows);
            }

			if (trackWindow.width == 0 || trackWindow.height == 0)
			{
				trackObject = 0;
			}

			ellipse( m_colorMat, trackBox, Scalar(0,0,255), 3 );

			// MapDepthFrameToColorFrame
			//NUI_DEPTH_IMAGE_PIXEL* pDepthPixel = reinterpret_cast<NUI_DEPTH_IMAGE_PIXEL*>( m_frameHelper.m_pDepthBuffer );
 			//td::vector<NUI_COLOR_IMAGE_POINT> pColorPoint( 640*480 );
			//pCordinateMapper->MapDepthFrameToColorFrame(m_depthResolution, 640*480, pDepthPixel, NUI_IMAGE_TYPE_COLOR, m_colorResolution, 640*480, &pColorPoint[0]);

			if (true)
			{
				// pre cele pole, ak je color pixel v maske a zaroven je v trackBox, tak daj 3D suradnice do buffera
				vector<DEPT_COLOR_PAIR> buffer;
				buffer.clear();

				Point2f trackBoxVertices[4];
				trackBox.points(trackBoxVertices);
				vector<Point2f> trackBoxVec;
				for (int i = 0; i < 4; i++)
				{
					trackBoxVec.push_back(trackBoxVertices[i]);
				}

				// draw trackBox as RotatedRect
				for( int j = 0; j < 4; j++ )
					{ line( m_colorMat, trackBoxVertices[j],  trackBoxVertices[(j+1)%4], Scalar( 255 ), 3, 8 ); }

				auto pColorPoint = m_frameHelper.pDepthToColorPoint;
				NUI_DEPTH_IMAGE_PIXEL* pDepthPixel = reinterpret_cast<NUI_DEPTH_IMAGE_PIXEL*>( m_frameHelper.m_pExtendedDepthBuffer );
				for (unsigned int i = 0; i < 640*480; i++)
				{
					if (pColorPoint[i].x < 0 || pColorPoint[i].x >= 640)
						continue;
					if (pColorPoint[i].y < 0 || pColorPoint[i].y >= 480)
						continue;

					if (fgColor.at<unsigned char>(pColorPoint[i].y, pColorPoint[i].x) == 255)
					{
						if (colorStaticMask.at<unsigned char>(pColorPoint[i].y, pColorPoint[i].x) > 0)
						{
							if (pointPolygonTest(trackBoxVec, Point2f(pColorPoint[i].x, pColorPoint[i].y), false) >= 0)
							{
								if (pDepthPixel[i].depth > 100 && pDepthPixel[i].depth < 10000)
								{
									NUI_DEPTH_IMAGE_POINT dip;
									dip.x = i / 640; // TODO: je to dobre?
									dip.y = i % 640;
									dip.depth = pDepthPixel[i].depth;
									buffer.push_back(DEPT_COLOR_PAIR(dip, pColorPoint[i]));
								}
							}
						}
					}

					if (manualPick)
					{
						if (pColorPoint[i].x == manualPickPoint.x && pColorPoint[i].y == manualPickPoint.y)
						{
							if (pDepthPixel[i].depth > 100 && pDepthPixel[i].depth < 10000)
							{
								manualPickDepthPoint.x = i / 640;
								manualPickDepthPoint.y = i % 640;
								manualPickDepthPoint.depth = pDepthPixel[i].depth;
							}
						}
					}
				}

				// test: prejdi buffer a vykresli bodky do obrazu
				//for (auto buffer_item : buffer)
				//{
				//	circle(m_colorMat, Point(buffer_item.second.x, buffer_item.second.y), 1, Scalar(0,0,255));
				//}

				// prejdi buffer a najdi najmensiu hlbku
				long min_depth = 9999;
				for (auto buffer_item : buffer)
				{
					if (buffer_item.first.depth < min_depth)
					{
						min_depth = buffer_item.first.depth;
					}
				}

				// akceptuj len hlbku do nejakej konstatnty od maximalnej
				long depth_limit = 300;
				//for (auto buffer_item = buffer.begin(); buffer_item < buffer.end();)
				//{
				//	if (buffer_item->first.depth > (min_depth + depth_limit))
				//	{
				//		buffer_item = buffer.erase(buffer_item);
				//	}
				//	else
				//	{
				//		++buffer_item;
				//	}
				//}

				// priemerne suradnice
				Point3f avgDepthPoint(0,0,0);
				int avfDepthPointCount = 0;
				for (auto buffer_item : buffer)
				{
					if (buffer_item.first.depth > (min_depth + depth_limit))
						continue;

					//circle(m_colorMat, Point(buffer_item.second.x, buffer_item.second.y), 1, Scalar(0,0,255));

					avfDepthPointCount++;
					avgDepthPoint.x += buffer_item.first.x;
					avgDepthPoint.y += buffer_item.first.y;
					avgDepthPoint.z += buffer_item.first.depth;
				}
				avgDepthPoint *= 1.0 / avfDepthPointCount;

				// projekcia suradnic do svetovych
				// https://stackoverflow.com/questions/17832238/kinect-intrinsic-parameters-from-field-of-view/18199938#18199938
				float f = 0;
				f = NUI_CAMERA_DEPTH_NOMINAL_FOCAL_LENGTH_IN_PIXELS; // toto asi nie?
				f = 640.0 / (2 * tanf(NUI_CAMERA_DEPTH_NOMINAL_HORIZONTAL_FOV / 2));

				Point3f cameraPt;
				cameraPt.z = avgDepthPoint.z;
				cameraPt.x = cameraPt.z * (avgDepthPoint.x - 320) / f;
				cameraPt.y = cameraPt.z * (avgDepthPoint.y - 240) / f;

				// world calibration (treba pointpicker...)
				vector<Point3f> pts;
				pts.push_back(cameraPt);
				vector<Point3f> dst;
				perspectiveTransform(pts, dst, worldTransformationMatrix);
				Point3f worldPt = dst[0];

				// print camera space coordinates
				//cout << cameraPt << endl;

				// print world space coordinates
				cout << worldPt << endl;

				outStream << to_string(worldPt.x) << ";" << to_string(worldPt.y) << ";" << to_string(worldPt.z) << ";" << to_string(m_frameHelper.m_depthTimestamp.QuadPart / 1000.0) << endl;
				outStream.flush();

				// draw clusters on map
				Mat cluster_img(480, 640, CV_8UC3);
				float cluster_img_scale = 640.0 / 3.0; // 3m = 640px
				cluster_img.setTo(Scalar(0,0,0));
				for (int x = 0; x <= 5 ; x++)
				{
					line(cluster_img, Point(cluster_img_scale*x*0.6, 0), Point(cluster_img_scale*x*0.6, 479), Scalar(255,255,255));
				}
				for (int y = 0; y <= 3 ; y++)
				{
					line(cluster_img, Point(0, 480-cluster_img_scale*y*0.6), Point(639, 480-cluster_img_scale*y*0.6), Scalar(255,255,255));
				}
				line(cluster_img, Point(cluster_img_scale*4.3*0.6, 0), Point(cluster_img_scale*4.3*0.6, 479), Scalar(255,255,255));
				line(cluster_img, Point(0, 480-cluster_img_scale*3.2*0.6), Point(639, 480-cluster_img_scale*3.2*0.6), Scalar(255,255,255));

				circle(cluster_img, Point(cluster_img_scale * (worldPt.x), 480-cluster_img_scale*worldPt.z), 3, Scalar(0,255,0));
				imshow("Clusters", cluster_img);



				if (manualPick)
				{
					Point3f manualPickCameraPt;
					manualPickCameraPt.z = manualPickDepthPoint.depth;
					manualPickCameraPt.x = manualPickCameraPt.z * (manualPickDepthPoint.x - 320) / f;
					manualPickCameraPt.y = manualPickCameraPt.z * (manualPickDepthPoint.y - 240) / f;

					// print camera space coordinates
					cout << "MP: colorPt=" << manualPickPoint << ", cameraPt=" << manualPickCameraPt << endl;					
				}

				

				// generuj subor x(metre desatinne);y;z;timestamp(sekundy desatinne)
			}
		}

		if (haveSomething)
		{
			rectangle(m_colorMat, Point(0,0), Point(200, 20), Scalar(0,0,0), CV_FILLED);
			putText(m_colorMat, to_string(m_frameHelper.m_depthTimestamp.QuadPart), Point(0,15), CV_FONT_HERSHEY_PLAIN, 1.0, Scalar(255,255,255));
			imshow("Color", m_colorMat);
			imshow("Fg mask", fgColor);
			//imshow("Depth", m_depthRgbMat);
			//imshow("Fg mask", fgDepth);
		}


		int key = waitKey(5);
		if(key >= 0)
		{
			break;
		}
	}

	outStream.close();

	destroyWindow("Color");
	destroyWindow("Fg mask");

	m_frameHelper.UnInitialize();
	return 0;
}
