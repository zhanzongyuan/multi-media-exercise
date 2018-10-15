# Assignment1

## Usage



### 1.1 task1

- **task**: Transfrom `asserts/nobel.jpg` to `asserts/lena.jpg` through an opening circle.

- **solution**: Run the script `nobel2lena.py` with following command *(python3, tqdm, opencv3 are needed)*.

```shell
python3 nobel2lena.py
```

The program will output result video `output/nobel2lena.mp4` which transform from `asserts/nobel.jpg` to `asserts/lena.jpg`, like:

![nobel2lena_output](asserts/nobel2lena.GIF)

refs:

>  [create a video with opencv](https://docs.opencv.org/3.4.3/d7/d9e/tutorial_video_write.html)



### 1.3 Dithering

> My dithering sample experiment.

Run:

```shell
python3 dithering_sample.py
```

Then, we get result `output/dithered_4x4mat.bmp`:

![dithered_4x4mat](asserts/dithered_4x4mat.bmp)

You are allow to change to 2x2 dither matrix by changing the script `dithering_sample.py` in line 17-18:

```python
MATRIX = DITHER_MATRIX2x2
# MATRIX = DITHER_MATRIX4x4
```

2x2 result is like that:

![dithered_2x2mat](asserts/dithered_2x2mat.bmp)

