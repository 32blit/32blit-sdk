#include <cstring>

#include "graphics/sprite.hpp"

#include "metadata.hpp"
#include "executable.hpp"
#include "ff.h"
#include "quadspi.h"

using namespace blit;

void parse_metadata(char *data, uint16_t metadata_len, BlitGameMetadata &metadata, bool unpack_images) {
  metadata.length = metadata_len;

  // parse strings
  uint16_t offset = 0;

  auto len = strlen(data);
  metadata.title = std::string(data, len);
  offset += len + 1;

  len = strlen(data + offset);
  metadata.description = std::string(data + offset, len);
  offset += len + 1;

  len = strlen(data + offset);
  metadata.version = std::string(data + offset, len);
  offset += len + 1;

  if(unpack_images && metadata.icon) {
    delete[] metadata.icon->data;
    delete[] metadata.icon->palette;
    delete metadata.icon;
    metadata.icon = nullptr;
  }

  if(unpack_images && metadata.splash) {
    delete[] metadata.splash->data;
    delete[] metadata.splash->palette;
    delete metadata.splash;
    metadata.splash = nullptr;
  }

  if(offset != metadata_len && unpack_images) {
    // icon/splash
    // not really sprite sheets, but that's where the load helper is...
    auto image = reinterpret_cast<packed_image *>(data + offset);
    metadata.icon = SpriteSheet::load(reinterpret_cast<uint8_t *>(data + offset));
    offset += sizeof(packed_image) + image->byte_count + image->palette_entry_count * 4;

    image = reinterpret_cast<packed_image *>(data + offset);
    metadata.splash = SpriteSheet::load(reinterpret_cast<uint8_t *>(data + offset));
  }
}

bool parse_flash_metadata(uint32_t offset, BlitGameMetadata &metadata, bool unpack_images) {

  BlitGameHeader header;

  if(qspi_read_buffer(offset, reinterpret_cast<uint8_t *>(&header), sizeof(header)) != QSPI_OK)
    return false;

  offset += header.end - 0x90000000;

  uint8_t buf[10];
  if(qspi_read_buffer(offset, buf, 10) != QSPI_OK)
    return false;

  if(memcmp(buf, "BLITMETA", 8) != 0)
    return false;

  auto metadata_len = *reinterpret_cast<uint16_t *>(buf + 8);
  uint8_t metadata_buf[0xFFFF];
  if(qspi_read_buffer(offset + 10, metadata_buf, metadata_len) != QSPI_OK) {
    return false;
  }

  parse_metadata(reinterpret_cast<char *>(metadata_buf), metadata_len, metadata, unpack_images);

  return true;
}

bool parse_file_metadata(const std::string &filename, BlitGameMetadata &metadata, bool unpack_images) {
  FIL fh;
  f_open(&fh, filename.c_str(), FA_READ);

  BlitGameHeader header;
  UINT bytes_read;
  f_read(&fh, &header, sizeof(header), &bytes_read);

  if(header.magic == blit_game_magic) {
    uint8_t buf[10];
    f_lseek(&fh, (header.end - 0x90000000));
    auto res = f_read(&fh, buf, 10, &bytes_read);

    if(bytes_read == 10 && memcmp(buf, "BLITMETA", 8) == 0) {
      auto metadata_len = *reinterpret_cast<uint16_t *>(buf + 8);

      uint8_t metadata_buf[0xFFFF];
      f_read(&fh, metadata_buf, metadata_len, &bytes_read);

      parse_metadata(reinterpret_cast<char *>(metadata_buf), metadata_len, metadata, unpack_images);

      f_close(&fh);
      return true;
    }
  }

  f_close(&fh);
  return false;
}