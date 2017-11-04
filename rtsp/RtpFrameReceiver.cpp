#include "stdafx.h"
#include "RtpFrameReceiver.h"
#include <algorithm>  
#include <iostream>
#include "rtp.h"
#include <boost/endian/conversion.hpp>
#include "rtp_jpeg.h"

RtpFrameReceiver::RtpFrameReceiver(boost::asio::ip::udp::socket& rtp_sock_,
									boost::asio::ip::udp::socket& rtcp_sock_) :
	rtp_sock(rtp_sock_), rtcp_sock(rtcp_sock_), 
	header_rtp(nullptr), header_jpeg(nullptr), header_qtable(nullptr),
	jpegFileHeaderSize(0), jpegFileBodySize(0)
{


}


RtpFrameReceiver::~RtpFrameReceiver()
{
}


void RtpFrameReceiver::BindRtp(uint16_t port)
{
	boost::asio::ip::udp::endpoint rtp_ep(boost::asio::ip::udp::v4(), port);
	rtp_sock.open(boost::asio::ip::udp::v4());
	rtp_sock.bind(rtp_ep);

	boost::asio::ip::udp::socket::receive_buffer_size b(50 * 1024);
	rtp_sock.set_option(b);
}

void RtpFrameReceiver::BindRtcp( uint16_t port)
{
	boost::asio::ip::udp::endpoint rtcp_ep(boost::asio::ip::udp::v4(), port);
	rtcp_sock.open(boost::asio::ip::udp::v4());
	rtcp_sock.bind(rtcp_ep);
}

void RtpFrameReceiver::ReceiveFrame(boost::asio::ip::udp::socket& s)
{
	size_t packetCounter = 0;
	size_t packetLength = 0;
	size_t offset = 0;
	size_t offsetToJpegPayload = 0;
	size_t jpegPayloadSize = 0;
	size_t currentTimestamp = 0;

	bool isFrameReceive = false;

	boost::asio::ip::udp::endpoint endp;

	jpegFileBodySize = 0;
	jpegFileHeaderSize = 0;
	
	do  {
		try {
			packetLength = s.receive_from(boost::asio::buffer(packet, MAX_PACKED_SIZE), endp);
			//std::cout << "[OK]" << std::endl;
		}
		catch (std::exception& e) {
			//std::cout << "[FAIL]" << std::endl;
			throw e;
		}

		header_rtp = reinterpret_cast<rtp_hdr_t*>(packet);
		header_jpeg = reinterpret_cast<jpeghdr*>( &packet[sizeof(rtp_hdr_t)] );

		uint16_t seq = boost::endian::big_to_native( static_cast<uint16_t>(header_rtp->seq) );

		offsetToJpegPayload = sizeof(rtp_hdr_t) + sizeof(jpeghdr);

		//(>> 8) beacause header_jpeg->off is 24bit value and casts to uint32_t
		offset = boost::endian::big_to_native(header_jpeg->off) >> 8;
		//std::cout << "offset:" << std::to_string(offset) << std::endl;
		//std::cout << "seq:" << std::to_string(seq) << std::endl;

		if (packetCounter == 0) {
			//search the first packet in the frame
			if (header_jpeg->off != 0)
				continue;
			
			//Is qtable followed after header_jpeg?
			if (header_jpeg->q > 127) {
				header_qtable = reinterpret_cast<jpeghdr_qtable*>(&packet[offsetToJpegPayload]);
				offsetToJpegPayload += sizeof(jpeghdr_qtable);

				header_qtable->length = boost::endian::big_to_native(header_qtable->length);
				memcpy(Qtable, &packet[offsetToJpegPayload], header_qtable->length);
				offsetToJpegPayload += header_qtable->length;

				//Generate Jpeg header
				jpegFileHeaderSize = MakeHeaders(jpeg_body,
										header_jpeg->type, header_jpeg->width, header_jpeg->height,
										Qtable, &Qtable[64], 0);
			}

			currentTimestamp = boost::endian::big_to_native(header_rtp->ts);
		}
		else if ( currentTimestamp != boost::endian::big_to_native(header_rtp->ts) ) {
			packetCounter = 0;
			//Drop if this block placed in other frame
			//std::cout << "sequence dropped";
			continue;
		}

		//Drop if not JPEG frame
		if ( ((header_rtp->pt << 1) != 0x1A) && ((header_rtp->pt << 1) != 0x9A)) {
			//std::cout << "NOT JPEG" << std::to_string(header_jpeg->type) <<  std::endl;
			continue;
		}
		++packetCounter;
		//std::cout << "receive packet #" << std::to_string(packetCounter) << std::endl;
		
		jpegPayloadSize = (packetLength - offsetToJpegPayload);

		if ( (offset + jpegPayloadSize) >= sizeof(jpeg_body) ) {
			throw std::exception("error: Overflow frame buffer");
		}

		memcpy( &jpeg_body[jpegFileHeaderSize+offset], &packet[offsetToJpegPayload], jpegPayloadSize );
		jpegFileBodySize += jpegPayloadSize;

		isFrameReceive = ( (header_rtp->pt << 1) == 0x9A );
	} while (!isFrameReceive);

	//std::cout << "Frame complete." << std::endl;
}

void RtpFrameReceiver::GetJpeg(std::vector<uint8_t>& v)
{
	size_t totalJpegSize = jpegFileHeaderSize + jpegFileBodySize;

	if (v.size() < totalJpegSize)
		v.resize(totalJpegSize);

	for (size_t i = 0; i < totalJpegSize; ++i)
		v[i] = jpeg_body[i];
}

