/*! \file sprite.cpp
    \brief Functions for drawing sprites.
*/
#include <cstring>

#include "surface.hpp"
#include "sprite.hpp"

#include "../engine/file.hpp"

using namespace blit;

namespace blit {

  /**
   * Return the bounds of a sprite by index
   *
   * Takes an index offset to the sprite in the spritesheet
   * and returns a `rect` describing the location and size of the sprite in pixels.
   *
   * \param[in] index Index of the sprite in the sheet
   * \return `rect` sprite x/y location (always a multiple of 8) and size (always 8x8)
   */
  Rect Surface::sprite_bounds(uint16_t index) {
    return Rect((index % cols) * 8, (index / cols) * 8, 8, 8);
  }

 /**
   * Return the bounds of a sprite by index
   *
   * Takes a `point` describing the x/y offset of the sprite in terms of tiles/units
   * and returns a `rect` describing the location of the sprite in pixels.
   *
   * \param[in] p `point` describing the x/y offset of the sprite in the spritesheet
   * \return `rect` sprite x/y location (always a multiple of 8) and size (always 8x8)
   */
  Rect Surface::sprite_bounds(const Point &p) {
    return Rect(p.x * 8, p.y * 8, 8, 8);
  }

 /**
   * Return the bounds of a sprite by index
   *
   * Takes a `rect` describing the x/y offset and size of the sprite in terms of tiles/units
   * and returns a `rect` describing the location and size of the sprite in pixels.
   *
   * \param[in] r `rect` describing the x/y offset and size of the sprite in sprite tiles
   * \return `rect` sprite x/y location (always a multiple of 8) and size (always 8x8)
   */
  Rect Surface::sprite_bounds(const Rect &r) {
    return r * 8;
  }


  // unscaled sprites

  /**
   * Draws a sprite to the surface
   *
   * \param[in] sprite Index of the sprite in the sheet
   * \param[in] position `point` at which to place the sprite in the target surface
   * \param[in] transform to apply
   */
  void Surface::sprite(uint16_t sprite, const Point &position, uint8_t transform) {
    if(sprites == nullptr) return;
    blit(
      sprites,
      sprites->sprite_bounds(sprite),
      position,
      transform);
  }

  /**
   * Draws a sprite to the surface
   *
   * \param[in] sprite `point` describing the x/y offset of the sprite in the spritesheet in tiles/units
   * \param[in] position `point` at which to place the sprite in the target surface
   * \param[in] transform to apply
   */
  void Surface::sprite(const Point &sprite, const Point &position, uint8_t transform) {
    if(sprites == nullptr) return;
    blit(
      sprites,
      sprites->sprite_bounds(sprite),
      position,
      transform);
  }

  /**
   * Draws a sprite to the surface
   *
   * \param[in] sprite `rect` describing the x/y offset and size of the sprite in the spritesheet in tiles/units
   * \param[in] position `point` at which to place the sprite in the target surface
   * \param[in] transform to apply
   */
  void Surface::sprite(const Rect &sprite, const Point &position, uint8_t transform) {
    if(sprites == nullptr) return;
    blit(
      sprites,
      sprites->sprite_bounds(sprite),
      position,
      transform);
  }

  // unscaled sprites with origin

  /**
   * Draws a sprite to the surface
   *
   * \param[in] sprite Index of the sprite in the sheet
   * \param[in] position `point` at which to place the sprite in the target surface
   * \param[in] origin `point` around which to transform the sprite
   * \param[in] transform to apply
   */
  void Surface::sprite(uint16_t sprite, const Point &position, const Point &origin, uint8_t transform) {
    Surface::sprite(sprite, position - origin, transform);
  }

  /**
   * Draws a sprite to the surface
   *
   * \param[in] sprite `point` describing the x/y offset of the sprite in the spritesheet in tiles/units
   * \param[in] position `point` at which to place the sprite in the target surface
   * \param[in] origin `point` around which to transform the sprite
   * \param[in] transform to apply
   */
  void Surface::sprite(const Point &sprite, const Point &position, const Point &origin, uint8_t transform) {
    Surface::sprite(sprite, position - origin, transform);
  }

  /**
   * Draws a sprite to the surface
   *
   * \param[in] sprite `rect` describing the x/y offset and size of the sprite in the spritesheet in tiles/units
   * \param[in] position `point` at which to place the sprite in the target surface
   * \param[in] origin `point` around which to transform the sprite
   * \param[in] transform to apply
   */
  void Surface::sprite(const Rect &sprite, const Point &position, const Point &origin, uint8_t transform) {
    Surface::sprite(sprite, position - origin, transform);
  }

  // scaled sprites with origin

  /**
   * Draws a sprite to the surface
   *
   * \param[in] sprite Index of the sprite in the sheet
   * \param[in] position `point` at which to place the sprite in the target surface
   * \param[in] origin `point` around which to transform & scale the sprite
   * \param[in] scale `vec2` x/y scale factor
   * \param[in] transform to apply
   */
  void Surface::sprite(uint16_t sprite, const Point &position, const Point &origin, const Vec2 &scale, uint8_t transform) {
    if(sprites == nullptr) return;

    Rect dest_rect(
      roundf(position.x - float(origin.x * scale.x)),
      roundf(position.y - float(origin.y * scale.y)),
      roundf(8.0f * scale.x),
      roundf(8.0f * scale.y)
    );

    stretch_blit(
      sprites,
      sprites->sprite_bounds(sprite),
      dest_rect,
      transform);
  }

  /**
   * Draws a sprite to the surface
   *
   * \param[in] sprite `point` describing the x/y offset of the sprite in the spritesheet in tiles/units
   * \param[in] position `point` at which to place the sprite in the target surface
   * \param[in] origin `point` around which to transform & scale the sprite
   * \param[in] scale `vec2` x/y scale factor
   * \param[in] transform to apply
   */
  void Surface::sprite(const Point &sprite, const Point &position, const Point &origin, const Vec2 &scale, uint8_t transform) {
    if(sprites == nullptr) return;

    Rect dest_rect(
      roundf(position.x - float(origin.x * scale.x)),
      roundf(position.y - float(origin.y * scale.y)),
      roundf(8.0f * scale.x),
      roundf(8.0f * scale.y)
    );

    stretch_blit(
      sprites,
      sprites->sprite_bounds(sprite),
      dest_rect,
      transform);
  }

  /**
   * Draws a sprite to the surface
   *
   * \param[in] sprite `rect` describing the x/y offset and size of the sprite in the spritesheet in tiles/units
   * \param[in] position `point` at which to place the sprite in the target surface
   * \param[in] origin `point` around which to transform  & scale the sprite
   * \param[in] scale `vec2` x/y scale factor
   * \param[in] transform to apply
   */
  void Surface::sprite(const Rect &sprite, const Point &position, const Point &origin, const Vec2 &scale, uint8_t transform) {
    if(sprites == nullptr) return;

    Rect dest_rect(
      roundf(position.x - float(origin.x * scale.x)),
      roundf(position.y - float(origin.y * scale.y)),
      roundf(sprite.w * 8.0f * scale.x),
      roundf(sprite.h * 8.0f * scale.y)
    );

    stretch_blit(
      sprites,
      sprites->sprite_bounds(sprite),
      dest_rect,
      transform);
  }

  /**
   * Draws a sprite to the surface
   *
   * \param[in] sprite Index of the sprite in the sheet
   * \param[in] position `point` at which to place the sprite in the target surface
   * \param[in] origin `point` around which to transform & scale the sprite
   * \param[in] scale `float` x/y scale factor
   * \param[in] transform to apply
   */
  void Surface::sprite(uint16_t sprite, const Point &position, const Point &origin, float scale, uint8_t transform) {
    Surface::sprite(sprite, position, origin, Vec2(scale, scale), transform);
  }

  /**
   * Draws a sprite to the surface
   *
   * \param[in] sprite `point` describing the x/y offset of the sprite in the spritesheet in tiles/units
   * \param[in] position `point` at which to place the sprite in the target surface
   * \param[in] origin `point` around which to transform & scale the sprite
   * \param[in] scale `float` x/y scale factor
   * \param[in] transform to apply
   */
  void Surface::sprite(const Point &sprite, const Point &position, const Point &origin, float scale, uint8_t transform) {
    Surface::sprite(sprite, position, origin, Vec2(scale, scale), transform);
  }

  /**
   * Draws a sprite to the surface
   *
   * \param[in] sprite `rect` describing the x/y offset and size of the sprite in the spritesheet in tiles/units
   * \param[in] position `point` at which to place the sprite in the target surface
   * \param[in] origin `point` around which to transform  & scale the sprite
   * \param[in] scale `float` x/y scale factor
   * \param[in] transform to apply
   */
  void Surface::sprite(const Rect &sprite, const Point &position, const Point &origin, float scale, uint8_t transform) {
    Surface::sprite(sprite, position, origin, Vec2(scale, scale), transform);
  }


  // unscaled sprites with origin and scale - optional transform (mirror/rotate)
  //void surface::sprite(const rect &source, const point &position, const point &origin, float scale, uint8_t transform = 0);
  //void surface::sprite(const point &source, const point &position, const point &origin, float scale, uint8_t transform = 0);

  // unscaled sprites with origin and scale (x/y) - optional transform (mirror/rotate)
  //void surface::sprite(const rect &source, const point &position, const point &origin, const vec2 &scale, uint8_t transform = 0);
  //void surface::sprite(const point &source, const point &position, const point &origin, const vec2 &scale, uint8_t transform = 0);

  /*
  void surface::sprite(const spritesheet &ss, const rect &sprite, const point &position, const sprite_p &properties) {

  }

  void surface::sprite(const spritesheet &ss, const point &sprite, const point &position, const sprite_p &properties) {
    if (properties.scale.x != 1.0f || properties.scale.y != 1.0f) {
      rect sr = ss.sprite_bounds(index);
      sr.w = span.w * ss.sprite_size.w;
      sr.h = span.h * ss.sprite_size.h;

      rect dr = rect(
        pos - (origin * scale),
        size(sr.w * scale, sr.h * scale)
      );

      this->stretch_blit(&ss, sr, dr);
    }
    else {
      rect sr = ss.sprite_bounds(index);
      sr.w = span.w * ss.sprite_size.w;
      sr.h = span.h * ss.sprite_size.h;

      this->blit(&ss, sr, pos - origin);

    }

  }*/
  /*
  void surface::sprite(spritesheet &ss, uint16_t index, point pos, size span, point origin, float scale) {
    if (scale != 1.0f) {
      rect sr = ss.sprite_bounds(index);
      sr.w = span.w * ss.sprite_size.w;
      sr.h = span.h * ss.sprite_size.h;

      rect dr = rect(
        pos - (origin * scale),
        size(sr.w * scale, sr.h * scale)
      );

      this->stretch_blit(&ss, sr, dr);
    } else {
      rect sr = ss.sprite_bounds(index);
      sr.w = span.w * ss.sprite_size.w;
      sr.h = span.h * ss.sprite_size.h;

      this->blit(&ss, sr, pos - origin);

    }
  }*/

}
