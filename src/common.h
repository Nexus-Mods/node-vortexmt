#include <napi.h>

#ifdef WIN32
#include <windows.h>

std::wstring from_utf8(const std::string& input) {
  std::wstring result;

  if (input.length() > 0) {
    UINT cp = CP_UTF8;
    // preflight to find out the required source size
    int outLength = MultiByteToWideChar(cp, 0, input.c_str(), static_cast<int>(input.length()), &result[0], 0);
    if (outLength == 0) {
      throw std::runtime_error("string conversion failed");
    }
    result.resize(outLength);
    outLength = MultiByteToWideChar(cp, 0, input.c_str(), static_cast<int>(input.length()), &result[0], outLength);
    if (outLength == 0) {
      throw std::runtime_error("string conversion failed");
    }
    while ((outLength > 0) && (result[outLength - 1] == L'\0')) {
      result.resize(--outLength);
    }
  }
  return result;
}

#else // WIN32
std::string from_utf8(const std::string& input) {
  return input;
}
#endif
