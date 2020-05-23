#ifndef _X_MESSAGE_UNPACKER_H_
#define _X_MESSAGE_UNPACKER_H_

#include <vector>

#include "x_message_serialize.h"

namespace xM
{
	namespace message
	{
        typedef class IUnpackEvent
        {
        public:
            virtual void Package(uint32_t _socket,uint32_t _msg_id, uint8_t* _buffer, size_t _length) {};
        }*PtrIUnpackEvent;

        class Unpacker
        {
        private:
            PtrIUnpackEvent event_;

            std::vector<uint8_t> cache_;
            int buf_status_;
            uint32_t msg_len_;
            uint16_t u16_value_;
            uint32_t u32_value_;
        public:
            Unpacker()
            {
                event_ = NULL;
                cache_.clear();
                buf_status_ = 1;
                msg_len_ = 0;
                u16_value_ = 0;
                u32_value_ = 0;
            }
            Unpacker(PtrIUnpackEvent _event)
            {
                event_ = _event;
                cache_.clear();
                buf_status_ = 1;
                msg_len_ = 0;
                u16_value_ = 0;
                u32_value_ = 0;
            }
            ~Unpacker()
            {
                cache_.clear();
            }
            void SetEvent(PtrIUnpackEvent _event)
            {
                event_ = _event;
            }
        public:
            void UpdateBuffer(const uint32_t _id, const uint8_t* _buffer, const size_t _buf_len)
            {
                cache_.insert(cache_.end(), _buffer, _buffer + _buf_len);

                while (true)
                {
                    switch (buf_status_)
                    {
                    case 1: //找头
                    {
                        if (cache_.size() >= 2)
                        {
                            if (0 != Serialize::BytesConvertToInteger<uint16_t>(u16_value_,&cache_[0]) && u16_value_ == X_MESSAGE_PROTOCOL_HEADER)
                            {
                                buf_status_ = 2;
                            }
                            else
                            {
                                cache_.erase(cache_.begin());
                            }
                        }
                        else
                        {
                            return;
                        }
                    }
                    break;

                    case 2: //找长度
                    {
                        if (cache_.size() >= 6)
                        {
                            if(0 != Serialize::BytesConvertToInteger<uint32_t>(msg_len_, &cache_[2]))
                            {
                                msg_len_ += X_MESSAGE_PROTOCOL_LENGTH;
                                buf_status_ = 3;
                            }
                            else
                            {
                                cache_.erase(cache_.begin());
                                buf_status_ = 1;
                            }
                        }
                        else
                        {
                            return;
                        }
                    }
                    break;

                    case 3: //等长度 验证尾巴
                    {
                        if (cache_.size() >= msg_len_)
                        {
                            if (0 != Serialize::BytesConvertToInteger<uint16_t>(u16_value_, &cache_[msg_len_ - 2]) && u16_value_ == X_MESSAGE_PROTOCOL_TAILER)
                            {
                                buf_status_ = 4;
                            }
                            else
                            {
                                cache_.erase(cache_.begin(), cache_.begin() + 2);
                                buf_status_ = 1;
                            }
                        }
                        else
                        {
                            return;
                        }
                    }
                    break;

                    case 4: //等待操作，清空
                    {
                        
                        if (0 != Serialize::BytesConvertToInteger<uint32_t>(u32_value_, &cache_[6]) && event_ != NULL)
                        {
                            event_->Package(_id, u32_value_, &cache_[0], msg_len_);
                        }

                        cache_.erase(cache_.begin(), cache_.begin() + msg_len_);
                        msg_len_ = 0;
                        buf_status_ = 1;
                    }
                    break;

                    default:
                        break;
                    }
                }
            }
        };
	}
}

#endif // _X_MESSAGE_UNPACKER_H_
