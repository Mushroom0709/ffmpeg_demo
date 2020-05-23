#include "x_socket_udp_helper.h"

namespace xM
{
	namespace socket
	{
		void SetClientAddress(sockaddr_in* _cli_addr, const int _port, const char* _ip)
		{
			memset(_cli_addr, 0, sizeof(sockaddr_in));
			_cli_addr->sin_family = AF_INET;
			_cli_addr->sin_port = htons(_port);
			if (_ip == NULL)
			{
				_cli_addr->sin_addr.s_addr = htonl(INADDR_ANY);
			}
			else
			{
				inet_pton(AF_INET, (PSTR)_ip, &_cli_addr->sin_addr);
			}
		}

		UDPHelper::UDPHelper()
		{
			sock_ = INVALID_SOCKET;
			event_ = NULL;
			run_flag_ = false;
		}
		UDPHelper::~UDPHelper()
		{
			Close();
		}

		bool UDPHelper::initialization(const int _port, const char* _ip)
		{
			WSADATA was_data;
			sockaddr_in addr_in;

			if (run_flag_ == true)
				return false;

			::WSACleanup();

			if (::WSAStartup(MAKEWORD(2, 2), &was_data) != 0)
			{
				printf("Failed to load Winsock.\n");
				return false;
			}

			sock_ = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
			if (sock_ == INVALID_SOCKET)
				return false;

			memset(&addr_in, 0, sizeof(sockaddr_in));
			addr_in.sin_family = AF_INET;
			addr_in.sin_port = htons(_port);
			if (_ip == NULL)
			{
				addr_in.sin_addr.s_addr = htonl(INADDR_ANY);
			}
			else
			{
				inet_pton(AF_INET, (PSTR)_ip, &addr_in.sin_addr);
			}

			if (::bind(sock_, (sockaddr*)&addr_in, sizeof(addr_in)) == SOCKET_ERROR)
			{
				close_socket();
				return false;
			}

			return true;
		}
		void UDPHelper::recv_function()
		{
			char buffer[X_LIBRARY_UDP_BUF_SIZE] = { 0 };
			int buf_len = 0;
			int sel_ret = 0;

			sockaddr_in cli_addr;
			int cli_addr_len;

			timeval timeout;
			FD_SET read_set;

			timeout.tv_sec = 0;
			timeout.tv_usec = 1000 * 100;

			while (run_flag_)
			{
				//memset(buffer, 0, buf_len);
				FD_ZERO(&read_set);
				FD_SET(sock_, &read_set);
				sel_ret = ::select(0, &read_set, NULL, NULL, NULL);
				if (sel_ret == SOCKET_ERROR)
				{
					run_flag_ = false;
					//printf("[LOG] [ERROR] [%d]\n", WSAGetLastError());

					close_socket();
				}
				else if (sel_ret == 0)
				{
					//
				}
				else
				{
					if (FD_ISSET(sock_, &read_set) != 0)
					{
						cli_addr_len = sizeof(sockaddr_in);
						memset(&cli_addr, 0, sizeof(sockaddr_in));

						buf_len = recvfrom(sock_, buffer, X_LIBRARY_UDP_BUF_SIZE, 0, (sockaddr*)&cli_addr, (socklen_t*)&cli_addr_len);
						if (buf_len == INVALID_SOCKET)
						{
							//printf("errno = %d\n", GetLastError());
							//run_flag_ = false;
							//close_socket();
						}
						else
						{
							if (event_ != nullptr)
							{
								event_->ReceiveFrom((unsigned char*)buffer, buf_len, cli_addr, cli_addr_len);
							}
						}
					}
				}
			}
		}
		void UDPHelper::close_socket()
		{
			if (sock_ != INVALID_SOCKET)
			{
				::closesocket(sock_);
				sock_ = INVALID_SOCKET;
				::WSACleanup();

				if (event_ != NULL)
					event_->Close();
			}
		}

		int  UDPHelper::SendTo(unsigned char* _buffer, const int _buf_len, sockaddr_in* _cli_addr)
		{
			return sendto(sock_, (const char*)_buffer, _buf_len, 0, (sockaddr*)_cli_addr, sizeof(sockaddr_in));
		}
		bool UDPHelper::Open(const int _port, const PtrIUDPEvent _event, const char* _ip)
		{
			if (run_flag_ == true)
				return false;

			if (false == initialization(_port, _ip))
				return false;

			if (_event != NULL)
			{
				event_ = _event;
				recv_thread_ = std::thread(&UDPHelper::recv_function, this);

				event_->Open();
			}
			run_flag_ = true;
			return true;
		}
		void UDPHelper::Close()
		{
			if (run_flag_ == false)
				return;

			run_flag_ = false;
			if (event_ != NULL)
			{
				if (recv_thread_.joinable())
					recv_thread_.join();

				event_->Close();
			}

			close_socket();
		}
	}
}