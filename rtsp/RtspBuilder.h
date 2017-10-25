#pragma once

#include <vector>
#include <string>


class RtspBuilder
{
	int cseq;
	const int rtspPort = 554;

	std::string rtspUrl;
	std::string rtspVersion;
	std::string camIp;
	const std::string userAppName = "RTSP Example App";

public:
	RtspBuilder(std::string address);
	~RtspBuilder();

	std::string Options();
	std::string Options(const std::string& user_pass);

	std::string Describe(const std::string& user_pass);
	std::string Setup(const std::string& user_pass, const std::string& threadName, uint16_t rtpPort, uint16_t rtcpPort);

	int ParseResponse(const std::vector<char>& buffer);

};

