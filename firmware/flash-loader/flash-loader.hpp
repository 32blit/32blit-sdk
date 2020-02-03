#include "32blit.hpp"
#include "CDCCommandHandler.h"
#include "fatfs.h"

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240
#define BUFFER_SIZE (256)
#define MAX_FILENAMES 30
#define MAX_FILENAME_LENGTH 32
#define ROW_SPACE 10
#define ROW(x) Point(0,x * ROW_SPACE)
#define MAX_FILENAME 256+1
#define MAX_FILELEN 5+1
#define PAGE_SIZE 256

class FlashLoader : public CDCCommandHandler
{
public:
	virtual StreamResult StreamData(CDCDataStream &dataStream);
	virtual bool StreamInit(CDCFourCC uCommand);


	void Init(void);
	void Render(uint32_t time);
	void Update(uint32_t time);

private:
	typedef enum {stFlashFile, stSaveFile, stFlashCDC, stLS, stSwitch} State;
	typedef enum {stFilename, stLength, stData} ParseState;

	bool Flash(const char *pszFilename);
	void FSInit(void);

	void RenderSaveFile(uint32_t time);
	void RenderFlashCDC(uint32_t time);
	void RenderFlashFile(uint32_t time);

	bool FlashData(uint32_t uOffset, uint8_t *pBuffer, uint32_t uLen);
	bool SaveData(uint8_t *pBuffer, uint32_t uLen);

	char 		m_filenames[MAX_FILENAMES][MAX_FILENAME_LENGTH+1] = {0};

	uint8_t m_buffer[PAGE_SIZE];
	uint8_t m_verifyBuffer[PAGE_SIZE];

	uint8_t m_uFileCount = 0;
	uint8_t m_uCurrentFile = 0;
	bool		m_bFsInit = false;
	State		m_state = stFlashFile;

	ParseState m_parseState = stFilename;

	char m_sFilename[MAX_FILENAME];
	char m_sFilelen[MAX_FILELEN];

	uint32_t m_uParseIndex = 0;
	uint32_t m_uFilelen = 0;

	FIL m_file;

	float m_fPercent;

};
