#include "32blit.hpp"
#include "CDCCommandHandler.h"
#include "fatfs.h"
#include "persistence.h"

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240
#define BUFFER_SIZE (256)
#define MAX_FILENAMES 24
#define MAX_FILENAME_LENGTH 32
#define MAX_FILESIZE_LENGTH 12
#define ROW_HEIGHT 10
#define ROW(x) Point(0,x * ROW_HEIGHT)
#define MAX_FILENAME 256+1
#define MAX_FILELEN 16+1
#define PAGE_SIZE 256

typedef struct {
	char sFilename[MAX_FILENAME_LENGTH + 1] = { 0 };
	FSIZE_t fstFilesize = 0;
	char sFilesize[MAX_FILESIZE_LENGTH + 1] = { 0 };
} FILEMETA;

class FlashLoader : public CDCCommandHandler
{
public:
	virtual StreamResult StreamData(CDCDataStream &dataStream);
	virtual bool StreamInit(CDCFourCC uCommand);


	void Init(void);
	void Render(uint32_t time);
	void Update(uint32_t time);

private:
	typedef enum {stFlashFile, stSaveFile, stFlashCDC, stLS, stSwitch, stMassStorage} State;
	typedef enum {stFilename, stLength, stData} ParseState;

	bool Flash(const char *pszFilename);
	void FSInit(void);

	void RenderSaveFile(uint32_t time);
	void RenderFlashCDC(uint32_t time);
	void RenderFlashFile(uint32_t time);
	void RenderMassStorage(uint32_t time);

	bool FlashData(uint32_t uOffset, uint8_t *pBuffer, uint32_t uLen);
	bool SaveData(uint8_t *pBuffer, uint32_t uLen);

	FILEMETA m_filemeta[MAX_FILENAMES] = { 0 };
	int32_t m_max_width_name = 0, m_max_width_size = 0;

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
