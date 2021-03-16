#include "FS.h"
#include "SD.h"

#ifndef BCBSD
#define BCBSD

xSemaphoreHandle semFile = xSemaphoreCreateMutex();

void initSDCard() {
  if (!SD.begin()) {
    return;
  }
}

void appendFile(fs::FS &fs, const char * path, const char * message) {

  if (xSemaphoreTake(semFile, 500)) {
    File file = fs.open(path, FILE_APPEND);

    if (!file) {
      return;
    }
    file.print(message);
    file.close();

    xSemaphoreGive(semFile);
  }
}

void renameFile(fs::FS &fs, const char * path1, const char * path2) {
  if (xSemaphoreTake(semFile, 500)) {
    fs.rename(path1, path2);
    xSemaphoreGive(semFile);
  }
}

void deleteFile(fs::FS &fs, const char * path) {
  if (xSemaphoreTake(semFile, 500)) {
    fs.remove(path);
    xSemaphoreGive(semFile);
  }
}

#endif
