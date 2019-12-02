class Renderer;

class VideoCapture {
	public:
		VideoCapture(const char *name);
		~VideoCapture();

		void start(const char *filename);
		void start();
		void capture(Renderer *source);
		void stop();
		bool recording() {return buffer;}

	private:
		const char *name;
		Uint8 *buffer = NULL;
};
