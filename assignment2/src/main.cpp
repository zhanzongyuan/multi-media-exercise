#include <cstdio>
#include <opencv2/core.hpp>
#include <opencv2/core/ocl.hpp>
#include <opencv2/core/utility.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include "mjpeg.hpp"
#include "measure.hpp"

using namespace std;

int main()
{
    cv::Mat ani = cv::imread("./input/animal.bmp");
    cv::Mat anictn = cv::imread("./input/animal_cartoon.bmp");

    // compress to gif
    cv::imwrite("./output/animal.gif", ani);
    cv::imwrite("./output/animal_cartoon.gif", anictn);

    // compress to mjpeg
    mini_jpeg::Jpeg ani_mjpeg(&ani);
    mini_jpeg::Jpeg anictn_mjpeg(&anictn);
    ani_mjpeg.save("./output/animal_mjpeg.dat");
    anictn_mjpeg.save("./output/animal_cartoon_mjpeg.dat");

    // compare compression ratio
    float ani_gif_cpr = compression_ratio("./intput/animal.bmp", "./output/animal.gif");
    float anictn_gif_cpr = compression_ratio("./intput/animal_cartoon.bmp", "./output/animal_cartoon.gif");
    float ani_mjpeg_cpr = compression_ratio("./intput/animal.bmp", "./output/animal_mjpeg.dat");
    float anictn_mjpeg_cpr = compression_ratio("./intput/animal_cartoon.bmp", "./output/animal_mjpeg.dat");

    printf("        [Compression Ratio]:\n");
    printf("---------------------------------\n");
    printf("        %8s | %16s\n", "[animal]", "[animal cartoon]");
    printf("[gif]:  %8.3f | %16.3f\n", ani_gif_cpr, anictn_gif_cpr);
    printf("[jpeg]: %8.3f | %16.3f\n\n", ani_mjpeg_cpr, anictn_mjpeg_cpr);

    // compare distortion
    cv::Mat ani_gif = cv::imread("./output/animal.gif");
    cv::Mat anictn_gif = cv::imread("./output/animal_cartoon.gif");
    cv::Mat ani_mjpeg_mat, anictn_mjpeg_mat; // decode from mini jpeg
    ani_mjpeg.decode(&ani_mjpeg_mat);
    anictn_mjpeg.decode(&anictn_mjpeg_mat);

    Distortion ani_gif_dt = distortion_measure(&ani, &ani_gif);
    Distortion anictn_gif_dt = distortion_measure(&anictn, &anictn_gif);
    Distortion ani_mjpeg_dt = distortion_measure(&ani, &ani_mjpeg_mat);
    Distortion anictn_mjpeg_dt = distortion_measure(&anictn, &anictn_mjpeg_mat);

    printf("        [Distortion Ratio (MSE|SNR|PSNR)]:\n");
    printf("---------------------------------\n");
    printf("        %24s | %48s\n", "[animal]", "[animal cartoon]");
    printf("[gif]:  %8.3f/%8.3f%8.3f | %16.3f/%16.3f/%16.3f\n",
           ani_gif_dt.MSE, ani_gif_dt.SNR, ani_gif_dt.PSNR,
           anictn_gif_dt.MSE, anictn_gif_dt.SNR, anictn_gif_dt.PSNR);
    printf("[jpeg]: %8.3f/%8.3f/%8.3f | %16.3f/%16.3f/%16.3f\n\n",
           ani_mjpeg_dt.MSE, ani_mjpeg_dt.SNR, ani_mjpeg_dt.PSNR,
           anictn_mjpeg_dt.MSE, anictn_mjpeg_dt.SNR, anictn_mjpeg_dt.PSNR);

    return 0;
}