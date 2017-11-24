#include "stdafx.h"

#include "IpCam.h"
#include <queue>
#include <mutex>
#include <thread>


using namespace cv;
using namespace std;
using namespace std::chrono;
using namespace boost::asio;


bool flagStop = false;

void CallBackFunc(int event, int x, int y, int flags, void* userdata)
{
	if (event == EVENT_LBUTTONDOWN)
	{
		flagStop = true;
	}
}

io_service service;
IpCam camera(service);
Mat img1;
Mat frame;
Mat fgMaskMOG;
Ptr<BackgroundSubtractor> pMOG;

int main()
{
	namedWindow("cam", CV_WINDOW_AUTOSIZE);
	setMouseCallback("cam", CallBackFunc, NULL);

	namedWindow("1oo", CV_WINDOW_AUTOSIZE);
	
	pMOG = cv::createBackgroundSubtractorMOG2();

	try {
	
		camera.Connect( std::string("192.168.0.102") );
		
		do {
			if ( !camera.GetFramesNo() )
				continue;

			camera.GetFrame(img1);

			pMOG->apply(img1, fgMaskMOG);

			// find the contours
			vector< vector<Point> > contours;
			findContours(fgMaskMOG, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);

			for (auto it = contours.begin(); it != contours.end(); ++it) {
				if (contourArea(*it) > 20) {
					CvRect rect = boundingRect( *it );
					rectangle( img1, rect, Scalar(0, 0, 255) );
				}
			}

			imshow("1oo", img1);
			imshow("cam", fgMaskMOG);

			cv::waitKey(1);
		} while ( !flagStop );

		camera.Disconnect();
	}
	catch (std::exception e) {
		return (-1);
	}

    return 0;
}