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
        typedef struct _LINK_INFO
        {
            SOCKET ID;
            void* Params;
        }LINK_INFO,*PLINK_INFO;

        class ITCPServerEvent
        {
        public:
            virtual void* Connected(SOCKET _socket) { return NULL; }
            virtual void DisConnected(SOCKET _socket, void* _info, int _error) {};
            virtual void Error(DWORD _error) {};
            virtual void Receive(SOCKET _socket, void* _info, uint8_t* _data, int _len) {};
        };

        class TCPServer
        {
            using MSocketInfoType = std::map<SOCKET, PLINK_INFO>;
            using PSocketInfoType = std::pair<SOCKET, PLINK_INFO>;
        private:
            ITCPServerEvent* event_;
        private:
            SOCKET listen_;
            MSocketInfoType links_;
            std::mutex links_lock_;

            std::thread work_thread_;
            volatile bool run_flag_;
        public:
            TCPServer();
            ~TCPServer();
        private:
            PLINK_INFO find_link(SOCKET _link);
            PLINK_INFO del_link(SOCKET _link);
            void del_link(PLINK_INFO _info);
            void add_link(PLINK_INFO _ptr_link);
        private:
            void close_socket(SOCKET& _sock);
            bool initialization(const char* _ip, const int _port);
            bool accept_function();
            bool recv_function(SOCKET _sock, char* _buffer, int _buf_size);
            void work_function();
        private:
            bool send_bytes(SOCKET _sock, uint8_t* _data, int _len);
        public:
            bool Send(SOCKET _sock, uint8_t* _data, int _len);
            bool SendAll(uint8_t* _data, int _len);
        public:
            bool Start(ITCPServerEvent* _event, const char* _addr_ip, const int _addr_port);
            void Stop();
        };
    }
}

#endif // _X_SOCKET_TCP_SERVER_H_