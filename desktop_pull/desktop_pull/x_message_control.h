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
			static const uint16_t XM_CONTROL_COMMAND_START_PUSH = 0x1001;		//开始推流
			static const uint16_t XM_CONTROL_COMMAND_STOP_PUSH = 0x1002;		//停止推流
			static const uint16_t XM_CONTROL_COMMAND_REQ_SCREEN_SIZE = 0x2001;	//请求流视频尺寸
			static const uint16_t XM_CONTROL_COMMAND_RSP_SCREEN_SIZE = 0x2002;	//返回流视频尺寸
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
			bool Encode(Communication* _sock, uint32_t _id, bool _all_flag)
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

				if (_all_flag == true && false == _sock->ProtocolSendAll(buffer, len))
					return false;
				else if (_all_flag == false && false == _sock->ProtocolSend(_id, buffer, len))
					return false;

				if (_all_flag == true && false == _sock->ProtocolSendAll(Info, InfoLength))
					return false;
				else if (_all_flag == false && false == _sock->ProtocolSend(_id, Info, InfoLength))
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