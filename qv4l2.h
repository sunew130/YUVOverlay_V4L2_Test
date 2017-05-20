#ifndef QV4L2_H
#define QV4L2_H

/*********************************************************************************
  *Copyright(C),2016-2017,亚泰光电有限公司
  *文件名： QV4L2
  *作者：sunew
  *版本：1.0
  *完成日期：
  *描述: linux下v4l2框架的摄像头设备获取，具有拍照，录制，放大，翻转，色彩调节功能
  *其他内容说明：无
  *修改事项：无
**********************************************************************************/

#include <QObject>
#include <QTimer>
#include <linux/types.h>
#include <linux/videodev2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <sys/stat.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>


#include <fcntl.h>
#include <unistd.h>
#include <malloc.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <jpeglib.h>
#include "qsdl.h"
#include "video/jpeg_decoder.h"
#include "colorspaces.h"
#include "photo/save_image.h"
#include "video/encoder.h"
#include "video/gview.h"
#include "video/gviewencoder.h"
#include "video/core_time.h"
#include <time.h>
#include <sys/time.h>
#include <sched.h>
#include <memory.h>
#include <QThread>
#include <QDebug>
#include <QTime>
#include <QFile>
#include <QMutex>
#include "video/videorecordingthread.h"
#include "video/toavithread.h"
#include "tools/videoutil.h"
#include "photo/photocaptrue.h"

#define	CLEAR(x)	memset (&x, 0, sizeof(x))


typedef	struct
{
    void	*start;
    size_t	length;
}buffer;


typedef	struct
{
    int			fd;
    int			n_buffers;
    char			*dev_name;
    buffer			*buffers;
    struct v4l2_buffer	buf;
    struct v4l2_format 	fmt;
}pass_data;



class QV4L2 : public QThread
{
    Q_OBJECT
public:
    QV4L2();
    ~QV4L2();

    enum{
        YUYV_FORMAT,/*YUV格式*/
        MJPEG_FORMAT/*MJPEG格式*/
    };

    int get_Dev(pass_data *s);//获取设备
    int init_Dev (pass_data *s);//初始化设备
    void stop_Dev (pass_data *s);//停止设备
    void read_Frame(pass_data *s);//读取帧
    void return_Data (pass_data *s);//返回帧空间

    void init_Mmap(pass_data *s);//初始化空间映射
    void init_Device(pass_data *s);//初始化设备
    int open_Device(pass_data *s);//打开设备
    void start_Capturing(pass_data *s);//开始获取
    void close_Device(pass_data *s);//关闭设备
    void stop_Capturing(pass_data *s);//停止获取
    void stop_Device(pass_data *s);//停止设备
    void errno_Exit(const char *s);//错误推出
    int xioctl(int fd, int request, void *arg);//操作设备

    //初始化SDL
    void init_SDL_Widget(long unsigned int winId,int width,int height,QString device);
    //开启SDL
    int begin_SDL_Capture();
    //停止SDL
    void stop_SDL_Capture();
    //暂停SDL
    void pause_SDL_Capture(bool flag);
    //释放SDL
    void close_SDL_Capture();

    //获取饱和度
    int get_Saturation();
    //获取色调
    int get_Hue();
    //获取白平衡
    int get_WhiteBalance();
    //获取对比度
    int get_Contrast();
    //获取亮度
    int get_Brightness();
    //获取锐度
    int get_Sharpness();

    //设置饱和度
    void set_Saturation(int value);
    //设置色调
    void set_Hue(int value);
    //设置锐度
    void set_Sharpness(int value);
    //设置对比度
    void set_Contrast(int value);
    //设置亮度
    void set_Brightness(int value);
    //设置白平衡
    void set_WhiteBalance(int value);
    //获取锁定状态
    bool get_Lock_Status();

    //设置控制
    void set_Control(struct v4l2_control *ctl);
    //获取控制
    int get_Control(struct v4l2_control *ctl);


    //拍照
    void set_isTakePhoto(bool val)
    {
        pthread.isCaptrueFrame = val;
    }

    //获取拍照状态
    bool get_isTakePhoto()
    {
        return  pthread.isCaptrueFrame;
    }

    //录制
    void set_isRecord(bool val)
    {
        isRecord = val;
        vthread.isRecord = val;

        if(val)
        {
            vthread.start();
        }
        else{
            if(vthread.isRunning())
            {
                vthread.isRecord = val;
            }
        }

    }

    //获取录制状态
    bool get_isRecord()
    {
        return  isRecord && vthread.isRecord;//
    }

    //获取录制完成装态
    bool get_isFinishRecord()
    {
        return vthread.isFinishRecord;
    }

    //设置照片名字
    void set_PhotoName(QString name)
    {
        photoname = name;
        pthread.setPhotoName(name);
    }

    //获取照片名字
    QString get_PhotoName()
    {
        return photoname;
    }


    //设置视频名字
    void set_VideoName(QString name)
    {
        videoname = name;
        vthread.setVideoName(videoname);

    }

    //获取视频名字
    QString get_VideoName()
    {
        return videoname;
    }

    //获取实时视频状态
    bool getDisplayTimer()
    {
        return timer->isActive();
    }

signals:
    void image_Thumbnail(QString);//图片缩略图
    void video_Thumbnail(QString);//视频缩略图

    void finishTakePhoto();//完成拍照


public slots:
    void show_SDL_Display();//显示SDL
    void show_Image_Display();//显示yv12图片
    bool rotate_SDL();//翻转
    int scale_SDL();//放大

public:
    bool isVideoDeviceErr;//视频设备错误

private:
    unsigned char* yuv;//源帧
    unsigned char* dstyuv;//目标帧
    unsigned char* newyuv;//新帧

    v4l2_frame_buff_t *frame;//帧结构体
    QSDL sdl;//SDL类
    pass_data pd;//帧
    int zoom ;//放大级数
    __u32 pixelformat;//图像格式

    int framewidth;//帧宽
    int frameheight;//帧高

    bool isRotate;//翻转
    bool isLock;//锁定
    bool isTakePhoto;//拍照
    bool isRecord;//录制
    bool isForceQuit; //强制退出


    QTimer *timer;//显示定时器
    QTimer *phototimer;//拍照定时器
    QMutex photomutex;//拍照锁

    QString devName;//设备名字

    QString photoname;//图片名字
    QString videoname;//视频名字

    VideoRecordingThread vthread;//录制线程
    PhotoCaptrue pthread;//拍照线程

    fd_set rfds,wfds;//
    struct timeval tv;

    int video_error_num;//错误帧数

};

#endif // QV4L2_H
