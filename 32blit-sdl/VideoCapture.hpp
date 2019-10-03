#pragma once
extern "C" {
#include <libavutil/avassert.h>
#include <libavutil/channel_layout.h>
#include <libavutil/opt.h>
#include <libavutil/mathematics.h>
#include <libavutil/timestamp.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
}

int open_stream(const char *filename, int width, int height, AVPixelFormat src_fmt, uint8_t *picture_source);
int close_stream(void);
int capture(void);