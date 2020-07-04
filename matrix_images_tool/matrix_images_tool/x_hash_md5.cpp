#include "x_hash_md5.h"

namespace xM
{
	namespace hash
	{

		char xMD5::sigle_number_to_hex(int _number, bool _big)
		{
			if (_number > 9)
				return static_cast<char>(_number - 10) + (_big == true ? 'A' : 'a');
			else
				return static_cast<char>(_number) + '0';
		}

		xMD5::xMD5()
		{
			memset(m5_buf_, 0, XM_HASH_MD5_BUFFER_SIZE * sizeof(unsigned char));
			memset(m5_str_, 0, (XM_HASH_MD5_STRING_LENGTH + 1) * sizeof(char));
		}
		xMD5::~xMD5()
		{
			//
		}

		bool xMD5::ReInit()
		{
			if (1 == MD5_Init(&m5_ctx_))
				return true;
			return false;
		}

		bool xMD5::Update(const void* _buf, const size_t _len)
		{
			if (1 == MD5_Update(&m5_ctx_, _buf, _len))
				return true;
			return false;
		}

		bool xMD5::Final(const unsigned char* _buf, const size_t _size)
		{
			if (1 == MD5_Final(const_cast<unsigned char*>(_buf), &m5_ctx_))
				return true;
			return false;
		}

		bool xMD5::Final(std::string& _str)
		{
			if (false == Final(m5_buf_, XM_HASH_MD5_BUFFER_SIZE))
				return false;

			for (int i = 0; i < XM_HASH_MD5_BUFFER_SIZE; i++)
			{
				m5_str_[2*i] = sigle_number_to_hex(m5_buf_[i] % 16);
				m5_str_[2*i + 1] = sigle_number_to_hex(m5_buf_[i] / 16);
			}

			_str = std::string(m5_str_, XM_HASH_MD5_STRING_LENGTH);
			return true;
		}
	}
}