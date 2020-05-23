#ifndef _X_SOCKET_UDP_HELPER_H_
#define _X_SOCKET_UDP_HELPER_H_

#include <stdio.h>
#include <thread>

#include <WinSock2.h>
#include <WS2tcpip.h>

# define X_LIBRARY_UDP_BUF_SIZE 1450

namespace xM
{
	namespace socket
	{
		typedef class IUDPEvent
		{
		public:
			virtual void ReceiveFrom(unsigned char* _buffer, const int _buf_len, sockaddr_in& _cli_addr, int& _cli_addr_len) {};
			virtual void Open() {};
			virtual void Close() {};
		}*PtrIUDPEvent;

		void SetClientAddress(sockaddr_in* _cli_addr, const int _port, const char* _ip);

		class UDPHelper
		{
		private:
			SOCKET sock_;
			PtrIUDPEvent event_;
			volatile bool run_flag_;
			std::thread recv_thread_;
		public:
			UDPHelper();
			~UDPHelper();
		private:
			bool initialization(const int _port, const char* _ip);
			void recv_function();
			void close_socket();
		public:
			int  SendTo(unsigned char* _buffer, const int _buf_len, sockaddr_in* _cli_addr);
			bool Open(const int _port, const PtrIUDPEvent _event, const char* _ip);
			void Close();
		};
	}
}

#endif //_X_SOCKET_UDP_HELPER_H_