#include "alsa.h"

//初始化-----------------------------------------------------
Sound::Sound()
{
snd_mixer_open(&mixer, 0);
snd_mixer_attach(mixer, "default");
snd_mixer_selem_register(mixer, NULL, NULL);
snd_mixer_load(mixer);
//找到Pcm对应的element,方法比较笨拙
pcm_element = snd_mixer_first_elem(mixer);

//设定音量范围
snd_mixer_selem_set_playback_volume_range(pcm_element, 0, 100);

}

//读音量值-----------------------------------------------------
int Sound::sound_read()
{
long ll, lr;
//处理事件
snd_mixer_handle_events(mixer);
//左声道
snd_mixer_selem_get_playback_volume(pcm_element,
SND_MIXER_SCHN_FRONT_LEFT, &ll);
//右声道
snd_mixer_selem_get_playback_volume(pcm_element,
SND_MIXER_SCHN_FRONT_RIGHT, &lr);
return (ll + lr) >> 1;
}

//写入音量-----------------------------------------------------
void Sound::sound_write(int leftright)
{
//左音量
snd_mixer_selem_set_playback_volume(pcm_element,
SND_MIXER_SCHN_FRONT_LEFT,
leftright);
//右音量
snd_mixer_selem_set_playback_volume(pcm_element,
SND_MIXER_SCHN_FRONT_RIGHT,
leftright);
}

//退出-----------------------------------------------------
Sound::~Sound()
{
snd_mixer_close(mixer);
}
