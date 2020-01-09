class Audio {
	public:
		Audio();
		~Audio();

	private:
        const unsigned int _sample_rate = 22050;

		SDL_AudioSpec *audio_spec = NULL;
        SDL_AudioDeviceID audio_device;
};

void _audio_bufferfill(short *pBuffer, int pBufferSize);
void _audio_callback(void *userdata, uint8_t *stream, int len);
