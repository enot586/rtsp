#include "stdafx.h"
#include "RtpReceiver.h"
#include <algorithm>  
#include <iostream>
#include "rtp.h"


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
	//boost::asio::ip::udp::endpoint rtp_ep(boost::asio::ip::address::from_string(ip), port);
	//rtp_sock.open(boost::asio::ip::udp::v4());
	//rtp_sock.bind(rtp_ep);
}

void RtpReceiver::BindRtcp(std::string& ip, uint16_t port)
{
	//boost::asio::ip::udp::endpoint rtcp_ep(boost::asio::ip::address::from_string(ip), port);
	//rtcp_sock.open(boost::asio::ip::udp::v4());
	//rtcp_sock.bind(rtcp_ep);
}

void RtpReceiver::ReceivePacket(boost::asio::ip::udp::socket& s)
{
	std::cout << "receiving header rtp packet...\t";
	
	size_t packetLength = 0;
	boost::asio::ip::udp::endpoint endp;

	try {
		packetLength = s.receive_from(boost::asio::buffer(packet, MAX_PACKED_SIZE), endp);
		std::cout << "[OK]" << std::endl;
	}
	catch (std::exception& e)
	{
		std::cout << "[FAIL]" << std::endl;
	}
	
	header_rtp = reinterpret_cast<rtp_hdr_t*>(packet);
	header_jpeg = reinterpret_cast<jpeghdr*>( &packet[sizeof(rtp_hdr_t)] );

	return;
}

