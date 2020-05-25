#ifndef _X_MESSAGE_PROTOCOL_H_
#define _X_MESSAGE_PROTOCOL_H_

#include "x_message_serialize.h"
#include "x_message_interface.h"

/*
|***********************************************|
|name       |type       |default    |size       |
|***********************************************|
|header     |uint16     |0x8611     |2          |
|-----------------------------------------------|
|length     |uint32     |           |4          |
|-----------------------------------------------|
|data       |uint8[]    |           |length     |
|-----------------------------------------------|
|tailer     |uint16     |0x83C7     |2          |
|-----------------------------------------------|
*/

namespace xM
{
    namespace message
    {
		template<class Communication = ICommunication, class Message = IMessage>
		class Protocol
		{
		public:
			Message Msg;
			uint32_t Length;
			uint32_t Command;
		public:
			Protocol()
			{
				Length = 0;
				Command = X_MESSAGE_ID_NONE;
			}
			~Protocol()
			{
				//
			}
		public:
			bool Encode(Communication* sock_, uint32_t _id = ~0, bool _all_flag = true)
			{
				int len = 0;
				int offset = 0;
				uint8_t buffer[64] = { 0 };

				Command = Msg.ID;
				Length = Msg.GetLength();

				if (offset = Serialize::IntegerConvertToBytes<uint16_t>(X_MESSAGE_PROTOCOL_HEADER, buffer + len), offset == 0)
					return false;
				len += offset;

				if (offset = Serialize::IntegerConvertToBytes<uint32_t>(Length, buffer + len), offset == 0)
					return false;
				len += offset;

				if (offset = Serialize::IntegerConvertToBytes<uint32_t>(Command, buffer + len), offset == 0)
					return false;
				len += offset;

				if (_all_flag == true && false == sock_->ProtocolSendAll(buffer, len))
					return false;
				else if (_all_flag == false && false == sock_->ProtocolSend(_id, buffer, len))
					return false;

				if (false == Msg.Encode<Communication>(sock_, _id, _all_flag))
					return false;

				if (offset = Serialize::IntegerConvertToBytes<uint16_t>(X_MESSAGE_PROTOCOL_TAILER, buffer), offset == 0)
					return false;

				if (_all_flag == true && false == sock_->ProtocolSendAll(buffer, offset))
					return false;
				else if (_all_flag == false && false == sock_->ProtocolSend(_id, buffer, offset))
					return false;

				return true;
			}
			bool Decode(const uint8_t* _buf, const int _len)
			{
				uint16_t temp;
				int offset = 0;
				int len = 0;
				uint8_t* buffer = (uint8_t*)_buf;

				if (offset = Serialize::BytesConvertToInteger<uint16_t>(temp, buffer + len), offset == 0 || temp != X_MESSAGE_PROTOCOL_HEADER)
					return false;
				len += offset;

				if (offset = Serialize::BytesConvertToInteger<uint32_t>(Length, buffer + len), offset == 0 || (Length + X_MESSAGE_PROTOCOL_LENGTH) != _len)
					return false;
				len += offset;

				if (offset = Serialize::BytesConvertToInteger<uint32_t>(Command, buffer + len), offset == 0)
					return false;
				len += offset;

				if (false == Msg.Decode(buffer + len, Length))
					return false;
				len += Length;

				if (offset = Serialize::BytesConvertToInteger<uint16_t>(temp, buffer + len), offset == 0 || temp != X_MESSAGE_PROTOCOL_TAILER)
					return false;
				len += offset;

				if (len != _len)
					return false;

				return true;
			}
		};
    }
}

#endif // _X_MESSAGE_PROTOCOL_H_