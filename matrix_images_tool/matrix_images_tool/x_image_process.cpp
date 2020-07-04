#include "x_image_process.h"

namespace xM
{
    namespace image
    {
        bool xCvHelper::convert_to_fuzzy_square_img(cv::Mat& _src, cv::Mat& _dst, const int _square_len, const cv::Point2i _blur_point)
        {
            if (_square_len < 1)
                return false;

            if (_src.empty())
                return false;

            cv::Size2i dst_size(_square_len, _square_len);

            if (_src.cols != _src.rows)
            {
                cv::Rect2i dst_rect;
                if (_src.cols > _src.rows)
                {
                    dst_rect.width = _src.rows;
                    dst_rect.height = _src.rows;
                    dst_rect.y = 0;
                    dst_rect.x = (_src.cols - _src.rows) / 2;
                }
                else
                {
                    dst_rect.width = _src.cols;
                    dst_rect.height = _src.cols;
                    dst_rect.y = (_src.rows - _src.cols) / 2;
                    dst_rect.x = 0;
                }

                _dst = _src(dst_rect);
            }
            else
            {
                _dst = _src;
            }

            cv::Mat square_img(dst_size, CV_8UC3);
            cv::resize(_dst, square_img, dst_size);

            _dst = cv::Mat(dst_size, CV_8UC3);
            cv::blur(square_img, _dst, _blur_point);

            return true;
        }
        bool xCvHelper::convert_to_fuzzy_square_img(cv::Mat& _src, cv::Mat& _dst, const int _square_len, const int _blur_x, const int _blur_y)
        {
            return convert_to_fuzzy_square_img(_src, _dst, _square_len,
                cv::Point2i(_blur_x == 0 ? _square_len / XM_IMAGE_CVHELPER_BLUR_DEFAULT_COEFFICIENT : _blur_x,
                    _blur_y == 0 ? _square_len / XM_IMAGE_CVHELPER_BLUR_DEFAULT_COEFFICIENT : _blur_y));
        }
        bool xCvHelper::scale_to_square_inset(cv::Mat& _src, cv::Mat& _dst, const int _square_len)
        {
            if (_square_len < 1)
                return false;

            if (_src.empty())
                return false;

            cv::Size2i dst_size;
            if (_src.rows >= _src.cols)
            {
                dst_size.height = _square_len;
                dst_size.width = static_cast<int>((_src.cols * 1.0 / _src.rows * 1.0) * _square_len);
            }
            else
            {
                dst_size.height = static_cast<int>((_src.rows * 1.0 / _src.cols * 1.0) * _square_len);
                dst_size.width = _square_len;
            }
            _dst = cv::Mat(dst_size, CV_8UC3);
            cv::resize(_src, _dst, dst_size);

            return true;
        }
        bool xCvHelper::create_fuzzy_square_background(cv::Mat& _src, cv::Mat& _dst, const int _square_len)
        {
            cv::Mat& background = _dst;
            if (false == convert_to_fuzzy_square_img(_src, background, _square_len))
                return false;

            cv::Mat foreground;
            if (false == scale_to_square_inset(_src, foreground, _square_len))
                return false;

            cv::Rect2i dst_rect;
            if (foreground.cols == _square_len)
            {
                dst_rect.y = (background.rows - foreground.rows) / 2;
                dst_rect.x = 0;
                dst_rect.width = foreground.cols;
                dst_rect.height = foreground.rows;
            }
            else if (foreground.rows == _square_len)
            {
                dst_rect.y = 0;
                dst_rect.x = (background.cols - foreground.cols) / 2;
                dst_rect.width = foreground.cols;
                dst_rect.height = foreground.rows;
            }

            foreground.copyTo(background(dst_rect));

            return true;
        }
        bool xCvHelper::calculate_image_average_color(cv::Mat& _src, cv::Vec<double, 3>& _res)
        {
            double pix_size = static_cast<double>(_src.cols) * static_cast<double>(_src.rows);

            _res[0] = 0.0;
            _res[1] = 0.0;
            _res[2] = 0.0;

            for (int row = 0; row < _src.rows; row++)
            {
                for (int col = 0; col < _src.cols; col++)
                {
                    auto& cell = _src.at<cv::Vec3b>(row, col);

                    _res[0] += cell[0] / pix_size;
                    _res[1] += cell[1] / pix_size;
                    _res[2] += cell[2] / pix_size;
                }
            }

            return true;
        }
    }
}