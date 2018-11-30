#ifndef MJPEG_HPP
#define MJPEG_HPP
#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/core/ocl.hpp>
#include <opencv2/core/utility.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

using namespace std;

namespace mini_jpeg
{
class Jpeg
{
private:
  uint8_t *data;
  int cap;
  int len;

public:
  Jpeg(const cv::Mat_<cv::Vec3b> *inputimg = NULL);
  ~Jpeg();
  void encode(const cv::Mat_<cv::Vec3b> *inputimg);
  void decode(cv::Mat_<cv::Vec3b> *outputimg);
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
  static void huffmanEncode(const cv::Mat_<cv::Vec3i> *src, uint8_t *&dst, int &bytelen, int &bytecap);
  static void huffmanDecode(const uint8_t *src, int bytecap, cv::Mat_<cv::Vec3i> *dst);
};

} // namespace mini_jpeg

#endif