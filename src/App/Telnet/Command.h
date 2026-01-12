#ifndef COMMAND_CLASS
#define COMMAND_CLASS

#include <App/Global.h>

namespace App {

  #define COLOR_RESET "\x1B[0m"
  #define COLOR_BLACK "\x1B[030m"
  #define COLOR_RED "\x1B[031m"
  #define COLOR_GREEN "\x1B[032m"
  #define COLOR_YELLOW "\x1B[033m"
  #define COLOR_BLUE "\x1B[034m"
  #define COLOR_MAGENTA "\x1B[035m"
  #define COLOR_CYAN "\x1B[036m"
  #define COLOR_WHITE "\x1B[037m"

  class Command_ {
    public:
      virtual ~Command_() = default;
      virtual const char *GetName() const = 0;
      virtual bool Execute(const char *tArguments, WiFiClient &tClient) = 0;
      virtual const char *Help() const = 0;
  };

}

#endif