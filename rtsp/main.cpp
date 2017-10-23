#include "stdafx.h"
#include <string>
#include <iostream>
#include <vector>
#include <boost\asio.hpp>
#include "RtspHeader.h"
#include "RtspBuilder.h"

uint32_t receive_until(boost::asio::ip::tcp::socket& s, std::vector<char>& tp, std::vector<char>& responseBuffer)
{
	bool endOfPacket = false;
	int receivedBytes = 0;
	uint32_t totalBytes = 0;

	do {
		char currentByte[1];

		receivedBytes = s.read_some( boost::asio::buffer(currentByte) );
		
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

	std::cout << "response complete" << std::endl;

	return totalBytes;
}

RtspHeader receive_header(boost::asio::ip::tcp::socket& s)
{
	std::vector<char> responseBuffer(512);

	std::fill(responseBuffer.begin(), responseBuffer.end(), 0);

	std::cout << "receive \t\t";
	try {
		std::vector<char> tp(4);
		tp[0] = '\r';
		tp[1] = '\n';
		tp[2] = '\r';
		tp[3] = '\n';

		uint32_t receiveBytes = receive_until(s, tp, responseBuffer);

		if ( receiveBytes < responseBuffer.size() )
			responseBuffer.erase(responseBuffer.begin() + receiveBytes, responseBuffer.end() );

		std::cout << "[OK]" << std::endl;
	}
	catch (boost::system::system_error e) {
		std::cout << "[FAIL]" << std::endl;
		throw e;
	}

	return RtspHeader(responseBuffer);
}

int main()
{
	std::string camIp = "192.168.0.102";
	RtspBuilder rtsp(camIp);

	boost::asio::io_service service;
	boost::asio::ip::tcp::endpoint ep(boost::asio::ip::address::from_string( camIp.c_str() ), 554);
	boost::asio::ip::tcp::socket sock(service);

	std::cout << "connect \t\t";
	try	{
		sock.connect(ep);
		std::cout << "[OK]" << std::endl;
	} catch (boost::system::system_error e) {
		std::cout << "[FAIL]" << std::endl;
		return (-1);
	}

	std::cout << "send OPTIONS \t\t";
	try {
		sock.write_some( boost::asio::buffer( rtsp.Options() ) );
		std::cout << "[OK]" << std::endl;
	}
	catch (boost::system::system_error e) {
		std::cout << "[FAIL]" << std::endl;
		return (-1);
	}

	RtspHeader header = receive_header(sock);

	if ( 401 == header.GetCode() ) {

		std::cout << "send OPTIONS with auth \t\t";
		try {
			sock.write_some(boost::asio::buffer( rtsp.Options( std::string("admin:admin") ) ) );
			std::cout << "[OK]" << std::endl;

			header = receive_header(sock);

			if ( 200 != header.GetCode() ) {
				std::cout << "error: unavailable connection " << std::endl;
				return (-1);
			}
		}
		catch (boost::system::system_error e) {
			std::cout << "[FAIL]" << std::endl;
			return (-1);
		}

		std::cout << "send DESCRIBE with auth \t\t";
		try {
			sock.write_some( boost::asio::buffer( rtsp.Describe( std::string("admin:admin") ) ) );
			std::cout << "[OK]" << std::endl;

			header = receive_header(sock);

			if ( 200 != header.GetCode() ) {
				std::cout << "error: unavailable connection " << std::endl;
				return (-1);
			}

			std::vector<char> body( header.GetContentLength() );

			size_t received_bytes = 0;
			while ( received_bytes < body.size() ) {
				received_bytes+= sock.read_some( boost::asio::buffer(body) );
			}
			


		}
		catch (boost::system::system_error e) {
			std::cout << "[FAIL]" << std::endl;
			return (-1);
		}
	}

	std::cout << "The end" << std::endl;
	std::cin;
    return 0;
}

