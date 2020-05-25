#ifndef _X_MESSAGE_X264_FRAME_MSG_H_
#define _X_MESSAGE_X264_FRAME_MSG_H_

#include "x_message_serialize.h"
#include "x_message_interface.h"

#include <malloc.h>

#define X_MESSAGE_FRAME_MSG_CHANNEL_MAX_SIZE 8

namespace xM
{
    namespace message
    {
        class X264FrameMsg:
            public IMessage
        {
        public:
            static const uint32_t ID = 0x00002003;
        public:
            int32_t group_size;
            int32_t* i_payload;
            uint8_t** p_payload;
        public:
            X264FrameMsg()
            {
                group_size = 0;
                i_payload = NULL;
                p_payload = NULL;
            }
            ~X264FrameMsg()
            {
                Free();
            }

        public:
            void Alloc(int32_t _size)
            {
                group_size = _size;
                i_payload = (int32_t*)calloc(group_size, sizeof(int32_t));
                p_payload = (uint8_t**)calloc(group_size, sizeof(uint8_t*));
            }
            void Free()
            {
                if (i_payload = NULL)
                {
                    free(i_payload);
                    i_payload = NULL;
                }

                if (p_payload = NULL)
                {
                    free(p_payload);
                    p_payload = NULL;
                }
            }
            uint32_t GetLength()
            {
                uint32_t len = sizeof(int32_t) +
                    (sizeof(int32_t) * group_size);

                for (int32_t i = 0; i < group_size; i++)
                {
                    len += i_payload[i];
                }

                return len;
            }

            template<class Communication = ICommunication>
            bool Encode(Communication* _sock, uint32_t _id, bool _all_flag)
            {
                int len = 0;
                int offset = 0;
                uint8_t buffer[32] = { 0 };

                if (offset = Serialize::IntegerConvertToBytes<int32_t>(group_size, buffer), offset == 0)
                    return false;
                len += offset;

                if (_all_flag == true && false == _sock->ProtocolSendAll(buffer, offset))
                    return false;
                else if (_all_flag == false && false == _sock->ProtocolSend(_id, buffer, offset))
                    return false;


                for (int32_t i = 0; i < group_size; i++)
                {
                    if (offset = Serialize::IntegerConvertToBytes<int32_t>(i_payload[i], buffer), offset == 0)
                        return false;
                    len += offset;


                    if (_all_flag == true && false == _sock->ProtocolSendAll(buffer, offset))
                        return false;
                    else if (_all_flag == false && false == _sock->ProtocolSend(_id, buffer, offset))
                        return false;
                }

                for (int32_t i = 0; i < group_size; i++)
                {
                    if (_all_flag == true && false == _sock->ProtocolSendAll(p_payload[i], i_payload[i]))
                        return false;
                    else if (_all_flag == false && false == _sock->ProtocolSend(_id, p_payload[i], i_payload[i]))
                        return false;

                    len += i_payload[i];
                }

                return true;
            }

            bool Decode(const uint8_t* _buf, const int _len)
            {
                int offset = 0;
                int len = 0;
                uint8_t* buffer = (uint8_t*)_buf;

                //if (offset = Serialize::BytesConvertToInteger<uint8_t>(channel, buffer + len), offset == 0)
                //    return false;
                //len += offset;


                if (offset = Serialize::BytesConvertToInteger<int32_t>(group_size, buffer + len), offset == 0)
                    return false;
                len += offset;

                Alloc(group_size);

                for (int32_t i = 0; i < group_size; i++)
                {
                    if (offset = Serialize::BytesConvertToInteger<int32_t>(i_payload[i], buffer + len), offset == 0)
                        return false;
                    len += offset;
                }

                for (int32_t i = 0; i < group_size; i++)
                {

                    p_payload[i] = buffer + len;
                    len += i_payload[i];
                }

                if (len != _len)
                    return false;

                return true;
            }
        };
    }
}
#endif // _X_MESSAGE_X264_FRAME_MSG_H_