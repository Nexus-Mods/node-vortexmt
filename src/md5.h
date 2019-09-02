#pragma once

#include <array>
#include <iterator>
#include <cstdint>
#include <functional>
#include <numeric>

/**
 * MD5 calculation class
 * based on https://codereview.stackexchange.com/questions/163872/md5-implementation-in-c11
 */
class MD5 {
public:
  template<typename IterT>
  MD5(IterT first, std::size_t length);

  template <typename ContainerT>
  ContainerT digest();

  template <typename ContainerT>
  ContainerT hexDigest();

  static std::string hashStream(std::istream &input);

private:
  template <class IterT>
  void update(IterT &first, std::size_t length);

  void resetMPos();

  void readChunk(const std::function<uint8_t()> &read, std::array<uint32_t, 16>::iterator arrayLast);

  void readLastWord(const std::function<uint8_t()> &read, std::ptrdiff_t length);

  void padWithZeros(const std::array<uint32_t, 16>::iterator &arrayLast);

  void appendLength(uint64_t lengthBits);

  void hashChunk();

private:
  static uint32_t rotLeft(uint32_t x, uint32_t c);

  template <class IterT>
  static void toByte(uint32_t n, IterT &first);

  template <class IterT>
  static void toHex(uint32_t n, IterT &out);

private:
  uint32_t m_A0 = 0x67452301;
  uint32_t m_B0 = 0xefcdab89;
  uint32_t m_C0 = 0x98badcfe;
  uint32_t m_D0 = 0x10325476;

  std::array<uint32_t, 16> m_Chunk;
  std::array<uint32_t, 16>::iterator m_MPos;
 
  static const std::array<uint32_t, 64> s_K;
  static const std::array<uint32_t, 64> s_Shifts;
};

template<typename IterT>
inline MD5::MD5(IterT first, std::size_t length) {
  update(first, length);
}

template<typename ContainerT>
inline ContainerT MD5::digest() {
  ContainerT container;
  container.resize(16);
  auto it = container.begin();

  toByte(m_A0, it);
  toByte(m_B0, it);
  toByte(m_C0, it);
  toByte(m_D0, it);
  return container;
}

template<typename ContainerT>
inline ContainerT MD5::hexDigest() {
  ContainerT container;
  container.resize(32);
  auto it = container.begin();

  toHex(m_A0, it);
  toHex(m_B0, it);
  toHex(m_C0, it);
  toHex(m_D0, it);

  return container;
}

template<class IterT>
inline void MD5::update(IterT & first, std::size_t length) {
  IterT orig = first;

  std::size_t remaining = length;
  auto readByte = [&]() {
    --remaining;
    return static_cast<uint8_t>(*first++);
  };

  while (remaining >= 64) {
    resetMPos();
    readChunk(readByte, m_Chunk.end());
    hashChunk();
  }

  // last chunk needs some data appended to pad to 512 bits
  resetMPos();
  std::size_t lastBlockSize = remaining;

  // read remaining chunk up to the last word (rounded down)
  readChunk(readByte, m_Chunk.begin() + (remaining / 4));
  // read the last 0-3 bytes from the stream a 1-bit as the msb of the
  // appended data and then all zeros
  readLastWord(readByte, remaining);

  if (lastBlockSize >= 56) {
    // if we can't fit the length into this block we need to append an entire
    // new block just for that so fill up this one, hash it, then create
    // another one
    padWithZeros(m_Chunk.end());
    hashChunk();

    resetMPos();
  }

  // append zeros up to bit 448
  padWithZeros(m_Chunk.end() - 2);
  appendLength(length * 8);
  hashChunk();
}

template<class IterT>
inline void MD5::toByte(uint32_t n, IterT & first) {
  *first++ = n & 0xff;
  *first++ = (n >> 8) & 0xff;
  *first++ = (n >> 16) & 0xff;
  *first++ = (n >> 24) & 0xff;
}

template<class IterT>
inline void MD5::toHex(uint32_t n, IterT & out) {
  static const char *hexDigits = "0123456789abcdef";

  for (int i = 0; i < 32; i += 4) {
    *out++ = hexDigits[(n >> (i ^ 4)) & 0xF];
  }
}
