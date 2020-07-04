#include "x_io_directory.h"

namespace xM
{
    namespace io
    {
        bool File::GetFiles(std::vector<std::string>& _ref_files, std::string _root, std::string _key)
        {
            HANDLE hfind = NULL;
            WIN32_FIND_DATAA w32_find_data = { 0 };
            _ref_files.clear();
            hfind = FindFirstFileA((_root + _key).c_str(), &w32_find_data);
            if (hfind == NULL)
                return false;

            do
            {
                if (w32_find_data.dwFileAttributes != 0 &&
                    (w32_find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != FILE_ATTRIBUTE_DIRECTORY)
                {
                    _ref_files.push_back(_root + w32_find_data.cFileName);
                }
            } while (FindNextFileA(hfind, &w32_find_data));

            FindClose(hfind);
            return true;
        }

        bool Directory::GetDirectories(std::vector<std::string>& _ref_dirs, std::string _root, std::string _key)
        {
            HANDLE hfind = NULL;
            WIN32_FIND_DATAA w32_find_data = { 0 };
            _ref_dirs.clear();
            hfind = FindFirstFileA((_root + _key).c_str(), &w32_find_data);
            if (hfind == NULL)
                return false;

            do
            {
                if (strcmp(w32_find_data.cFileName, "..") == 0 || strcmp(w32_find_data.cFileName, ".") == 0)
                    continue;

                if ((w32_find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY)
                {
                    _ref_dirs.push_back(_root + w32_find_data.cFileName + "\\");
                }
            } while (FindNextFileA(hfind, &w32_find_data));

            FindClose(hfind);
            return true;
        }

        bool AllDirectoriesAndFiles(
            std::vector<std::string>& _ref_dirs,
            std::vector<std::string>& _ref_files,
            std::string _root,
            std::string _dir_key,
            std::string _file_key)
        {
            std::vector<std::string> temp_list;
            std::queue<std::string> cache;
            cache.push(_root);

            _ref_dirs.clear();
            _ref_files.clear();

            while (cache.empty() == false)
            {
                _root = cache.front();
                cache.pop();

                if (false == xM::io::Directory::GetDirectories(temp_list, _root, _dir_key))
                    return false;

                for (auto& item : temp_list)
                {
                    cache.push(item);
                    _ref_dirs.push_back(item);
                }

                if (false == xM::io::File::GetFiles(temp_list, _root, _file_key))
                    return false;

                for (auto& item : temp_list)
                {
                    _ref_files.push_back(item);
                }
            }

            return true;
        }

        bool AllFiles(
            std::vector<std::string>& _ref_files,
            std::string _root,
            std::string _file_key,
            std::string _dir_key)
        {
            std::vector<std::string> temp_list;
            std::queue<std::string> cache;
            cache.push(_root);

            _ref_files.clear();

            while (cache.empty() == false)
            {
                _root = cache.front();
                cache.pop();

                if (false == xM::io::Directory::GetDirectories(temp_list, _root, _dir_key))
                    return false;

                for (auto& item : temp_list)
                {
                    cache.push(item);
                }

                if (false == xM::io::File::GetFiles(temp_list, _root, _file_key))
                    return false;

                for (auto& item : temp_list)
                {
                    _ref_files.push_back(item);
                }
            }

            return true;
        }
    }
}