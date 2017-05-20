#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/mman.h>
#include <linux/videodev2.h>

#include "gather_picture.h"

int g_videofd = -1;		//�豸������

struct v4l2_buffer buf;			  //�����е�һ֡ͼ�񻺴棬��Ӧ����VIDIOC_QUERYBUF

//Ͷ��һ���յ���Ƶ����������Ƶ��������������� ��
int video_enqueue(int i)
{
	buf.index = i;
	if(ioctl(g_videofd, VIDIOC_QBUF, &buf) < 0){
		perror("enqueue failed");
		return -1;
	}
	
	return 0;
}

//����ͷ�豸��Ϣ��ѯ������
int init_V4L2()
{
	struct v4l2_capability cap ;//��Ƶ�豸�Ĺ��ܣ���Ӧ����VIDIOC_QUERYCAP
	struct v4l2_fmtdesc stfmt; //��ǰ��Ƶ֧�ֵĸ�ʽ����Ӧ����VIDIOC_ENUM_FMT
	struct v4l2_format  fmt;   //��ǰ������Ƶ�����ʽʽ����Ӧ����VIDIOC_G_FMT��VIDIOC_S_FMT
		
	//1).������ͷ�豸
	g_videofd = open(VIDEO_DEV, O_RDWR);
	if(g_videofd == -1){
		perror("open");
		return -1;
	}

	//2).��ѯ����ͷ�豸�Ļ�����Ϣ������ //query ��cap������ʲô��˼
/*
	if(ioctl(g_videofd, VIDIOC_QUERYCAP, &cap) < 0){
		perror("ioctl");
		return -1;
	}
	else{
       	printf("driver:\t\t%s\n",cap.driver);
       	printf("card:\t\t%s\n",cap.card);
      	printf("bus_info:\t%s\n",cap.bus_info);
      	printf("version:\t%d\n",cap.version);
      	printf("capabilities:\t%#x\n",cap.capabilities);
	    if ((cap.capabilities & V4L2_CAP_VIDEO_CAPTURE) == V4L2_CAP_VIDEO_CAPTURE) 
        {
           printf("Device %s: supports capture.\n", VIDEO_DEV);
	    }
  
         if ((cap.capabilities & V4L2_CAP_STREAMING) == V4L2_CAP_STREAMING) 
         {
           printf("Device %s: supports streaming.\n", VIDEO_DEV);
         } 
	}

	//3).�о�����ͷ��֧�����ظ�ʽ��//ENUMʲô��˼���Լ����ȥ
	memset(&stfmt, 0, sizeof(stfmt));
	stfmt.index = 0;
	stfmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	printf("device support:\n");
	while(ioctl(g_videofd, VIDIOC_ENUM_FMT, &stfmt) != -1){
		printf("\t%d:  %s \n\n", stfmt.index++, stfmt.description);
		//stfmt.index++;
	}
	
	memset(&fmt, 0 ,sizeof(fmt));
        fmt.type=V4L2_BUF_TYPE_VIDEO_CAPTURE; 
         ioctl(g_videofd, VIDIOC_G_FMT, &fmt);

         printf("Current data format information:\n\twidth:%d\n\theight:%d\n",

fmt.fmt.pix.width,fmt.fmt.pix.height);

   memset(&fmt, 0 ,sizeof(fmt));
    fmt.type=V4L2_BUF_TYPE_VIDEO_CAPTURE; 
    //fmt.fmt.pix.pixelformat=V4L2_PIX_FMT_RGB32; 
  fmt.fmt.pix.pixelformat=V4L2_PIX_FMT_MJPEG; 
    fmt.fmt.pix.height = 100;//HEIGHT;
    fmt.fmt.pix.width = WIDTH;
    if(ioctl(g_videofd,VIDIOC_TRY_FMT,&fmt)==-1) 
     printf("not support format RGB32!\n");	

// printf("not support format RGB32!\n");

*/
//��ȡ����ͷ��֧�ֵķֱ���

    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    struct v4l2_fmtdesc fmt_1;
    struct v4l2_frmsizeenum frmsize;
    struct v4l2_frmivalenum frmival;

    fmt_1.index = 0;
    fmt_1.type = type;
    while (ioctl(g_videofd, VIDIOC_ENUM_FMT, &fmt_1) >= 0) {
		
      frmsize.pixel_format = fmt_1.pixelformat;
      frmsize.index = 0;
      while (ioctl(g_videofd, VIDIOC_ENUM_FRAMESIZES, &frmsize) >= 0){

if(frmsize.type == V4L2_FRMSIZE_TYPE_DISCRETE){
 printf("index:%d\nline:%d %dx%d\n",__LINE__, frmsize.index, frmsize.discrete.width, frmsize.discrete.height);
}else if(frmsize.type == V4L2_FRMSIZE_TYPE_STEPWISE){
 printf("index:%d\nline:%d %dx%d\n",__LINE__, frmsize.index, frmsize.discrete.width, frmsize.discrete.height);
}

frmsize.index++;
      }

      fmt_1.index++;
    }



	//4).���õ�ǰ������Ƶ�����ʽ,// //ֱ�Ӹ����㣬s��set����˼
	memset(&fmt, 0 ,sizeof(fmt));	
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG;
    fmt.fmt.pix.height = HEIGHT;
    fmt.fmt.pix.width = WIDTH;
    fmt.fmt.pix.field = V4L2_FIELD_INTERLACED;
    ioctl(g_videofd, VIDIOC_S_FMT, &fmt);
	printf("Size: %d,%d\n", fmt.fmt.pix.width, fmt.fmt.pix.height);
	printf("Video stored type: %d\n",fmt.fmt.pix.pixelformat);
    if(fmt.fmt.pix.height != 480){
      printf("%s,%d: Unable to set format\n",__func__,__LINE__);
      return -1;
    }     
	

	
	return 0;
}


//�ɼ���ʼ��
int gather_picture_init()
{	
	struct v4l2_requestbuffers reqbuf;//����֡���棬��Ӧ����VIDIOC_REQBUFS
	
	int i = 0;

	//1).����������֡����
	reqbuf.count = 4;// ����������Ҳ����˵�ڻ�������ﱣ�ֶ�������Ƭ 
 	reqbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	reqbuf.memory = V4L2_MEMORY_MMAP;
	if(ioctl(g_videofd, VIDIOC_REQBUFS, &reqbuf)==-1)
   	{
		perror("VIDEO_REQBUFS");
		return -1;
    }

	//2).��ȡ�����ÿ���������Ϣ����mmap���û��ռ�
	for (i = 0; i < reqbuf.count; i++) 
 	{
 	   buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
 	   buf.memory = V4L2_MEMORY_MMAP;
 	   buf.index = i;
 	   if (ioctl (g_videofd, VIDIOC_QUERYBUF, &buf) == -1)
 	   {
 	       printf("query buffer error\n");
	       return -1;
	   }
	   frame_buf.length[i] = buf.length;
	   frame_buf.start[i] = mmap(NULL, buf.length, PROT_READ |PROT_WRITE, MAP_SHARED, g_videofd, buf.m.offset);
       if(frame_buf.start[i] == MAP_FAILED)
       {
	        printf("buffer map error:%s,%d\n", __func__, __LINE__);
	        return-1;
       }
	   video_enqueue(i);//֡�������
	 }

	return 0;
	
}

//��ʼ�ɼ�
int gather_on()
{

 enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    struct v4l2_fmtdesc fmt_1;
    struct v4l2_frmsizeenum frmsize;
    struct v4l2_frmivalenum frmival;

    fmt_1.index = 0;
    fmt_1.type = type;
    while (ioctl(g_videofd, VIDIOC_ENUM_FMT, &fmt_1) >= 0) {
		
      frmsize.pixel_format = fmt_1.pixelformat;
      frmsize.index = 0;
      while (ioctl(g_videofd, VIDIOC_ENUM_FRAMESIZES, &frmsize) >= 0){

if(frmsize.type == V4L2_FRMSIZE_TYPE_DISCRETE){
 printf("index:%d\nline:%d %dx%d\n",__LINE__, frmsize.index, frmsize.discrete.width, frmsize.discrete.height);
}else if(frmsize.type == V4L2_FRMSIZE_TYPE_STEPWISE){
 printf("index:%d\nline:%d %dx%d\n",__LINE__, frmsize.index, frmsize.discrete.width, frmsize.discrete.height);
}

frmsize.index++;
      }

      fmt_1.index++;
    }

	// enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	 if(ioctl(g_videofd, VIDIOC_STREAMON, &type) < 0){
		perror("stream on");
		return -1;
	 }
	 return 0;
}

//�ɼ�����
int gather_off()
{
	enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if(ioctl(g_videofd, VIDIOC_STREAMOFF, &type) < 0){
		perror("stream on");
		return -1;
	}
	return 0;
}

//ȡ����Ƶ�����������������ȡ��һ���Ѿ�
//������һ֡��Ƶ���ݵ���Ƶ��������
int video_getframe(int i)
{
	buf.index = i;
	if(ioctl(g_videofd, VIDIOC_DQBUF, &buf) < 0){
		perror("release failed");
		return -1;
	}
	return 0;	
}

