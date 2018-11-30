#ifndef MEASURE_HPP
#define MEASURE_HPP

#include <cmath>
#include <fstream>
#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/core/ocl.hpp>
#include <opencv2/core/utility.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

using namespace std;

/**
 * This function computes compression ratio from file origin file to compression file
 * 
 * ratio = B0 / B1
 * 
 */
float compression_ratio(const char *orifn, const char *cpfn)
{
    int ori_len, cp_len;
    ifstream orifn_fi(orifn, ifstream::binary);
    ifstream cpfn_fi(cpfn, ifstream::binary);

    if (!orifn_fi)
        throw std::invalid_argument(orifn);
    if (!cpfn_fi)
        throw std::invalid_argument(cpfn);

    orifn_fi.seekg(0, orifn_fi.end);
    cpfn_fi.seekg(0, cpfn_fi.end);

    double ratio = cpfn_fi.tellg() * 1.0 / orifn_fi.tellg();

    cpfn_fi.close();
    orifn_fi.close();
    return ratio;
}

/**
 * The function computes distortion
 * 
 * refs: Fundamentals of Multimedia page 247.
 */
struct Distortion
{
    double MSE, SNR;
};

Distortion distortion_measure(const cv::Mat_<cv::Vec3b> *oimg, const cv::Mat_<cv::Vec3b> *cimg)
{
    Distortion d;
    d.MSE = 0;
    double ox = 0;
    double N = oimg->rows * oimg->cols * 3;
    for (int r = 0; r < oimg->rows; r++)
    {
        for (int c = 0; c < oimg->cols; c++)
        {
            for (int chl = 0; chl < 3; chl++)
            {
                d.MSE += pow((*oimg)[r][c][chl] - (*cimg)[r][c][chl], 2) / N;
                ox += pow((*oimg)[r][c][chl], 2) / N;
            }
        }
    }
    d.SNR = 10 * log(ox / d.MSE) / log(10);
    return d;
}
#endif