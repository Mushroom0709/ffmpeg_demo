#ifndef _X_MESSAGE_INTERFACE_H_
#define _X_MESSAGE_INTERFACE_H_

#include <inttypes.h>

namespace xM
{
	namespace message
	{
		class ICommunication
		{
		public:
			virtual bool ProtocolSend(const uint8_t* _buf, const int _len) { return false; }
		};

		class IMessage
		{
		public:
			virtual bool Decode(const uint8_t* _buf, const int _len) { return false; }
			template<class Communication = ICommunication>
			bool Encode(Communication* _sock) { return false; }
			virtual uint32_t GetLength() { return 0; }
		};
	}
}

#endif // _X_MESSAGE_INTERFACE_H_