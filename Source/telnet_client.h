#pragma once

#include <deque>
#include <iostream>
#include <fstream>

#ifndef ASIO_INCLUDED
	#include <boost/asio.hpp>
	#define ASIO_INCLUDED
#endif

#ifdef POSIX
#include <termios.h>
#endif

using boost::asio::ip::tcp;
using namespace std;

#define CONNECT_TIMEOUT		3

#define max_read_length		512

class telnet_client
{
private:	bool alreadyWriting;
			
			bool active_; // remains true while this object is still operating
			boost::asio::io_service& io_service_; // the main IO service that runs this connection
			tcp::socket socket_; // the socket this instance is connected to
			boost::asio::deadline_timer connect_timer_;
			boost::posix_time::time_duration connection_timeout; // time to wait for the connection to succeed
			char read_msg_[max_read_length]; // data read from the socket
			deque<char> write_msgs_; // buffered write data

			std::ofstream netlog;

private:	void connect_start(tcp::resolver::iterator endpoint_iterator);
			void connect_complete(const boost::system::error_code& error, tcp::resolver::iterator endpoint_iterator);
			void read_start(void);
			void read_complete(const boost::system::error_code& error, size_t bytes_transferred);
			void do_write(const char msg);
			void do_write_str(std::string msg);
			void write_start(void);
			void write_complete(const boost::system::error_code& error);
			void write_completeString(const boost::system::error_code& error);
			void do_close(const boost::system::error_code& error);

public:		telnet_client(boost::asio::io_service& io_service, tcp::resolver::iterator endpoint_iterator);
			~telnet_client();

			void write(const char msg);
			void write_str(std::string msg);
			void writeString(const char* msg);

			void close();
			bool active();
};