
#include "ff.h"
#include "diskio.h"

extern "C" {
#include "fatfs_sd.h"
}

DSTATUS disk_initialize(BYTE pdrv) {
  return SD_disk_initialize(pdrv);
}

DSTATUS disk_status(BYTE pdrv) {
  return SD_disk_status(pdrv);
}

DRESULT disk_read(BYTE pdrv, BYTE *buff, LBA_t sector, UINT count) {
  return SD_disk_read(pdrv, buff, sector, count);
}

#if !FF_FS_READONLY
DRESULT disk_write(BYTE pdrv, const BYTE *buff, LBA_t sector, UINT count) {
  return SD_disk_write(pdrv, buff, sector, count);
}
#endif

DRESULT disk_ioctl(BYTE pdrv, BYTE cmd, void* buff) {
  return SD_disk_ioctl(pdrv, cmd, buff);
}
