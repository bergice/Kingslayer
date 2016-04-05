#include "telnet_client.h"

#include <boost/bind.hpp>
#include <boost/thread.hpp>

#include "program.h"
#include "interface.h"
#include "boost\\lexical_cast.hpp"

telnet_client::telnet_client(boost::asio::io_service& io_service, tcp::resolver::iterator endpoint_iterator)
	: active_(true),
	  io_service_(io_service), socket_(io_service),
	  connect_timer_(io_service), connection_timeout(boost::posix_time::seconds(CONNECT_TIMEOUT))
{
	netlog.open ("telnet.log");
	netlog << "Starting log...\n";
	alreadyWriting=false;
	connect_start(endpoint_iterator);
}

telnet_client::~telnet_client()
{
	netlog << "Ending log...\n";
	netlog.close();
	socket_.close();
	active_ = false;
}

void telnet_client::write(const char msg) // pass the write data to the do_write function via the io service in the other thread
{
	io_service_.post(boost::bind(&telnet_client::do_write, this, msg));
}

void telnet_client::write_str(std::string msg) // pass the write data to the do_write function via the io service in the other thread
{
	io_service_.post(boost::bind(&telnet_client::do_write_str, this, msg));
}

void telnet_client::close() // call the do_close function via the io service in the other thread
{
	io_service_.post(boost::bind(&telnet_client::do_close, this, boost::system::error_code()));
}

bool telnet_client::active() // return true if the socket is still active
{
	return active_;
}

void telnet_client::connect_start(tcp::resolver::iterator endpoint_iterator)
{ // asynchronously connect a socket to the specified remote endpoint and call connect_complete when it completes or fails
	tcp::endpoint endpoint = *endpoint_iterator;
	socket_.async_connect(endpoint, boost::bind(&telnet_client::connect_complete, this, boost::asio::placeholders::error, ++endpoint_iterator));
	// start a timer that will expire and close the connection if the connection cannot connect within a certain time
	connect_timer_.expires_from_now(connection_timeout); //boost::posix_time::seconds(connection_timeout));
	connect_timer_.async_wait(boost::bind(&telnet_client::do_close, this, boost::asio::placeholders::error));
}

void telnet_client::connect_complete(const boost::system::error_code& error, tcp::resolver::iterator endpoint_iterator)
{ // the connection to the server has now completed or failed and returned an error
	if (!error) // success, so start waiting for read data
	{
		connect_timer_.cancel(); // the connection was successful, so cancel the timeout
		read_start();
	}
	else
		do_close(error);
}

void telnet_client::read_start(void)
{ // Start an asynchronous read and call read_complete when it completes or fails
	socket_.async_read_some( boost::asio::buffer(read_msg_, max_read_length), 
							 boost::bind(&telnet_client::read_complete, 
							 this, 
							 boost::asio::placeholders::error, 
							 boost::asio::placeholders::bytes_transferred));
}

void telnet_client::read_complete(const boost::system::error_code& error, size_t bytes_transferred)
{ // the asynchronous read operation has now completed or failed and returned an error
	if (!error)
	{ 
		// read completed, so process the data
		cout.write(read_msg_, bytes_transferred); // echo to standard output
		std::string new_msg(read_msg_, bytes_transferred);
		program->PushMessage(new_msg);

		read_start(); // start waiting for another asynchronous read again
	}
	else
		do_close(error);
}

void telnet_client::do_write(const char msg)
{ // callback to handle write call from outside this class
	bool write_in_progress = !write_msgs_.empty(); // is there anything currently being written?
	write_msgs_.push_back(msg); // store in write buffer
	if (!write_in_progress) // if nothing is currently being written, then start
		write_start();
	//else
	//	form->AddConsoleMessage("Trying to write '" + boost::lexical_cast<std::string>(msg) + "' but there is already something being written! (LINE " + boost::lexical_cast<std::string>(__LINE__) + ")");
}

void telnet_client::do_write_str(std::string msg)
{ // callback to handle write call from outside this class
	bool write_in_progress = !write_msgs_.empty(); // is there anything currently being written?
	for(unsigned i=0; i<msg.length(); i++)
		write_msgs_.push_back(msg[i]); // store in write buffer
	if (!write_in_progress) // if nothing is currently being written, then start
		write_start();
	//else
	//	form->AddConsoleMessage("Trying to write '" + boost::lexical_cast<std::string>(msg) + "' but there is already something being written! (LINE " + boost::lexical_cast<std::string>(__LINE__) + ")");
}

void telnet_client::write_start(void)
{ // Start an asynchronous write and call write_complete when it completes or fails
	boost::asio::async_write(socket_,
		boost::asio::buffer(&write_msgs_.front(), 1),
		boost::bind(&telnet_client::write_complete,
			this,
			boost::asio::placeholders::error));
}

void telnet_client::write_complete(const boost::system::error_code& error)
{ // the asynchronous read operation has now completed or failed and returned an error
	if (!error)
	{ // write completed, so send next write data
		netlog << write_msgs_[0];

		write_msgs_.pop_front(); // remove the completed data
		if (!write_msgs_.empty()) // if there is anthing left to be written
			write_start(); // then start sending the next item in the buffer
	}
	else
		do_close(error);
}

void telnet_client::write_completeString(const boost::system::error_code& error)
{ // the asynchronous read operation has now completed or failed and returned an error
	if (!error)
	{
	}
	else
		do_close(error);
}

void telnet_client::writeString(const char* msg)
{
	if (active())
	{
		while(alreadyWriting)
		{/* Do nothing*/}
		alreadyWriting=true;
		//std::string msg_send = msg;
		//msg_send.append("\r\n");
		//write_str(msg_send);
		for(unsigned i=0; i<strlen(msg); i++)
			write(msg[i]);
		write('\r');write('\n');
		alreadyWriting=false;
	}
}

void telnet_client::do_close(const boost::system::error_code& error)
{ // something has gone wrong, so close the socket & make this object inactive
	if (error == boost::asio::error::operation_aborted) // if this call is the result of a timer cancel()
		return; // ignore it because the connection cancelled the timer
	if (error)
		MessageBox(0, const_cast < char* > (error.message().c_str()),"Error",MB_OK);
	else
		MessageBox(0, "Error: Connection did not succeed.\n","Error",MB_OK);
	socket_.close();
	active_ = false;
}