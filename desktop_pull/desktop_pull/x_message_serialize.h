#ifndef _X_MESSAGE_SERIALIZE_H_
#define _X_MESSAGE_SERIALIZE_H_

#include <inttypes.h>

#define X_MESSAGE_BIG_ENDIAN_FLAG 0x01
#define X_MESSAGE_LITTLE_ENDIAN_FLAG 0x00

#define X_MESSAGE_PROTOCOL_HEADER 0x8611
#define X_MESSAGE_PROTOCOL_TAILER 0x83C7

constexpr uint32_t X_MESSAGE_PROTOCOL_LENGTH = (sizeof(uint16_t) + sizeof(uint16_t) + sizeof(uint32_t) + sizeof(uint32_t));
constexpr uint32_t X_MESSAGE_ID_NONE = (~0);

namespace xM
{
	namespace message
	{
		class Serialize
		{
		public:
			template<typename Type>
			static int IntegerConvertToBytes(Type _src, uint8_t* _buf, int _type_size = sizeof(Type), uint8_t _endianness = X_MESSAGE_BIG_ENDIAN_FLAG)
			{
				int use_len = 0;
				switch (_endianness)
				{
				case X_MESSAGE_BIG_ENDIAN_FLAG:
				{
					uint64_t clean_flag = 0xFF;

					for (int i = _type_size - 1; i >= 0; i--)
					{
						_buf[use_len++] = (uint8_t)((_src >> (i * 8)) & 0xFF);
					}
				}
				break;

				case X_MESSAGE_LITTLE_ENDIAN_FLAG:
				{
					for (int i = 0; i < _endianness; i++)
					{
						_buf[use_len++] = (uint8_t)((_src >> (i * 8)) & 0xFF);
					}
				}
				break;

				default:
					// error
					break;
				}

				return use_len;
			}

			template<typename Type>
			static int BytesConvertToInteger(Type& _dst, uint8_t* _buf, int _type_size = sizeof(Type), uint8_t _endianness = X_MESSAGE_BIG_ENDIAN_FLAG)
			{
				int use_len = 0;
				_dst = 0x00;
				Type temp;
				switch (_endianness)
				{
				case X_MESSAGE_BIG_ENDIAN_FLAG:
				{
					for (int i = _type_size - 1; i >= 0; i--)
					{
						temp = _buf[use_len++];
						_dst |= temp << (i * 8);
					}
				}
				break;

				case X_MESSAGE_LITTLE_ENDIAN_FLAG:
				{
					for (int i = 0; i < _endianness; i++)
					{
						temp = _buf[use_len++];
						_dst |= temp << (i * 8);
					}
				}
				break;

				default:
					// error
					break;
				}

				return use_len;
			}
		};
	}
}

#endif // _X_MESSAGE_SERIALIZE_H_