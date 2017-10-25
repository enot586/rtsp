#include "stdafx.h"
#include <algorithm>  
#include "RtspHeader.h"


RtspHeader::RtspHeader(std::vector<char>& responseBuffer) :
	header(responseBuffer)
{

}

RtspHeader::~RtspHeader()
{
}

uint32_t RtspHeader::GetCode()
{
	std::string rtspVersion = "RTSP/1.0";

	if (header.size() >= rtspVersion.length() + 4) {
		if (std::equal(header.begin(),
			header.begin() + rtspVersion.length(),
			rtspVersion.begin())) {
			std::string code;

			code += header[rtspVersion.length() + 1];
			code += header[rtspVersion.length() + 2];
			code += header[rtspVersion.length() + 3];

			return std::stoi(code);
		}
	}

	return (-1);
}

uint32_t RtspHeader::GetParamValue(std::string& paramName)
{
	std::vector<char>::iterator it = std::search(header.begin(), header.end(),
		paramName.begin(), paramName.end());

	if (it == header.end())
		return 0;

	it += paramName.length()+1;

	std::string length;

	while ( ( it != header.end() ) && ( (*it != '\r') && ( *(it + 1) != '\n') ) ) {
		length += *it++;
	}

	return std::stoi(length);
}

std::string RtspHeader::GetParamSring(std::string& paramName)
{
	std::vector<char>::iterator it = std::search(header.begin(), header.end(),
		paramName.begin(), paramName.end());

	if (it == header.end())
		return 0;

	it += paramName.length()+1;

	std::string param;

	while ( ( it != header.end() ) && ( (*it != '\r') && (*(it + 1) != '\n') ) ) {
		param += *it++;
	}

	return param;
}

size_t RtspHeader::GetContentLength()
{
	return GetParamValue( std::string("Content-Length") );
}

std::string RtspHeader::GetContentType()
{
	return GetParamSring( std::string("Content-Type") );
}

std::string RtspHeader::GetContentBase()
{
	return GetParamSring( std::string("Content-Base") );
}

uint32_t RtspHeader::GetCseq()
{
	return GetParamValue( std::string("CSeq") );
}

std::string RtspHeader::GetDate()
{
	return GetParamSring( std::string("Date") );
}

std::string RtspHeader::GetTransport()
{
	return GetParamSring( std::string("Transport") );
}

std::pair<uint16_t, uint16_t> RtspHeader::GetTransportServerPorts()
{
	std::string first;
	std::string second;
	std::string param = "server_port=";
	std::string ts = GetTransport();

	std::string::iterator it = std::search( ts.begin(), ts.end(), param.begin(), param.end() );

	if ( it != ts.end() ) {


		while (*it != '-') {
			first += *it++;
		}

		while ( (it != ts.end()) && (*it != '\r') && (*it != '\n') ) {
			second += *it++;
		}
	}

	return std::pair<uint16_t, uint16_t>( stoi(first), stoi(second) );
}