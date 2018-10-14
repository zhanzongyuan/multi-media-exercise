import cv2

OUT_CFG = {
        "filename": "output.mp4",
        "fourcc": cv2.VideoWriter_fourcc('m', 'p', '4', 'v'),
        "fps": 60
}

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
    
    for i in range(60):
        vi_output.write(im_lena)
    for i in range(60):
        vi_output.write(im_nobel)

    vi_output.release()

if __name__ == "__main__":
    main()
