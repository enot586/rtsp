#pragma once

#include <vector>

class RtspHeader
{
	std::vector<char> header;

public:
	RtspHeader(std::vector<char>& responseBuffer);
	~RtspHeader();

	uint32_t GetCode();
	std::string GetContentBase();
	size_t GetContentLength();
	std::string GetContentType();
	uint32_t GetCseq();
	std::string GetDate();

private:
	uint32_t	GetParamValue(std::string& paramName);
	std::string GetParamSring(std::string& paramName);
};

