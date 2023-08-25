
#include "video.hpp"

bool video_reader_open(VideoReaderState* state,const char* filename) {
	auto& av_format_ctx = state->av_format_ctx;
	auto& av_codec_ctx = state->av_codec_ctx;
	auto& av_frame = state->av_frame;
	auto& av_packet = state->av_packet;
	auto& width = state->width;
	auto& height = state->height;
	auto& video_stream_idx = state->video_stream_idx;

	av_format_ctx = avformat_alloc_context();
	if (!av_format_ctx) {
		printf("Failed to create av format context");
		return false;
	}
	if (avformat_open_input(&av_format_ctx, filename, NULL, NULL) < 0) {
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
		printf("Couldn't intialize AvCodecContext\n");
		return false;
	}
	if (avcodec_open2(av_codec_ctx, av_codec, NULL) < 0) {
		printf("Couldn't open AVCodec\n");
		return false;
	}

	av_frame = av_frame_alloc();
	av_packet = av_packet_alloc();
	return true;
}

bool read_frame(VideoReaderState* state, uint8_t* data) {
	auto& av_format_ctx = state->av_format_ctx;
	auto& av_codec_ctx = state->av_codec_ctx;
	auto& av_frame = state->av_frame;
	auto& av_packet = state->av_packet;
	auto& width = state->width;
	auto& height = state->height;
	auto& video_stream_idx = state->video_stream_idx;
	auto& sws_ctx = state->sws_ctx;

	while ((av_read_frame(av_format_ctx, av_packet) >= 0)) {
		if (av_packet->stream_index != video_stream_idx)
			continue;
		int response;
		response = avcodec_send_packet(av_codec_ctx, av_packet);
		if (response < 0) {

			printf("Failed to decode frame %s\n", av_error(response));
			return false;
		}
		response = avcodec_receive_frame(av_codec_ctx,  av_frame);
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
			NULL, NULL, NULL);
	}
	if (!sws_ctx) {
		printf("Failed to create SwsContext\n");
		return -1;
	}
	uint8_t* dest[4] = { data, NULL, NULL, NULL };
	int dest_linesizes[4] = { width * 4, 0, 0, 0 };
	sws_scale(sws_ctx, av_frame->data, av_frame->linesize, 0, av_frame->height, dest, dest_linesizes);
	
	return true;
}
bool video_reader_close(VideoReaderState* state) {
	sws_freeContext(state->sws_ctx);
	avformat_close_input(&state->av_format_ctx);
	avformat_free_context(state->av_format_ctx);
	avcodec_free_context(&state->av_codec_ctx);
	av_frame_free(&state->av_frame);
	av_packet_free(&state->av_packet);
	return true;
}
