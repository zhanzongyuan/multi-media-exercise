#include <cstdio>
#include <opencv2/core.hpp>
#include <opencv2/core/ocl.hpp>
#include <opencv2/core/utility.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <vector>
#include <iostream>
#include <cmath>
#include "mjpeg.hpp"
#include "mjpeg_cfg.hpp"
#include "measure.hpp"
using namespace std;
using namespace mini_jpeg;

cv::Mat_<double> MM = (cv::Mat_<double>(2, 1) << cos(1 * mini_jpeg::PI), cos(1));

int main()
{
    cv::Mat_<cv::Vec3b> rgbimg_in = cv::imread("input/animal.bmp");
    cv::Mat_<cv::Vec3b> ycrcbimg_in;
    cv::Mat_<cv::Vec3d> dct_in;
    cv::Mat_<cv::Vec3i> qt_in;

    // encode
    Jpeg::rgb2ycrcb(&rgbimg_in, &ycrcbimg_in);
    Jpeg::dct(&ycrcbimg_in, &dct_in);
    Jpeg::quantization(&dct_in, &qt_in);

    cv::Mat_<cv::Vec3b> ycrcbimg_out;
    cv::Mat_<cv::Vec3b> rgbimg_out;
    cv::Mat_<cv::Vec3d> dct_out;
    cv::Mat_<cv::Vec3i> qt_out;

    // decode
    Jpeg::iquantization(&qt_in, &dct_out);
    Jpeg::idct(&dct_out, &ycrcbimg_out);
    Jpeg::ycrcb2rgb(&ycrcbimg_out, &rgbimg_out);

    printf("showing...\n");
    cv::imshow("rgbimg_out", rgbimg_out);
    cv::waitKey(0);
    // cv::Mat_<double> m1 = (cv::Mat_<double>(2, 3) << 1, 2, 3, 4, 5, 6);
    // cv::Mat_<double> m2 = (cv::Mat_<double>(2, 3) << 2, 2, 2, 2, 2, 2);
    // cout << m1.mul(m2) << endl;
    // cout << MM << endl;
    return 0;
}