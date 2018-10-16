import cv2
import numpy as np
import os
from KDtree.KDtree import KDtree, build_kdtree

def To8bits(im_redapple, mode, rgbdimorder=None):
    kdtree = KDtree()
    im_median_cut = im_redapple.copy()
    
    assert mode == "best" or mode == "order"
    if mode == "best":
        ######### best kdtree ##########
        build_kdtree(kdtree, im_redapple.reshape(-1, 3), 8)
    elif mode == "order":
        ######### fix order kdtree #########
        assert type(rgbdimorder) != type(None) and len(rgbdimorder) == 8
        rgbdecoder = {"r": 2, "g": 1, "b": 0}
        dimorder = [rgbdecoder[x] for x in rgbdimorder]
        build_kdtree(kdtree, im_redapple.reshape(-1, 3), 8, dimorder)  
        
    for r in range(im_median_cut.shape[0]):
        for c in range(im_median_cut.shape[1]):
            im_median_cut[r, c], code = kdtree.encode_point(im_median_cut[r, c])
    
    return im_median_cut


def main():
    im_redapple = cv2.imread("input/redapple.jpg")
    
    # best mode
    print("processing 'output/redapple_best.jpg'...")
    im_best = To8bits(im_redapple, "best")
    cv2.imwrite("output/redapple_best.jpg", im_best) 

    # order mode
    print("processing 'output/redapple_order_rgbrgbrg.jpg'...")
    im_order_1 = To8bits(im_redapple, "order", "rgbrgbrg")
    cv2.imwrite("output/redapple_order_rgbrgbrg.jpg", im_order_1)

    print("processing 'output/redapple_order_rgbrgbrr.jpg'...")
    im_order_2 = To8bits(im_redapple, "order", "rgbrgbrr")
    cv2.imwrite("output/redapple_order_rgbrgbrr.jpg", im_order_2)
    
    print("processing 'output/redapple_order_rgbrrrrr.jpg'...")
    im_order_3 = To8bits(im_redapple, "order", "rgbrrrrr")
    cv2.imwrite("output/redapple_order_rgbrrrrr.jpg", im_order_3)
    
    print("\n == Mean square error ==")
    print("[BEST]:            %.4f"%np.mean((im_redapple-im_best)**2))
    print("[ORDER: rgbrgbrg]: %.4f"%np.mean((im_redapple-im_order_1)**2))
    print("[ORDER: rgbrgbrr]: %.4f"%np.mean((im_redapple-im_order_2)**2))
    print("[ORDER: rgbrrrrr]: %.4f"%np.mean((im_redapple-im_order_3)**2))



    


if __name__ == "__main__":
    main()
