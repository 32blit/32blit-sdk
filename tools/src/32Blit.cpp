#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string>
#include <string.h>
#include <time.h>

#if defined(WIN32) || defined(__MINGW32__)
#include <windows.h>
#else
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <errno.h>
#include <dirent.h>

#ifdef __APPLE__
#include <IOKit/IOKitLib.h>
#include <IOKit/serial/IOSerialKeys.h>
#include <IOKit/usb/USBSpec.h>
#endif
#endif

template <int a, int b, int c, int d>
struct FourCCMake
{
  static const unsigned int value = (((((d << 8) | c) << 8) | b) << 8) | a;
};

bool Get32BlitInfo(uint32_t &uAck);

void usage(void)
{
  printf("Usage: 32blit <process> <comport> <binfile> <options>\n");
  printf("  <process> : Either _RST, SAVE or PROG\n");
  printf("  <comport> : Com port, eg COM1 or /dev/cu.usbmodem\n");
  printf("  <binfile> : Bin file path (optional, needed for SAVE and PROG)\n");
  printf("\nOptions:\n");
  printf("\t--reconnect: Re-open port after PROG to show debug output\n");
}

const char *getFileName(const char *pszPath)
{
  const char *pszFilename = strrchr(pszPath, '\\');
  if (pszFilename == NULL)
    pszFilename = strrchr(pszPath, '/');

  if (pszFilename == NULL)
    pszFilename = pszPath;
  else
    pszFilename++;

  return pszFilename;
}




#ifdef WIN32
#ifndef __MINGW32__
typedef long ssize_t;
#endif
HANDLE hComm = INVALID_HANDLE_VALUE;
OVERLAPPED osRX = { 0 };
OVERLAPPED osTX = { 0 };
DWORD dwWritten = 0; // should be in WriteCom() but doesn't work on stack, windows guy needs to look at this
bool bWaitingOnRx = false;

void CloseCom(void)
{
  CloseHandle(osRX.hEvent);
  CloseHandle(osTX.hEvent);
  CloseHandle(hComm);
}

bool OpenComPort(const char *pszComPort, bool bTestConnection = false)
{
  bWaitingOnRx = false;

  if (hComm != INVALID_HANDLE_VALUE)
    CloseCom();

  osRX.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
  osTX.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

  char pszFullPortName[32];
  snprintf(pszFullPortName, 32, "\\\\.\\%s", pszComPort);
  hComm = CreateFile(pszFullPortName, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, 0);
  return hComm != INVALID_HANDLE_VALUE;
}


uint32_t WriteCom(char *pBuffer, uint32_t uLen)
{
  if (!WriteFile(hComm, pBuffer, uLen, &dwWritten, &osTX))
  {
    GetOverlappedResult(hComm, &osTX, &dwWritten, TRUE);
  }
  return dwWritten;
}

bool GetRXByte(char &rxByte)
{
  bool bResult = false;

  DWORD  dwReadBytes;
  if (!bWaitingOnRx)
  {
    if (ReadFile(hComm, &rxByte, 1, &dwReadBytes, &osRX))
      bResult = true;
    else
      bWaitingOnRx = true;
  }
  else
  {
    DWORD dwRes = WaitForSingleObject(osRX.hEvent, 0);
    if (dwRes == WAIT_OBJECT_0)
    {
      GetOverlappedResult(hComm, &osRX, &dwReadBytes, FALSE);
      bResult = true;
      bWaitingOnRx = false;
    }
  }

  return bResult;
}

bool HandleRX(void)
{
  bool bResult = true;

  char  rxByte = 0;
  DWORD  dwReadBytes;
  if (!bWaitingOnRx)
  {
    while (ReadFile(hComm, &rxByte, 1, &dwReadBytes, &osRX))
      putchar(rxByte);

    if (GetLastError() == ERROR_IO_PENDING)
      bWaitingOnRx = true;
    else
      bResult = false;
  }
  else
  {
    DWORD dwRes = WaitForSingleObject(osRX.hEvent, 10);
    if (dwRes == WAIT_OBJECT_0)
    {
      if (GetOverlappedResult(hComm, &osRX, &dwReadBytes, FALSE))
      {
        putchar(rxByte);
        bWaitingOnRx = false;
      }
      else
        bResult = false;
    }
    else
      if (dwRes == WAIT_FAILED)
        bResult = false;
  }

  return bResult;
}

std::string GuessPortName()
{
  TCHAR targetPath[1024];

  for (int i = 1; i < 256; i++)
  {
    auto portName = "COM" + std::to_string(i);

    // just return the first USB serial
    if (QueryDosDevice(portName.c_str(), targetPath, 1024) && std::string(targetPath).find("USB") != std::string::npos)
      return portName;
  }

  return "";
}

void usleep(uint32_t uSecs)
{
  Sleep(uSecs / 1000);
}
#else

int fdCom = -1;

ssize_t WriteCom(char *pBuffer, uint32_t uLen)
{
  uint32_t uRemaining = uLen;
  bool bError = false;
  while (!bError && uRemaining)
  {
    ssize_t nWritten = write(fdCom, pBuffer + (uLen - uRemaining), uRemaining);
    if (nWritten == -1)
      bError = true;
    else
      uRemaining -= nWritten;
  }

  if (!bError)
    tcdrain(fdCom);

  return uLen - uRemaining;;
}

bool GetRXByte(char &rxByte)
{
  return read(fdCom, &rxByte, 1) == 1;
}

bool HandleRX(void)
{
  bool bResult = true;

  char ch;
  ssize_t res = read(fdCom, &ch, 1);
  if (res == 1)
    putchar(ch);
  else
    if ((res == -1) && (errno != EAGAIN))
      bResult = false;

  return bResult;
}

void CloseCom(void)
{
  close(fdCom);
  fdCom = -1;
}

bool OpenComPort(const char *pszComPort, bool bTestConnection = false)
{
  bool bComPortOpen = false;
  if (fdCom != -1)
    close(fdCom);

  fdCom = open(pszComPort, O_RDWR | O_NOCTTY | O_SYNC);
  if (fdCom >= 0)
  {
    int flags = fcntl(fdCom, F_GETFL, 0);
    fcntl(fdCom, F_SETFL, flags | O_NONBLOCK);

    struct termios tio;
    tcgetattr(fdCom, &tio);
    cfmakeraw(&tio);
    tcsetattr(fdCom, TCSANOW, &tio);

    if (bTestConnection)
    {
      uint32_t uAck;
      bComPortOpen = Get32BlitInfo(uAck);
      if (!bComPortOpen)
        CloseCom();
    }
    else
      bComPortOpen = true;
  }
  return bComPortOpen;
}

std::string GuessPortName()
{
#ifdef __linux__
  // look for a serial device with "32Blit" in its name
  std::string sSearchDir = "/dev/serial/by-id";

  DIR *pDir = opendir(sSearchDir.c_str());

  if(!pDir)
    return "";

  struct dirent *pEntry;

  while((pEntry = readdir(pDir)))
  {
    std::string sName(pEntry->d_name);
    if(sName.find("32Blit") != std::string::npos)
      return sSearchDir + "/" + sName;
  }

  return "";

#elif __APPLE__
  auto classesToMatch = IOServiceMatching(kIOSerialBSDServiceValue);
  if(!classesToMatch)
    return "";

  mach_port_t masterPort;
  auto kernResult = IOMasterPort(MACH_PORT_NULL, &masterPort);
  if(kernResult != KERN_SUCCESS)
    return "";

  io_iterator_t matchingServices;
  kernResult = IOServiceGetMatchingServices(masterPort, classesToMatch, &matchingServices);
  if(kernResult != KERN_SUCCESS)
    return "";

  io_object_t portService;
  char devicePath[1024] = {};
  bool found = false;

  while((portService = IOIteratorNext(matchingServices)) && !found)
  {
    // check product string
    auto productString = (CFStringRef)IORegistryEntrySearchCFProperty(portService, kIOServicePlane, CFSTR(kUSBProductString), kCFAllocatorDefault, kIORegistryIterateRecursively | kIORegistryIterateParents);

    if(!productString)
    {
      IOObjectRelease(portService);
      continue;
    }

    if(CFStringCompare(productString, CFSTR("32Blit CDC"), 0) != 0)
    {
      // not a blit
      CFRelease(productString);
      IOObjectRelease(portService);
      continue;
    }

    CFRelease(productString);

    // get device path
    auto devicePathAsCFString = (CFStringRef)IORegistryEntryCreateCFProperty(portService, CFSTR(kIOCalloutDeviceKey), kCFAllocatorDefault, 0);
    if(devicePathAsCFString)
    {
      if(CFStringGetCString(devicePathAsCFString, devicePath, 1024, kCFStringEncodingASCII))
        found = true;

      CFRelease(devicePathAsCFString);
    }

    IOObjectRelease(portService);
  }

  return std::string(devicePath);
#else
  // TODO: macOS
  return "";
#endif
}

#endif

bool WaitForHeader(void)
{
  bool    bHeaderFound = false;
  bool    bTimedOut = false;

  char    header[] = "32BL";
  uint8_t uHeaderPos = 0;

  clock_t uTimeoutClk = CLOCKS_PER_SEC * 1 + clock();

  while (!bHeaderFound && !bTimedOut)
  {
    char ch;
    if (GetRXByte(ch))
    {
      if (ch == header[uHeaderPos])
      {
        uHeaderPos++;
        bHeaderFound = (uHeaderPos == 4);
      }
      else
        uHeaderPos = 0;
    }
    else
      bTimedOut = clock() > uTimeoutClk;
  }

  return bHeaderFound;
}

bool GetFourCC(uint32_t &uFourCC)
{
  bool    bFourCCFound = false;
  bool    bTimedOut = false;

  char    buffer[4];
  uint8_t uBufferPos = 0;

  clock_t uTimeoutClk = CLOCKS_PER_SEC + clock();

  while (!bFourCCFound && !bTimedOut)
  {
    char ch;
    if (GetRXByte(ch))
    {
      buffer[uBufferPos++] = ch;
      bFourCCFound = (uBufferPos == 4);
    }
    else
      bTimedOut = clock() > uTimeoutClk;
  }

  if (bFourCCFound)
    uFourCC = (((((buffer[3] << 8) | buffer[2]) << 8) | buffer[1]) << 8) | buffer[0];
  return bFourCCFound;
}

bool Get32BlitInfo(uint32_t &uAck)
{
  bool bResult = false;

  char rstCommand[] = "32BLINFO";
  ssize_t res = WriteCom(rstCommand, 8);
  if (res != 8)
  {
    int shit = errno;
    printf("errno=%d\n", shit);
  }

  if (WaitForHeader())
    bResult = GetFourCC(uAck);

  return bResult;
}



bool ResetIfNeeded(const char *pszComPort)
{
  bool bResetNeeded = false;
  bool bFound32Blit = false;

  printf("Getting info from 32Blit...");

  uint32_t uAck;
  if (Get32BlitInfo(uAck))
  {
    switch (uAck)
    {
    case FourCCMake<'_', 'E', 'X', 'T'>::value:
      printf("Reset needed.\n");
      bResetNeeded = true;
      bFound32Blit = true;
      break;

    case FourCCMake<'_', 'I', 'N', 'T'>::value:
      printf("No reset needed.\n");
      bFound32Blit = true;
      break;

    default:
      printf("ERROR: Failed to get ack info from 32Blit, you may need a manual reset.\n");
      break;
    }
  }
  else
    printf("ERROR: Failed to get info header from 32Blit, you may need a manual reset.\n");

  if (bResetNeeded)
  {
    printf("Resetting 32Blit and waiting for USB connection, please wait...\n");
    // need to reset 32blit
    char rstCommand[] = "32BL_RST";
    WriteCom(rstCommand, (uint32_t)strlen(rstCommand));
    CloseCom();

    // wait for reconnect
    bool bReconnected = false;
    while (!bReconnected)
    {
      usleep(250000);
      bReconnected = OpenComPort(pszComPort, true);
    }
    printf("Reconnected to 32Blit after reset.\n");
  }

  return bFound32Blit;
}

int main(int argc, char *argv[])
{
  if (argc < 3)
  {
    usage();
    exit(1);
  }

  const char *pszProcess = argv[1];
  std::string sComPort = argv[2];
  const char *pszBinPath = NULL;
  const char *pszBinFile = NULL;
  bool bShouldReconnect = false;

  if (argc >= 4)
  {
    pszBinPath = argv[3];
    pszBinFile = getFileName(pszBinPath);
  }

  const uint32_t *puProcess = (uint32_t *)pszProcess;
  if (*puProcess != FourCCMake<'P', 'R', 'O', 'G'>::value && *puProcess != FourCCMake<'S', 'A', 'V', 'E'>::value && *puProcess != FourCCMake<'_', 'R', 'S', 'T'>::value)
  {
    usage();
    printf("ERROR <process> is %s, must be _RST, PROG or SAVE\n", pszProcess);
    exit(2);
  }
  else
  {
    if (*puProcess != FourCCMake<'_', 'R', 'S', 'T'>::value && argc < 4)
    {
      usage();
      exit(1);
    }
  }

  for(int i = 4; i < argc; i++)
  {
    std::string sArg(argv[i]);

    if(sArg == "--reconnect")
      bShouldReconnect = true;
  }

  bool bComPortOpen = false;

  if(sComPort == "AUTO")
  {
    auto sDetectedPort = GuessPortName();
    if(sDetectedPort.empty())
      printf("Failed to autodetect port\n");
    else
    {
      printf("Detected port as: %s\n", sDetectedPort.c_str());
      sComPort = sDetectedPort;
    }
  }

  bComPortOpen = OpenComPort(sComPort.c_str());

  if (!bComPortOpen)
  {
    usage();
    printf("ERROR <comport> Cannot open %s\n", sComPort.c_str());
    exit(6);
  }

  //while (1)
  //{
  //  char ch;
  //  if (GetRXByte(ch))
  //    putchar(ch);
  //  //HandleRX();
  //}

  // _RST
  if (!pszBinPath)
  {
    char header[1024];
    snprintf(header, 1024, "32BL%s%c", pszProcess, 0);
    size_t uLen = strlen(header) + 1;
    WriteCom(header, (uint32_t)uLen);
    CloseCom();
    exit(0);
  }

  FILE *pfBin = fopen(pszBinPath, "rb");
  if (!pfBin)
  {
    usage();
    printf("ERROR <binfile> Cannot open %s\n", pszBinPath);
    CloseCom();
    exit(5);
  }

  fseek(pfBin, 0L, SEEK_END);
  long nSize = ftell(pfBin);
  fseek(pfBin, 0L, SEEK_SET);

  if (nSize < 1)
  {
    usage();
    printf("ERROR <binfile> contains no data.");
    CloseCom();
    exit(3);
  }

  if (!ResetIfNeeded(sComPort.c_str()))
    exit(0);

  // check we can still talk to 32blit
  bool bAlive = false;
  while (!bAlive)
  {
    uint32_t uAck;
    if (Get32BlitInfo(uAck))
      bAlive = true;
    else
    {
      printf("Cannot talk to 32Blit, trying reconnect\n");
      // try to reconnect
      bComPortOpen = OpenComPort(sComPort.c_str());
    }
  }

  printf("Sending binary file ");
  char header[1024];
  snprintf(header, 1024, "32BL%s%s%c%ld%c", pszProcess, pszBinFile, '*', nSize, '*');
  size_t uLen = strlen(header);
  snprintf(header, 1024, "32BL%s%s%c%ld%c", pszProcess, pszBinFile, 0, nSize, 0);
  if (WriteCom(header, (uint32_t)uLen) != (ssize_t)uLen)
  {
    printf("Error: Failed to write header to 32Blit.\n");
    CloseCom();
    exit(0);
  }

  char buffer[64];
  bool bFinishedTX = false;

  long nTotalWritten = 0;
  long nTotalRead = 0;
  uint32_t uProgressCount = 20;
  while (!bFinishedTX)
  {
    // TX
    size_t uRead = fread(buffer, 1, 64, pfBin);
    nTotalRead += uRead;
    if (uRead)
    {
      ssize_t nWritten = WriteCom(buffer, (uint32_t)uRead);
      if (nWritten == (ssize_t)uRead)
        nTotalWritten += nWritten;
      else
      {
        printf("Error: failed to write data.\n");
        bFinishedTX = true;
      }
    }
    else
    {
      printf("\n");
      fclose(pfBin);
      bFinishedTX = true;
    }

    if (uProgressCount-- == 0)
    {
      uProgressCount = 20;
      putchar('*');
      fflush(stdout);
    }
  }

  // RX
  if (nTotalWritten != nSize)
  {
    printf("ERROR Incorrect number of bytes written, wrote %ld expected %ld\n", nTotalWritten, nSize);
    CloseCom();
    exit(0);
  }

  printf("Sending complete.\n");
#ifdef WIN32
  Sleep(1000);
#else
  sleep(1);
#endif
  if (*puProcess == FourCCMake<'P', 'R', 'O', 'G'>::value && bShouldReconnect)
  {
    printf("Waiting for USB connection for debug logging, please wait...\n");
    // wait for reconnect
    bool bReconnected = false;
    while (!bReconnected)
    {
      usleep(250000);
      bReconnected = OpenComPort(sComPort.c_str());
      if (bReconnected)
      {
        uint32_t uAck;
        bReconnected = Get32BlitInfo(uAck);
      }
    }
    printf("Connected to 32Blit.\n");

    while (true)
    {
      if (!HandleRX())
      {
        CloseCom();
        printf("USB Connection lost, attempting to reconnect, please wait...\n");
        // wait for reconnect
        bool bReconnected = false;
        while (!bReconnected)
        {
          usleep(250000);
          bReconnected = OpenComPort(sComPort.c_str());
        }
        printf("Reconnected to 32Blit.\n");
      }
    }
  }

  CloseCom();
  exit(0);
}
