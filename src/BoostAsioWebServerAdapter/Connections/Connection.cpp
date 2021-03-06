#include "Connection.h"

#include "Agents/IRequestParserAgent.h"
#include "Services/IReplyBufferBuilderService.h"
#include "Services/IReplyCORSHeadersBuilderService.h"
#include "Services/IRequestHandlingService.h"
#include "Services/IRequestURIParserService.h"

#include "WebServerAdapterInterface/Model/Configuration.h"
#include "WebServerAdapterInterface/Model/Reply.h"
#include "WebServerAdapterInterface/Model/Request.h"

#include <boost/bind.hpp>
#include <vector>


namespace systelab { namespace web_server { namespace boostasio {

	Connection::Connection(boost::asio::io_service& io_service,
						   std::unique_ptr<IRequestParserAgent> requestParserAgent,
						   std::unique_ptr<IRequestURIParserService> requestURIParserService,
						   std::unique_ptr<IRequestHandlingService> requestHandlingService,
						   std::unique_ptr<IReplyCORSHeadersBuilderService> replyCORSHeadersBuilderService,
						   std::unique_ptr<IReplyBufferBuilderService> replyBuffersBuilderService)
		:m_strand(io_service)
		,m_socket(io_service)
		,m_requestBuffer()
		,m_replyBuffer()
		,m_requestParserAgent(std::move(requestParserAgent))
		,m_requestURIParserService(std::move(requestURIParserService))
		,m_requestHandlingService(std::move(requestHandlingService))
		,m_replyBufferBuilderService(std::move(replyBuffersBuilderService))
		,m_replyCORSHeadersBuilderService(std::move(replyCORSHeadersBuilderService))
		,m_request()
		,m_reply()
	{
	}

	Connection::~Connection() = default;

	void Connection::start()
	{
		m_request = std::make_unique<Request>();
		m_reply.reset();

		m_socket.async_read_some(
			boost::asio::buffer(m_requestBuffer),
			m_strand.wrap(boost::bind(&Connection::handleRead, shared_from_this(),
						  boost::asio::placeholders::error,
						  boost::asio::placeholders::bytes_transferred)));
	}

	boost::asio::basic_socket<boost::asio::ip::tcp>& Connection::socket()
	{
		return m_socket;
	}

	void Connection::handleRead(const boost::system::error_code& e, std::size_t bytes_transferred)
	{
		if (!e)
		{
			boost::optional<bool> result = m_requestParserAgent->parseBuffer(m_requestBuffer.data(), bytes_transferred, *m_request);

			if (!result.is_initialized())
			{
				m_socket.async_read_some(
					boost::asio::buffer(m_requestBuffer),
						m_strand.wrap(
							boost::bind(&Connection::handleRead, shared_from_this(),
							boost::asio::placeholders::error,
							boost::asio::placeholders::bytes_transferred)));
			}
			else
			{
				if (result)
				{
					result = m_requestURIParserService->parse(*m_request);
				}

				if (result)
				{
					m_reply = m_requestHandlingService->processRequest(*m_request);
				}
				else
				{
					m_reply.reset(new Reply());
					m_reply->setStatus(Reply::BAD_REQUEST);
				}

				m_replyCORSHeadersBuilderService->addCORSHeaders(*m_request, *m_reply);

				m_replyBuffer = m_replyBufferBuilderService->buildBuffer(*m_reply);

				boost::asio::async_write(m_socket, boost::asio::buffer(m_replyBuffer),
					m_strand.wrap(boost::bind(
								  &Connection::handleWrite, shared_from_this(),
								  boost::asio::placeholders::error)));
			}
		}
	}

	void Connection::handleWrite(const boost::system::error_code& e)
	{
		if (!e)
		{
			boost::system::error_code ignored_ec;
			m_socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignored_ec);
		}
	}

}}}

