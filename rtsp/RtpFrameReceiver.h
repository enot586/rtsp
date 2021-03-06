#pragma once
#include <boost\asio.hpp>
#include "rtp.h"
#include "rtp_jpeg.h"
#include <memory>
#include "RtcpBuilder.h"

class RtpFrameReceiver
{
	boost::asio::ip::udp::socket& rtp_sock;

	RtcpBuilder& rtcp;

	static const size_t MAX_PACKED_SIZE = 100 * 1024;
	uint8_t packet[MAX_PACKED_SIZE];

	static const size_t MAX_JPEG_SIZE = 1024 * 1024;
	uint8_t jpeg_body[MAX_JPEG_SIZE];

	static const size_t MAX_QTALBE_SIZE = 128;
	uint8_t Qtable[MAX_QTALBE_SIZE];

	rtp_hdr_t* header_rtp;
	jpeghdr* header_jpeg;
	jpeghdr_qtable* header_qtable;

	size_t jpegFileHeaderSize;
	size_t jpegFileBodySize;

	uint16_t firstPacketInFrame;
	uint16_t lastPacketInFrame;
	std::chrono::system_clock::time_point initTime;

public:
	RtpFrameReceiver(boost::asio::ip::udp::socket& rtp_sock_,
						RtcpBuilder& rtcp_);
	~RtpFrameReceiver();

	void BindRtp(uint16_t port);

	void ReceiveFrame(boost::asio::ip::udp::socket& s);
	void GetJpeg(std::vector<uint8_t>& v);

	uint32_t RtpFrameReceiver::GetSsrc();
};

