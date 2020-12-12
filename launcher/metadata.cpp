#include <cstring>

#include "graphics/sprite.hpp"

#include "metadata.hpp"
#include "executable.hpp"

using namespace blit;

void parse_metadata(char *data, uint16_t metadata_len, BlitGameMetadata &metadata, bool unpack_images) {
  metadata.length = metadata_len;

  auto raw_meta = reinterpret_cast<RawMetadata *>(data);
  metadata.crc32 = raw_meta->crc32;

  metadata.title = raw_meta->title;
  metadata.description = raw_meta->description;
  metadata.version = raw_meta->version;
  metadata.author = raw_meta->author;

  uint16_t offset = sizeof(RawMetadata);

  if(unpack_images && metadata.icon)
    metadata.free_surfaces();

  if(offset != metadata_len && unpack_images) {
    // icon/splash
    auto image = reinterpret_cast<packed_image *>(data + offset);
    metadata.icon = Surface::load(reinterpret_cast<uint8_t *>(data + offset));
    offset += image->byte_count;

    image = reinterpret_cast<packed_image *>(data + offset);
    metadata.splash = Surface::load(reinterpret_cast<uint8_t *>(data + offset));
  }
}

bool parse_file_metadata(const std::string &filename, BlitGameMetadata &metadata, bool unpack_images) {
  blit::File f(filename);
  uint32_t offset = 0;

  BlitGameHeader header;
  auto read = f.read(offset, sizeof(header), (char *)&header);

  // skip relocation data
  if(header.magic == 0x4F4C4552 /* RELO */) {
    uint32_t num_relocs;
    f.read(4, 4, (char *)&num_relocs);

    offset = num_relocs * 4 + 8;
    // re-read header
    f.read(offset, sizeof(header), (char *)&header);
  }

  if(header.magic == blit_game_magic) {
    uint8_t buf[10];

    offset += (header.end & 0x1FFFFFF);
    auto bytes_read = f.read(offset, 10, (char *)buf);

    if(bytes_read == 10 && memcmp(buf, "BLITMETA", 8) == 0) {
      // don't bother reading the whole thing if we don't want the images
      auto metadata_len = unpack_images ? *reinterpret_cast<uint16_t *>(buf + 8) : sizeof(RawMetadata);

      uint8_t metadata_buf[0xFFFF];
      f.read(offset + 10, metadata_len, (char *)metadata_buf);

      parse_metadata(reinterpret_cast<char *>(metadata_buf), metadata_len, metadata, unpack_images);

      return true;
    }
  }

  return false;
}