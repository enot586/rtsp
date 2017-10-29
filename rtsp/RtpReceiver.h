#pragma once
#include <boost\asio.hpp>
#include "rtp.h"
#include "rtp_jpeg.h"

class RtpReceiver
{
	boost::asio::ip::udp::socket& rtp_sock;
	boost::asio::ip::udp::socket& rtcp_sock;

	static const size_t MAX_PACKED_SIZE = 2 * 1024;
	uint8_t packet[MAX_PACKED_SIZE];

	static const size_t MAX_JPEG_SIZE = 1024 * 1024;
	uint8_t jpeg_body[MAX_JPEG_SIZE];

	static const size_t MAX_QTALBE_SIZE = 128;
	uint8_t Qtable[MAX_QTALBE_SIZE];

	rtp_hdr_t* header_rtp;
	jpeghdr* header_jpeg;
	jpeghdr_qtable* header_qtable;

public:
	RtpReceiver(boost::asio::ip::udp::socket& rtp_sock_,
				boost::asio::ip::udp::socket& rtcp_sock_);
	~RtpReceiver();

	void BindRtp(std::string& ip, uint16_t port);
	void BindRtcp(std::string& ip, uint16_t port);

//private:
	uint32_t ReceiveFrame(boost::asio::ip::udp::socket& s);
};

