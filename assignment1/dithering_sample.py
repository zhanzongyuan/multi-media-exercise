# coding=utf-8
import cv2
import numpy as np
import matplotlib.pyplot as plt

DITHER_MATRIX4x4 = np.array([
    [0, 8, 2, 10],
    [12, 4, 14, 6],
    [3, 11, 1, 9],
    [15, 7, 13, 5]], dtype="uint8")
            
DITHER_MATRIX2x2 = np.array([
    [0, 2],
    [3, 1]], dtype="uint8")

def main():
    # MATRIX = DITHER_MATRIX2x2
    MATRIX = DITHER_MATRIX4x4
    scale = MATRIX.shape[0]
    im_redapple = cv2.imread("./asserts/redapple.jpg")
    im_dithered = np.zeros([x*scale for x in im_redapple.shape[0:2]], dtype="uint8")
    

    for r in range(im_redapple.shape[0]):
        for c in range(im_redapple.shape[1]):
            grey_value = np.mean(im_redapple[r][c])
            grey_value_mat = np.uint8(grey_value/(256.0/(scale**2+1)))

            # Fill matrix for every pixel.
            for r_mat in range(MATRIX.shape[0]):
                for c_mat in range(MATRIX.shape[1]):
                    rd = r*scale+r_mat
                    cd = c*scale+c_mat

                    if grey_value_mat > MATRIX[r_mat][c_mat]:
                        im_dithered[rd][cd] = 255
    
    print("Dithering done.")
    """
    cv2.namedWindow("im_dithered", cv2.WINDOW_NORMAL)
    cv2.resizeWindow("im_dithered", im_redapple.shape[1], im_redapple.shape[0])
    cv2.imshow("im_dithered", im_dithered)
    cv2.waitKey(0)
    """
    cv2.imwrite("output/dithered_%dx%dmat.bmp"%(scale, scale), im_dithered)
    
    """
    plt.figure()
    plt.gray()
    plt.imshow(im_dithered)
    plt.show() 
    """

if __name__ == "__main__":
    main()
