#include "stdafx.h"
#include "RtpReceiver.h"


RtpReceiver::RtpReceiver(boost::asio::ip::udp::socket& rtp_sock_,
						 boost::asio::ip::udp::socket& rtcp_sock_) :
	rtp_sock(rtp_sock_), rtcp_sock(rtcp_sock_)
{
}


RtpReceiver::~RtpReceiver()
{
}


void RtpReceiver::BindRtp(std::string& ip, uint16_t port)
{
	boost::asio::ip::udp::endpoint rtp_ep(boost::asio::ip::address::from_string(ip), port);
	rtp_sock.open(boost::asio::ip::udp::v4());
	rtp_sock.bind(rtp_ep);
}

void RtpReceiver::BindRtcp(std::string& ip, uint16_t port)
{
	boost::asio::ip::udp::endpoint rtcp_ep(boost::asio::ip::address::from_string(ip), port);
	rtcp_sock.open(boost::asio::ip::udp::v4());
	rtcp_sock.bind(rtcp_ep);
}