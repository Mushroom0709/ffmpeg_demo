#include "x_socket_tcp_server.h"

namespace xM
{
    namespace socket
    {
        TCPServer::TCPServer()
        {
            event_ = NULL;
            listen_ = INVALID_SOCKET;
            run_flag_ = false;
        }
        TCPServer::~TCPServer()
        {
            Stop();
        }

        bool TCPServer::add_link(SOCKET _link)
        {
            if (std::find(links_.begin(), links_.end(), _link) == links_.end())
            {
                links_.push_back(_link);
                return true;
            }
            return false;
        }
        bool TCPServer::del_link(SOCKET _link)
        {
            std::vector<SOCKET>::iterator res = std::find(links_.begin(), links_.end(), _link);
            if (res != links_.end())
            {
                close_socket(*res);
                links_.erase(res);
            }
            return true;
        }

        void TCPServer::close_socket(SOCKET _sock)
        {
            if (_sock != INVALID_SOCKET)
            {
                ::closesocket(_sock);
            }
        }
        bool TCPServer::initialization(const char* _ip, const int _port)
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

            listen_ = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
            if (listen_ == INVALID_SOCKET)
                return false;

            memset(&addr_in, 0, sizeof(sockaddr_in));
            addr_in.sin_family = AF_INET;
            addr_in.sin_port = htons(_port);
            inet_pton(AF_INET, (PSTR)_ip, &addr_in.sin_addr);
            //addr_in.sin_addr.S_un.S_addr = inet_addr(_ip);

            if (::bind(listen_, (sockaddr*)&addr_in, sizeof(sockaddr_in)) == SOCKET_ERROR)
            {
                close_socket(listen_);
                return false;
            }

            if (::listen(listen_, 64) == SOCKET_ERROR)
            {
                close_socket(listen_);
                return false;
            }

            return true;
        }
        bool TCPServer::accept_function()
        {
            sockaddr_in cli_addr = { 0 };
            int cli_addr_len = sizeof(sockaddr_in);

            SOCKET stemp = accept(listen_, (sockaddr*)&cli_addr, &cli_addr_len);
            if (stemp == SOCKET_ERROR)
            {
                event_->Error(GetLastError());
                return false;
            }

            event_->Connected(stemp);

            return add_link(stemp);
        }
        bool TCPServer::recv_function(SOCKET _sock, char* _buffer, int _buf_size)
        {
            int res = recv(_sock, _buffer, _buf_size, 0);
            if (res <= 0)
            {
                if (res == 0)
                    event_->DisConnected(_sock, 0);
                else
                    event_->DisConnected(_sock, GetLastError());

                del_link(_sock);
            }
            else
            {
                event_->Receive(_sock, (uint8_t*)_buffer, res);
            }

            return true;
        }
        void TCPServer::work_function()
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
                FD_ZERO(&read_set);
                FD_SET(listen_, &read_set);

                for (auto item : links_)
                {
                    FD_SET(item, &read_set);
                }

                sel_ret = ::select(0, &read_set, NULL, NULL, &timeout);
                if (sel_ret == SOCKET_ERROR)
                {
                    run_flag_ = false;

                    event_->Error(GetLastError());
                }
                else if (sel_ret == 0)
                {
                    //
                }
                else
                {
                    for (size_t i = 0; i < read_set.fd_count; i++)
                    {
                        auto& sk = read_set.fd_array[0];
                        if (FD_ISSET(sk, &read_set) != 0)
                        {
                            if (sk == listen_)
                            {
                                run_flag_ = accept_function();
                            }

                            else
                            {
                                run_flag_ = recv_function(sk, buffer, X_SOCKET_TCP_BUF_SIZE);
                            }
                        }
                    }

                }
            }
            close_socket(listen_);
            for (auto item : links_)
            {
                event_->DisConnected(item, 0);
                close_socket(item);
            }
            links_.clear();
        }

        bool TCPServer::send_bytes(SOCKET _sock, uint8_t* _data, int _len)
        {
            int has_sent = 0;
            int temp_sent = 0;

            while (has_sent < _len)
            {
                temp_sent = send(_sock, (const char*)(_data + has_sent), _len - has_sent, 0);
                if (temp_sent == SOCKET_ERROR)
                    return false;
                has_sent += temp_sent;
            }

            return true;
        }

        bool TCPServer::Send(SOCKET _sock, uint8_t* _data, int _len)
        {
            if (run_flag_ == false)
                return false;
            if (false == send_bytes(_sock, _data, _len))
            {
                close_socket(_sock);
                return false;
            }
            return true;
        }

        bool TCPServer::Start(ITCPServerEvent* _event, const char* _addr_ip, const int _addr_port)
        {
            if (run_flag_ == true || _event == NULL)
                return false;

            event_ = _event;

            if (false == initialization(_addr_ip, _addr_port))
                return false;

            run_flag_ = true;
            work_thread_ = std::thread(&TCPServer::work_function, this);

            return true;
        }
        void TCPServer::Stop()
        {
            if (run_flag_ == false)
                return;

            run_flag_ = false;
            if (work_thread_.joinable())
            {
                work_thread_.join();
            }
        }
    }
}