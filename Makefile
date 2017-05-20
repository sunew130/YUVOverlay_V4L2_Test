all:test 

CC=gcc
CFLAGS=	-Wall -O2 -c
#INCLUDE=-I/opt/ffmpeg-0.10.1/include
#LIBS=-L/opt/ffmpeg-0.10.1/lib

OBJS=  main.o alsa.o
LIBS=  -lasound  #-lavcodec -lavformat -lswscale -lavutil -lpthread 

.c.o:
	$(CC) $(CFLAGS) $(LIBS) $<

test: $(OBJS)
	$(CC) -o $@ $^ $(LIBS)
	#/media/yatai/691862cf-2366-483d-9fe6-6fde7301ce80/home/yatai/tmp/YUVOverlay_V4L2_Test/a.sh
clean:
	rm -f *.o
	rm -f test.exe
	rm -f test.zip
