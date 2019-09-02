#include <nan.h>


v8::Local<v8::String> operator "" _n(const char *input, size_t) {
  return Nan::New(input).ToLocalChecked();
}

#ifdef WIN32
typedef std::wstring native_string;
#else // WIN32
typedef std::string native_string;
#endif

native_string from_utf8(const std::string &input) {
#ifdef WIN32
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
    while (result[outLength - 1] == L'\0') {
      result.resize(--outLength);
    }
  }

  return result;
#else // WIN32
  return input;
#endif
}
