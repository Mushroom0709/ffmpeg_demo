#ifndef _X_MESSAGE_PKT_MSG_H_
#define _X_MESSAGE_PKT_MSG_H_

#include "x_message_serialize.h"
#include "x_message_interface.h"

/*
|***********************************************|
|name       |type       |default    |size       |
|***********************************************|
|pts        |int64      |           |8          |
|-----------------------------------------------|
|dts        |int64      |           |8          |
|-----------------------------------------------|
|side_type  |uint8      |           |1  |
|-----------------------------------------------|
|side_size  |int32      |           |4          |
|-----------------------------------------------|
|size       |int32      |           |4          |
|-----------------------------------------------|
|side_data  |uint8[]    |           |side_size  |
|-----------------------------------------------|
|data       |uint8[]    |           |size       |
|-----------------------------------------------|
*/

namespace xM
{
    namespace message
    {
		class PacketMsg:
			public IMessage
		{
		public:
			static const uint32_t ID = 0x00002001;
		public:
			int64_t pts;
			int64_t dts;
			uint8_t side_type;
			int32_t side_size;
			int32_t size;
			uint8_t* side_data;
			uint8_t* data;
		public:
			PacketMsg()
			{
				pts = -1;
				dts = -1;
				side_type = 0xFF;
				side_size = 0;
				size = 0;

				side_data = NULL;
				data = NULL;
			}
			~PacketMsg()
			{
				//
			}

		public:
			uint32_t GetLength()
			{
				return sizeof(int64_t) + 
					sizeof(int64_t) + 
					sizeof(uint8_t) + 
					sizeof(int32_t) +
					sizeof(int32_t) +
					static_cast<uint32_t>(side_size) +
					static_cast<uint32_t>(size);
			}

			template<class Communication = ICommunication>
			bool Encode(Communication* _sock)
			{
				int len = 0;
				int offset = 0;
				uint8_t buffer[32] = { 0 };

				if (offset = Serialize::IntegerConvertToBytes<int64_t>(pts, buffer + len), offset == 0)
					return false;
				len += offset;

				if (offset = Serialize::IntegerConvertToBytes<int64_t>(dts, buffer + len), offset == 0)
					return false;
				len += offset;

				if (offset = Serialize::IntegerConvertToBytes<uint8_t>(side_type, buffer + len), offset == 0)
					return false;
				len += offset;

				if (offset = Serialize::IntegerConvertToBytes<int32_t>(side_size, buffer + len), offset == 0)
					return false;
				len += offset;

				if (offset = Serialize::IntegerConvertToBytes<int32_t>(size, buffer + len), offset == 0)
					return false;
				len += offset;

				if (false == _sock->ProtocolSend(buffer, len))
					return false;

				if (false == _sock->ProtocolSend(side_data, side_size))
					return false;

				if (false == _sock->ProtocolSend(data, size))
					return false;

				return true;
			}

			bool Decode(const uint8_t* _buf, const int _len)
			{
				int offset = 0;
				int len = 0;
				uint8_t* buffer = (uint8_t*)_buf;

				if (offset = Serialize::BytesConvertToInteger<int64_t>(pts, buffer + len), offset == 0)
					return false;
				len += offset;

				if (offset = Serialize::BytesConvertToInteger<int64_t>(dts, buffer + len), offset == 0)
					return false;
				len += offset;

				if (offset = Serialize::BytesConvertToInteger<uint8_t>(side_type, buffer + len), offset == 0)
					return false;
				len += offset;

				if (offset = Serialize::BytesConvertToInteger<int32_t>(side_size, buffer + len), offset == 0)
					return false;
				len += offset;

				if (offset = Serialize::BytesConvertToInteger<int32_t>(size, buffer + len), offset == 0)
					return false;
				len += offset;

				side_data = buffer + len;
				len += side_size;

				data = buffer + len;
				len += size;

				if (len != _len)
					return false;

				return true;
			}
		};
    }
}

#endif // _X_MESSAGE_PKT_MSG_H_