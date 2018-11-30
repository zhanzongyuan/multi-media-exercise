# Assignment2

Here, I implement JPEG baseline compression standard describle in  *Fundamental of Multimedia* with c++ coding program.

## Usage

Please check you has install opencv lib and link it to pkg-config:

```shell
pkg-config --libs --cflags opencv
```

Then make file and run execuable file `asg2.out` with script:

```
make clean complie && ./asg2.out
```

Then it will output `.dat` file in output directory which is compressed binary file and output `.jpeg` file as result. Also the program will print `.gif` vs `.jpeg` compression ratio and jpeg distortion ratio (MSE & SNR)

You can also use python script to compute `.gif` file distortion ratio:

```shell
python3 measure_gif.py
```



> refs:
>
> BOOKS: Fundamental of Multimedia
>
> https://www.ijg.org/files/Wallace.JPEG.pdf
>
> typical Huffman tables for AC coefficient encoding:https://www.w3.org/Graphics/JPEG/itu-t81.pdf
>
>