#include <alsa/asoundlib.h>

//类定义
class Sound {
public:
Sound();
~Sound();
int sound_read();
void sound_write(int);
private:
snd_mixer_t * mixer;
snd_mixer_elem_t *pcm_element;
};
