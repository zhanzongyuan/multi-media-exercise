#ifndef MEASURE_HPP
#define MEASURE_HPP

#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/core/ocl.hpp>
#include <opencv2/core/utility.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>

using namespace std;

/**
 * This function computes compression ratio from file origin file to compression file
 * 
 * ratio = B0 / B1
 * 
 */
float compression_ratio(const char *orifn, const char *cpfn)
{
    return 0;
}

/**
 * The function computes distortion
 * 
 * refs: Fundamentals of Multimedia page 247.
 */
struct Distortion
{
    float MSE, SNR, PSNR;
};

Distortion distortion_measure(const cv::Mat *oimg, const cv::Mat *cimg)
{
    return Distortion();
}
#endif