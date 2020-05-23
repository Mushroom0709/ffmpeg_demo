#include "x_socket_tcp_client.h"

namespace xM
{
	namespace socket
	{
		TCPClient::TCPClient()
		{
			conn_ = INVALID_SOCKET;
			event_ = nullptr;
			run_flag_ = false;
		}
		TCPClient::~TCPClient()
		{
			DisConnect();
		}

		bool TCPClient::Connect(ITCPClientEvent* _event, const char* _ip, const int _port)
		{
			WSADATA was_data;
			sockaddr_in addr_in;

			if (run_flag_ == true || _event == NULL)
				return false;

			event_ = _event;

			::WSACleanup();

			if (::WSAStartup(MAKEWORD(2, 2), &was_data) != 0)
			{
				return false;
			}

			conn_ = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
			if (conn_ == INVALID_SOCKET)
				return false;

			memset(&addr_in, 0, sizeof(sockaddr_in));
			addr_in.sin_family = AF_INET;
			addr_in.sin_port = htons(_port);
			inet_pton(AF_INET, (PSTR)_ip, &addr_in.sin_addr);
			//addr_in.sin_addr.S_un.S_addr = inet_addr(_ip);

			if (::connect(conn_, (sockaddr*)&addr_in, sizeof(sockaddr_in)) == SOCKET_ERROR)
			{
				close_socket();
				return false;
			}

			event_->Connected();

			run_flag_ = true;
			recv_thread_ = std::thread(&TCPClient::recv_function, this);
			return true;
		}
		void TCPClient::DisConnect()
		{
			if (run_flag_ == true)
			{
				run_flag_ = false;
			}

			if (recv_thread_.joinable())
			{
				recv_thread_.join();
			}

			if (conn_ != INVALID_SOCKET)
			{
				close_socket();
				event_->DisConnected(0);
				conn_ = INVALID_SOCKET;
			}
		}

		bool TCPClient::Send(const char* _buffer, const int _buf_len)
		{
			if (run_flag_ == false)
				return false;

			int has_sent = 0;
			int temp_sent = 0;

			while (has_sent < _buf_len)
			{
				temp_sent = send(conn_, _buffer + has_sent, _buf_len - has_sent, 0);
				if (temp_sent == SOCKET_ERROR)
				{
					run_flag_ = false;
					event_->DisConnected(GetLastError());
					close_socket();
					return false;
				}

				has_sent += temp_sent;
			}

			return true;
		}
		void TCPClient::recv_function()
		{
			char buffer[X_SOCKET_TCP_BUF_SIZE] = { 0 };
			int buf_len = 0;
			int sel_ret = 0;

			timeval timeout;
			FD_SET read_set;

			timeout.tv_sec = 0;
			timeout.tv_usec = 1000 * 100;

			while (run_flag_)
			{
				//memset(buffer, 0, buf_len);
				FD_ZERO(&read_set);
				FD_SET(conn_, &read_set);
				sel_ret = ::select(0, &read_set, NULL, NULL, &timeout);
				if (sel_ret == SOCKET_ERROR)
				{
					event_->DisConnected(GetLastError());
					run_flag_ = false;
					close_socket();
				}
				else if (sel_ret == 0)
				{
					//
				}
				else
				{
					if (FD_ISSET(conn_, &read_set) != 0)
					{
						buf_len = recv(conn_, buffer, X_SOCKET_TCP_BUF_SIZE, 0);
						if (buf_len <= 0)
						{
							if (buf_len == 0)
								event_->DisConnected(0);
							else
								event_->DisConnected(GetLastError());
							run_flag_ = false;
							close_socket();
						}
						else
						{
							event_->Receive((unsigned char*)buffer, buf_len);
						}
					}
				}
			}
		}
		void TCPClient::close_socket()
		{
			if (conn_ != INVALID_SOCKET)
			{
				::closesocket(conn_);
				conn_ = INVALID_SOCKET;
				::WSACleanup();
			}
		}
	}
}