#include <napi.h>
#include <fstream>
#include <iterator>
#include <iostream>
#include "common.h"
#include "md5.h"

#include <sstream>

std::string toHex(uint8_t *buf, size_t len) {
  static const char *hex_chars = "0123456789abcdef";

  std::string result;

  for (int i = 0; i < len; ++i) {
    result.push_back(hex_chars[buf[i] >> 4]);
    result.push_back(hex_chars[buf[i] & 0xf]);
  }
  return result;
}

class MD5Worker : public Napi::AsyncWorker {
public:
  MD5Worker(const std::string &filePath, const Napi::Function &appCallback)
    : Napi::AsyncWorker(appCallback)
    , m_FilePath(filePath)
  {
  }

  virtual void Execute() override {
    std::ifstream file(from_utf8(m_FilePath), std::ifstream::binary);
    if (!file.is_open()) {
      SetError("Failed to open");
      return;
    }

    try {
      MD5_CTX ctx;
      MD5_Init(&ctx);
      static const size_t BUF_SIZE = 8192;
      char buffer[BUF_SIZE];
      while (file.read(buffer, BUF_SIZE)) {
        MD5_Update(&ctx, buffer, BUF_SIZE);
      }
      MD5_Update(&ctx, buffer, file.gcount());
      uint8_t result[16];
      MD5_Final(result, &ctx);
      std::stringstream ss;
      ss << std::this_thread::get_id();
      m_Result = ss.str();

      // m_Result = toHex(result, 16);
    }
    catch (const std::exception &e) {
      SetError(e.what());
    }
  }

  virtual void OnOK() override {
    Callback().Call(Receiver().Value(), std::initializer_list<napi_value>{ Env().Null(), Napi::String::New(Env(), m_Result.c_str()) });
  }

private:
  std::string m_FilePath;
  std::string m_Result;
};

Napi::Value fileMD5(const Napi::CallbackInfo& info) {
  try {
    if (info.Length() != 2) {
      throw std::runtime_error("Expected two parameters (path, callback)");
    }

    Napi::Function callback = info[1].As<Napi::Function>();

    auto worker = new MD5Worker(info[0].ToString().Utf8Value(), callback);
    worker->Queue();
  }
  catch (const std::exception &e) {
    napi_throw_error(info.Env(), "UNKNOWN", e.what());
  }
  return info.Env().Undefined();
}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
  exports.Set("fileMD5", Napi::Function::New(env, fileMD5));

  return exports;
}

NODE_API_MODULE(vortexmt, Init)
