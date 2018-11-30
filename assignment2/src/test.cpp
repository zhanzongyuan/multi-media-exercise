#include "measure.hpp"
#include "mjpeg.hpp"
#include "mjpeg_cfg.hpp"
#include <bitset>
#include <cmath>
#include <cstdio>
#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/core/ocl.hpp>
#include <opencv2/core/utility.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <vector>

using namespace std;
using namespace mini_jpeg;

int main()
{

    cv::Mat_<cv::Vec3b> rgbimg_in = cv::imread("input/animal.bmp");
    cv::Mat_<cv::Vec3b> ycrcbimg_in;
    cv::Mat_<cv::Vec3d> dct_in;
    cv::Mat_<cv::Vec3i> qt_in;
    uint8_t *dst;
    int bytelen = 0, bytecap = 0;

    printf("encoding...\n");
    // encode
    Jpeg::rgb2ycrcb(&rgbimg_in, &ycrcbimg_in);
    Jpeg::dct(&ycrcbimg_in, &dct_in);
    Jpeg::quantization(&dct_in, &qt_in);
    Jpeg::huffmanEncode(&qt_in, dst, bytelen, bytecap);

    cv::Mat_<cv::Vec3b> ycrcbimg_out;
    cv::Mat_<cv::Vec3b> rgbimg_out;
    cv::Mat_<cv::Vec3d> dct_out;
    cv::Mat_<cv::Vec3i> qt_out;

    printf("decoding...\n");
    // decode
    Jpeg::huffmanDecode(dst, bytecap, &qt_out);
    Jpeg::iquantization(&qt_out, &dct_out);
    Jpeg::idct(&dct_out, &ycrcbimg_out);
    Jpeg::ycrcb2rgb(&ycrcbimg_out, &rgbimg_out);

    printf("showing (%d, %d)...\n", rgbimg_out.rows, rgbimg_out.cols);
    cv::imshow("rgbimg_out", rgbimg_out);
    cv::waitKey(0);

    return 0;
}