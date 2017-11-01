#include "stdafx.h"
#include <string>
#include <algorithm>  
#include <iostream>
#include <vector>
#include <boost\asio.hpp>
#include "Rtsp.h"
#include "RtpFrameReceiver.h"

boost::asio::io_service service;
boost::asio::ip::tcp::socket rtsp_s(service);

boost::asio::ip::udp::endpoint endp(boost::asio::ip::udp::v4(), 55780);
boost::asio::ip::udp::socket rtp_s(service, endp);

boost::asio::ip::udp::socket rtcp_s(service);

Rtsp rtsp(rtsp_s);
RtpFrameReceiver rtp(rtp_s, rtcp_s);

int main()
{
	try {
		rtsp.Connect( std::string("192.168.0.102") );
		
		rtp.BindRtp(std::string("127.0.0.1"), rtsp.GetClientRtpPort());
		rtp.BindRtcp(std::string("127.0.0.1"), rtsp.GetClientRtcpPort());

		rtsp.Play();

		
		rtp.ReceiveFrame(rtp_s);

		rtsp.Teardown();
		Frame* f = rtp.GetJpeg();

		f->ToFile();

		delete f;
		
	}
	catch (std::exception e) {
		return (-1);
	}

	std::cout << "The end" << std::endl;
	int a;
	std::cin >> a;
    return 0;
}

