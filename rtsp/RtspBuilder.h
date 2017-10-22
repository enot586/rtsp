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

	int ParseResponse(const std::vector<char>& buffer);

};

