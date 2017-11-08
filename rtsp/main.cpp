#include "stdafx.h"

#include "Rtsp.h"
#include "RtpFrameReceiver.h"
#include "RtcpBuilder.h"
#include <queue>
#include <mutex>
#include <thread>


using namespace cv;
using namespace std;
using namespace std::chrono;
using namespace boost::asio;

io_service service;

ip::tcp::socket rtsp_s(service);
ip::udp::socket rtp_s(service);
ip::udp::socket rtcp_s(service);

RtcpBuilder rtcp;
Rtsp rtsp(rtsp_s);
RtpFrameReceiver rtp(rtp_s, rtcp_s, rtcp);

std::chrono::system_clock::time_point t1;

std::queue<Mat> frames;

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


std::mutex rtp_socket;

void Receiver()
{
	try
	{
		std::vector<uint8_t> imgbuf;
		Mat img;

		while (!flagStop) {
			//std::lock(rtp_socket);
			rtp.ReceiveFrame(rtp_s);
			rtp.GetJpeg(imgbuf);
			img = imdecode(imgbuf, CV_LOAD_IMAGE_COLOR);
			frames.push(img);
			//rtp_socket.unlock();

			if ( (frames.size() > 1) && !rtcp.GetCamSsrc() ) {
				rtcp.SetCamSsrc( rtp.GetSsrc() );
			}
		}
	}
	catch (std::exception& e) {

	}
}

std::unique_ptr<std::thread> receiver;

std::string cam = "192.168.0.102";

int main()
{
	namedWindow("cam", CV_WINDOW_AUTOSIZE);
	setMouseCallback("cam", CallBackFunc, NULL);

	try {
		rtsp.Connect(cam);
		rtcp.ClearData();

		rtp.BindRtp( rtsp.GetClientRtpPort() );

		receiver.reset( new std::thread(Receiver) );

		rtsp.Play();
				
		t1 = std::chrono::system_clock::now();

		bool isRtcpTimeout = false;

		do
		{
			isRtcpTimeout = duration_cast<seconds>(std::chrono::system_clock::now() - t1) >= std::chrono::seconds(10);

			if ( isRtcpTimeout && rtcp.GetCamSsrc() ) {
				t1 = std::chrono::system_clock::now();

				boost::asio::ip::udp::endpoint rtcp_ep( boost::asio::ip::address::from_string(cam), 
														rtsp.GetCameraRtcpPort() );

				boost::asio::ip::udp::socket rtcp_s(service);

				uint8_t packet_buffer[128];
				size_t packetSize = rtcp.BuildRR(packet_buffer, sizeof(packet_buffer) );
				
				rtcp_s.open( boost::asio::ip::udp::v4() );

				if ( packetSize == rtcp_s.send_to(boost::asio::buffer(packet_buffer, packetSize), rtcp_ep) ) {
					std::cout << "RR SEND" << std::endl;
				}
				else {
					std::cout << "RR FAIL" << std::endl;
				}
			}

			Mat img;

			while ( !frames.empty() ) {
				img = frames.front();
				imshow("cam", img);
				frames.pop();
			}

			cv::waitKey(5);
		} while ( !flagStop );
		
		receiver->join();

		rtsp.Teardown();
	}
	catch (std::exception e) {
		return (-1);
	}

    return 0;
}

