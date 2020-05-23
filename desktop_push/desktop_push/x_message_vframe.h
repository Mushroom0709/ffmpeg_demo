#ifndef _X_MESSAGE_VFRAME_MSG_H_
#define _X_MESSAGE_VFRAME_MSG_H_

#include "x_message_serialize.h"
#include "x_message_interface.h"

#include <libavutil/pixfmt.h>

#define X_MESSAGE_FRAME_MSG_CHANNEL_MAX_SIZE 8

namespace xM
{
    namespace message
    {
        class VFrameMsg :
            public IMessage
        {
        private:
            uint8_t channel_;
        public:
            static const uint32_t ID = 0x00002002;
        public:
            int32_t width;
            int32_t height;
            int16_t format;
            int64_t pts;
            int32_t linesize[X_MESSAGE_FRAME_MSG_CHANNEL_MAX_SIZE];
            uint8_t* data[X_MESSAGE_FRAME_MSG_CHANNEL_MAX_SIZE];
        public:
            VFrameMsg()
            {
                channel_ = 0;
                width = 0;
                height = 0;
                format = AV_PIX_FMT_NONE;
                pts = -1;
                memset(linesize, 0, sizeof(int32_t) * X_MESSAGE_FRAME_MSG_CHANNEL_MAX_SIZE);
                memset(data, 0, sizeof(uint8_t*) * X_MESSAGE_FRAME_MSG_CHANNEL_MAX_SIZE);
            }
            ~VFrameMsg()
            {
                //
            }

        public:
            uint32_t GetLength()
            {
                uint32_t len = sizeof(int32_t) +
                    sizeof(int32_t) +
                    sizeof(int16_t) +
                    sizeof(int16_t) +
                    sizeof(int64_t);

                switch ((AVPixelFormat)format)
                {
                case AV_PIX_FMT_YUV420P:
                {
                    channel_ = 3;
                    len += (sizeof(int32_t) * channel_);
                    len += ((width * height)) + ((width * height) / 4) + ((width * height) / 4);
                }
                default:
                    return 0;
                }

                return len;
            }

            template<class Communication = ICommunication>
            bool Encode(Communication* _sock)
            {
                int len = 0;
                int offset = 0;
                uint8_t buffer[64] = { 0 };

                //if (offset = Serialize::IntegerConvertToBytes<uint8_t>(channel_, buffer + len), offset == 0)
                //    return false;
                //len += offset;

                if (offset = Serialize::IntegerConvertToBytes<int32_t>(width, buffer + len), offset == 0)
                    return false;
                len += offset;

                if (offset = Serialize::IntegerConvertToBytes<int32_t>(height, buffer + len), offset == 0)
                    return false;
                len += offset;

                if (offset = Serialize::IntegerConvertToBytes<int16_t>(format, buffer + len), offset == 0)
                    return false;
                len += offset;

                if (offset = Serialize::IntegerConvertToBytes<int64_t>(pts, buffer + len), offset == 0)
                    return false;
                len += offset;


                switch ((AVPixelFormat)format)
                {
                case AV_PIX_FMT_YUV420P:
                {
                    channel_ = 3;

                    for (size_t i = 0; i < channel_; i++)
                    {
                        if (offset = Serialize::IntegerConvertToBytes<int32_t>(linesize[i], buffer + len), offset == 0)
                            return false;
                        len += offset;
                    }

                    if (false == _sock->ProtocolSend(buffer, len))
                        return false;

                    if (false == _sock->ProtocolSend(data[0], width * height))
                        return false;

                    if (false == _sock->ProtocolSend(data[1], width * height / 4))
                        return false;

                    if (false == _sock->ProtocolSend(data[2], width * height / 4))
                        return false;
                }
                break;
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

                if (offset = Serialize::BytesConvertToInteger<int32_t>(width, buffer + len), offset == 0)
                    return false;
                len += offset;

                if (offset = Serialize::BytesConvertToInteger<int32_t>(height, buffer + len), offset == 0)
                    return false;
                len += offset;

                if (offset = Serialize::BytesConvertToInteger<int16_t>(format, buffer + len), offset == 0)
                    return false;
                len += offset;

                if (offset = Serialize::BytesConvertToInteger<int64_t>(pts, buffer + len), offset == 0)
                    return false;
                len += offset;

                switch ((AVPixelFormat)format)
                {
                case AV_PIX_FMT_YUV420P:
                {
                    channel_ = 3;

                    for (size_t i = 0; i < channel_; i++)
                    {
                        if (offset = Serialize::BytesConvertToInteger<int32_t>(linesize[i], buffer + len), offset == 0)
                            return false;
                        len += offset;
                    }

                    data[0] = buffer + len;
                    len += (width * height);

                    data[1] = buffer + len;
                    len += (width * height / 4);

                    data[2] = buffer + len;
                    len += (width * height / 4);
                }
                break;
                }


                if (len != _len)
                    return false;

                return true;
            }
        };
    }
}

#endif // _X_MESSAGE_VFRAME_MSG_H_