#include "mjpeg.hpp"
#include "mjpeg_cfg.hpp"
#include <opencv2/core.hpp>
#include <opencv2/core/ocl.hpp>
#include <opencv2/core/utility.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>
#include <cmath>

// constant function for DCT transform
#define CFunc(x) (x == 0 ? std::sqrt(2.0) / 2.0 : 1.0)

namespace mini_jpeg
{

Jpeg::Jpeg(const cv::Mat *inputimg)
{
    if (inputimg != NULL)
    {
        this->encode(inputimg);
    }
}
void Jpeg::encode(const cv::Mat *inputimg)
{
}
void Jpeg::decode(cv::Mat *outputimg)
{
}
void Jpeg::save(const char *filename)
{
}
void Jpeg::load(const char *filename)
{
}

// rgb <==> ycrcb schema(4,2,0)
void Jpeg::rgb2ycrcb(const cv::Mat_<cv::Vec3b> *src, cv::Mat_<cv::Vec3b> *dst)
{
    cv::Mat_<cv::Vec3d> dmat = src->clone();

    // convert to continue value of ycrcb
    for (int r = 0; r < dmat.rows; r++)
    {
        for (int c = 0; c < dmat.cols; c++)
        {
            dmat[r][c] = cv::Mat_<double>(RGB2YCRCB_T * cv::Mat_<double>(dmat[r][c]) + RGB2YCRCB_RE);
        }
    }

    /**
     * schema(4, 2, 0) subsample
     * ori: 
     *      [y1, cr1, cb1], [y2, cr2, cb2]
     *      [y3, cr3, cb3], [y4, cr4, cb4]
     * 
     * subsampled:
     *      [y1, cr1, cb3], [y2, cr1, cb3]
     *      [y3, cr1, cb3], [y4, cr1, cb3]  
     */
    for (int r = 0; r < dmat.rows; r += 2)
    {
        for (int c = 0; c < dmat.cols; c += 2)
        {
            double cr_color = dmat[r][c][1];
            double cb_color = r + 1 <= dmat.rows ? dmat[r + 1][c][2] : dmat[r][c][2];
            for (int dr = 0; dr < 2 & dr + r < dmat.rows; dr++)
            {
                for (int dc = 0; dc < 2 & dc + r < dmat.cols; dc++)
                {
                    // truncate
                    dmat[r + dr][c + dc][0] = trunc(dmat[r + dr][c + dc][0]);
                    dmat[r + dr][c + dc][1] = trunc(cr_color);
                    dmat[r + dr][c + dc][2] = trunc(cb_color);
                }
            }
        }
    }
    (*dst) = cv::Mat_<cv::Vec3b>(dmat);
}
void Jpeg::ycrcb2rgb(const cv::Mat_<cv::Vec3b> *src, cv::Mat_<cv::Vec3b> *dst)
{
    cv::Mat_<cv::Vec3d> dmat = src->clone();

    // convert to continue value of ycrcb
    for (int r = 0; r < dmat.rows; r++)
    {
        for (int c = 0; c < dmat.cols; c++)
        {
            dmat[r][c] = cv::Mat_<double>(YCRCB2RGB_T * (cv::Mat_<double>(dmat[r][c]) + YCRCB2RGB_RE));
        }
    }
    (*dst) = cv::Mat_<cv::Vec3b>(dmat);
}

// ycrcb <==> dct
// Compute DCT for every 8x8 block.
void Jpeg::dct(const cv::Mat_<cv::Vec3b> *src, cv::Mat_<cv::Vec3d> *dst)
{
    cv::Mat_<cv::Vec3d> dmat(src->rows, src->cols);
    for (int r = 0; r < dmat.rows; r += 8)
    {
        for (int c = 0; c < dmat.cols; c += 8)
        {
            // compute 8x8 block
            for (int u = 0; u < 8 && u + r < dmat.rows; u++)
            {
                for (int v = 0; v < 8 && v + c < dmat.cols; v++)
                {
                    // compute F(u, v)
                    double Fuv_Y = 0, Fuv_Cr = 0, Fuv_Cb = 0;
                    for (int i = 0; i < 8 && i + r < dmat.rows; i++)
                    {
                        for (int j = 0; j < 8 && j + c < dmat.cols; j++)
                        {
                            double coef = std::cos((2.0 * i + 1.0) / 16.0 * u * PI) * std::cos((2.0 * j + 1.0) / 16.0 * v * PI);
                            Fuv_Y += coef * (*src)[r + i][c + j][0];
                            Fuv_Cr += coef * (*src)[r + i][c + j][1];
                            Fuv_Cb += coef * (*src)[r + i][c + j][2];
                        }
                    }
                    dmat[r + u][c + v][0] = CFunc(u) * CFunc(v) * Fuv_Y / 4.0;
                    dmat[r + u][c + v][1] = CFunc(u) * CFunc(v) * Fuv_Cr / 4.0;
                    dmat[r + u][c + v][2] = CFunc(u) * CFunc(v) * Fuv_Cb / 4.0;
                }
            }
        }
    }
    (*dst) = dmat;
}
void Jpeg::idct(const cv::Mat_<cv::Vec3d> *src, cv::Mat_<cv::Vec3b> *dst)
{
    cv::Mat_<cv::Vec3d> dmat(src->rows, src->cols);
    for (int r = 0; r < dmat.rows; r += 8)
    {
        for (int c = 0; c < dmat.cols; c += 8)
        {
            // compute 8x8 block
            for (int i = 0; i < 8 && i + r < dmat.rows; i++)
            {
                for (int j = 0; j < 8 && j + c < dmat.cols; j++)
                {
                    // compute f(i, j)
                    double fij_Y = 0, fij_Cr = 0, fij_Cb = 0;
                    for (int u = 0; u < 8 && u + r < dmat.rows; u++)
                    {
                        for (int v = 0; v < 8 && v + c < dmat.cols; v++)
                        {
                            double coef = CFunc(u) * CFunc(v) / 4.0 * std::cos((2.0 * i + 1.0) / 16.0 * u * PI) * std::cos((2.0 * j + 1.0) / 16.0 * v * PI);
                            fij_Y += coef * (*src)[r + u][c + v][0];
                            fij_Cr += coef * (*src)[r + u][c + v][1];
                            fij_Cb += coef * (*src)[r + u][c + v][2];
                        }
                    }
                    dmat[r + i][c + j][0] = fij_Y;
                    dmat[r + i][c + j][1] = fij_Cr;
                    dmat[r + i][c + j][2] = fij_Cb;
                }
            }
        }
    }
    (*dst) = dmat;
}

// quantization <==> iquantization
void Jpeg::quantization(const cv::Mat_<cv::Vec3d> *src, cv::Mat_<cv::Vec3i> *dst)
{
    (*dst) = src->clone();
    for (int r = 0; r < dst->rows; r += 8)
    {
        for (int c = 0; c < dst->cols; c += 8)
        {
            // compute 8x8 block
            for (int u = 0; u < 8 && u + r < dst->rows; u++)
            {
                for (int v = 0; v < 8 && v + c < dst->cols; v++)
                {
                    (*dst)[r + u][c + v][0] = std::round(int((*dst)[r + u][c + v][0]) / LUMINANCE_QT[u][v]);
                    (*dst)[r + u][c + v][1] = std::round(int((*dst)[r + u][c + v][1]) / CHROMINANCE_QT[u][v]);
                    (*dst)[r + u][c + v][2] = std::round(int((*dst)[r + u][c + v][2]) / CHROMINANCE_QT[u][v]);
                }
            }
        }
    }
}
void Jpeg::iquantization(const cv::Mat_<cv::Vec3i> *src, cv::Mat_<cv::Vec3d> *dst)
{
    (*dst) = src->clone();
    for (int r = 0; r < dst->rows; r += 8)
    {
        for (int c = 0; c < dst->cols; c += 8)
        {
            // compute 8x8 block
            for (int u = 0; u < 8 && u + r < dst->rows; u++)
            {
                for (int v = 0; v < 8 && v + c < dst->cols; v++)
                {
                    (*dst)[r + u][c + v][0] = (*dst)[r + u][c + v][0] * LUMINANCE_QT[u][v];
                    (*dst)[r + u][c + v][1] = (*dst)[r + u][c + v][1] * CHROMINANCE_QT[u][v];
                    (*dst)[r + u][c + v][2] = (*dst)[r + u][c + v][2] * CHROMINANCE_QT[u][v];
                }
            }
        }
    }
}

// huffman encoding <==> decoding
void Jpeg::huffmanEncode(const cv::Mat *src)
{
}
void Jpeg::huffmanDecode(const cv::Mat *dst)
{
}
} // namespace mini_jpeg