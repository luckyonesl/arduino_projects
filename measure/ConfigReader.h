#ifndef ConfigReader_h
#define ConfigReader_h
#include "Arduino.h"
#define __ASSERT_USE_STDERR
#include <assert.h>
#include <FS.h>

class ConfigReader
{
  public:
    ConfigReader(const String configfile);
    String getConfigValue(char *keyname);
  private:
    String ConfigFile;
};

#endif
