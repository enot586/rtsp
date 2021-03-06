#include "stdafx.h"
#include "RtpFrameReceiver.h"
#include <algorithm>  
#include <iostream>
#include <limits>
#include "rtp.h"
#include <boost/endian/conversion.hpp>
#include "rtp_jpeg.h"

RtpFrameReceiver::RtpFrameReceiver(boost::asio::ip::udp::socket& rtp_sock_,
									RtcpBuilder& rtcp_) :
	rtp_sock(rtp_sock_), rtcp(rtcp_),
	header_rtp(nullptr), header_jpeg(nullptr), header_qtable(nullptr),
	jpegFileHeaderSize(0), jpegFileBodySize(0), initTime(std::chrono::system_clock::now())
{
	//initTime = std::chrono::system_clock::now();
	
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

	std::chrono::system_clock::time_point receiveTimestamp;
	
	do  {
		try {
			packetLength = s.receive_from(boost::asio::buffer(packet, MAX_PACKED_SIZE), endp);
			receiveTimestamp = std::chrono::system_clock::now();
			//std::cout << "[OK]" << std::endl;
		}
		catch (std::exception& e) {
			//std::cout << "[FAIL]" << std::endl;
			throw e;
		}

		header_rtp = reinterpret_cast<rtp_hdr_t*>(packet);
		header_jpeg = reinterpret_cast<jpeghdr*>( &packet[sizeof(rtp_hdr_t)] );

		offsetToJpegPayload = sizeof(rtp_hdr_t) + sizeof(jpeghdr);

		//(>> 8) beacause header_jpeg->off is 24bit value and casts to uint32_t
		offset = boost::endian::big_to_native(header_jpeg->off) >> 8;
		//std::cout << "offset:" << std::to_string(offset) << std::endl;
		//std::cout << "seq:" << std::to_string(seq) << std::endl;

		if (packetCounter == 0) {
			isFrameReceive = false;

			//search the first packet in the frame
			if (header_jpeg->off != 0)
				continue;
			
			//Is qtable followed after header_jpeg?
			if (header_jpeg->q > 127) {
				header_qtable = reinterpret_cast<jpeghdr_qtable*>(&packet[offsetToJpegPayload]);
				offsetToJpegPayload += sizeof(jpeghdr_qtable);

				header_qtable->length = boost::endian::big_to_native(header_qtable->length);

				if (header_qtable->length > 128)
					continue;

				memcpy(Qtable, &packet[offsetToJpegPayload], header_qtable->length);
				offsetToJpegPayload += header_qtable->length;
			}

			firstPacketInFrame = boost::endian::big_to_native( static_cast<uint16_t>(header_rtp->seq) );

			//Generate Jpeg header
			jpegFileHeaderSize = MakeHeaders(jpeg_body,
												header_jpeg->type, header_jpeg->width, header_jpeg->height,
												Qtable, &Qtable[64], 0);

			currentTimestamp = boost::endian::big_to_native(header_rtp->ts);
		}
		else if ( currentTimestamp != boost::endian::big_to_native(header_rtp->ts) ) {
			packetCounter = 0;
			//Drop if this block placed in other frame
			//std::cout << "sequence dropped";
			continue;
		}
		
		//Drop if not JPEG frame
		if ( ((header_rtp->pt << 1) != 0x1A) && ((header_rtp->pt << 1) != 0x9A) ) {
			//std::cout << "NOT JPEG" << std::to_string(header_jpeg->type) <<  std::endl;
			continue;
		}

		std::chrono::milliseconds ms =
			std::chrono::duration_cast<std::chrono::milliseconds>(receiveTimestamp - initTime);

		rtcp.UpdateSeq(header_rtp->seq);
		rtcp.UpdateJitter( header_rtp->ts, static_cast<uint32_t>(ms.count())*90 ); // (*90) - transform milliseconds to 90000Hz intervals

		++packetCounter;
		//std::cout << "receive packet #" << std::to_string(packetCounter) << std::endl;
		
		jpegPayloadSize = (packetLength - offsetToJpegPayload);

		if ( (offset + jpegPayloadSize) >= sizeof(jpeg_body) ) {
			throw std::exception("error: Overflow frame buffer");
		}

		memcpy( &jpeg_body[jpegFileHeaderSize+offset], &packet[offsetToJpegPayload], jpegPayloadSize );
		jpegFileBodySize += jpegPayloadSize;

		//Check EOF marker and test for correct packet numbers
		if ( (header_rtp->pt << 1) == 0x9A ) {
			lastPacketInFrame = boost::endian::big_to_native( static_cast<uint16_t>(header_rtp->seq) );

			uint16_t totalPackets = 0;

			if (lastPacketInFrame >= firstPacketInFrame)
			{
				totalPackets = (lastPacketInFrame - firstPacketInFrame) + 1;
			}
			else
			{
				totalPackets = ((std::numeric_limits<uint16_t>::max() - firstPacketInFrame) + 1) + lastPacketInFrame;
			}
			 
			isFrameReceive = (totalPackets == packetCounter);
			packetCounter = 0;
		}

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

uint32_t RtpFrameReceiver::GetSsrc()
{
	return boost::endian::big_to_native(header_rtp->ssrc);
}