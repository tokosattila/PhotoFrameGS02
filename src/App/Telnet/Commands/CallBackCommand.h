#ifndef CALLBACK_COMMAND_CLASS
#define CALLBACK_COMMAND_CLASS

#include <App/Telnet/Command.h>

/*TLN.RegisterCommand(new CallbackCommand_(
  "mem", 
  [](const char *tCommand, const char *tArguments, WiFiClient &tClient) -> bool {
    tClient.printf("\r\nFree heap: %u bytes\r\n\r\n", ESP.getFreeHeap());
    return true;
  }, 
  "mem                    | show memory info"
));*/

namespace App {

  class CallbackCommand_ : public Command_ {
    public:
      using Callback = std::function<bool(const char *tArguments, const char *tCommand, WiFiClient &tClient)>;
      CallbackCommand_(const char *tName, Callback tCallback, const char *tHelp = nullptr) : mName(tName), mCallback(std::move(tCallback)), mHelp(tHelp ? tHelp : "") {}
      const char *GetName() const override { 
        return mName; 
      }
      bool Execute(const char *tArguments, WiFiClient &tClient) override {
        const char *tSpace = tArguments ? strchr(tArguments, ' ') : nullptr;
        const char *tParams = tSpace ? tSpace + 1 : "";
        return mCallback(tParams, tArguments, tClient);
      }
      const char *Help() const override {
        return mHelp;
      }
    private:
      const char *mName;
      Callback mCallback;
      const char *mHelp;
    };

}

#endif