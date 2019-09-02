#include <nan.h>
#include <fstream>
#include <iterator>
#include "common.h"
#include "md5.h"

using namespace Nan;
using namespace v8;

class MD5Worker : public AsyncWorker {
public:
  MD5Worker(const std::string &filePath, Nan::Callback *appCallback)
    : Nan::AsyncWorker(appCallback)
    , m_FilePath(filePath)
  {
  }

  void Execute() {
    std::ifstream file(from_utf8(m_FilePath), std::ifstream::binary);
    if (!file.is_open()) {
      SetErrorMessage("Failed to open");
    }
    else {
      try {
        m_Result = MD5::hashStream(file);
      }
      catch (const std::exception &e) {
        SetErrorMessage(e.what());
      }
    }
  }

  void HandleOKCallback() {
    Nan::HandleScope scope;
    v8::Isolate* isolate = v8::Isolate::GetCurrent();

    v8::Local<v8::Value> argv[] = {
      Nan::Null(),
      Nan::New<String>(m_Result).ToLocalChecked(),
    };

    callback->Call(2, argv, async_resource);

  }

private:
  std::string m_FilePath;
  std::string m_Result;
};

NAN_METHOD(fileMD5) {
  Isolate *isolate = Isolate::GetCurrent();
  Local<Context> context = Nan::GetCurrentContext();

  try {
    if (info.Length() != 2) {
      Nan::ThrowError("Expected two parameters (path, callback)");
      return;
    }

    String::Utf8Value pathV8(isolate, info[0]->ToString(context).ToLocalChecked());
    Callback *callback = new Callback(To<v8::Function>(info[1]).ToLocalChecked());

    Nan::AsyncQueueWorker(new MD5Worker(*pathV8, callback));
  }
  catch (const std::exception &e) {
    Nan::ThrowError(e.what());
  }
}

NAN_MODULE_INIT(Init) {
  Nan::Set(target, "fileMD5"_n,
    GetFunction(New<FunctionTemplate>(fileMD5)).ToLocalChecked());
}

NODE_MODULE(vortexmt, Init)