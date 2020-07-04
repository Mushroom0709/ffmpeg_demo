#ifndef _X_IO_DIRECTORY_H_
#define _X_IO_DIRECTORY_H_

#include <Windows.h>

#include <string>
#include <vector>
#include <queue>

namespace xM
{
    namespace io
    {
		class File
		{
		public:
			static bool GetFiles(std::vector<std::string>& _ref_files, std::string _root, std::string _key = "*.*");
		};
		class Directory
		{
		public:
			static bool GetDirectories(std::vector<std::string>& _ref_dirs, std::string _root, std::string _key = "*");
		};

		bool AllDirectoriesAndFiles(
			std::vector<std::string>& _ref_dirs,
			std::vector<std::string>& _ref_files,
			std::string _root,
			std::string _dir_key = "*",
			std::string _file_key = "*.*");

		bool AllFiles(
			std::vector<std::string>& _ref_files,
			std::string _root,
			std::string _file_key = "*.*",
			std::string _dir_key = "*");
    }
}

#endif // !_X_IO_DIRECTORY_H_