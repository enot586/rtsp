#pragma once

#include "stdafx.h"
#include <string>
#include <algorithm>  
#include <iostream>
#include <vector>
#include <boost\asio.hpp>
#include <memory>
#include "RtspHeader.h"
#include "RtspBuilder.h"

class Rtsp
{
	boost::asio::ip::tcp::socket& sock;

	const uint16_t RTP_PORT_DEFAULT = 55780;
	const uint16_t RTCP_PORT_DEFAULT = 55781;

	std::pair<uint16_t, uint16_t> cp;
	std::pair<uint16_t, uint16_t> sp;

	std::string addr;
	std::string sessionId;

	std::unique_ptr<RtspBuilder> rtspRequest;

public:
	Rtsp(boost::asio::ip::tcp::socket& sock_);
	~Rtsp();

	void Connect(std::string& address);
	void Play();
	void Teardown();

	uint16_t Rtsp::GetClientRtpPort()
	{
		return cp.first;
	}

	uint16_t Rtsp::GetClientRtcpPort()
	{
		return cp.second;
	}

	uint16_t Rtsp::GetServerRtpPort()
	{
		return sp.first;
	}

	uint16_t Rtsp::GetServerRtcpPort()
	{
		return sp.second;
	}
private:
	uint32_t ReceiveUntil(boost::asio::ip::tcp::socket& s, std::vector<char>& tp, std::vector<char>& responseBuffer);
	RtspHeader ReceiveHeader(boost::asio::ip::tcp::socket& s);
	std::string GetSdpAttributeValue(std::string attr, std::vector<char>& source);
	
};

