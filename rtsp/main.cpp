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
Mat img2;
Mat result;

int main()
{
	namedWindow("cam", CV_WINDOW_AUTOSIZE);
	setMouseCallback("cam", CallBackFunc, NULL);

	namedWindow("1oo", CV_WINDOW_AUTOSIZE);
	namedWindow("2oo", CV_WINDOW_AUTOSIZE);
	
	try {
	
		camera.Connect( std::string("192.168.0.102") );
		
		do {
			if (camera.GetFramesNo() < 2)
				continue;

			camera.GetFrame(img1);
			camera.GetFrame(img2);

			if ( (img1.data != NULL) && (img2.data != NULL) ) {
				result = img2 - img1;

				imshow("1oo", img1);
				imshow("2oo", img2);
				imshow("cam", result);
			}

			cv::waitKey(1);
		} while ( !flagStop );

		camera.Disconnect();
	}
	catch (std::exception e) {
		return (-1);
	}

    return 0;
}

