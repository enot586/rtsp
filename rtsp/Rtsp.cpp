#include "stdafx.h"
#include "Rtsp.h"

Rtsp::Rtsp(boost::asio::ip::tcp::socket& sock_) :
	sock(sock_), cp(RTP_PORT_DEFAULT, RTCP_PORT_DEFAULT), sp(0, 0)
{
}

Rtsp::~Rtsp()
{
}

uint32_t Rtsp::ReceiveUntil(boost::asio::ip::tcp::socket& s, std::vector<char>& tp, std::vector<char>& responseBuffer)
{
	bool endOfPacket = false;
	int receivedBytes = 0;
	uint32_t totalBytes = 0;

	do {
		char currentByte[1];

		receivedBytes = s.read_some(boost::asio::buffer(currentByte));

		if (receivedBytes) {
			responseBuffer.at(totalBytes) = currentByte[0];

			++totalBytes;

			if (totalBytes >= tp.size()) {
				std::vector<char>::iterator itEndPacket = (responseBuffer.begin() + totalBytes);
				std::vector<char>::iterator it =
					std::search(itEndPacket - tp.size(), itEndPacket, tp.begin(), tp.end());

				endOfPacket = (it != itEndPacket);
			}
			else {
				endOfPacket = false;
			}
		}
	} while (!endOfPacket);

	return totalBytes;
}

RtspHeader Rtsp::ReceiveHeader(boost::asio::ip::tcp::socket& s)
{
	std::vector<char> responseBuffer(512);

	std::fill(responseBuffer.begin(), responseBuffer.end(), 0);

	//std::cout << "receive \t\t";
	try {
		std::vector<char> tp(4);
		tp[0] = '\r';
		tp[1] = '\n';
		tp[2] = '\r';
		tp[3] = '\n';

		uint32_t receiveBytes = ReceiveUntil(s, tp, responseBuffer);

		if (receiveBytes < responseBuffer.size())
			responseBuffer.erase(responseBuffer.begin() + receiveBytes, responseBuffer.end());

		//std::cout << "[OK]" << std::endl;
	}
	catch (boost::system::system_error e) {
		//std::cout << "[FAIL]" << std::endl;
		throw e;
	}

	return RtspHeader(responseBuffer);
}

std::string Rtsp::GetSdpAttributeValue(std::string attr, std::vector<char>& source)
{
	std::string fullAttr = "a=" + attr + ":";
	std::string result;
	std::vector<char>::iterator it = std::search(source.begin(), source.end(), fullAttr.begin(), fullAttr.end());

	if (it != source.end()) {
		it += fullAttr.length();

		while ((it != source.end()) && (*it != '\r') && (*it != '\n')) {
			result += *it;
			++it;
		}
	}

	return result;
}

void Rtsp::Connect(std::string& address)
{
	rtspRequest.reset( new RtspBuilder(address) );
	
	boost::asio::ip::tcp::endpoint ep(boost::asio::ip::address::from_string(address.c_str()), 554);
	
	//std::cout << "connect \t\t";
	try {
		sock.connect(ep);
		//std::cout << "[OK]" << std::endl;
	}
	catch (boost::system::system_error e) {
		//std::cout << "[FAIL]" << std::endl;
		throw e;
	}

	//std::cout << "send OPTIONS \t\t";
	try {
		sock.write_some(boost::asio::buffer(rtspRequest->Options()));
		//std::cout << "[OK]" << std::endl;
	}
	catch (boost::system::system_error e) {
		//std::cout << "[FAIL]" << std::endl;
		throw e;
	}

	RtspHeader header = ReceiveHeader(sock);

	if (401 == header.GetCode()) {

		//std::cout << "send OPTIONS with auth \t\t";
		try {
			sock.write_some(boost::asio::buffer(rtspRequest->Options(std::string("admin:admin"))));
			//std::cout << "[OK]" << std::endl;

			header = ReceiveHeader(sock);

			if (200 != header.GetCode()) {
				throw std::exception(std::string("error: " + std::to_string(header.GetCode())).c_str());
			}
		}
		catch (boost::system::system_error e) {
			//std::cout << "[FAIL]" << std::endl;
			throw e;
		}

		//std::cout << "send DESCRIBE with auth \t\t";
		try {
			sock.write_some(boost::asio::buffer(rtspRequest->Describe(std::string("admin:admin"))));
			//std::cout << "[OK]" << std::endl;

			header = ReceiveHeader(sock);

			if (200 != header.GetCode()) {
				throw std::exception(std::string("error: " + std::to_string(header.GetCode())).c_str());
			}

			//receive sdp payload
			std::vector<char> sdp_description(header.GetContentLength());
			size_t received_bytes = 0;
			while (received_bytes < sdp_description.size()) {
				received_bytes += sock.read_some(boost::asio::buffer(sdp_description));
			}

			addr = GetSdpAttributeValue("control", sdp_description);
		}
		catch (boost::system::system_error e) {
			//std::cout << "[FAIL]" << std::endl;
			throw e;
		}

		//std::cout << "send SETUP with auth \t\t";
		try {
			sock.write_some(boost::asio::buffer(rtspRequest->Setup(std::string("admin:admin"), addr, cp.first, cp.second)));
			//std::cout << "[OK]" << std::endl;

			header = ReceiveHeader(sock);

			if (200 != header.GetCode()) {
				throw std::exception(std::string("error: " + std::to_string(header.GetCode())).c_str());
			}

			sp = header.GetTransportServerPorts();
			sessionId = header.GetSessionId();
		}
		catch (boost::system::system_error e) {
			//std::cout << "[FAIL]" << std::endl;
			throw e;
		}
	}
	else
	{
		throw std::exception("supports only authorized access");
	}
}
void Rtsp::Play()
{
	//std::cout << "send PLAY with auth \t\t";
	try {
		sock.write_some(boost::asio::buffer(rtspRequest->Play(std::string("admin:admin"), sessionId)));
		//std::cout << "[OK]" << std::endl;

		RtspHeader header = ReceiveHeader(sock);

		if (200 != header.GetCode()) {
			throw std::exception(std::string("error: " + std::to_string(header.GetCode())).c_str());
		}
	}
	catch (boost::system::system_error e) {
		//std::cout << "[FAIL]" << std::endl;
		throw e;
	}
}

void Rtsp::Teardown()
{
	//std::cout << "send TEARDOWN with auth \t\t";
	try {
		sock.write_some(boost::asio::buffer(rtspRequest->Teardown(std::string("admin:admin"), sessionId)));
		//std::cout << "[OK]" << std::endl;

		RtspHeader header = ReceiveHeader(sock);

		if (200 != header.GetCode()) {
			throw std::exception(std::string("error: " + std::to_string(header.GetCode())).c_str());
		}
	}
	catch (boost::system::system_error e) {
		//std::cout << "[FAIL]" << std::endl;
		throw e;
	}
}

