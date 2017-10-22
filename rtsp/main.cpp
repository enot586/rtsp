#include "stdafx.h"
#include <string>
#include <iostream>
#include <vector>
#include <boost\asio.hpp>
#include "RtspBuilder.h"

std::vector<char> response_buffer(512);

void receive_until(boost::asio::ip::tcp::socket& s, std::vector<char>& tp, std::vector<char>& responseBuffer)
{
	uint32_t receivedBytes = 0;
	bool endOfPacket = false;

	do {
		receivedBytes += s.read_some( boost::asio::buffer(responseBuffer) );
		
		if ( receivedBytes >= tp.size() ) {
			std::vector<char>::iterator it =
				std::search( (responseBuffer.begin()+ receivedBytes) - tp.size(), responseBuffer.end(),
					tp.begin(), tp.end());

			endOfPacket = it != responseBuffer.end();
		}
		else
		{
			endOfPacket = false;
		}
	} while (!endOfPacket);

	std::cout << "response complete" << std::endl;
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

	std::cout << "receive \t\t";
	try {
		std::vector<char> tp(4);
		tp[0] = '\r';
		tp[1] = '\n';
		tp[2] = '\r';
		tp[3] = '\n';

		receive_until(sock, tp, response_buffer);
		std::cout << "[OK]" << std::endl;
	}
	catch (boost::system::system_error e) {
		std::cout << "[FAIL]" << std::endl;
		return (-1);
	}

	if ( 401 == rtsp.ParseResponse(response_buffer) ) {

		std::cout << "send OPTIONS with auth \t\t";
		try {
			sock.write_some(boost::asio::buffer( rtsp.Options( std::string("admin:admin") ) ) );
			std::cout << "[OK]" << std::endl;
		}
		catch (boost::system::system_error e) {
			std::cout << "[FAIL]" << std::endl;
			return (-1);
		}

		std::cout << "receive \t\t";
		try {
			std::vector<char> tp(4);
			tp[0] = '\r';
			tp[1] = '\n';
			tp[2] = '\r';
			tp[3] = '\n';

			receive_until(sock, tp, response_buffer);
			std::cout << "[OK]" << std::endl;
		}
		catch (boost::system::system_error e) {
			std::cout << "[FAIL]" << std::endl;
			return (-1);
		}

		if ( 200 != rtsp.ParseResponse(response_buffer) )		{
			std::cout << "error: unavailable connection " << std::endl;
			return (-1);
		}

		std::cout << "sucessfull rtsp connection" << std::endl;

		std::cout << "send DESCRIBE with auth \t\t";
		try {
			sock.write_some(boost::asio::buffer(rtsp.Describe(std::string("admin:admin"))));
			std::cout << "[OK]" << std::endl;
		}
		catch (boost::system::system_error e) {
			std::cout << "[FAIL]" << std::endl;
			return (-1);
		}

		std::cout << "receive \t\t";
		try {
			std::vector<char> tp(4);
			tp[0] = '\r';
			tp[1] = '\n';
			tp[2] = '\r';
			tp[3] = '\n';

			receive_until(sock, tp, response_buffer);
			std::cout << "[OK]" << std::endl;
		}
		catch (boost::system::system_error e) {
			std::cout << "[FAIL]" << std::endl;
			return (-1);
		}

		if (200 != rtsp.ParseResponse(response_buffer)) {
			std::cout << "error: unavailable connection " << std::endl;
			return (-1);
		}
		

	}

	std::cout << "The end" << std::endl;
	std::cin;
    return 0;
}

