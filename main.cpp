#include <stdio.h>
#include "alsa.h"
#include <stdlib.h>

int main(int agrv,char **agrc)
{

	int v = atoi(agrc[1]);		
	Sound s;
		
	printf("sound volume:%d \n",s.sound_read());
	s.sound_write(v);
	printf("change sound volume:%d \n",s.sound_read());	

}
