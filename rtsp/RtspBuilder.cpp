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
	std::string request = "OPTIONS " + rtspUrl + " " + rtspVersion + "\r\n" +
		"CSeq:" + std::to_string(cseq++) + "\r\n" + 
		"User-agent: " + userAppName + "\r\n" +
		"\r\n";

	return request;
}

std::string RtspBuilder::Options(const std::string& user_pass)
{
	std::string request = "OPTIONS " + rtspUrl + " " + rtspVersion + "\r\n" +
		"CSeq:" + std::to_string(cseq++) + "\r\n";

	request+= "Authorization: Basic " + base64_encode( user_pass.c_str(), user_pass.length() ) + "\r\n";

	request += "User-agent: "+ userAppName + "\r\n" +
					"\r\n";

	return request;
}

std::string RtspBuilder::Describe(const std::string& user_pass)
{
	std::string request = "DESCRIBE " + rtspUrl + " " + rtspVersion + "\r\n" +
		"CSeq:" + std::to_string(cseq++) + "\r\n" +
		"Authorization: Basic " + base64_encode( user_pass.c_str(), user_pass.length() ) + "\r\n" +
		"User-agent: " + userAppName + "\r\n" +
		"Accept: application/sdp\r\n" +
		"\r\n";

	return request;
}

std::string RtspBuilder::Setup(const std::string& user_pass, const std::string& threadName, uint16_t rtpPort, uint16_t rtcpPort)
{
	std::string request = "SETUP " + rtspUrl + "/" + threadName + rtspVersion + "\r\n" +
		"CSeq:" + std::to_string(cseq++) + "\r\n" +
		"Authorization: Basic " + base64_encode(user_pass.c_str(), user_pass.length()) + "\r\n" +
		"User-agent: " + userAppName + "\r\n" +
		"Transport: RTP/AVP;unicast;client_port=" + std::to_string(rtpPort) + "-" + std::to_string(rtcpPort) + "\r\n" +
		"\r\n";

	return request;
}

std::string RtspBuilder::Play(const std::string& user_pass, std::string& sessionId)
{
	std::string request = "PLAY " + rtspUrl + " " +  rtspVersion + "\r\n" +
		"CSeq:" + std::to_string(cseq++) + "\r\n" +
		"Authorization: Basic " + base64_encode( user_pass.c_str(), user_pass.length() ) + "\r\n" +
		"User-agent: " + userAppName + "\r\n" +
		"Session: " + sessionId + "\r\n" +
		"Range: npt=0.000-" + "\r\n" +
		"\r\n";

	return request;
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
