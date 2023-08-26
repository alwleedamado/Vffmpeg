//
// Created by Alwaleed Ahmed on 8/26/2023.
//

#ifndef VFFMPEG_VIDEODECODER_H
#define VFFMPEG_VIDEODECODER_H

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <inttypes.h>
#include <libswscale/swscale.h>
}

template<int I>
struct Array2 {
    Array2() {};
    char data[I];
};

#define av_error(errnum) av_make_error_string( \
Array2<AV_ERROR_MAX_STRING_SIZE>().data, \
	AV_ERROR_MAX_STRING_SIZE, \
	errnum) \


class VideoDecoder {
public:
    bool openFIle(const char* filename);
    bool decodeFrame(uint8_t* outData);

    void close() {
        sws_freeContext(this->sws_ctx);
        avformat_close_input(&this->av_format_ctx);
        avformat_free_context(this->av_format_ctx);
        avcodec_free_context(&this->av_codec_ctx);
        av_frame_free(&this->av_frame);
        av_packet_free(&this->av_packet);
    }

    ~VideoDecoder() {
        close();
    }

public:
    [[nodiscard]] int getWidth() const { return width; }
    [[nodiscard]] int getHeight() const { return height; }

private:
    int width, height;
    unsigned int video_stream_idx = -1;

    AVFormatContext* av_format_ctx = NULL;
    AVCodecContext* av_codec_ctx = NULL;
    AVFrame* av_frame = NULL;
    AVPacket* av_packet = NULL;
    SwsContext* sws_ctx = NULL;
};


#endif //VFFMPEG_VIDEODECODER_H
