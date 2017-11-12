#include "stdafx.h"
#include "IpCam.h"
#include <thread>
#include <functional>

using namespace cv;
using namespace std;
using namespace std::chrono;
using namespace boost::asio;

IpCam::IpCam(io_service& service_) :
	service(service_),
	rtsp_s(service_),
	rtp_s(service_),
	rtcp_s(service_),

	rtsp(rtsp_s),
	rtp(rtp_s, rtcp_s, rtcp),
	receiver(nullptr),
	rtcpHandler(nullptr),

	flagStop(false)
{

}

IpCam::~IpCam()
{
	try {
		Disconnect();
	}
	catch (std::exception& e) {
		std::cout << e.what() << std::endl;
	}
}

void IpCam::Receiver()
{
	try	{
		std::vector<uint8_t> imgbuf;
		Mat img;

		while (!flagStop) {
			rtp.ReceiveFrame(rtp_s);
			rtp.GetJpeg(imgbuf);

			img = imdecode(imgbuf, CV_LOAD_IMAGE_COLOR);
			if (img.data != NULL) {

				if (frames.size() >= MAX_QUEUE_LENGTH) {
					frames.pop();
				}

				frames.push(img);
			}

			if ( (frames.size() > 1) && !rtcp.GetCamSsrc() ) {
				rtcp.SetCamSsrc( rtp.GetSsrc() );
			}
		}
	}
	catch (std::exception& e) {
		std::cout << " IpCam::Receiver():" << std::endl;
		std::cout << e.what() << std::endl;
	}
}

void IpCam::RtcpHandler()
{
	do
	{
		ip::udp::endpoint rtcp_ep( ip::address::from_string(cam),
								   rtsp.GetCameraRtcpPort() );

		ip::udp::socket rtcp_s(service);

		uint8_t packet_buffer[128];
		size_t packetSize = rtcp.BuildRR( packet_buffer, sizeof(packet_buffer) );

		rtcp_s.open( ip::udp::v4() );

		if ( !( packetSize == rtcp_s.send_to( buffer(packet_buffer, packetSize), rtcp_ep) ) ) {
			std::cout << "RR FAIL" << std::endl;
		}

		std::this_thread::sleep_for( std::chrono::seconds(10) );
	} while (!flagStop);
}

void IpCam::Connect(std::string& address)
{
	cam = address;
	try {
		rtsp.Connect(cam);
		rtcp.ClearData();

		rtp.BindRtp( rtsp.GetClientRtpPort() );

		receiver.reset( new std::thread( std::bind(&IpCam::Receiver, this) ) );

		rtsp.Play();

		rtcpHandler.reset (new std::thread( std::bind(&IpCam::RtcpHandler, this) ) );
	}
	catch (std::exception e) {
		std::cout << e.what() << std::endl;
		throw e;
	}
}

void IpCam::Disconnect()
{
	try {
		rtsp.Teardown();
	}
	catch (std::exception& e) {
		std::cout << e.what() << std::endl;
	}
	
	flagStop = true;

	rtp_s.close();
	rtsp_s.close();
	rtcp_s.close();

	if (receiver.get() != nullptr) receiver->join();
	if (rtcpHandler.get() != nullptr) rtcpHandler->join();
}

void IpCam::GetFrame(cv::Mat& s)
{
	if ( frames.empty() )
		return;

	s = frames.front();
	frames.pop();
}