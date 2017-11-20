#pragma once

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

class IpCam
{
	static const uint8_t MAX_QUEUE_LENGTH = 200;

	io_service& service;

	std::string cam;

	ip::tcp::socket rtsp_s;
	ip::udp::socket rtp_s;

	RtcpBuilder rtcp;
	Rtsp rtsp;
	RtpFrameReceiver rtp;

	std::queue<Mat> frames;
	std::unique_ptr<std::thread> receiver;
	std::unique_ptr<std::thread> rtcpHandler;
	
	bool flagStop;

public:
	IpCam(io_service& service_);
	~IpCam();

	void Connect(std::string& address);
	void Disconnect();

	size_t GetFramesNo();
	bool GetFrame(cv::Mat& s);
	
private:
	void RtcpHandler();
	void Receiver();

};

