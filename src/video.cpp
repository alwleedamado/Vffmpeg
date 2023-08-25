extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
}
bool load_frame(const char* filename, int* width, int* height, unsigned char** data) {
	AVFormatContext* av_format_ctx = avformat_alloc_context();
	if (!av_format_ctx) {
		printf("Failed to create av format context");
		return false;
	}
	if(avformat_open_input(&av_format_ctx, filename, NULL, NULL) < 0) {
		printf("couldn't open file");
		return false;
	}
		
	
	return false;
}
