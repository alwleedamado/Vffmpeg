#pragma once

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <inttypes.h>
#include <libswscale/swscale.h>
}

struct VideoReaderState {
	int width, height;
	int video_stream_idx;

	AVFormatContext* av_format_ctx;
	AVCodecContext* av_codec_ctx;
	AVFrame* av_frame;
	AVPacket* av_packet;
	SwsContext* sws_ctx = NULL;
};


template<int I>
struct Array2 {
	Array2() {};
	char data[I];
};
#define av_error(errnum) av_make_error_string( \
Array2<AV_ERROR_MAX_STRING_SIZE>().data, \
	AV_ERROR_MAX_STRING_SIZE, \
	errnum) \


bool video_reader_open(VideoReaderState* state, const char* filename);

bool read_frame(VideoReaderState* state, uint8_t* data);

bool video_reader_close(VideoReaderState* state);