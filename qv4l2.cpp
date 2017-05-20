#include "qv4l2.h"
#include "colorspaces.h"

QV4L2::QV4L2()
{

    /*初始化参数*/
    zoom = 0;
    video_error_num = 0;//错误帧数
    isRotate = true;
    isTakePhoto = false;
    isRecord = false;
    isLock = false;
    isForceQuit = false;
    isVideoDeviceErr = false;

    /*创建显示定时器*/
    timer = new QTimer(this);
    connect(timer,SIGNAL(timeout()),this,SLOT(show_Image_Display()));

    /*链接拍照，录像的信号，并生成缩略图*/
    connect(&vthread,SIGNAL(thumbnail(QString)),this,SIGNAL(video_Thumbnail(QString)));
    connect(&pthread,SIGNAL(image_Thumbnail(QString)),this,SIGNAL(image_Thumbnail(QString)));
    connect(&pthread,SIGNAL(image_Thumbnail(QString)),this,SIGNAL(finishTakePhoto()));

    /*拍照间隔时间*/
    phototimer = new QTimer(this);
    connect(phototimer,SIGNAL(timeout()),this,SIGNAL(finishTakePhoto()));
}



QV4L2::~QV4L2()
{

    /*若线程没有结束，停止线程*/
    if(vthread.isRunning())
        vthread.terminate();

}


//获取摄像设备号
int QV4L2::get_Dev(pass_data *s)
{
    /*打开设备*/
    int flag = open_Device(s);
    if (flag != 0) {

        return flag;
    }
    /*初始化设备*/
    init_Device(s);

    return flag;
}

//初始化摄像头设备
int QV4L2::init_Dev (pass_data *s)
{
    /*初始化映射空间*/
    init_Mmap(s);
    /*开始捕获*/
    start_Capturing(s);
    fprintf(stdout, "'%s' initialize finish ...\n", s->dev_name);
    return 0;
}

//停止摄像头设备
void QV4L2::stop_Dev (pass_data *s)
{
    /*停止捕获*/
    stop_Capturing(s);
    /*停止设备*/
    stop_Device(s);
    /*关闭设备*/
    close_Device(s);
    fprintf(stdout, "close '%s' ...\n", s->dev_name);
}

//获取视频帧
void QV4L2::read_Frame(pass_data *s)
{
    CLEAR (s->buf);

    s->buf.type	=	V4L2_BUF_TYPE_VIDEO_CAPTURE;
    s->buf.memory	= 	V4L2_MEMORY_MMAP;

    /*判断摄像头设备是否存在*/
    QFile file(devName);
    if(!file.exists())
    {
        isVideoDeviceErr = false;
        timer->stop();
        if(isRecord)
        {
            isRecord = false;
            vthread.isRecord = false;
        }
        close_Device(&pd);
        file.close();
        return;
    }
    file.close();

    if (xioctl (s->fd, VIDIOC_DQBUF, &s->buf) == -1)
    {
        switch (errno)
        {
        case EAGAIN:
            errno_Exit ("VIDIOC_DQBUF");
        case EIO:
            /* could ignore EIO, see spec. */
        default:
            errno_Exit ("VIDIOC_DQBUF");
        }

    }

    assert ((int)s->buf.index < s->n_buffers);
}

//回收视频帧空间
void QV4L2::return_Data (pass_data *s)
{
    if (xioctl (s->fd, VIDIOC_QBUF, &s->buf) == -1)	errno_Exit ("VIDIOC_QBUF");
}

//开始捕获视频
void QV4L2::start_Capturing(pass_data *s)
{
    int i;
    enum v4l2_buf_type type;

    for (i = 0; i < s->n_buffers; ++ i)
    {
        struct v4l2_buffer buf;
        CLEAR (buf);

        buf.type	= 	V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory	=	V4L2_MEMORY_MMAP;
        buf.index	=	i;

        if (xioctl (s->fd, VIDIOC_QBUF, &buf) == -1)
            errno_Exit("VIDIOC_QBUF");
    }
    type	=	V4L2_BUF_TYPE_VIDEO_CAPTURE;

    if (xioctl (s->fd, VIDIOC_STREAMON, &type))
        errno_Exit("VIDIOC_STREAMON");
}

//停止捕获视频
void QV4L2::stop_Capturing(pass_data *s)
{
    enum v4l2_buf_type type;

    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    if (xioctl (s->fd, VIDIOC_STREAMOFF, &type))
        errno_Exit("VIDIOC_STREAMOFF");
}

//初始化映射空间
void QV4L2::init_Mmap(pass_data *s)
{
    struct v4l2_requestbuffers req;
    CLEAR (req);
    req.count	=	4;
    req.type	=	V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory	=	V4L2_MEMORY_MMAP;

    if (xioctl (s->fd, VIDIOC_REQBUFS, &req))
    {
        if (EINVAL == errno)
        {
            fprintf(stderr, "%s does not support 'memory mapping'\n", s->dev_name);
            exit (EXIT_FAILURE);
        }
        else
        {
            errno_Exit ("VIDIOC_REQBUFS");
        }
    }

    if (req.count < 2)
    {
        fprintf(stderr, "Insufficient buffer memory on %s\n", s->dev_name);
        exit (EXIT_FAILURE);
    }

    if ((s->buffers = (buffer *)calloc (req.count, sizeof (*s->buffers))) == NULL)
    {
        fprintf(stderr, "Out of memory\n");
        exit ( EXIT_FAILURE);
    }

    for (s->n_buffers = 0; s->n_buffers < (int)req.count; ++ s->n_buffers)
    {
        struct v4l2_buffer buf;
        CLEAR (buf);

        buf.type	=	V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory	=	V4L2_MEMORY_MMAP;
        buf.index	=	s->n_buffers;

        if (xioctl (s->fd, VIDIOC_QUERYBUF, &buf) == -1)
            errno_Exit("VIDIOC_QUERYBUF");

        s->buffers[s->n_buffers].length	=	buf.length;
        s->buffers[s->n_buffers].start	=
                mmap(   NULL,
                        buf.length,
                        PROT_READ | PROT_WRITE,
                        MAP_SHARED,
                        s->fd,
                        buf.m.offset
                        );

        if (s->buffers[s->n_buffers].start == MAP_FAILED)
            errno_Exit ("mmap");
#if 	_DEBUG_
        fprintf(stdout, "%d -> %p\n", s->n_buffers, s->buffers[s->n_buffers].start);
#endif
    }
}

//v4l2初始化设备
void QV4L2::init_Device(pass_data* s)
{
    struct v4l2_capability cap;
    struct v4l2_cropcap cropcap;
    struct v4l2_crop crop;
    unsigned int min;

    if (xioctl (s->fd, VIDIOC_QUERYCAP, &cap) == -1)
    {
        if (EINVAL == errno)
        {
            fprintf (stderr, "%s is no V4L2 device\n", s->dev_name);
            exit (EXIT_FAILURE);
        }
        else
        {
            errno_Exit ("VIDIOC_QUERYCAP");
        }
    }

    if (! (cap.capabilities & V4L2_CAP_VIDEO_CAPTURE))
    {
        fprintf(stderr, "%s is no video captrue device\n", s->dev_name);
        exit(EXIT_FAILURE);
    }

    if (! (cap.capabilities & V4L2_CAP_STREAMING))
    {
        fprintf(stderr, "%s does not support straming I/O\n", s->dev_name);
        exit(EXIT_FAILURE);
    }

    if (! (cap.capabilities & (V4L2_CAP_VIDEO_CAPTURE|V4L2_CAP_STREAMING)))
    {
        fprintf(stderr, "%s does not support captrue device and straming I/O\n", s->dev_name);
        exit(EXIT_FAILURE);
    }


    CLEAR(cropcap);

    cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    if (xioctl (s->fd, VIDIOC_CROPCAP, &cropcap) == 0)
    {
        crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        crop.c = cropcap.defrect;

        if (xioctl (s->fd, VIDIOC_S_CROP, &crop))
        {
            switch (errno)
            {
            case EINVAL:
                break;
            default:
                break;
            }
        }
        else
        {
            /* Errors ignored */
        }
    }
    CLEAR (s->fmt);

    struct v4l2_fmtdesc fmtdesc;
    CLEAR(fmtdesc);

    fmtdesc.index = 0;
    fmtdesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    //pixelformat = MJPEG_FORMAT;

    while(xioctl(s->fd,VIDIOC_ENUM_FMT,&fmtdesc) != -1)
    {
        qDebug("====================================\n");
        qDebug("index is %x\n",fmtdesc.index);
        qDebug("type is %x\n",fmtdesc.type);
        qDebug("flags is %#x\n",fmtdesc.flags);
        qDebug("description is %s\n",fmtdesc.description);
        qDebug("pixelformat is %c %c %c %c\n",
               fmtdesc.pixelformat & 0xFF,
               (fmtdesc.pixelformat >> 8) & 0xFF,
               (fmtdesc.pixelformat >> 16) & 0xFF,
               (fmtdesc.pixelformat >> 24) & 0xFF);
        qDebug()<<fmtdesc.pixelformat;
        pixelformat = fmtdesc.pixelformat;
        fmtdesc.index++;
    }

    struct v4l2_format format;
    format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if(xioctl(s->fd,VIDIOC_G_FMT,&format) < 0)
    {
        exit(EXIT_FAILURE);
    }

    s->fmt.type = format.type ;
    s->fmt.fmt.pix.field = V4L2_FIELD_NONE;
    s->fmt.fmt.pix.width = format.fmt.pix.width;
    s->fmt.fmt.pix.height = format.fmt.pix.height;
    s->fmt.fmt.pix.pixelformat = pixelformat;
    framewidth = format.fmt.pix.width;
    frameheight = format.fmt.pix.height;

#if 0
    if(pixelformat == MJPEG_FORMAT)
    {
        //s->fmt.fmt.pix.width = SD_WIDTH;
        //s->fmt.fmt.pix.height = SD_HEIGHT;
        //s->fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG;
       // framewidth = SD_WIDTH;
        //frameheight = SD_HEIGHT;
    }
    else
    {
        //s->fmt.fmt.pix.width = VGA_WIDTH;
        //s->fmt.fmt.pix.height = VGA_HEIGHT;
        //s->fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
        //framewidth = VGA_WIDTH;
        //frameheight = VGA_HEIGHT;
    }
#endif
    if (xioctl (s->fd, VIDIOC_S_FMT, &s->fmt) == -1)
        errno_Exit("VIDIOC_S_FMT");

    min = s->fmt.fmt.pix.width * 2;
    if (s->fmt.fmt.pix.bytesperline < min)
        s->fmt.fmt.pix.bytesperline = min;

    min = s->fmt.fmt.pix.bytesperline * s->fmt.fmt.pix.height;
    if (s->fmt.fmt.pix.sizeimage < min)
        s->fmt.fmt.pix.sizeimage = min;


    /*分配帧空间*/
    frame = (v4l2_frame_buff_t *)malloc(sizeof(v4l2_frame_buff_t ));
    frame->width = framewidth;
    frame->height = frameheight;
    yuv =  (unsigned char *)malloc(framewidth*frameheight* 3 /2);
    dstyuv =  (unsigned char *)malloc(framewidth*frameheight* 3 /2);
    newyuv = (uint8_t *)malloc(framewidth*frameheight*3/2);

    /*拍照，录像线程初始化*/
    vthread.videoRecordInit(framewidth,frameheight,pixelformat);
    pthread.captrueInit(framewidth,frameheight);

    /*设置SDL大小*/
    sdl.setSDLFrameSize(framewidth,frameheight);

}


//断开设备映射
void QV4L2::stop_Device(pass_data *s)
{
    int i;

    for (i = 0; i < s->n_buffers; ++i)
        if (munmap (s->buffers[i].start, s->buffers[i].length) == -1)
            errno_Exit("munmap");
}

//打开摄像头设备
int QV4L2::open_Device(pass_data *s)
{
    struct stat st;

    if (stat (s->dev_name, &st) == -1)
    {
        fprintf(stderr, "Can't identify '%s':[%d] %s\n", s->dev_name, errno, strerror(errno));
        return -1;
    }

    if (!S_ISCHR (st.st_mode))
    {
        fprintf(stderr, "%s is no device\n", s->dev_name);
        return -2;
    }

    if ((s->fd = open (s->dev_name, O_RDWR, 0)) == -1 )
    {
        fprintf(stderr, "Can't oprn '%s': error %d, %s\n", s->dev_name, errno, strerror(errno));
        return -2;
    }
    return 0;
}

//关闭设备
void QV4L2::close_Device(pass_data *s)
{
    close (s->fd);
}

//操作设备
int QV4L2::xioctl(int fd, int request, void *arg)
{
    int r;

    do r = ioctl(fd, request, arg);

    while(r == -1 && EINTR == errno);

    return r;
}

//摄像头错误退出
void QV4L2::errno_Exit(const char *s)
{

    fprintf(stderr, "%s error %d, %s\n", s, errno, strerror(errno));
    exit(EXIT_FAILURE);
}


//设置视频控制
void QV4L2::set_Control(struct v4l2_control *ctl)
{

    struct v4l2_queryctrl qctl;
    qctl.id = ctl->id;

    if(xioctl(pd.fd,VIDIOC_QUERYCTRL,&qctl)< 0)
    {
        qDebug()<<"FAILED";
    }

    if (1) {
        if (ctl->value < 0) {
            ctl->value = qctl.default_value + ctl->value *
                    (qctl.default_value - qctl.minimum) / 100;
        } else {
            ctl->value = qctl.default_value + ctl->value *
                    (qctl.maximum - qctl.default_value) / 100;
        }
    } else {
        if (ctl->value < 50) {
            ctl->value = qctl.default_value + (ctl->value-50) *
                    (qctl.default_value - qctl.minimum) / 50;
        } else {
            ctl->value = qctl.default_value + (ctl->value-50) *
                    (qctl.maximum - qctl.default_value) / 50;
        }
    }

    if(xioctl(pd.fd,VIDIOC_S_CTRL,ctl) < 0)
    {
        exit(EXIT_FAILURE);
    }

}

//设置色调
void QV4L2::set_Saturation(int value)
{
    struct v4l2_control ctl;
    ctl.id=V4L2_CID_SATURATION;
    ctl.value= value;
    set_Control(&ctl);

}



//设置锐度
void QV4L2::set_Sharpness(int value)
{
    struct v4l2_control ctl;
    ctl.id=V4L2_CID_SHARPNESS;
    ctl.value= value;
    set_Control(&ctl);

}

//设置亮度
void QV4L2::set_Brightness(int value)
{
    struct v4l2_control ctl;
    ctl.id=V4L2_CID_BRIGHTNESS;
    ctl.value= value;
    set_Control(&ctl);
}

//设置饱和度
void QV4L2::set_Contrast(int value)
{
    struct v4l2_control ctl;
    ctl.id=V4L2_CID_CONTRAST;
    ctl.value= value;
    set_Control(&ctl);
}

//设置对比度
void QV4L2::set_Hue(int value)
{
    struct v4l2_control ctl;
    ctl.id=V4L2_CID_HUE;
    ctl.value= value;
    set_Control(&ctl);

}

//设置白平衡
void QV4L2::set_WhiteBalance(int value)
{
    struct v4l2_control ctl;
    ctl.id=V4L2_CID_DO_WHITE_BALANCE;
    ctl.value= value;
    set_Control(&ctl);
}


//获取视频控制
int QV4L2::get_Control(struct v4l2_control *control)
{
    struct v4l2_queryctrl        qctrl;

    qctrl.id = control->id;
    if (ioctl(pd.fd, VIDIOC_QUERYCTRL, &qctrl) < 0) {
        qDebug()<<"VIDIOC_QUERYCTRL FAILED";
    }

    if (ioctl(pd.fd, VIDIOC_G_CTRL, control) < 0) {
        qDebug()<<"VIDIOC_G_CTRL FAILED";
    }

    if (1) {
        if (control->value < qctrl.default_value) {
            control->value = (control->value - qctrl.default_value) * 100 /
                    (qctrl.default_value - qctrl.minimum);
        } else {
            control->value = (control->value - qctrl.default_value) * 100 /
                    (qctrl.maximum - qctrl.default_value);
        }
    } else {
        if (control->value < qctrl.default_value) {
            control->value = (control->value - qctrl.default_value) * 50 /
                    (qctrl.default_value - qctrl.minimum) + 50;
        } else {
            control->value = (control->value - qctrl.default_value) * 50 /
                    (qctrl.maximum - qctrl.default_value) + 50;
        }
    }

    return control->value;
}


//获取色调
int QV4L2::get_Saturation()
{
    struct v4l2_control ctrl;
    ctrl.id = V4L2_CID_SATURATION;
    return get_Control(&ctrl);
}

//获取对比度
int QV4L2::get_Hue()
{
    struct v4l2_control ctrl;
    ctrl.id = V4L2_CID_HUE;
    return get_Control(&ctrl);

}

//获取白平衡
int QV4L2::get_WhiteBalance()
{
    struct v4l2_control ctrl;
    ctrl.id = V4L2_CID_DO_WHITE_BALANCE;
    return get_Control(&ctrl);
}

//获取饱和度
int QV4L2::get_Contrast()
{
    struct v4l2_control ctrl;
    ctrl.id = V4L2_CID_CONTRAST;
    return get_Control(&ctrl);
}

//获取亮度
int QV4L2::get_Brightness()
{
    struct v4l2_control ctrl;
    ctrl.id = V4L2_CID_BRIGHTNESS;
    return get_Control(&ctrl);
}

//获取锐度
int QV4L2::get_Sharpness()
{
    struct v4l2_control ctrl;
    ctrl.id = V4L2_CID_SHARPNESS;
    return get_Control(&ctrl);
}

//获取锁定屏幕状态
bool QV4L2::get_Lock_Status()
{
    return isLock;
}

//翻转视频
bool QV4L2::rotate_SDL()
{
    if(isRotate)
        isRotate = false;
    else
        isRotate = true;
    return isRotate;
}

//初始化ｓｄｌ视频
void QV4L2::init_SDL_Widget(long unsigned int winId,int width,int height,QString device)
{
    devName = device;
    pd.dev_name = device.toLocal8Bit().data();

    if(get_Dev(&pd) < 0)
    {
         qDebug()<<"init error";
    }

    sdl.createSDLWindow(winId,width,height);

}


//开始ｓｄｌ
int QV4L2::begin_SDL_Capture()
{
    init_Dev(&pd);
    timer->setInterval(10);
    timer->start();
    return 0;
}

//停止ｓｄｌ
void QV4L2::stop_SDL_Capture()
{
    if(!isForceQuit)/*是否强制退出，若是，则释放空间*/
    {
        isForceQuit = true;
        timer->stop();

        delete timer;
        delete phototimer;

        free(yuv);
        free(newyuv);
        free(dstyuv);
        free(frame);

        stop_Dev(&pd);
        close_SDL_Capture();

    }
}

//暂定sdl视频
void QV4L2::pause_SDL_Capture(bool flag)
{
    isLock = flag;
}

//关闭ｓｄｌ视频
void QV4L2::close_SDL_Capture()
{
    sdl.freeSDL();
}

//显示视频帧
void QV4L2::show_Image_Display()
{

    tv.tv_sec = 0;
    tv.tv_usec = 700000;
    FD_ZERO(&rfds);
    FD_SET(pd.fd, &rfds);
    FD_ZERO(&wfds);
    FD_SET(pd.fd, &wfds);
    int ret = select(pd.fd + 1, &rfds, &wfds, NULL, &tv);

    /*判断帧获取是否超时，若是，则报错设备错误*/
    if((video_error_num > 0) && (video_error_num < 8) && (ret == 1))
        video_error_num =0;

    if(ret  == 0)
    {
        video_error_num++;

        if(video_error_num >= 8)
            isVideoDeviceErr = true;
    }
    else
    {
        if(FD_ISSET(pd.fd, &rfds))
            read_Frame(&pd);

        if(!timer->isActive())
        {
            return_Data(&pd);
            stop_SDL_Capture();
            return;
        }

        /*摄像头获取格式*/
        if(pixelformat == V4L2_PIX_FMT_MJPEG)
        {
            jpeg_decode((uint8_t *)yuv, (uint8_t *)pd.buffers[pd.buf.index].start, pd.buffers[pd.buf.index].length);
            //  YUV420ConvertImage(yuv,SD_WIDTH,SD_HEIGHT,100,0,0);
        }
        else
        {
            yuyv_to_yv12((uint8_t *)yuv,(uint8_t *)pd.buffers[pd.buf.index].start,VGA_WIDTH,VGA_HEIGHT);
            // YUV420ConvertImage(yuv,VGA_WIDTH,VGA_HEIGHT,100,0,0);
        }

        /*显示摄像头*/
        show_SDL_Display();
    }
    return_Data(&pd);
}

//显示SDL
void QV4L2::show_SDL_Display()
{
    if(isLock)
        return;

    if(!isRotate)/*是否翻转180度*/
    {
        rotate_180_frame((uint8_t *)yuv,framewidth,frameheight);
    }

    if(zoom != 0)/*是否放大*/
    {
        if(zoom == 6)
        {
            zoom = 0;

        }
        crop_frame(dstyuv,yuv,framewidth*zoom/16,frameheight*zoom/16,framewidth,frameheight,\
                   framewidth - framewidth*zoom/8,frameheight - frameheight*zoom/8);

    }

    /*拍照*/
    pthread.addFrametoPhoto(yuv);

    if(isRecord)/*是否录像*/
    {
        frame->isKeyframe = 0;
        frame->timestamp = ns_time_monotonic();
        frame->yuv_frame = yuv;
        qDebug()<<"recording";

        vthread.videoRecordCapture(frame);

    }

    sdl.showSDL((unsigned char *)yuv);
}



//缩小放大
int QV4L2::scale_SDL()
{
    int zoomnum = ++zoom;

    if(zoomnum == 6)
        return 0;
    return zoomnum;

}



