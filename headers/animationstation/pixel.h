#ifndef PIXEL_HPP_
#define PIXEL_HPP_

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>

struct Pixel {
  Pixel(int index, uint32_t mask = 0) : index(index), mask(mask), gpio(-1) { }
  Pixel(int index, std::vector<uint8_t> positions) : index(index), positions(positions), gpio(-1) { }
  Pixel(int index, uint32_t mask, std::vector<uint8_t> positions) : index(index), mask(mask), positions(positions), gpio(-1) { }
  Pixel(int index, uint32_t mask, std::vector<uint8_t> positions, int8_t gpioPin) : index(index), mask(mask), positions(positions), gpio(gpioPin) { }

  int index;                      // The pixel index
  uint32_t mask;                  // Used to detect per-pixel lighting
  std::vector<uint8_t> positions; // The actual LED indexes on the chain
  int8_t gpio;                    // Physical GPIO for press lighting; -1 uses remapped mask
};

inline const Pixel NO_PIXEL(-1);

struct PixelMatrix {
  PixelMatrix() { }

  std::vector<std::vector<Pixel>> pixels;
  uint8_t ledsPerPixel;
  void setup(std::vector<std::vector<Pixel>> pixels, int ledsPerPixel = -1) {
    this->pixels = pixels;
    this->ledsPerPixel = ledsPerPixel;
  }

  inline int getLedCount() {
    int count = 0;
    for (auto &col : pixels)
      for (auto &pixel : col)
        if (pixel.index == NO_PIXEL.index)
          continue;
        else
          count += pixel.positions.size();

    return count;
  }

  inline uint16_t getPixelCount() const {
    uint16_t count = 0;
    for (auto &col : pixels)
      count += col.size();

    return count;
  }

};

inline bool operator==(const Pixel &lhs, const Pixel &rhs) {
  return lhs.index == rhs.index;
}

#endif
