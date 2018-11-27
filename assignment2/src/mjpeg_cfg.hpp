#ifndef MJPEG_CFG_HPP
#define MJPEG_CFG_HPP
#include <opencv2/core.hpp>
#include <opencv2/core/ocl.hpp>
#include <opencv2/core/utility.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <cmath>

namespace mini_jpeg
{
static const double PI = std::atan(1) * 4;
// rgb <==> ycrcb
static const cv::Mat_<double> RGB2YCRCB_T =
    (cv::Mat_<double>(3, 3) << 65.481 / 255, 128.553 / 255, 24.966 / 255,
     -37.797 / 255, -74.203 / 255, 112.0 / 255,
     112.0 / 255, -93.786 / 255, -18.214 / 255);
static const cv::Mat_<double> RGB2YCRCB_RE = (cv::Mat_<double>(3, 1) << 16, 128, 128);
static const cv::Mat_<double> YCRCB2RGB_T =
    (cv::Mat_<double>(3, 3) << 255.0 / 219.0, 0.0, 255.0 / 224.0 * 1.402,
     255.0 / 219.0, -255.0 / 224.0 * 1.772 * 0.114 / 0.587, -255.0 / 224.0 * 1.402 * 0.299 / 0.587,
     255.0 / 219.0, 225.0 / 224 * 1.772, 0.0);
static const cv::Mat_<double> YCRCB2RGB_RE = (cv::Mat_<double>(3, 1) << -16, -128, -128);

// quantization table
static const cv::Mat_<int> LUMINANCE_QT =
    (cv::Mat_<int>(8, 8) << 16, 11, 10, 16, 24, 40, 51, 61,
     12, 12, 14, 19, 26, 58, 60, 55,
     14, 13, 16, 24, 40, 57, 69, 56,
     14, 17, 22, 29, 51, 87, 80, 62,
     18, 22, 37, 56, 68, 109, 103, 77,
     24, 35, 55, 64, 81, 104, 113, 92,
     49, 64, 78, 87, 103, 121, 120, 101,
     72, 92, 95, 98, 112, 100, 103, 99);
static const cv::Mat_<int> CHROMINANCE_QT =
    (cv::Mat_<int>(8, 8) << 17, 18, 24, 47, 99, 99, 99, 99,
     18, 21, 26, 66, 99, 99, 99, 99,
     24, 26, 56, 99, 99, 99, 99, 99,
     47, 66, 99, 99, 99, 99, 99, 99,
     99, 99, 99, 99, 99, 99, 99, 99,
     99, 99, 99, 99, 99, 99, 99, 99,
     99, 99, 99, 99, 99, 99, 99, 99);

// huffman tables

} // namespace mini_jpeg

#endif