//log.cpp

#include <stdlib.h>
#include <stdio.h>
#include "system.h" // SysShutdown();
#include "log.h"

void Log (char *message) {
  FILE *file;
  
  file = fopen(LOGFILE, "a");
    
  if (file) {
    fputs(message, file);
    fclose(file);
  }
}

void LogErr (char *message) {
  Log(message);
  Log("\n");
  SysShutdown();
}
