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

uint32_t RtpReceiver::ReceiveFrame(boost::asio::ip::udp::socket& s)
{
	size_t packetCounter = 0;
	size_t packetLength = 0;
	size_t offset = 0;
	size_t offsetToJpegPayload = 0;
	size_t jpegPayloadSize = 0;

	bool isFrameReceive = false;
	bool isCorrectPacket = true;

	boost::asio::ip::udp::endpoint endp;

	while (!isFrameReceive) {
		
		std::cout << "receiving packet #" << std::to_string(++packetCounter) << "...\t";

		try {
			packetLength = s.receive_from(boost::asio::buffer(packet, MAX_PACKED_SIZE), endp);
			std::cout << "[OK]" << std::endl;
		}
		catch (std::exception& e) {
			std::cout << "[FAIL]" << std::endl;
			throw e;
		}

		isCorrectPacket = true;

		header_rtp = reinterpret_cast<rtp_hdr_t*>(packet);
		header_jpeg = reinterpret_cast<jpeghdr*>(&packet[sizeof(rtp_hdr_t)]);

		if ( ((header_rtp->pt << 1) != 0x1A) && ((header_rtp->pt << 1) != 0x9A)) {
			std::cout << "NOT JPEG" << std::to_string(header_jpeg->type) <<  std::endl;
			continue;
		}

		offsetToJpegPayload = sizeof(rtp_hdr_t) + sizeof(jpeghdr);

		offset = static_cast<uint32_t>((header_jpeg->off & 0x0000FF) << 16) | static_cast<uint32_t>(header_jpeg->off & 0x00FF00) | static_cast<uint8_t>(header_jpeg->off >> 16);

		std::cout << "offset:" << std::to_string(offset) << std::endl;

		if (packetCounter == 1) {
			header_qtable = reinterpret_cast<jpeghdr_qtable*>( &packet[offsetToJpegPayload] );
			offsetToJpegPayload += sizeof(jpeghdr_qtable);

			header_qtable->length = static_cast<uint16_t>(header_qtable->length << 8) | static_cast<uint8_t>(header_qtable->length >> 8);

			memcpy(Qtable, &packet[offsetToJpegPayload], header_qtable->length);
			offsetToJpegPayload += header_qtable->length;
		}

		jpegPayloadSize = (packetLength - offsetToJpegPayload);

		if ( offset + jpegPayloadSize >= sizeof(jpeg_body) ) {
			throw std::exception("error: large frame size");
		}

		if (isCorrectPacket) {
			memcpy( &jpeg_body[offset], &packet[offsetToJpegPayload], jpegPayloadSize );
			isFrameReceive = ((header_rtp->pt << 1) == 0x9A);
		}
	}

	std::cout << "Frame complete." << std::endl;

	return (offset + jpegPayloadSize);
}

