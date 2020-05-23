#ifndef _X_SOCKET_TCP_CLIENT_H_
#define _X_SOCKET_TCP_CLIENT_H_

#include <inttypes.h>
#include <WinSock2.h>
#include <WS2tcpip.h>

#include <thread>

# define X_SOCKET_TCP_BUF_SIZE 1024

namespace xM
{
    namespace socket
    {
        class ITCPClientEvent
        {
        public:
            virtual void Connected() { ; }
            virtual void DisConnected(int _error) {};
            virtual void Receive(uint8_t* _data, int _len) {};
        };

        class TCPClient
        {
		private:
			ITCPClientEvent* event_;

			std::thread recv_thread_;
			volatile bool run_flag_;

			SOCKET conn_;
		public:
			TCPClient();
			~TCPClient();
		public:
			bool Connect(ITCPClientEvent* _event,const char* _ip, const int _port);
			void DisConnect();
		public:
			bool Send(const char* _buffer, const int _buf_len);
		private:
			void close_socket();
			void recv_function();
        };
    }
}

#endif // _X_SOCKET_TCP_CLIENT_H_