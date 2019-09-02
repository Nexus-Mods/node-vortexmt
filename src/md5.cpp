#include "md5.h"

// binary integer part of the sines of integers (radians)
// (precomputed table taken from wikipedia)
const std::array<uint32_t, 64> MD5::s_K = {
  0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee,
  0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
  0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,
  0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
  0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa,
  0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
  0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed,
  0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,
  0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c,
  0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
  0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05,
  0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
  0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039,
  0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
  0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,
  0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391
};

// shift amount per round
const std::array<uint32_t, 64> MD5::s_Shifts = {
    7, 12, 17, 22,  7, 12, 17, 22,  7, 12, 17, 22,  7, 12, 17, 22,
    5,  9, 14, 20,  5,  9, 14, 20,  5,  9, 14, 20,  5,  9, 14, 20,
    4, 11, 16, 23,  4, 11, 16, 23,  4, 11, 16, 23,  4, 11, 16, 23,
    6, 10, 15, 21,  6, 10, 15, 21,  6, 10, 15, 21,  6, 10, 15, 21
};

std::string MD5::hashStream(std::istream & input) {
  auto start = std::istreambuf_iterator<char>(input.rdbuf());
  input.seekg(0, std::ios::end);
  std::size_t length = input.tellg();
  input.seekg(0);
  MD5 *res = new MD5(start, length);
  return res->hexDigest<std::string>();
}

void MD5::resetMPos() {
  m_MPos = m_Chunk.begin();
}

void MD5::readChunk(const std::function<uint8_t()>& read, std::array<uint32_t, 16>::iterator arrayLast) {
  for (; m_MPos != arrayLast; ++m_MPos) {
    *m_MPos = read();
    *m_MPos |= read() << 8;
    *m_MPos |= read() << 16;
    *m_MPos |= read() << 24;
  }
}

void MD5::readLastWord(const std::function<uint8_t()>& read, std::ptrdiff_t length) {
  *m_MPos = 0;

  for (int i = 0; i < length; ++i) {
    *m_MPos |= read() << (i * 8);
  }

  *m_MPos |= (0x00000080 << (length * 8));

  ++m_MPos;
}

void MD5::padWithZeros(const std::array<uint32_t, 16>::iterator & arrayLast) {
  for (; m_MPos != arrayLast; ++m_MPos) {
    *m_MPos = 0;
  }
}

void MD5::appendLength(uint64_t lengthBits) {
  *m_MPos++ = lengthBits;
  *m_MPos++ = lengthBits >> 32;
}

void MD5::hashChunk() {
  uint32_t a = m_A0;
  uint32_t b = m_B0;
  uint32_t c = m_C0;
  uint32_t d = m_D0;

  uint32_t f;
  unsigned int g;

  for (unsigned int i = 0; i < 64; ++i) {
    if (i < 16) {
      f = (b & c) | ((~b) & d);
      g = i;
    }
    else if (i < 32) {
      f = (d & b) | (c & (~d));
      g = ((5 * i) + 1) % 16;
    }
    else if (i < 48) {
      f = b ^ c ^ d;
      g = ((3 * i) + 5) % 16;
    }
    else {
      f = c ^ (b | (~d));
      g = (7 * i) % 16;
    }

    f += a + s_K[i] + m_Chunk[g];
    a = d;
    d = c;
    c = b;
    b += rotLeft(f, s_Shifts[i]);
  }

  m_A0 += a;
  m_B0 += b;
  m_C0 += c;
  m_D0 += d;
}

uint32_t MD5::rotLeft(uint32_t x, uint32_t c) {
  return (x << c) | (x >> (32 - c));
}
