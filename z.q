#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
/*
#include <sys/mman.h>
#include <linux/videodev2.h>
#include <linux/fb.h>
#include <pthread.h>

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>

//#include <SDL/SDL.h>
//#include <SDL/SDL_thread.h>

#include "gather_picture.h"
*/
#include <sys/statfs.h>
#include <stdio.h> 

#define PRINT                 printf("LINE: %d\n",__LINE__)


int main(int argc,char **argv)
{   
  FILE *fp = fopen("/mnt/sdcard/a.c","wb");

    if(fp == NULL)
    {
        printf("sd card not use");
    }
    else
    {
        fclose(fp);
        printf("sdcard can be used");
    }
}
 
/* 
int mainq(int argc,char **argv) 
{ 
    struct statfs diskInfo; 
      
    statfs(argv[1], &diskInfo); 
    unsigned long long blocksize = diskInfo.f_bsize; //每个block里包含的字节数 
    unsigned long long totalsize = blocksize * diskInfo.f_blocks; //总的字节数，f_blocks为block的数目 
    printf("Total_size = %llu B = %llu KB = %llu MB = %llu GB\n", 
        totalsize, totalsize>>10, totalsize>>20, totalsize>>30); 
      
    unsigned long long freeDisk = diskInfo.f_bfree * blocksize; //剩余空间的大小 
    unsigned long long availableDisk = diskInfo.f_bavail * blocksize; //可用空间大小 
    printf("Disk_free = %llu MB = %llu GB\nDisk_available = %llu MB = %llu GB\n", 
        freeDisk>>20, freeDisk>>30, availableDisk>>20, availableDisk>>30); 
		
	
      
    return 0; 
}


int mainw(int argc, char **argv)
{
#if 0
	int n = 1, i;	
	unsigned char frmrate;
	unsigned char *p = NULL;
	char* status = NULL;
	SDL_Overlay *bmp = NULL;
  	SDL_Surface *screen = NULL;
  	SDL_Rect rect;
  	SDL_Event event;
	struct SwsContext *sws_ctx = NULL;
	

	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER)) {
    		fprintf(stderr, "Could not initialize SDL - %s\n", SDL_GetError());
    		exit(1);
  	}

	#ifndef __DARWIN__
        	screen = SDL_SetVideoMode(WIDTH, HEIGHT, 0, 0);
	#else
        	screen = SDL_SetVideoMode(WIDTH, HEIGHT, 24, 0);
	#endif
	
  	if(!screen) {
   		 fprintf(stderr, "SDL: could not set video mode - exiting\n");
   		 exit(1);
 	 }

	  bmp = SDL_CreateYUVOverlay(WIDTH,
		HEIGHT, SDL_YUY2_OVERLAY, screen);

	  p = (unsigned char *) bmp->pixels[0];

	  rect.x = 0;
	  rect.y = 0;
	  rect.w = WIDTH;
	  rect.h = HEIGHT;
#endif	
	init_V4L2();
	gather_picture_init();
	gather_on();
#if 0	
	while(1){
		for(i = 0; i < 4; i++){
			video_getframe(i);
			//write(sock, frame_buf[i].start, frame_buf[i].length);
			
			SDL_LockYUVOverlay(bmp);
			memcpy(p, frame_buf.start[i], WIDTH* HEIGHT* 2);
			SDL_UnlockYUVOverlay(bmp);
			
			SDL_DisplayYUVOverlay(bmp, &rect);	

			if(video_enqueue(i) < 0){
				printf("video_enqueue() failed\n");
				return -1;
			}

			SDL_PollEvent(&event);
    			switch(event.type) {
    				case SDL_QUIT:
					SDL_FreeYUVOverlay(bmp);
      					SDL_Quit();
      					return 0;
      					break;
    				default:
      					break;
			}
		}
	}
#endif
	return 0;
}
*/
