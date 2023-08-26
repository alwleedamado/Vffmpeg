//
// Created by Alwaleed Ahmed on 8/26/2023.
//

#include "VideoDecoder.h"

bool VideoDecoder::openFIle(const char *filename) {
    av_format_ctx = avformat_alloc_context();
    if (!av_format_ctx) {
        printf("Failed to create av format context\n");
        return false;
    }
    if (avformat_open_input(&av_format_ctx, filename, nullptr, nullptr) < 0) {
        printf("couldn't open file");
        return false;
    }

    video_stream_idx = -1;
    AVCodecParameters* av_codec_params;
    const AVCodec* av_codec;
    for (unsigned int i = 0; i < av_format_ctx->nb_streams; ++i) {
        auto stream = av_format_ctx->streams[i];
        av_codec_params = stream->codecpar;
        av_codec = avcodec_find_decoder(av_codec_params->codec_id);
        if (!av_codec) {
            continue;
        }
        if (av_codec_params->codec_type == AVMEDIA_TYPE_VIDEO) {
            video_stream_idx = i;
            width = av_codec_params->width;
            height = av_codec_params->height;
            break;
        }
    }

    if (video_stream_idx == -1) {
        printf("Couldn't find video stream\n");
        return false;
    }

    av_codec_ctx = avcodec_alloc_context3(av_codec);
    if (!av_codec_ctx) {
        printf("Couldn't create AVCodecContext");
        return false;
    }
    if (avcodec_parameters_to_context(av_codec_ctx, av_codec_params) < 0) {
        printf("Couldn't initialize AvCodecContext\n");
        return false;
    }
    if (avcodec_open2(av_codec_ctx, av_codec, nullptr) < 0) {
        printf("Couldn't open AVCodec\n");
        return false;
    }

    av_frame = av_frame_alloc();
    av_packet = av_packet_alloc();
    return true;
}

bool VideoDecoder::decodeFrame(uint8_t *outData) {
    while ((av_read_frame(av_format_ctx, av_packet) >= 0)) {
        if (av_packet->stream_index != video_stream_idx) {
            av_packet_unref(av_packet);
            continue;
        }
        int response;
        response = avcodec_send_packet(av_codec_ctx, av_packet);
        if (response < 0) {
            av_packet_unref(av_packet);
            printf("Failed to decode frame %s\n", av_error(response));
            return false;
        }
        response = avcodec_receive_frame(av_codec_ctx, av_frame);
        if (response == AVERROR(EAGAIN) || response == AVERROR_EOF)
            continue;
        else if (response < 0) {
            printf("failed to decode packet %s\n", av_error(response));
            return false;
        }
        av_packet_unref(av_packet);
        break;
    }
    if (!sws_ctx) {
        sws_ctx = sws_getContext(width, height, av_codec_ctx->pix_fmt,
            width, height, AV_PIX_FMT_RGB0, SWS_BILINEAR,
            nullptr, nullptr, nullptr);
    }
    if (!sws_ctx) {
        printf("Failed to create SwsContext\n");
        return -1;
    }
    uint8_t* dest[4] = { outData, nullptr, nullptr, nullptr };
    int destLineSize[4] = {width * 4, 0, 0, 0 };
    sws_scale(sws_ctx, av_frame->data, av_frame->linesize, 0, av_frame->height, dest, destLineSize);

    return true;
}