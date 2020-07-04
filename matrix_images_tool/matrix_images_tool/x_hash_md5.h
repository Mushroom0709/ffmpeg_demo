#ifndef _X_HASH_MD5_H_
#define _X_HASH_MD5_H_

#include <string.h>

#include <string>

#include <openssl/md5.h>

namespace xM
{
    namespace hash
    {
		class xMD5
		{
#define XM_HASH_MD5_BUFFER_SIZE 16
#define XM_HASH_MD5_STRING_LENGTH 32
		private:
			unsigned char m5_buf_[XM_HASH_MD5_BUFFER_SIZE];
			char m5_str_[XM_HASH_MD5_STRING_LENGTH + 1];

			MD5_CTX m5_ctx_;
		private:
			char sigle_number_to_hex(int _number, bool _big = true);
		public:
			xMD5();
			~xMD5();
		public:
			bool ReInit();

			bool Update(const void* _buf, const size_t _len);

			bool Final(const unsigned char* _buf, const size_t _size);

			bool Final(std::string& _str);
		};
    }
}
#endif // !_X_HASH_MD5_H_