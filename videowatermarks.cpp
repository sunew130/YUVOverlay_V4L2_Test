#if 1
//#include "videoutil.h"
#include <stdio.h>

/**
 * 最简单的基于FFmpeg的AVFilter例子（叠加水印）
 * 本程序使用FFmpeg的AVfilter实现了视频的水印叠加功能。
 * 可以将一张PNG图片作为水印叠加到视频上。
 * 是最简单的FFmpeg的AVFilter方面的教程。
 * 适合FFmpeg的初学者。
 *
 * This software uses FFmpeg's AVFilter to add watermark in a video file.
 * It can add a PNG format picture as watermark to a video file.
 * It's the simplest example based on FFmpeg's AVFilter.
 * Suitable for beginner of FFmpeg
 *
 */

#include <QDebug>
extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
//#include <libavfilter/avfiltergraph.h>
//#include <libavfilter/buffersink.h>
//#include <libavfilter/buffersrc.h>
#include <libavutil/avutil.h>
#include <libswscale/swscale.h>
#include <SDL/SDL.h>
};
#if 0
//const char *filter_descr = "movie=logo.png[wm];[in][wm]overlay=5:5[out]";
const char *filter_descr = "color=color=red@3:sie=100x30[in]overlay=5:5[out]";
static AVCodecContext *pCodecCtx;
AVFilterContext *buffersink_ctx;
AVFilterContext *buffersrc_ctx;
AVFilterGraph *filter_graph;

static int init_filters(const char *filters_descr)
{
    char args[512];
    int ret;
    AVFilter *buffersrc  = avfilter_get_by_name("buffer");
    AVFilter *buffersink = avfilter_get_by_name("buffersink");
    AVFilterInOut *outputs = avfilter_inout_alloc();
    AVFilterInOut *inputs  = avfilter_inout_alloc();
    enum AVPixelFormat pix_fmts[] = { AV_PIX_FMT_YUVJ420P, AV_PIX_FMT_NONE };
    AVBufferSinkParams *buffersink_params;

    filter_graph = avfilter_graph_alloc();

    /* buffer video source: the decoded frames from the decoder will be inserted here. */
    snprintf(args, sizeof(args),
             "video_size=1280x720:pix_fmt=0:time_base=1/13:pixel_aspect=0/1",
             AV_PIX_FMT_YUVJ420P);

    ret = avfilter_graph_create_filter(&buffersrc_ctx, buffersrc, "in",
                                       args, NULL, filter_graph);
    if (ret < 0) {
        printf("Cannot create buffer source\n");
        return ret;
    }

    /* buffer video sink: to terminate the filter chain. */
    buffersink_params = av_buffersink_params_alloc();
    buffersink_params->pixel_fmts = pix_fmts;
    ret = avfilter_graph_create_filter(&buffersink_ctx, buffersink, "out",
                                       NULL, buffersink_params, filter_graph);
    av_free(buffersink_params);
    if (ret < 0) {
        printf("Cannot create buffer sink\n");
        return ret;
    }

    /* Endpoints for the filter graph. */
    outputs->name       = av_strdup("in");
    outputs->filter_ctx = buffersrc_ctx;
    outputs->pad_idx    = 0;
    outputs->next       = NULL;

    inputs->name       = av_strdup("out");
    inputs->filter_ctx = buffersink_ctx;
    inputs->pad_idx    = 0;
    inputs->next       = NULL;

    if ((ret = avfilter_graph_parse_ptr(filter_graph, filters_descr,
                                        &inputs, &outputs, NULL)) < 0)
        return ret;

    if ((ret = avfilter_graph_config(filter_graph, NULL)) < 0)
        return ret;
    return 0;
}



//添加水印图片
int frameAddWaterMarks(uint8_t *dstframe,uint8_t *srcframe,int width,int height)
{
    int ret;
    AVFrame *pFrame;
    AVFrame *pFrame_in;
    AVFrame *pFrame_out;

    av_register_all();
    avfilter_register_all();


    if ((ret = init_filters(filter_descr)) < 0)
    {
        avfilter_graph_free(&filter_graph);
        if (pCodecCtx)
            avcodec_close(pCodecCtx);

        if (ret < 0 && ret != AVERROR_EOF) {
            return -1;
        }
    }

    pFrame=av_frame_alloc();
    pFrame_out=av_frame_alloc();
    pFrame_in = av_frame_alloc();
    avpicture_fill((AVPicture *)pFrame, srcframe, AV_PIX_FMT_YUV420P,width, height);
    avpicture_fill((AVPicture *)pFrame_out, dstframe, AV_PIX_FMT_YUVJ420P,width, height);

    pFrame->width = width;
    pFrame->height = height;
    pFrame->format = AV_PIX_FMT_YUVJ420P;
    pFrame->sample_aspect_ratio.den = 1;
    pFrame->sample_aspect_ratio.num = 0;
    pFrame->pkt_pts = 12;
    pFrame->pkt_dts = 12;
    pFrame->pts = 12;
    pFrame->sample_rate = 12;
    pFrame->key_frame = 1;
    pFrame->pts = av_frame_get_best_effort_timestamp(pFrame);

    SwsContext* pSWSCtx = sws_getContext(width, height, AV_PIX_FMT_YUVJ420P, width,height, AV_PIX_FMT_YUVJ420P, SWS_BICUBIC, NULL, NULL, NULL);

    /* push the decoded frame into the filtergraph */
    if (av_buffersrc_add_frame(buffersrc_ctx, pFrame) < 0)
    {
        printf( "Error while feeding the filtergraph\n");
    }

    /* pull filtered pictures from the filtergraph */

    if (av_buffersink_get_frame(buffersink_ctx, pFrame_in) < 0 )
    {
        printf( "Error while feeding the filtergraph\n");
    }


    sws_scale(pSWSCtx, pFrame_in->data, pFrame_in->linesize,0, height, pFrame_out->data, pFrame_out->linesize);

    printf("pFrame_out\n");


    //av_frame_unref(pFrame_out);

    //av_frame_unref(pFrame);

    //free RGB
    //av_free(pFrame_out);

    //av_free(pFrame);


    avfilter_graph_free(&filter_graph);
    if (pCodecCtx)
        avcodec_close(pCodecCtx);

    if (ret < 0 && ret != AVERROR_EOF) {
        return -1;
    }

    return 0;
}

#endif
//init 一帧
int init_frame(AVFrame* frame,int width,int height,uint8_t* dst_buff)
{

    if(!avpicture_fill((AVPicture *) frame, dst_buff, AV_PIX_FMT_YUV420P,width,height))
    {
        puts("init frame error");
        av_free(&frame);
        return NULL;
    }
    frame->width=width;
    frame->height=height;
    frame->format = AV_PIX_FMT_YUV420P;
    return 0;
}


int frame_cover_white( AVFrame* dst_frame, AVFrame* src_frame, AVFrame* cover_frame)
{
    if(dst_frame == NULL || src_frame == NULL || cover_frame == NULL)//Œì²éºÏ·šÐÔ
    {
        puts("frame_cover_white input or output frame is NULL");
        return -1;
    }
    char* tempblack = (char*)malloc(sizeof(char) * cover_frame->linesize[0]);//²Î¿ŽËã·šÌÖÂÛ1
    memset(tempblack, 16, cover_frame->linesize[0]);

    int w2 = cover_frame->width;
    int h2 = cover_frame->height;

    int i = 0,j = 0;
    int temp,a,u,v,a2;
    float rat;
    int yindex = 0;
    int yindex2,uindex2;
    for(i = 0;i<h2;i++)
    {
        yindex = i*cover_frame->linesize[0];
        if(strncmp((char*)cover_frame->data[0]+yindex,tempblack,cover_frame->linesize[0])==0)//²Î¿Ž×¢1
        {
            continue;
        }
        yindex2 = i*src_frame->linesize[0];
        uindex2 = (i>>1)*src_frame->linesize[1];

        for(j=0;j<w2;j++)
        {

            a2 = cover_frame->data[0][yindex];
            if(a2 <= 40)//²Î¿ŽËã·šÌÖÂÛ2
            {
                yindex++;
                yindex2++;
                if(j%2!=0)
                    uindex2++;
                continue;
            }
            a = src_frame->data[0][yindex2];
            rat = a2*a2/40000.0;//²Î¿ŽËã·šÌÖÂÛ2

            if(rat > 1)
                rat = 1;

            temp = a+(255-a)*rat;

            dst_frame->data[0][yindex2] = (char)temp;
            u = src_frame->data[1][uindex2];
            v = src_frame->data[2][uindex2];
            dst_frame->data[1][uindex2]= u+(int)((128-u)*rat);//²Î¿ŽËã·šÌÖÂÛ3
            dst_frame->data[2][uindex2]= v+(int)((128-v)*rat);
            yindex++;
            yindex2++;
            if(j%2!=0)
                uindex2++;
        }
    }

    free(tempblack);
    return 0;
}


int frame_cover_color( AVFrame* dst_frame, AVFrame* src_frame, AVFrame* cover_frame,int whiteskip,int colorskip,int top)
{
    if(dst_frame == NULL || src_frame == NULL || cover_frame == NULL)
    {
        puts("frame_cover_white input or output frame is NULL");
        return -1;
    }
    char* tempblack = (char*)malloc(sizeof(char) * cover_frame->linesize[0]);
    memset(tempblack, 16, cover_frame->linesize[0]);
    int w2 = cover_frame->width;
    int h2 = cover_frame->height;
    int i = 0,j = 0;
    int temp,a,u,v,a2,u2,v2;
    float rat;
    int yindex = 0;
    int yindex2,uindex2;
    int udiv,vdiv;
    for(i = 0;i<h2;i++)
    {
        yindex = i*cover_frame->linesize[0];
        if(strncmp((char*)cover_frame->data[0]+yindex,tempblack,cover_frame->linesize[0])==0)
        {
            continue;
        }
        yindex2 = i*src_frame->linesize[0];
        uindex2 = (i>>1)*src_frame->linesize[1];
        for(j=0;j<w2;j++)
        {
            a2 = cover_frame->data[0][yindex];
            v2 = cover_frame->data[2][uindex2];
            u2 = cover_frame->data[1][uindex2];
            udiv = u2 - 128;
            vdiv = v2 - 128;
            if(udiv < 0) udiv = -udiv;
            if(vdiv < 0) vdiv = -vdiv;
            if((a2 <= whiteskip&& udiv+vdiv == 0)||(udiv+vdiv != 0)&&a2 <= colorskip)
            {
                yindex++;
                yindex2++;
                if(j%2!=0)
                    uindex2++;
                continue;
            }
            a = src_frame->data[0][yindex2];
            u = src_frame->data[1][uindex2];
            v = src_frame->data[2][uindex2];
            v2 = cover_frame->data[2][uindex2];
            u2 = cover_frame->data[1][uindex2];
            rat = (a2+udiv+vdiv)*(a2+udiv+vdiv)/(float)(top*top);
            if(rat > 1)
                rat = 1;
            dst_frame->data[0][yindex2] = (char)a+(a2-a)*rat;
            dst_frame->data[1][uindex2]= u+(int)((u2-u)*rat);
            dst_frame->data[2][uindex2]= v+(int)((v2-v)*rat);
            yindex++;
            yindex2++;
            if(j%2!=0)
                uindex2++;
        }
    }
    free(tempblack);
    return 0;
}



int initflag = 0;
uint8_t* watermark ;
AVFrame *scale_cover_frame;
//添加水印图片
int frameAddWaterMarks(uint8_t *srcframe,int width,int height)
{
    av_register_all();

    AVFrame *pFrame;

    pFrame=avcodec_alloc_frame();


    init_frame(pFrame,width,height,srcframe);

    if(initflag == 0)
    {
        scale_cover_frame =avcodec_alloc_frame();

        FILE* coverframe = (FILE*)fopen("/home/yateks/logo.yuv","rb");

        watermark = (uint8_t*)malloc(width*height*3/2);

        init_frame(scale_cover_frame,width,height,watermark);
        while(fread(watermark,1,width*height*3/2,coverframe));//读取原视频帧
        initflag = 1;

        fclose(coverframe);
    }

    frame_cover_color(pFrame,pFrame,scale_cover_frame,0,0,0);
    //frame_cover_white(pFrame, pFrame,scale_cover_frame);

}


void closeframeWaterMarks()
{
    av_free(scale_cover_frame);
    free(watermark);
    initflag = 0;
}



#endif
