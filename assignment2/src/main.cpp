#include "measure.hpp"
#include "mjpeg.hpp"
#include <cstdio>
#include <opencv2/core.hpp>
#include <opencv2/core/ocl.hpp>
#include <opencv2/core/utility.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

using namespace std;

int main()
{
       cv::Mat_<cv::Vec3b> ani = cv::imread("./input/animal.bmp");
       cv::Mat_<cv::Vec3b> anictn = cv::imread("./input/animal_cartoon.bmp");

       // compress to GIF
       // cv::imwrite("./output/animal.GIF", ani);
       // cv::imwrite("./output/animal_cartoon.GIF", anictn);

       // compress to mjpeg
       mini_jpeg::Jpeg ani_mjpeg(&ani);
       mini_jpeg::Jpeg anictn_mjpeg(&anictn);
       printf("saving...\n");
       ani_mjpeg.save("./output/animal_mjpeg.dat");
       anictn_mjpeg.save("./output/animal_cartoon_mjpeg.dat");

       // load from dat file
       ani_mjpeg.load("./output/animal_mjpeg.dat");
       anictn_mjpeg.load("./output/animal_cartoon_mjpeg.dat");
       cv::Mat_<cv::Vec3b> ani_mjpeg_mat, anictn_mjpeg_mat; // decode from mini jpeg

       // decode to mat
       ani_mjpeg.decode(&ani_mjpeg_mat);
       anictn_mjpeg.decode(&anictn_mjpeg_mat);
       cv::imwrite("./output/animal_out.jpeg", ani_mjpeg_mat);
       cv::imwrite("./output/animal_cartoon_out.jpeg", anictn_mjpeg_mat);

       // compare compression ratio
       float ani_GIF_cpr = compression_ratio("./input/animal.bmp", "input/animal.GIF");
       float anictn_GIF_cpr = compression_ratio("./input/animal_cartoon.bmp", "./input/animal_cartoon.GIF");
       // float ani_mjpeg_cpr = compression_ratio("./input/animal.bmp", "./output/animal_out.jpeg");
       // float anictn_mjpeg_cpr = compression_ratio("./input/animal_cartoon.bmp", "./output/animal_cartoon_out.jpeg");
       float ani_mjpeg_cpr = compression_ratio("./input/animal.bmp", "./output/animal_mjpeg.dat");
       float anictn_mjpeg_cpr = compression_ratio("./input/animal_cartoon.bmp", "./output/animal_mjpeg.dat");

       printf("        [Compression Ratio]:\n");
       printf("---------------------------------\n");
       printf("        %8s | %16s\n", "[animal]", "[animal cartoon]");
       printf("[GIF]:  %8.3f | %16.3f\n", ani_GIF_cpr, anictn_GIF_cpr);
       printf("[jpeg]: %8.3f | %16.3f\n\n", ani_mjpeg_cpr, anictn_mjpeg_cpr);

       // compare distortion
       cv::Mat_<cv::Vec3b> ani_GIF = cv::imread("./input/animal.GIF");
       cv::Mat_<cv::Vec3b> anictn_GIF = cv::imread("./input/animal_cartoon.GIF");

       Distortion ani_mjpeg_dt = distortion_measure(&ani, &ani_mjpeg_mat);
       Distortion anictn_mjpeg_dt = distortion_measure(&anictn, &anictn_mjpeg_mat);

       printf("        [Distortion Ratio (MSE|SNR]:\n");
       printf("---------------------------------\n");
       printf("        %16s | %24s\n", "[animal]", "[animal cartoon]");
       printf("[jpeg]: %8.3f%8.3f | %12.3f%12.3f\n\n",
              ani_mjpeg_dt.MSE, ani_mjpeg_dt.SNR,
              anictn_mjpeg_dt.MSE, anictn_mjpeg_dt.SNR);

       return 0;
}