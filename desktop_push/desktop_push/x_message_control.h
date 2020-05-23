#ifndef _X_MESSAGE_CONTROL_H_
#define _X_MESSAGE_CONTROL_H_

#include "x_message_serialize.h"
#include "x_message_protocol.h"

#define X_MESSAGE_CONTROL_INFO_SIZE_MAX 0xFFFF

namespace xM
{
	namespace message
	{
		class ControlMsg :
			public IMessage

		{
		public:
			static const uint32_t ID = 0x00001001;
		public:
			static const uint16_t XM_CONTROL_COMMAND_NONE = 0x1000;
			static const uint16_t XM_CONTROL_COMMAND_START_PUSH = 0x1001;
			static const uint16_t XM_CONTROL_COMMAND_STOP_PUSH = 0x1002;
		public:
			uint16_t Command;
			uint16_t InfoLength;
			uint8_t* Info;
		public:
			ControlMsg()
			{
				Command = XM_CONTROL_COMMAND_NONE;
				InfoLength = 0;
				Info = NULL;
			}
			~ControlMsg()
			{

			}
		public:
			uint32_t GetLength()
			{
				return sizeof(uint16_t) + sizeof(uint16_t) + InfoLength;
				return 0;
			}

			template<class Communication = ICommunication>
			bool Encode(Communication* _sock)
			{
				int len = 0;
				int offset = 0;
				uint8_t buffer[8] = { 0 };

				if (offset = Serialize::IntegerConvertToBytes<uint16_t>(Command, buffer + len), offset == 0)
					return false;
				len += offset;

				if (offset = Serialize::IntegerConvertToBytes<uint16_t>(InfoLength, buffer + len), offset == 0)
					return false;
				len += offset;

				if (false == _sock->ProtocolSend(buffer, len))
					return false;

				if (false == _sock->ProtocolSend(Info, InfoLength))
					return false;

				return true;
			}

			bool Decode(const uint8_t* _buf, const int _len)
			{
				int len = 0;
				int offset = 0;
				uint8_t* buffer = (uint8_t*)_buf;

				if (offset = Serialize::BytesConvertToInteger<uint16_t>(Command, buffer + len), offset == 0)
					return false;
				len += offset;

				if (offset = Serialize::BytesConvertToInteger<uint16_t>(InfoLength, buffer + len), offset == 0)
					return false;
				len += offset;

				Info = buffer + len;

				return true;
			}
		};
	}
}

#endif // _X_MESSAGE_CONTROL_H_