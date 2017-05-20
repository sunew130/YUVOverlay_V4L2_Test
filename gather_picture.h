#ifndef GATHER_PICTURE_H
#define GATHER_PICTURE_H

#define VIDEO_DEV	"/dev/video9"	//��Ƶ�豸
#define WIDTH            640
#define HEIGHT           480

struct frame_buf{  //һ֡ͼ�񻺳���
		int length[4];
		void * start[4];
	};

struct frame_buf frame_buf;


int init_V4L2();
int gather_picture_init();
int gather_on();
int gather_off();
int video_getframe(int i);
int video_enqueue(int i);

#endif