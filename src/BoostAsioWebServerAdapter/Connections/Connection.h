#pragma once

#include "IConnection.h"

#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>
#include <boost/enable_shared_from_this.hpp>


namespace systelab { namespace web_server {
	class Reply;
	class Request;
}}

namespace systelab { namespace web_server { namespace boostasio {

	class IReplyBufferBuilderService;
	class IReplyCORSHeadersBuilderService;
	class IRequestHandlingService;
	class IRequestParserAgent;
	class IRequestURIParserService;

	class Connection : public IConnection
					 , public boost::enable_shared_from_this<Connection>
	{
	public:
		Connection(boost::asio::io_service&,
				   std::unique_ptr<IRequestParserAgent>,
				   std::unique_ptr<IRequestURIParserService>,
				   std::unique_ptr<IRequestHandlingService>,
				   std::unique_ptr<IReplyCORSHeadersBuilderService>,
				   std::unique_ptr<IReplyBufferBuilderService>);
		virtual ~Connection();

		void start();
		boost::asio::basic_socket<boost::asio::ip::tcp>& socket();

	private:
		void handleRead(const boost::system::error_code&, std::size_t bytes_transferred);
		void handleWrite(const boost::system::error_code&);

	private:
		boost::asio::io_service::strand m_strand;
		boost::asio::ip::tcp::socket m_socket;
		boost::array<char, 8192> m_requestBuffer;
		std::string m_replyBuffer;

		std::unique_ptr<IRequestParserAgent> m_requestParserAgent;
		std::unique_ptr<IRequestURIParserService> m_requestURIParserService;
		std::unique_ptr<IRequestHandlingService> m_requestHandlingService;
		std::unique_ptr<IReplyCORSHeadersBuilderService> m_replyCORSHeadersBuilderService;
		std::unique_ptr<IReplyBufferBuilderService> m_replyBufferBuilderService;

		std::unique_ptr<Request> m_request;
		std::unique_ptr<Reply> m_reply;
	};

}}}
