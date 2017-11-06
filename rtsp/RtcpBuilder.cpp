#include "stdafx.h"
#include "RtcpBuilder.h"
#include <cmath>
#include <boost/endian/conversion.hpp>

RtcpBuilder::RtcpBuilder()
{
	ClearData();
}

RtcpBuilder::~RtcpBuilder()
{

}

void RtcpBuilder::InitSeq(uint16_t seq)
{
	init_seq(&data, seq);
}

void RtcpBuilder::UpdateSeq(uint16_t seq)
{
	update_seq(&data, seq);
}

void RtcpBuilder::UpdateReceive()
{
	++data.received;
}

void RtcpBuilder::UpdateJitter(uint32_t packetTimestamp, uint32_t receiveTimestamp)
{
	uint32_t D = std::labs( (receiveTimestamp - packetTimestamp) - Dold );

	Dold = std::labs( receiveTimestamp - packetTimestamp );

	packet.r.rr.rr[0].jitter = packet.r.rr.rr[0].jitter + ( (D - packet.r.rr.rr[0].jitter) >> 4 );
}

size_t RtcpBuilder::BuildRR(uint8_t* buffer, size_t size)
{
	size_t packetSize = sizeof(packet.common) + sizeof(packet.r.rr);

	if (size >= packetSize)
	{
		memcpy(buffer, &packet, packetSize);
		return packetSize;
	}

	return 0;
}

void RtcpBuilder::ClearData()
{
	size_t packetSize = sizeof(packet.common) + sizeof(packet.r.rr);

	memset( &data, 0x00, sizeof(data) );
	data.probation = 2;

	packet.common.version = 2;
	packet.common.p = 0;
	packet.common.count = 1;
	packet.common.pt = RTCP_RR;
	packet.common.length = boost::endian::native_to_big( static_cast<uint16_t>(packetSize/sizeof(uint32_t)-1) );
	packet.r.rr.ssrc = boost::endian::native_to_big(0x3feea501);
}