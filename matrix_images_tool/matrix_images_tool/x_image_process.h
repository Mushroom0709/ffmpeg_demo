#ifndef _X_IMAGE_PROCESS_H_
#define _X_IMAGE_PROCESS_H_

#include <iostream>

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>


namespace xM
{
    namespace image
    {
		class xCvHelper
		{
#define XM_IMAGE_CVHELPER_BLUR_DEFAULT_COEFFICIENT 20
        public:
            static bool convert_to_fuzzy_square_img(cv::Mat& _src, cv::Mat& _dst, const int _square_len, const cv::Point2i _blur_point);
            static bool convert_to_fuzzy_square_img(cv::Mat& _src, cv::Mat& _dst, const int _square_len, const int _blur_x = 0, const int _blur_y = 0);
        public:
            static bool scale_to_square_inset(cv::Mat& _src, cv::Mat& _dst, const int _square_len);
        public:
            static bool create_fuzzy_square_background(cv::Mat& _src, cv::Mat& _dst, const int _square_len);
        public:
            static bool calculate_image_average_color(cv::Mat& _src, cv::Vec<double, 3>& _res);
		};
    }
}

#endif //!_X_IMAGE_PROCESS_H_