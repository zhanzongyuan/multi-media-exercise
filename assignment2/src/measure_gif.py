# coding=utf-8
from PIL import Image
import numpy as np
import math
import cv2


def distortion_measure(oimg, cimg):
    # MSE = 0.0
    # SNR = 0.0
    # ox = 0.0
    # print(oimg.shape, cimg.shape)
    # N = (oimg.shape[0]*oimg.shape[1]*oimg.shape[2])*1.0
    # for r in range(oimg.shape[0]):
    #     for c in range(oimg.shape[1]):
    #         for chl in range(oimg.shape[2]):
    #             ox += oimg[r][c][chl]**2/N
    #             MSE += ((oimg[r][c][chl]-cimg[r][c][chl])**2)/N
    # SNR = 10*math.log(ox/MSE)/math.log(10)
    # return MSE, SNR

    N = oimg.shape[0]*oimg.shape[1]*oimg.shape[2]
    MSE = np.sum((oimg-cimg)**2)/N
    ox = np.sum(oimg**2)/N
    SNR = 10*np.log(ox/MSE)/np.log(10)
    return MSE, SNR


if __name__ == "__main__":
    ani_bmp = np.array(Image.open("input/animal.bmp").convert("RGB"))
    anic_bmp = np.array(Image.open(
        "input/animal_cartoon.bmp").convert("RGB"))

    ani_gif = np.array(Image.open(
        "input/animal.GIF").convert("RGB"), dtype=np.float64)
    anic_gif = np.array(Image.open(
        "input/animal_cartoon.GIF").convert("RGB"), dtype=np.float64)

    (ani_MSE, ani_SNR) = distortion_measure(ani_bmp, ani_gif)
    (anic_MSE, anic_SNR) = distortion_measure(anic_bmp, anic_gif)

    print("        [Distortion Ratio (MSE|SNR]:\n")
    print("---------------------------------\n")
    print("        %16s | %24s\n" % ("[animal]", "[animal cartoon]"))
    print("[gif]: %8.3f%8.3f | %12.3f%12.3f\n\n" %
          (ani_MSE, ani_SNR,  anic_MSE, anic_SNR))
