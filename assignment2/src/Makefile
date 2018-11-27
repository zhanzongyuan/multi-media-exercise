# OPENCV_LIBS = $(shell pkg-config --libs opencv)
# OPENCV_HEADERS = $(shell pkg-config --cflags opencv)
CLANG = clang++
OPENCV_LIBS = `pkg-config --libs opencv`
OPENCV_HEADERS = `pkg-config --cflags opencv`

complie: ./build/mjpeg.o
	$(CLANG) main.cpp  $(OPENCV_LIBS) $(OPENCV_HEADERS) ./build/mjpeg.o -o asg2.out

./build/mjpeg.o:
	$(CLANG) -c mjpeg.cpp -o ./build/mjpeg.o

test: ./build/mjpeg_test.o
	$(CLANG) test.cpp  $(OPENCV_LIBS) $(OPENCV_HEADERS) ./build/mjpeg_test.o -o test.out -g && ./test.out

./build/mjpeg_test.o:
	$(CLANG) -c mjpeg.cpp -o ./build/mjpeg_test.o -g


clean:
	rm -rf build/*