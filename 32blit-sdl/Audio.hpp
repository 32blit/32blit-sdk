class Audio {
	public:
		Audio();
		~Audio();

	private:
        const unsigned int _sample_rate = 22050;

        SDL_AudioDeviceID audio_device;
};
