#include "stdafx.h"
#include <string>
#include <algorithm>  
#include <iostream>
#include <vector>

#include "Rtsp.h"
#include "RtpFrameReceiver.h"

#include <thread>         // std::this_thread::sleep_for
#include <chrono>         // std::chrono::seconds

using namespace cv;
using namespace std;

boost::asio::io_service service;
boost::asio::ip::tcp::socket rtsp_s(service);

boost::asio::ip::udp::endpoint endp(boost::asio::ip::udp::v4(), 55780);
boost::asio::ip::udp::socket rtp_s(service, endp);

boost::asio::ip::udp::socket rtcp_s(service);

Rtsp rtsp(rtsp_s);
RtpFrameReceiver rtp(rtp_s, rtcp_s);
Mat img;
std::vector<uint8_t> imgbuf;

Frame f;

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
		
		rtp.BindRtp(std::string("127.0.0.1"), rtsp.GetClientRtpPort());
		rtp.BindRtcp(std::string("127.0.0.1"), rtsp.GetClientRtcpPort());

		boost::asio::ip::udp::socket::receive_buffer_size b(50 * 1024);
		rtp_s.set_option(b);

		rtsp.Play();
					
		do
		{
			rtp.ReceiveFrame(rtp_s);

			rtp.GetJpeg(f);

			img = imdecode( f.ToVector(), CV_LOAD_IMAGE_COLOR );

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

