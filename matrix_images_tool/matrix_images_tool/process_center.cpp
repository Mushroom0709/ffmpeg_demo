#include "process_center.h"


ProcessCenter::ProcessCenter()
{

}
ProcessCenter::~ProcessCenter()
{
    db_.Close();
}

bool ProcessCenter::load_logo(cv::Mat& _dst,std::string _path, int _squart)
{
    cv::Mat lg = cv::imread(_path);
    if (lg.empty() == true)
    {
        return false;
    }
    int dst_cols = static_cast<int>(_squart * 0.40f);
    int dst_rows = static_cast<int>((dst_cols * 1.0f / lg.cols * 1.0f) * lg.rows);
    cv::resize(lg, _dst, cv::Size2i(dst_cols, dst_rows));
    return true;
}
time_t ProcessCenter::get_current_system_time()
{
    return std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
}
bool ProcessCenter::check_database_for_the_image(const char* _md5_str)
{
    char sql_str[128] = { 0 };
    sprintf_s(sql_str, 128, "SELECT id,img_path FROM image_info WHERE img_md5 = '%s';", _md5_str);

    xM::db::xSqlite3Result db_res;
    if (false == db_.Query(sql_str, db_res))
        return false;

    if (db_res.size() > XM_DB_SQLITYE3_RESULT_ROW_FRIST_INDEX)
        return false;

    return true;
}
bool ProcessCenter::add_to_database(cv::Mat& _image, std::string& _md5_str, std::string _path)
{
    char sql_str[256] = { 0 };

    cv::Vec<double, 3> avg_color;

    if (false == xM::image::xCvHelper::calculate_image_average_color(_image, avg_color))
        return false;

    sprintf_s(sql_str, 256,
        "INSERT INTO image_info(img_path, img_md5, width, height, channel_key_1, channel_key_2, channel_key_3) \
                VALUES('%s', '%s', %d, %d, %d, %d, %d);",
        _path.c_str(),
        _md5_str.c_str(),
        _image.cols, _image.rows,
        static_cast<int>(avg_color[0]),
        static_cast<int>(avg_color[1]),
        static_cast<int>(avg_color[2]));

    if (false == db_.ExecuteNonQuery(sql_str))
        return false;

    return true;
}
bool ProcessCenter::clear_database()
{
    return db_.ExecuteNonQuery("DELETE FROM image_info;");
}
bool ProcessCenter::add_to_database_and_save_to_file(cv::Mat& _image, std::string& _md5_str, std::string _path)
{
    char sql_str[256] = { 0 };

    cv::Vec<double, 3> avg_color;

    if (false == xM::image::xCvHelper::calculate_image_average_color(_image, avg_color))
        return false;

    sprintf_s(sql_str, 256,
        "INSERT INTO image_info(img_path, img_md5, width, height, channel_key_1, channel_key_2, channel_key_3) \
                VALUES('%s', '%s', %d, %d, %d, %d, %d);",
        _path.c_str(),
        _md5_str.c_str(),
        _image.cols, _image.rows,
        static_cast<int>(avg_color[0]),
        static_cast<int>(avg_color[1]),
        static_cast<int>(avg_color[2]));

    if (false == cv::imwrite(_path, _image))
        return false;

    if (false == db_.ExecuteNonQuery(sql_str))
        return false;

    return true;
}
bool ProcessCenter::select_thumb_of_use_flag(int _channel_1, int _channel_2, int _channel_3, int& _id, std::string& _path, int& _flag)
{
    char sql_str[256] = { 0 };
    sprintf_s(sql_str, 256,
        "SELECT id,img_path,used_flag FROM image_info WHERE (used_flag = %d) "
        "ORDER BY "
        "used_flag ASC, (ABS(%d - channel_key_1) + ABS(%d - channel_key_2) + ABS(%d - channel_key_3)) "
        "ASC LIMIT 1;",
        _flag,
        _channel_1, _channel_2, _channel_3);

    xM::db::xSqlite3Result db_res;
    if (false == db_.Query(sql_str, db_res))
        return false;

    if (db_res.size() <= XM_DB_SQLITYE3_RESULT_ROW_FRIST_INDEX)
        return false;

    _id = atoi(db_res[XM_DB_SQLITYE3_RESULT_ROW_FRIST_INDEX][0].c_str());
    _path = db_res[XM_DB_SQLITYE3_RESULT_ROW_FRIST_INDEX][1];
    _flag = atoi(db_res[XM_DB_SQLITYE3_RESULT_ROW_FRIST_INDEX][2].c_str());

    return true;
}
bool ProcessCenter::select_thumb_of_offset(int _channel_1, int _channel_2, int _channel_3, int _offset, int use_flag, int& _id, std::string& _path)
{
    char sql_str[512] = { 0 };
    sprintf_s(sql_str, 512,
        "SELECT id,img_path FROM image_info "
        "WHERE(used_flag = %d) AND "
        "((ABS(%d - channel_key_1) + ABS(%d - channel_key_2) + ABS(%d - channel_key_3)) < %d) "
        "ORDER BY "
        "(ABS(%d - channel_key_1) + ABS(%d - channel_key_2) + ABS(%d - channel_key_3)) "
        "ASC LIMIT 1; ",
        use_flag,
        _channel_1, _channel_2, _channel_3,
        _offset,
        _channel_1, _channel_2, _channel_3);

    xM::db::xSqlite3Result db_res;
    if (false == db_.Query(sql_str, db_res))
        return false;

    if (db_res.size() <= XM_DB_SQLITYE3_RESULT_ROW_FRIST_INDEX)
        return false;

    _id = atoi(db_res[XM_DB_SQLITYE3_RESULT_ROW_FRIST_INDEX][0].c_str());
    _path = db_res[XM_DB_SQLITYE3_RESULT_ROW_FRIST_INDEX][1];

    return true;
}
bool ProcessCenter::select_thumb(int _channel_1, int _channel_2, int _channel_3,int& _id, std::string& _path, int use_flag)
{
    bool flag = true;

    //if (flag == true)
    //	flag = !select_thumb_of_offset(_channel_1, _channel_2, _channel_3, 60, 0, _id, _path);

    //if (flag == true)
    //	flag = !select_thumb_of_offset(_channel_1, _channel_2, _channel_3, 60, 1, _id, _path);

    if (flag == true)
        flag = !select_thumb_of_use_flag(_channel_1, _channel_2, _channel_3, _id, _path, use_flag);

    return !flag;
}
bool ProcessCenter::set_thumb_use_flag(int _id, int _flag, bool _all)
{
    char sql_str[64] = { 0 };
    char* err_msg = NULL;

    if (_all == false)
    {
        sprintf_s(sql_str, 64,
            "UPDATE image_info SET used_flag = %d WHERE id = %d;",
            _flag,
            _id);
    }
    else
    {
        sprintf_s(sql_str, 64,
            "UPDATE image_info SET used_flag = %d;",
            _flag);
    }

    return db_.ExecuteNonQuery(sql_str);
}
bool ProcessCenter::init_database()
{
    if (false == db_.Open(PROCESS_CENTER_DATABASE_NAME))
        return false;

    const char* create_sql =
        "CREATE TABLE IF NOT EXISTS image_info( \
                id             INTEGER        PRIMARY KEY  AUTOINCREMENT, \
                img_path       TEXT           NOT NULL, \
                img_md5        TEXT           NOT NULL, \
                width          INT            NULL, \
                height         INT            NULL, \
                channel_key_1  DECIMAL(10, 3)  DEFAULT 0.0, \
                channel_key_2  DECIMAL(10, 3)  DEFAULT 0.0, \
                channel_key_3  DECIMAL(10, 3)  DEFAULT 0.0, \
                channel_key_4  DECIMAL(10, 3)  DEFAULT 0.0, \
                channel_key_5  DECIMAL(10, 3)  DEFAULT 0.0, \
                channel_key_6  DECIMAL(10, 3)  DEFAULT 0.0, \
                channel_key_7  DECIMAL(10, 3)  DEFAULT 0.0, \
                channel_key_8  DECIMAL(10, 3)  DEFAULT 0.0, \
                used_flag      INT            DEFAULT 0\
                ); ";

    if (false == db_.ExecuteNonQuery(create_sql))
        return false;

    return true;
}
bool ProcessCenter::init_directory()
{
    CreateDirectoryA(PROCESS_CENTER_THUBM_ROOT_DIRECTORY, NULL);

    return true;
}

bool ProcessCenter::ProcessCenter::Init()
{
    if (false == init_database())
        return false;

    if (false == init_directory())
        return false;

    return true;
}

bool ProcessCenter::LoadingImageLibrary(const char* _src_img_root, int _squart)
{
    std::vector<std::string> paths;
    if (false == xM::io::AllFiles(paths, _src_img_root, "*.jpg"))
        return false;

    time_t now_time = get_current_system_time();
    char file_path[256] = { 0 };

    int path_index = 0;

    for (std::string& path : paths)
    {
        path_index++;
        cv::Mat src_img = cv::imread(path, cv::IMREAD_COLOR);
        if (src_img.empty() == false && src_img.rows > 1000 && src_img.rows > 1000)
        {
            cv::Mat dst_img;
            if (false == xM::image::xCvHelper::create_fuzzy_square_background(src_img, dst_img, _squart))
            {
                xErrorPrintln("[ProcessCenter] [LoadingImageLibrary] [%.2f%%] [attempt to add blur backgroud image to the image %d faild:%s]", (path_index * 100.0f / (paths.size() - 1)), path_index, path.c_str());
            }
            else
            {
                std::string md5_str;
                md5_.ReInit();
                md5_.Update(dst_img.data, dst_img.cols * dst_img.rows * dst_img.elemSize());
                md5_.Final(md5_str);

                if (true == check_database_for_the_image(md5_str.c_str()))
                {

                    sprintf_s(file_path, 256, "%s%lld%06d.jpg", PROCESS_CENTER_THUBM_ROOT_DIRECTORY, now_time, path_index);
                    if (false == add_to_database(dst_img, md5_str, file_path))
                    {
                        xErrorPrintln("[ProcessCenter] [LoadingImageLibrary] [%.2f%%] [add thubm to database faild:%s]", (path_index * 100.0f / (paths.size() - 1)), path.c_str());
                    }
                    else
                    {
                        xInfoPrintln("[ProcessCenter] [LoadingImageLibrary] [%.2f%%] [add image %d to the database successful:%s]", (path_index * 100.0f / (paths.size() - 1)), path_index, file_path);
                    }
                }
                else
                {
                    xInfoPrintln("[ProcessCenter] [LoadingImageLibrary] [%.2f%%] [The image %d already exists in the database:%s]", (path_index * 100.0f / (paths.size() - 1)), path_index, path.c_str());
                }
            }
        }
        else
        {
            xErrorPrintln("[ProcessCenter] [LoadingImageLibrary] [%.2f%%] [read the %d image faild:%s]", (path_index * 100.0f / (paths.size() - 1)), path_index, path.c_str());
        }
    }
    return true;
}
bool ProcessCenter::ReLoadingImageLibraryChannel()
{
    std::vector<std::string> paths;
    if (false == xM::io::AllFiles(paths, PROCESS_CENTER_THUBM_ROOT_DIRECTORY, "*.jpg"))
        return false;

    time_t now_time = get_current_system_time();
    char file_path[256] = { 0 };

    int path_index = 0;

    for (std::string& path : paths)
    {
        path_index++;
        cv::Mat src_img = cv::imread(path, cv::IMREAD_COLOR);
        if (src_img.empty() == false && src_img.rows == src_img.rows)
        {
            std::string md5_str;
            md5_.ReInit();
            md5_.Update(src_img.data, src_img.cols * src_img.rows * src_img.elemSize());
            md5_.Final(md5_str);

            if (true == check_database_for_the_image(md5_str.c_str()))
            {
                sprintf_s(file_path, 256, "%s%lld%06d.jpg", PROCESS_CENTER_THUBM_ROOT_DIRECTORY, now_time, path_index);
                if (false == add_to_database_and_save_to_file(src_img, md5_str, file_path))
                {
                    xErrorPrintln("[ProcessCenter] [LoadingImageLibrary] [%.2f%%] [add thubm to database faild:%s]", (path_index * 100.0f / (paths.size() - 1)), path.c_str());
                }
                else
                {
                    xInfoPrintln("[ProcessCenter] [LoadingImageLibrary] [%.2f%%] [add image %d to the database successful:%s]", (path_index * 100.0f / (paths.size() - 1)), path_index, file_path);
                }
            }
            else
            {
                xInfoPrintln("[ProcessCenter] [LoadingImageLibrary] [%.2f%%] [The image %d already exists in the database:%s]", (path_index * 100.0f / (paths.size() - 1)), path_index, path.c_str());
            }
        }
        else
        {
            xErrorPrintln("[ProcessCenter] [LoadingImageLibrary] [%.2f%%] [read the %d image faild:%s]", (path_index * 100.0f / (paths.size() - 1)), path_index, path.c_str());
        }
    }
    return true;
}

bool ProcessCenter::MakeThumbMatrix(const char* _src_img_root, const char* _dst_file_name,
    int _matrix_w,
    int _matrix_h,
    int _squart,
    std::string _logo)
{
    cv::Mat lg;
    cv::Mat mlg;
    if (load_logo(lg, _logo, _squart) == false)
        return false;

    cv::Mat src_img = cv::imread(_src_img_root, cv::IMREAD_COLOR);
    if (src_img.empty())
    {
        return false;
    }

    if (_matrix_w == -1 && _matrix_h == -1)
    {
        if (src_img.cols <= src_img.rows)
        {
            _matrix_w = PROCESS_CENTER_MATRIX_SQUARE_IMAGE_DEFAULT_LENGTH;
            _matrix_h = static_cast<int>((_matrix_w * 1.0) / (src_img.cols * 1.0) * src_img.rows);
        }
        else
        {
            _matrix_h = PROCESS_CENTER_MATRIX_SQUARE_IMAGE_DEFAULT_LENGTH;
            _matrix_w = static_cast<int>((_matrix_h * 1.0) / (src_img.rows * 1.0) * src_img.cols);
        }
    }
    else if (_matrix_w == -1)
    {
        _matrix_w = static_cast<int>((_matrix_h * 1.0) / (src_img.rows * 1.0) * src_img.cols);
    }
    else if (_matrix_h == -1)
    {
        _matrix_h = static_cast<int>((_matrix_w * 1.0) / (src_img.cols * 1.0) * src_img.rows);
    }

    std::string file_path;
    int thbumb_id = 0;
    int used_flag = 0;

    set_thumb_use_flag(0, 0, true);

    cv::Mat resize_img;
    cv::resize(src_img, resize_img, cv::Size2i(_matrix_w, _matrix_h));

    cv::Mat dst_img(cv::Size2i(_matrix_w * _squart, _matrix_h * _squart), CV_8UC3);
    for (int i = 0; i < resize_img.rows; i++)
    {
        for (int j = 0; j < resize_img.cols; j++)
        {
            if (false == select_thumb(
                resize_img.at<cv::Vec3b>(i, j)[0],
                resize_img.at<cv::Vec3b>(i, j)[1],
                resize_img.at<cv::Vec3b>(i, j)[2],
                thbumb_id,
                file_path,
                used_flag
            ))
            {
                xErrorPrintln("[ProcessCenter] [MakeThumbMatrix][select thubm fail:%d,%d,%d]",
                    resize_img.at<cv::Vec3b>(i, j)[0],
                    resize_img.at<cv::Vec3b>(i, j)[1],
                    resize_img.at<cv::Vec3b>(i, j)[2]);
                return false;
            }

            set_thumb_use_flag(thbumb_id, used_flag + 1);

            cv::Mat thumb = cv::imread(file_path, cv::IMREAD_COLOR);
            if (thumb.empty())
            {
                xErrorPrintln("[ProcessCenter] [MakeThumbMatrix][read thubm fail:%s]", file_path.c_str());
                return false;
            }

            if (thumb.cols != _squart || thumb.rows != _squart)
            {
                cv::Mat thumb_resize;
                cv::resize(thumb, thumb_resize, cv::Size2i(_squart, _squart));
                thumb = thumb_resize;
            }


            cv::Mat thumb_roi = thumb(cv::Rect(0, 0, lg.cols, lg.rows));
            cv::addWeighted(thumb_roi, 1.0, lg, 0.5, 0, thumb_roi);
        
            //cv::Rect copy_rect(i * _squart, j * _squart, _squart, _squart);
            thumb.copyTo(dst_img(cv::Rect(j * _squart, i * _squart, _squart, _squart)));

            xInfoPrintln("[ProcessCenter] [MakeThumbMatrix] [%.2f%%] [copy thumb to the matrix successful:%s]", ((i * _matrix_w + j) * 100.0 / (_matrix_w * _matrix_h - 1)), file_path.c_str());
        }
    }

    if (false == cv::imwrite(_dst_file_name, dst_img))
    {
        xErrorPrintln("[ProcessCenter] [MakeThumbMatrix][svae matrix fail:%s]", _dst_file_name);
        return false;
    }

    return true;
}
