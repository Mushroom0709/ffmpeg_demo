#ifndef _PROCESS_CENTER_H_
#define _PROCESS_CENTER_H_

#include "x_io_directory.h"
#include "x_hash_md5.h"
#include "x_db_sqlite3.h"
#include "x_image_process.h"

class ProcessCenter
{
#define PROCESS_CENTER_TARGET_SQUARE_IMAGE_DEFAULT_LENGTH 640
#define PROCESS_CENTER_MATRIX_SQUARE_IMAGE_DEFAULT_LENGTH 32
#define PROCESS_CENTER_DATABASE_NAME "imgdb.db"
#define PROCESS_CENTER_THUBM_ROOT_DIRECTORY ".\\thubm\\"

private:
	xM::db::xSqlite3 db_;
	xM::hash::xMD5 md5_;
public:
	ProcessCenter();
	~ProcessCenter();
private:
	time_t get_current_system_time();
	bool check_database_for_the_image(const char* _md5_str);
	bool add_to_database(cv::Mat& _image, std::string& _md5_str, std::string _path);
	bool clear_database();
	bool add_to_database_and_save_to_file(cv::Mat& _image, std::string& _md5_str, std::string _path);
	bool select_thumb_of_use_flag(int _channel_1, int _channel_2, int _channel_3, int _flag, int& _id, std::string& _path);
	bool select_thumb_of_offset(int _channel_1, int _channel_2, int _channel_3, int _offset, int use_flag, int& _id, std::string& _path);
	bool select_thumb(int _channel_1, int _channel_2, int _channel_3, int& _id, std::string& _path);
	bool set_thumb_use_flag(int _id, int _flag, bool _all = false);
	bool init_database();
	bool init_directory();
public:
	bool Init();
public:
	bool LoadingImageLibrary(const char* _src_img_root, int _squart = PROCESS_CENTER_TARGET_SQUARE_IMAGE_DEFAULT_LENGTH);
	bool ReLoadingImageLibraryChannel();

	bool MakeThumbMatrix(const char* _src_img_root, const char* _dst_file_name,
		int _matrix_w = -1,
		int _matrix_h = -1,
		int _squart = PROCESS_CENTER_TARGET_SQUARE_IMAGE_DEFAULT_LENGTH);
};

#endif // !_PROCESS_CENTER_H_