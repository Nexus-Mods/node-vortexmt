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

class MD5Worker : public Napi::AsyncProgressWorker<size_t> {
public:
  MD5Worker(const std::string &filePath, const Napi::Function &progressCallback, const Napi::Function &appCallback)
    : Napi::AsyncProgressWorker<size_t>(appCallback)
    , m_FilePath(filePath)
    , m_Progress(Napi::Persistent(progressCallback))
  {
  }

  virtual void Execute(const ExecutionProgress& progress) override {
    std::ifstream file(from_utf8(m_FilePath), std::ifstream::binary);
    if (!file.is_open()) {
      SetError("Failed to open");
      return;
    }

    file.seekg(0, std::ios::end);
    size_t fileSize = file.tellg();
    file.seekg(0);

    size_t filePos = 0;;
    time_t lastProgress = 0;

    try {
      MD5_CTX ctx;
      MD5_Init(&ctx);
      static const size_t BUF_SIZE = 8192;
      char buffer[BUF_SIZE];
      while (file.read(buffer, BUF_SIZE)) {
        filePos += BUF_SIZE;
        MD5_Update(&ctx, buffer, BUF_SIZE);
        time_t now = time(nullptr);
        if ((now > lastProgress) && !m_Progress.IsEmpty()) {
          size_t sizes[] = { filePos, fileSize };
          progress.Send(sizes, 2);
          lastProgress = now;
        }
      }
      MD5_Update(&ctx, buffer, file.gcount());
      uint8_t result[16];
      MD5_Final(result, &ctx);
      m_Result = toHex(result, 16);
    }
    catch (const std::exception &e) {
      SetError(e.what());
    }
  }

  virtual void OnOK() override {
    Callback().Call(Receiver().Value(), std::initializer_list<napi_value>{ Env().Null(), Napi::String::New(Env(), m_Result.c_str()) });
  }

  virtual void OnProgress(const size_t *data, size_t count) override {
    m_Progress.Call(Receiver().Value(), std::initializer_list<napi_value>{ Napi::Number::New(Env(), data[0]), Napi::Number::New(Env(), data[1]) });
  }

private:
  std::string m_FilePath;
  std::string m_Result;
  Napi::FunctionReference m_Progress;
};

Napi::Value fileMD5(const Napi::CallbackInfo& info) {
  try {
    if ((info.Length() < 2) || (info.Length() > 3)) {
      throw std::runtime_error("Expected two or three parameters (path, callback, progressCB?)");
    }

    Napi::Function callback = info[1].As<Napi::Function>();
    Napi::Function progress = (info.Length() > 2)
      ? info[2].As<Napi::Function>()
      : Napi::Function();

    auto worker = new MD5Worker(info[0].ToString().Utf8Value(), progress, callback);
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
