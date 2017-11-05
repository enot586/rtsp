#pragma once

#include "rtcp.h"

class RtcpBuilder
{
	source data;

	rtcp_t packet;

	uint32_t Dold;
	uint32_t jitter;
public:
	RtcpBuilder();
	~RtcpBuilder();

	void InitSeq(uint16_t seq);
	void UpdateSeq(uint16_t seq);
	void UpdateReceive();

	void UpdateJitter(uint32_t packetTimestamp, uint32_t receiveTimestamp);

	void BuildRR(uint8_t* buffer, size_t size);
};

