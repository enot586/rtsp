#include "stdafx.h"

#include "Rtsp.h"
#include "RtpFrameReceiver.h"
#include "RtcpBuilder.h"

using namespace cv;
using namespace std;

boost::asio::io_service service;

boost::asio::ip::tcp::socket rtsp_s(service);
boost::asio::ip::udp::socket rtp_s(service);
boost::asio::ip::udp::socket rtcp_s(service);

Rtsp rtsp(rtsp_s);
RtpFrameReceiver rtp(rtp_s, rtcp_s);
RtcpBuilder rtcp;

Mat img;
std::vector<uint8_t> imgbuf;

bool flagStop = false;

void CallBackFunc(int event, int x, int y, int flags, void* userdata)
{
	if (event == EVENT_LBUTTONDOWN)
	{
		flagStop = true;
	}
	//else if (event == EVENT_RBUTTONDOWN)
	//{
	//	cout << "Right button of the mouse is clicked - position (" << x << ", " << y << ")" << endl;
	//}
	//else if (event == EVENT_MBUTTONDOWN)
	//{
	//	cout << "Middle button of the mouse is clicked - position (" << x << ", " << y << ")" << endl;
	//}
	//else if (event == EVENT_MOUSEMOVE)
	//{
	//	cout << "Mouse move over the window - position (" << x << ", " << y << ")" << endl;
	//}
}

int main()
{
	namedWindow("cam", CV_WINDOW_AUTOSIZE);
	setMouseCallback("cam", CallBackFunc, NULL);

	try {
		rtsp.Connect( std::string("192.168.0.102") );
		
		rtp.BindRtp( rtsp.GetClientRtpPort() );
		rtp.BindRtcp( rtsp.GetClientRtcpPort() );

		rtsp.Play();
				
		std::chrono::system_clock::time_point receiveTimestamp;

		do
		{
			rtp.ReceiveFrame(rtp_s);
			receiveTimestamp = std::chrono::system_clock::now();

			rtp.GetJpeg(imgbuf);

			img = imdecode(imgbuf, CV_LOAD_IMAGE_COLOR );

			imshow("cam", img);
			cv::waitKey(5);
		} while ( !flagStop );
		
		rtsp.Teardown();
	}
	catch (std::exception e) {
		return (-1);
	}

    return 0;
}

