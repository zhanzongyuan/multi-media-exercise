#ifndef MJPEG_HPP
#define MJPEG_HPP
#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/core/ocl.hpp>
#include <opencv2/core/utility.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>

using namespace std;

namespace mini_jpeg
{
class Jpeg
{
  private:
  public:
    Jpeg(const cv::Mat *inputimg = NULL);
    void encode(const cv::Mat *inputimg);
    void decode(cv::Mat *outputimg);
    void save(const char *filename);
    void load(const char *filename);

    // rgb <==> ycrcb
    static void rgb2ycrcb(const cv::Mat_<cv::Vec3b> *src, cv::Mat_<cv::Vec3b> *dst);
    static void ycrcb2rgb(const cv::Mat_<cv::Vec3b> *src, cv::Mat_<cv::Vec3b> *dst);

    // ycrcb <==> dct
    static void dct(const cv::Mat_<cv::Vec3b> *src, cv::Mat_<cv::Vec3d> *dst);
    static void idct(const cv::Mat_<cv::Vec3d> *src, cv::Mat_<cv::Vec3b> *dst);

    // quantization
    static void quantization(const cv::Mat_<cv::Vec3d> *src, cv::Mat_<cv::Vec3i> *dst);
    static void iquantization(const cv::Mat_<cv::Vec3i> *src, cv::Mat_<cv::Vec3d> *dst);

    // huffman encoding <==> decoding
    static void huffmanEncode(const cv::Mat *src);
    static void huffmanDecode(const cv::Mat *dst);
};

} // namespace mini_jpeg

#endif