#ifndef _X_SOCKET_TCP_SERVER_H_
#define _X_SOCKET_TCP_SERVER_H_

#include <stdio.h>
#include <thread>

#include <WinSock2.h>
#include <WS2tcpip.h>

#include <mutex>
#include <vector>
#include <map>

# define X_SOCKET_TCP_BUF_SIZE 1024

namespace xM
{
    namespace socket
    {
        class ITCPServerEvent
        {
        public:
            virtual void Connected(SOCKET _socket) { }
            virtual void DisConnected(SOCKET _socket, int _error) {};
            virtual void Error(DWORD _error) {};
            virtual void Receive(SOCKET _socket, uint8_t* _data, int _len) {};
        };

        class TCPServer
        {
        private:
            ITCPServerEvent* event_;
        private:
            SOCKET listen_;

            std::vector<SOCKET> links_;

            std::thread work_thread_;
            volatile bool run_flag_;
        public:
            TCPServer();
            ~TCPServer();
        private:
            bool add_link(SOCKET _link);
            bool del_link(SOCKET _link);
        private:
            void close_socket(SOCKET _sock);
            bool initialization(const char* _ip, const int _port);
            bool accept_function();
            bool recv_function(SOCKET _sock, char* _buffer, int _buf_size);
            void work_function();
        private:
            bool send_bytes(SOCKET _sock, uint8_t* _data, int _len);
        public:
            bool Send(SOCKET _sock, uint8_t* _data, int _len);
        public:
            bool Start(ITCPServerEvent* _event, const char* _addr_ip, const int _addr_port);
            void Stop();
        };
    }
}

#endif // _X_SOCKET_TCP_SERVER_H_