#include <iostream>
#include <string>
#include <fstream>
#include <sys/time.h>

#include <stdint.h>
#include <unistd.h>
#include <cstdlib>
#include <libv4l2.h>
#include <linux/videodev2.h>
#include <malloc.h>
#include <pthread.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <errno.h>
#include <assert.h>


#include "nvbuf_utils.h"
#include "nvbufsurface.h"
#include "nvbufsurftransform.h"
#include "v4l2_nv_extensions.h"

using namespace std;

#include "video_decode.h"

#undef Status
#include <opencv4/opencv2/opencv.hpp>

typedef struct {
    char *data;
    int length;
    int bytesused;
    int eos;
} FrameData;

void cb(void* ctx, YuvStruct* yuv, void* ret)
{
    std::cout<<"width: " << yuv->width << "height: " << yuv->height << "size: " << yuv->len << std::endl;
}

extern "C" {
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libavutil/imgutils.h>
}

int main()
{
    int i = 0;
    std::string rtsp_url = "xxxxxxxx";
    AVFormatContext* format_context;
    int video_index;
    int ret = 0;
    AVPacket *packet;

    avformat_network_init();

    format_context = avformat_alloc_context();
    AVDictionary* options = NULL;
    av_dict_set(&options, "fflags", "nobuffer", 0);
    av_dict_set(&options, "buffer_size", "102400", 0); //设置缓存大小，1080p可将值调大
    av_dict_set(&options, "rtsp_transport", "udp", 0); //默认以udp方式打开，改为tcp
    av_dict_set(&options, "stimeout", "20000", 0); //设置超时断开连接时间，单位微秒
    av_dict_set(&options, "max_delay", "500", 0); //设置最大时延

    if(avformat_open_input(&format_context, rtsp_url.c_str(), NULL, &options) < 0)
    {
        printf("Couldn't open input stream.\n");
        return -1;
    }

    if(avformat_find_stream_info(format_context, NULL)<0)
    {
        printf("Couldn't find stream information.\n");
        return -1;
    }

    video_index = -1;
    video_index = av_find_best_stream(format_context, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
    if(video_index == -1)
    {
        printf("Couldn't find a stream video.\n");
        return -1;
    }

    int width = format_context->streams[video_index]->codecpar->width;
    int height = format_context->streams[video_index]->codecpar->height;
    if(width <= 0 || height <= 0)
    {
        return -1;
    }
    printf("width:%d ", width);
    printf("height:%d\n", height);
    packet = av_packet_alloc();
    struct timeval timenow;
    gettimeofday(&timenow,NULL);

    void* ctx = (void*)InitDecode(1920, 1080, Cb, NULL);

    FrameData buffer;
    buffer.length = 8 * 1024 * 1024;
    buffer.data = (char*)malloc(sizeof(char) * buffer.length);
    buffer.bytesused = 0;
    buffer.eos = 0;

    static int x = 0;

    FILE *fp = fopen("x.h264", "a+");
    if (fp == NULL)
    {
        printf("fopen x.h264 failed\n");
        return -1;
    }

    while(i++ < 100000)
    {
        memset(packet, 0, sizeof(packet));
        if(av_read_frame(format_context, packet) < 0)
        {
            continue;
        }
        if(packet->stream_index != video_index)
        {
            continue;
        }

        if(packet->flags & AV_PKT_FLAG_KEY) {
            printf("packet flags %d I frame\n", packet->flags);
        }

        fwrite(packet->data, packet->size, 1, fp);
        DoDecodeProcess(ctx, (const char*)packet->data, packet->size);
    }
    if (fp != NULL)
    {
        fclose(fp);
        fp = NULL;
    }
}
