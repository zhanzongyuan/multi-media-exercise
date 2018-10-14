import cv2
import math
import numpy as np
from tqdm import tqdm

OUT_CFG = {
        "filename": "nobel2lena.mp4",
        "fourcc": cv2.VideoWriter_fourcc('m', 'p', '4', 'v'),
        "fps": 60
}

VI_LEN = 5 # second

def drawcircle(imagebg, imagefront, R, center=None, preR=None):
    if center == None:
        center = [int(x/2) for x in imagebg.shape[0:2]]
    assert center[0] < imagebg.shape[0] and center[0] >= 0 and \
        center[1] < imagebg.shape[1] and center[1] >= 0
    assert R >= 0
    
    r_b, r_e = int(imagebg.shape[0]/2-R), int(imagebg.shape[0]/2+R)
    c_b, c_e = int(imagebg.shape[1]/2-R), int(imagebg.shape[1]/2+R)
    if r_b < 0: r_b = 0 
    if r_e >= imagebg.shape[0]: r_e = imagebg.shape[0]
    if c_b < 0: c_b = 0
    if c_e >= imagebg.shape[1]: c_e = imagebg.shape[1]
    for r in range(r_b, r_e):        
        for c in range(c_b, c_e):
            r2center = math.sqrt(abs(r-center[0])**2+abs(c-center[1])**2)
            if preR != None and r2center <= preR:
                continue
            if r2center <= R:
                f_r = int((imagefront.shape[0]-2*(center[0]-r))/2)
                f_c = int((imagefront.shape[1]-2*(center[1]-c))/2)

                if f_r < 0: f_r = 0
                if f_c < 0: f_c = 0
                if f_r >= imagefront.shape[0]: f_r = imagefront.shape[0]-1
                if f_c >= imagefront.shape[1]: f_c = imagefront.shape[1]-1
                imagebg[r][c] = imagefront[f_r][f_c]


def main():
    im_lena = cv2.imread("./asserts/lena.jpg")
    im_nobel = cv2.imread("./asserts/nobel.jpg")
    print("lena shape: ", im_lena.shape)
    print("nobel shape: ", im_nobel.shape)

    # Open output.mp4 writer 
    OUT_CFG["frameSize"] = im_lena.shape[0:2]
    vi_output = cv2.VideoWriter()
    retval = vi_output.open(**OUT_CFG)

    if not retval:
        print("VideoWriter: Open error")
        return
    
    header_footer_len = 0.5 # second
    hf_fnum = int(header_footer_len*OUT_CFG['fps'])
    c_fnum = OUT_CFG['fps']*VI_LEN - hf_fnum*2
    
    for i in range(hf_fnum):
        vi_output.write(im_nobel)

    MaxR = math.sqrt(im_nobel.shape[0]**2 + im_nobel.shape[1]**2)/2
    im_temp = im_nobel.copy()
    pR = None
    for i in tqdm(range(c_fnum)):
        R = (float(i)/c_fnum)*MaxR
        drawcircle(im_temp, im_lena, R, preR=pR)
        vi_output.write(im_temp)
        pR=R*3/4
    for i in range(hf_fnum):
        vi_output.write(im_lena)

    vi_output.release()

if __name__ == "__main__":
    main()
