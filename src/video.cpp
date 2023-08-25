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

bool load_frame(const char* filename, int* width_out, int* height_out, unsigned char** data_out) {
	AVFormatContext* av_format_ctx = avformat_alloc_context();
	if (!av_format_ctx) {
		printf("Failed to create av format context");
		return false;
	}
	if(avformat_open_input(&av_format_ctx, filename, NULL, NULL) < 0) {
		printf("couldn't open file");
		return false;
	}
		
	int video_stream_idx = -1;
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
			break;
		}
	}

	if (video_stream_idx == -1) {
		printf("Couldn't find video stream\n");
		return false;
	}

	AVCodecContext* av_codec_ctx = avcodec_alloc_context3(av_codec);
	if (!av_codec_ctx) {
		printf("Couldn't create AVCodecContext");
		return false;
	}
	if (avcodec_parameters_to_context(av_codec_ctx, av_codec_params) < 0) {
		printf("Couldn't intialize AvCodecContext\n");
		return false;
	}
	if (avcodec_open2(av_codec_ctx, av_codec, NULL) < 0) {
		printf("Couldn't open AVCodec\n");
		return false;
	}

	AVFrame* av_frame = av_frame_alloc();
	AVPacket* av_packet = av_packet_alloc();

	while ((av_read_frame(av_format_ctx, av_packet) >= 0)) {
		if (av_packet->stream_index != video_stream_idx)
			continue;
		int response;
		response = avcodec_send_packet(av_codec_ctx, av_packet);
		if (response < 0) {

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
	
	SwsContext* sws_ctx = sws_getContext(av_frame->width, av_frame->height, av_codec_ctx->pix_fmt,
										av_frame->width, av_frame->height, AV_PIX_FMT_RGB0, SWS_BILINEAR,
										NULL, NULL, NULL);
	if (!sws_ctx) {
		printf("Failed to create SwsContext\n");
		return -1;
	}
	uint8_t* data = new uint8_t[av_frame->width * av_frame->height * 4];
	uint8_t* dest[4] = { data, NULL, NULL, NULL };
	int dest_linesizes[4] = {av_frame->width * 4, 0, 0, 0};
	sws_scale(sws_ctx, av_frame->data, av_frame->linesize, 0, av_frame->height, dest, dest_linesizes);
	*width_out = av_frame->width;
	*height_out = av_frame->height;
	*data_out = data;
	sws_freeContext(sws_ctx);
	avformat_close_input(&av_format_ctx);
	avformat_free_context(av_format_ctx);
	avcodec_free_context(&av_codec_ctx);
	av_frame_free(&av_frame);
	av_packet_free(&av_packet);
	return true;
}
