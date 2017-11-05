#include "stdafx.h"
#include "RtcpBuilder.h"
#include <cmath>

RtcpBuilder::RtcpBuilder()
{
	memset( &data, 0x00, sizeof(data) );
	data.probation = 2;

	packet.common.version = 2;
	packet.common.p = 0;
	packet.common.count = 1;
	packet.common.pt = RTCP_RR;
	packet.r.rr.ssrc = 0x3feea501;
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

void RtcpBuilder::BuildRR(uint8_t* buffer, size_t size)
{
	size_t packetSize = sizeof(packet.common) + sizeof(packet.r.rr);

	if (size >= packetSize)
		memcpy(&packet, buffer, packetSize);
}