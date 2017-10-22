#include "stdafx.h"
#include "RtspBuilder.h"
#include <boost/asio.hpp>


RtspBuilder::RtspBuilder(std::string address) :
	camIp(address)
{
	cseq = 1;

	rtspVersion = "RTSP/1.0";

	rtspUrl = "rtsp://" + camIp + ":" + std::to_string(rtspPort) + "/video.mjpg";
}

RtspBuilder::~RtspBuilder()
{
}

std::string RtspBuilder::Options()
{
	std::string rtspOPTIONS = "OPTIONS " + rtspUrl + " " + rtspVersion + "\r\n" +
		"CSeq:" + std::to_string(cseq++) + "\r\n" + 
		"User-agent: " + userAppName + "\r\n" +
		"\r\n";

	return rtspOPTIONS;
}

std::string RtspBuilder::Options(const std::string& user_pass)
{
	std::string rtspOPTIONS = "OPTIONS " + rtspUrl + " " + rtspVersion + "\r\n" +
		"CSeq:" + std::to_string(cseq++) + "\r\n";

	rtspOPTIONS+= "Authorization: Basic " + base64_encode( user_pass.c_str(), user_pass.length() ) + "\r\n";

	rtspOPTIONS += "User-agent: "+ userAppName + "\r\n" +
					"\r\n";

	return rtspOPTIONS;
}

int RtspBuilder::ParseResponse(const std::vector<char>& buffer)
{
	if (buffer.size() >= rtspVersion.length() + 4) {
		if (std::equal(buffer.begin(), 
						buffer.begin() + rtspVersion.length(),
						rtspVersion.begin())) {
			std::string code;

			code += buffer[rtspVersion.length() + 1];
			code += buffer[rtspVersion.length() + 2];
			code += buffer[rtspVersion.length() + 3];

			return std::stoi(code);
		}
	}

	return (-1);
}
