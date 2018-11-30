#include "mjpeg.hpp"
#include "mjpeg_cfg.hpp"
#include <bitset>
#include <cmath>
#include <fstream>
#include <iostream>
#include <map>
#include <opencv2/core.hpp>
#include <opencv2/core/ocl.hpp>
#include <opencv2/core/utility.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

// constant function for DCT transform
#define CFunc(x) (x == 0 ? std::sqrt(2.0) / 2.0 : 1.0)

namespace mini_jpeg
{

Jpeg::Jpeg(const cv::Mat_<cv::Vec3b> *inputimg)
{
    this->data = NULL;
    this->cap = 0;
    this->len = 0;
    if (inputimg != NULL)
    {
        this->encode(inputimg);
    }
}

Jpeg::~Jpeg()
{
    if (this->data != NULL)
    {
        delete[] this->data;
    }
}
void Jpeg::encode(const cv::Mat_<cv::Vec3b> *inputimg)
{
    if (inputimg->rows == 0 || inputimg->cols == 0)
        return;
    cv::Mat_<cv::Vec3b> ycrcbimg_in;
    cv::Mat_<cv::Vec3d> dct_in;
    cv::Mat_<cv::Vec3i> qt_in;

    // encode
    Jpeg::rgb2ycrcb(inputimg, &ycrcbimg_in);
    Jpeg::dct(&ycrcbimg_in, &dct_in);
    Jpeg::quantization(&dct_in, &qt_in);
    Jpeg::huffmanEncode(&qt_in, this->data, this->len, this->cap);
}
void Jpeg::decode(cv::Mat_<cv::Vec3b> *outputimg)
{
    if (this->data == NULL)
    {
        return;
    }
    cv::Mat_<cv::Vec3b> ycrcbimg_out;
    cv::Mat_<cv::Vec3b> rgbimg_out;
    cv::Mat_<cv::Vec3d> dct_out;
    cv::Mat_<cv::Vec3i> qt_out;

    Jpeg::huffmanDecode(this->data, this->len, &qt_out);
    Jpeg::iquantization(&qt_out, &dct_out);
    Jpeg::idct(&dct_out, &ycrcbimg_out);
    Jpeg::ycrcb2rgb(&ycrcbimg_out, &rgbimg_out);
    (*outputimg) = rgbimg_out;
}
void Jpeg::save(const char *filename)
{

    if (this->len > 0)
    {
        std::ofstream myFile(filename, ios::out | ios::binary);
        myFile.write((char *)this->data, this->len + 4);
        myFile.close();
    }
}
void Jpeg::load(const char *filename)
{
    ifstream file(filename, ios::in | ios::binary);
    if (file)
    {
        // get length of file:
        file.seekg(0, file.end);
        int length = file.tellg();
        file.seekg(0, file.beg);

        if (this->data != NULL)
        {
            delete[] this->data;
            this->data = NULL;
        }
        this->cap = length + 4;
        this->len = length;
        this->data = new uint8_t[length + 4];

        // read data as a block:
        file.read((char *)this->data, length);

        if (!file)
            throw std::invalid_argument("file reading error!");
        file.close();
    }
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
            dmat[r][c] = cv::Mat_<double>(RGB2YCRCB_T * cv::Mat_<double>(dmat[r][c]) +
                                          RGB2YCRCB_RE);
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
            double cb_color = r + 1 < dmat.rows ? dmat[r + 1][c][2] : dmat[r][c][2];
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
            dmat[r][c] = cv::Mat_<double>(
                YCRCB2RGB_T * (cv::Mat_<double>(dmat[r][c]) + YCRCB2RGB_RE));
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
                            double coef = std::cos((2.0 * i + 1.0) / 16.0 * u * PI) *
                                          std::cos((2.0 * j + 1.0) / 16.0 * v * PI);
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
                            double coef = CFunc(u) * CFunc(v) / 4.0 *
                                          std::cos((2.0 * i + 1.0) / 16.0 * u * PI) *
                                          std::cos((2.0 * j + 1.0) / 16.0 * v * PI);
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
    (*dst) = cv::Mat_<cv::Vec3i>(src->rows, src->cols);
    for (int r = 0; r < dst->rows; r += 8)
    {
        for (int c = 0; c < dst->cols; c += 8)
        {
            // compute 8x8 block
            for (int u = 0; u < 8 && u + r < dst->rows; u++)
            {
                for (int v = 0; v < 8 && v + c < dst->cols; v++)
                {
                    (*dst)[r + u][c + v][0] =
                        std::round(int((*src)[r + u][c + v][0]) * 1.0 / LUM_QT[u][v]);
                    (*dst)[r + u][c + v][1] =
                        std::round(int((*src)[r + u][c + v][1]) * 1.0 / CHR_QT[u][v]);
                    (*dst)[r + u][c + v][2] =
                        std::round(int((*src)[r + u][c + v][2]) * 1.0 / CHR_QT[u][v]);
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
                    (*dst)[r + u][c + v][0] = (*dst)[r + u][c + v][0] * LUM_QT[u][v];
                    (*dst)[r + u][c + v][1] = (*dst)[r + u][c + v][1] * CHR_QT[u][v];
                    (*dst)[r + u][c + v][2] = (*dst)[r + u][c + v][2] * CHR_QT[u][v];
                }
            }
        }
    }
}

// bit stream control
void _grow(uint8_t *&arr, int &bytecap, bool exp = true)
{
    uint8_t *narr;
    if (exp)
        narr = new uint8_t[bytecap * 2];
    else
        narr = new uint8_t[bytecap + 4];
    for (int i = 0; i < bytecap; i++)
    {
        narr[i] = arr[i];
    }
    delete[] arr;
    arr = narr;
    bytecap *= 2;
}

int _bitsize(int i)
{
    uint16_t ui = abs(i);
    int size = 16;
    uint16_t mask = (1 << 15);
    while ((mask & ui) == 0 && mask != 0)
    {
        size--;
        mask >>= 1;
    }
    return size + 1;
}

void _bitwriter(_bits bits, uint8_t *&dst, int &bytelen, int &bitbias, int &bytecap)
{
    // Warning: can't use uint32_t to fill byte, as for big end and small end problem.
    while (bytelen + 4 > bytecap)
    {
        _grow(dst, bytecap);
    }

    int wb_l = bits.write(dst + bytelen, bitbias);
    bytelen += wb_l;
}

void _bitreader(const uint8_t *src, _bits &dstbits, int &curpos, int &bitbias, int &bytecap)
{
    if (dstbits._len == 0)
    {
        // please specific dstbits._len for reading length != 0.
        return;
    }
    uint32_t *src_b32 = (uint32_t *)(&src[curpos]);

    int rb_l = dstbits.read(src + curpos, bitbias);
    curpos += rb_l;
}

void _int_bitreader(const uint8_t *src, int &dstint, int size, int &curpos, int &bitbias, int &bytecap)
{
    _bits dstbits(0, size);
    _bitreader(src, dstbits, curpos, bitbias, bytecap);
    if (size > 8)
    {
        if ((dstbits.bits[0] & (1 << (size - 8 - 1))) == 0)
        {
            // positive
            dstint = (int(dstbits.bits[0]) << 8) | (int(dstbits.bits[1]));
        }
        else
        {
            // negative
            dstint = ((int(dstbits.bits[0]) | ((~int(0)) << (size - 8))) << 8) | int(dstbits.bits[1]);
        }
    }
    else
    {
        if ((dstbits.bits[1] & (1 << (size - 1))) == 0)
        {
            // positive
            dstint = dstbits.bits[1];
        }
        else
        {
            // negative
            dstint = ((~int(0)) << size) | int(dstbits.bits[1]);
        }
    }
}

void _decodDC(const uint8_t *src, int &dcvalue, int &curpos, int &bitbias, int &bytecap, const std::map<_bits, int> *I_DC_TABLE)
{
    for (int l = 2; l < 12; l++)
    {
        _bits candbits;
        candbits._len = l;
        int bitbias_tp = bitbias, bytecap_tp = bytecap, curpos_tp = curpos;
        _bitreader(src, candbits, curpos_tp, bitbias_tp, bytecap_tp);
        if (I_DC_TABLE->find(candbits) != I_DC_TABLE->end())
        {
            bitbias = bitbias_tp;
            bytecap = bytecap_tp;
            curpos = curpos_tp;
            _int_bitreader(src, dcvalue, I_DC_TABLE->find(candbits)->second, curpos, bitbias, bytecap);
            return;
        }
    }
}

void _decodeAC(const uint8_t *src, cv::Vec2i &runlen_amplitude, int &curpos, int &bitbias, int &bytecap, const std::map<_bits, cv::Vec2i> *I_AC_TABLE)
{
    for (int l = 2; l < 17; l++)
    {
        _bits candbits;
        candbits._len = l;
        int bitbias_tp = bitbias, bytecap_tp = bytecap, curpos_tp = curpos;
        _bitreader(src, candbits, curpos_tp, bitbias_tp, bytecap_tp);
        if (I_AC_TABLE->find(candbits) != I_AC_TABLE->end())
        {
            cv::Vec2i run_size = I_AC_TABLE->find(candbits)->second;
            runlen_amplitude[0] = run_size[0];
            bitbias = bitbias_tp;
            bytecap = bytecap_tp;
            curpos = curpos_tp;

            if (run_size[1] != 0)
                _int_bitreader(src, runlen_amplitude[1], run_size[1], curpos, bitbias, bytecap);
            else
                runlen_amplitude[1] = 0;

            return;
        }
    }
}

// huffman encoding <==> decoding
void Jpeg::huffmanEncode(const cv::Mat_<cv::Vec3i> *src, uint8_t *&dst, int &bytelen, int &bytecap)
{
    bytecap = 1;
    bytelen = 0;
    int bitbias = 0;
    dst = new uint8_t[bytecap];
    // record rows, cols
    _bitwriter(_bits(src->rows, 16), dst, bytelen, bitbias, bytecap);
    _bitwriter(_bits(src->cols, 16), dst, bytelen, bitbias, bytecap);
    cv::Vec3i pre_dc{0, 0, 0};
    for (int r = 0; r < src->rows; r += 8)
    {
        for (int c = 0; c < src->cols; c += 8)
        {
            for (int chl = 0; chl < 3; chl++)
            {
                // DC DPCM encode
                int dif = (*src)[r][c][chl] - pre_dc[chl];
                pre_dc[chl] = (*src)[r][c][chl];
                int dcsize = _bitsize(dif);
                _bits dcbits = chl == 0 ? LU_DC[dcsize] : CHR_DC[dcsize];
                // write DC bits
                _bitwriter(dcbits, dst, bytelen, bitbias, bytecap);
                _bitwriter(_bits(dif, dcsize), dst, bytelen, bitbias, bytecap);

                // AC zigzag RLC encode
                int acr = 0, acc = 0, runlen = 0, blocknum15_0 = 0;
                std::vector<cv::Vec2i> runlen_amplitude;
                int l = 1;
                while (l < 64)
                {
                    acr = ZIGZAG[l][0];
                    acc = ZIGZAG[l][1];
                    l++;
                    if (r + acr < src->rows && c + acc < src->cols)
                    {
                        if ((*src)[r + acr][c + acc][chl] == 0)
                        {
                            runlen++;
                            if (runlen == 16)
                            {
                                // add (15, 0) block;
                                runlen_amplitude.push_back(cv::Vec2i(15, 0));
                                runlen = 0;
                                blocknum15_0++;
                            }
                        }
                        else
                        {
                            runlen_amplitude.push_back(cv::Vec2i(runlen, (*src)[r + acr][c + acc][chl]));
                            runlen = 0;
                            blocknum15_0 = 0;
                        }
                    }
                }
                // write AC bits
                const _bits &EOB = chl == 0 ? LU_AC_EOB : CHR_AC_EOB;
                const _bits &ZRL = chl == 0 ? LU_AC_ZRL : CHR_AC_ZRL;
                const std::vector<std::vector<_bits>> &AC_TABLE = chl == 0 ? LU_AC : CHR_AC;
                int blocknum_proc = runlen_amplitude.size() - blocknum15_0;
                for (int b = 0; b < blocknum_proc; b++)
                {
                    int run = runlen_amplitude[b][0], amp = runlen_amplitude[b][1];
                    if (amp != 0)
                    {
                        int size = _bitsize(amp);
                        _bitwriter(AC_TABLE[run][size - 1], dst, bytelen, bitbias, bytecap);
                        _bitwriter(_bits(amp, size), dst, bytelen, bitbias, bytecap);
                    }
                    else
                    {
                        _bitwriter(ZRL, dst, bytelen, bitbias, bytecap);
                    }
                }
                _bitwriter(EOB, dst, bytelen, bitbias, bytecap);
            }
        }
    }
}
void Jpeg::huffmanDecode(const uint8_t *src, int bytecap, cv::Mat_<cv::Vec3i> *dst)
{
    int curpos = 0;
    int bitbias = 0;
    int rows = 0, cols = 0;
    _int_bitreader(src, rows, 16, curpos, bitbias, bytecap);
    _int_bitreader(src, cols, 16, curpos, bitbias, bytecap);
    (*dst) = cv::Mat_<cv::Vec3i>(rows, cols);
    cv::Vec3i pre_dc{0, 0, 0};
    for (int r = 0; r < dst->rows; r += 8)
    {
        for (int c = 0; c < dst->cols; c += 8)
        {
            for (int chl = 0; chl < 3; chl++)
            {
                // DC DPCM decode
                int dcdif = 0;
                const std::map<_bits, int> *I_DC_TABLE = (chl == 0) ? &I_LU_DC : &I_CHR_DC;
                _decodDC(src, dcdif, curpos, bitbias, bytecap, I_DC_TABLE);
                pre_dc[chl] = dcdif + pre_dc[chl];
                (*dst)[r][c][chl] = pre_dc[chl];

                // AC zigzag RLC decode
                const std::map<_bits, cv::Vec2i> *I_AC_TABLE = (chl == 0) ? &I_LU_AC : &I_CHR_AC;
                int acr = 0, acc = 0, runlen = 0, blocknum15_0 = 0;
                cv::Vec2i runlen_amplitude(0, 0);
                bool eob_flag = false;
                _decodeAC(src, runlen_amplitude, curpos, bitbias, bytecap, I_AC_TABLE);
                if (runlen_amplitude[0] == 0 && runlen_amplitude[1] == 0)
                    eob_flag = true;
                int l = 1;
                while (l < 64)
                {
                    acr = ZIGZAG[l][0];
                    acc = ZIGZAG[l][1];
                    l++;

                    if (r + acr < dst->rows && c + acc < dst->cols)
                    {
                        if (eob_flag)
                        {
                            (*dst)[r + acr][c + acc][chl] = 0;
                        }
                        else if (runlen_amplitude[0] == 0)
                        {
                            (*dst)[r + acr][c + acc][chl] = runlen_amplitude[1];
                            _decodeAC(src, runlen_amplitude, curpos, bitbias, bytecap, I_AC_TABLE);
                            if (runlen_amplitude[0] == 0 && runlen_amplitude[1] == 0)
                                eob_flag = true;
                        }
                        else
                        {
                            (*dst)[r + acr][c + acc][chl] = 0;
                            runlen_amplitude[0]--;
                        }
                    }
                }
            }
        }
    }
}
} // namespace mini_jpeg