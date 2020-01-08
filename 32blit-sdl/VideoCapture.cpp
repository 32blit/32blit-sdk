#include <iomanip>
#include <sstream>
#include "SDL.h"

#include "VideoCapture.hpp"

#include "Renderer.hpp"
#include "System.hpp"

#include "VideoCaptureFfmpeg.hpp"

inline std::tm localtime_xp(std::time_t timer)
{
	// Don't ignore unsafe warnings - https://stackoverflow.com/questions/38034033/c-localtime-this-function-or-variable-may-be-unsafe
	std::tm bt{};
#if defined(__unix__)
	localtime_r(&timer, &bt);
#elif defined(_MSC_VER)
	localtime_s(&bt, &timer);
#else
	static std::mutex mtx;
	std::lock_guard<std::mutex> lock(mtx);
	bt = *std::localtime(&timer);
#endif
	return bt;
}

VideoCapture::VideoCapture(const char *name) : name(name) {
	;
}

VideoCapture::~VideoCapture() {
	if (buffer) {
		fprintf(stderr, "Warning: recording was not stopped before exiting.\n");
		stop();
	}
}

void VideoCapture::start(const char *filename) {
	buffer = (Uint8 *)malloc(System::width * System::height * SDL_BYTESPERPIXEL(SDL_PIXELFORMAT_RGB24));
	ffmpeg_open_stream(filename, System::width, System::height, buffer);
	fprintf(stderr, "Started with filename %s\n", filename);
}

void VideoCapture::start() {
	auto bt = localtime_xp(std::time(0));
	std::stringstream filename;
	filename << name;
	filename << std::put_time(&bt, "-capture-%Y-%m-%d-%H-%M-%S.mpg");
	start(filename.str().c_str());
}

void VideoCapture::capture(Renderer *source) {
	if (buffer) {
		source->read_pixels(System::width, System::height, SDL_PIXELFORMAT_RGB24, buffer);
		ffmpeg_capture();
	} else {
		fprintf(stderr, "Not recording.\n");
	}
}

void VideoCapture::stop() {
	ffmpeg_close_stream();
	free(buffer);
	buffer = NULL;
	fprintf(stderr, "Stopped.\n");
}
