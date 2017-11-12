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
Mat img;

int main()
{
	namedWindow("cam", CV_WINDOW_AUTOSIZE);
	setMouseCallback("cam", CallBackFunc, NULL);

	try {
	
		camera.Connect( std::string("192.168.0.102") );
		
		do
		{
			camera.GetFrame(img);

			if (img.data != NULL)
				imshow("cam", img);

			cv::waitKey(1);
		} while ( !flagStop );

		camera.Disconnect();
	}
	catch (std::exception e) {
		return (-1);
	}

    return 0;
}

