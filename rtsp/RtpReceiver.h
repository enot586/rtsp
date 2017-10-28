#pragma once
#include <boost\asio.hpp>

class RtpReceiver
{
	boost::asio::ip::udp::socket& rtp_sock;
	boost::asio::ip::udp::socket& rtcp_sock;

public:
	RtpReceiver(boost::asio::ip::udp::socket& rtp_sock_,
				boost::asio::ip::udp::socket& rtcp_sock_);
	~RtpReceiver();

	void BindRtp(std::string& ip, uint16_t port);
	void BindRtcp(std::string& ip, uint16_t port);
};

