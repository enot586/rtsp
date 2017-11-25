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

void CallbackFunc(int event, int x, int y, int flags, void* userdata)
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

vector < vector<Point> > securityContour(1);

int main()
{
	namedWindow("cam", CV_WINDOW_AUTOSIZE);
	
	namedWindow("1oo", CV_WINDOW_AUTOSIZE);
	setMouseCallback("1oo", CallbackFunc, NULL);

	pMOG = cv::createBackgroundSubtractorMOG2();
	
	securityContour[0].resize(5);
	securityContour[0].at(0) = Point(250, 150);
	securityContour[0].at(1) = Point(350, 150);
	securityContour[0].at(2) = Point(350, 250);
	securityContour[0].at(3) = Point(250, 250);
	securityContour[0].at(4) = Point(150, 225);

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

			Scalar color = Scalar(0, 255, 0);

			for (auto it = contours.begin(); it != contours.end(); ++it) {
				if (contourArea(*it) > 50) {
					CvRect rect = boundingRect(*it);
					rectangle( img1, rect, Scalar(0, 0, 255) );

					vector<Point> convexContour;
					convexHull(*it, convexContour);

					vector<Point> intersect;

					if (intersectConvexConvex(securityContour[0], convexContour, intersect)) {
						color = Scalar(0, 255, 255);
					}
				}
			}


			drawContours(img1, securityContour, 0, color);

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