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

        PLINK_INFO TCPServer::find_link(SOCKET _link)
        {
            PLINK_INFO info = NULL;
            std::lock_guard<std::mutex> auto_lock(links_lock_);
            auto find_res = links_.find(_link);
            if (find_res != links_.end())
            {
                info = find_res->second;
            }
            return info;
        }
        PLINK_INFO TCPServer::del_link(SOCKET _link)
        {
            PLINK_INFO info = NULL;
            std::lock_guard<std::mutex> auto_lock(links_lock_);
            auto find_res = links_.find(_link);
            if (find_res != links_.end())
            {
                info = find_res->second;
                links_.erase(find_res);
            }
            return info;
        }
        void TCPServer::del_link(PLINK_INFO _info)
        {
            PLINK_INFO info = NULL;
            std::lock_guard<std::mutex> auto_lock(links_lock_);
            if (links_.find(_info->ID) != links_.end())
            {
                links_.erase(_info->ID);
            }
        }
        void TCPServer::add_link(PLINK_INFO _ptr_link)
        {
            std::lock_guard<std::mutex> auto_lock(links_lock_);
            links_.insert(PSocketInfoType(_ptr_link->ID, _ptr_link));
        }

        void TCPServer::close_socket(SOCKET& _sock)
        {
            if (_sock != INVALID_SOCKET)
            {
                ::closesocket(_sock);
                _sock = INVALID_SOCKET;
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

            PLINK_INFO info = (PLINK_INFO)calloc(1, sizeof(LINK_INFO));
            info->ID = stemp;

            info->Params = event_->Connected(info->ID);

            add_link(info);

            return true;
        }
        bool TCPServer::recv_function(SOCKET _sock, char* _buffer, int _buf_size)
        {
            PLINK_INFO info = find_link(_sock);

            if (info != NULL)
            {
                int res = recv(_sock, _buffer, _buf_size, 0);
                if (res <= 0)
                {
                    del_link(info);

                    if (info != NULL)
                    {
                        if (res == 0)
                            event_->DisConnected(info->ID, info->Params, 0);
                        else
                            event_->DisConnected(info->ID, info->Params, GetLastError());
                    }

                    close_socket(info->ID);
                    info->Params = NULL;

                    free(info);
                }
                else
                {
                    event_->Receive(info->ID, info->Params, (uint8_t*)_buffer, res);
                }
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

                links_lock_.lock();
                for (auto& item : links_)
                {
                    FD_SET(item.first, &read_set);
                }
                links_lock_.unlock();

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

            links_lock_.lock();
            close_socket(listen_);
            for (auto& item : links_)
            {
                close_socket(item.second->ID);
                event_->DisConnected(item.second->ID, item.second->Params, 0);
                free(item.second);
            }
            links_.clear();
            links_lock_.unlock();
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
        bool TCPServer::SendAll(uint8_t* _data, int _len)
        {
            if (run_flag_ == false)
                return false;

            std::vector<SOCKET> temp_info;

            links_lock_.lock();
            for (auto& item : links_)
            {
                temp_info.push_back(item.first);
            }
            links_lock_.unlock();

            for (auto& item : temp_info)
            {
                if (false == Send(item, _data, _len))
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