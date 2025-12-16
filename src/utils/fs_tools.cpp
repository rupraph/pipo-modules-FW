#include <utils/fs_tools.h>

/* You only need to format LittleFS the first time you run a
   test or else use the LITTLEFS plugin to create a partition
   https://github.com/lorol/arduino-esp32littlefs-plugin */

// please don't use printf

void init_filesystem() {
  // Init LittleFS
  if (!LittleFS.begin(FORMAT_LITTLEFS_IF_FAILED)) {
    log_e("LittleFS Mount Failed");
    return;
  }
  log_i("LittleFS Mount Success");
  // listDir(LittleFS, "/config", 2);
  // listDir(LittleFS, "/webpage", 2);
  log_i("Used filesystem bytes: %u", LittleFS.usedBytes());
  if (DEBUG_HEAP)
    pipoDebugHeapFull("End setup filesystem");
}

void listDir(fs::FS& fs, const char* dirname, uint8_t levels) {
  log_d("Listing directory: %s", dirname);

  File root = fs.open(dirname);
  if (!root) {
    log_e("- failed to open directory");
    return;
  }
  if (!root.isDirectory()) {
    log_e(" - not a directory");
    return;
  }

  File file = root.openNextFile();
  while (file) {
    if (file.isDirectory()) {
      log_d("  DIR : %s", file.name());
      time_t t = file.getLastWrite();
      struct tm* tmstruct = localtime(&t);
      // Serial.printf("  LAST WRITE: %d-%02d-%02d
      // %02d:%02d:%02d\n",(tmstruct->tm_year)+1900,( tmstruct->tm_mon)+1,
      // tmstruct->tm_mday,tmstruct->tm_hour , tmstruct->tm_min,
      // tmstruct->tm_sec);

      if (levels) {
        listDir(fs, file.name(), levels - 1);
      }
    } else {
      log_d("  FILE: %s  SIZE: %u", file.name(), file.size());
      time_t t = file.getLastWrite();
      struct tm* tmstruct = localtime(&t);
      // Serial.printf("  LAST WRITE: %d-%02d-%02d
      // %02d:%02d:%02d\n",(tmstruct->tm_year)+1900,( tmstruct->tm_mon)+1,
      // tmstruct->tm_mday,tmstruct->tm_hour , tmstruct->tm_min,
      // tmstruct->tm_sec);
    }
    file = root.openNextFile();
  }
  file.close();
  root.close();
}

void createDir(fs::FS& fs, const char* path) {
  log_d("Creating Dir: %s", path);
  if (fs.mkdir(path)) {
    log_d("Dir created");
  } else {
    log_e("mkdir failed");
  }
}

void removeDir(fs::FS& fs, const char* path) {
  // Serial.printf("Removing Dir: %s\n", path);
  log_d("Removing Dir: %s", path);
  if (fs.rmdir(path)) {
    log_d("Dir removed");
  } else {
    log_e("rmdir failed");
  }
}

// issue with the pipo analog might also be related to the file itself.
std::string readFile(fs::FS& fs, const char* path) {
  log_d("Start reading file: %s", path);

  File file = fs.open(path, "r");
  if (!file || file.isDirectory()) {
    log_e("- failed to open file for reading");
    return std::string();
  }

  log_d("- read file:");
  std::string fileContents;
  char buffer[128];
  while (file.available()) {
    size_t bytesRead = file.readBytes(buffer, 128);
    fileContents.append(buffer, bytesRead);
  }
  file.close();
  log_d("- file read done");

  return fileContents;
}

void writeFile(fs::FS& fs, const char* path, const char* message) {
  log_d("Writing file: %s", path);

  File file = fs.open(path, FILE_WRITE);
  if (!file) {
    log_e("- failed to open file for writing");
    return;
  }
  if (file.print(message)) {
    log_d("- file written");
  } else {
    log_e("- write failed");
  }
  file.close();
}

void appendFile(fs::FS& fs, const char* path, const char* message) {
  log_d("Appending to file: %s", path);

  File file = fs.open(path, FILE_APPEND);
  if (!file) {
    log_e("- failed to open file for appending");
    return;
  }
  if (file.print(message)) {
    log_d("- message appended");
  } else {
    log_e("- append failed");
  }
  file.close();
}

void renameFile(fs::FS& fs, const char* path1, const char* path2) {
  log_d("Renaming file %s to %s", path1, path2);
  if (fs.rename(path1, path2)) {
    log_d("- file renamed");
  } else {
    log_e("- rename failed");
  }
}

void deleteFile(fs::FS& fs, const char* path) {
  log_d("Deleting file: %s", path);
  if (fs.remove(path)) {
    log_d("- file deleted");
  } else {
    log_e("- delete failed");
  }
}

void copyFile(fs::FS& fs, const char* path1, const char* path2) {
  log_d("Copying file from: %s to %s", path1, path2);

  File file = fs.open(path1, "r");
  if (!file || file.isDirectory()) {
    log_e("- failed to open file for reading");
    return;
  }

  File file2 = fs.open(path2, "w");
  if (!file2) {
    log_e("- failed to open file for writing");
    return;
  }

  while (file.available()) {
    file2.write(file.read());
  }

  file.close();
  file2.close();
  log_d("- file copied");
}

// SPIFFS-like write and delete file

// See:
// https://github.com/esp8266/Arduino/blob/master/libraries/LittleFS/src/LittleFS.cpp#L60
void writeFile2(fs::FS& fs, const char* path, const char* message) {
  if (!fs.exists(path)) {
    if (strchr(path, '/')) {
      // Serial.printf("Create missing folders of: %s\r\n", path);
      char* pathStr = strdup(path);
      if (pathStr) {
        char* ptr = strchr(pathStr, '/');
        while (ptr) {
          *ptr = 0;
          fs.mkdir(pathStr);
          *ptr = '/';
          ptr = strchr(ptr + 1, '/');
        }
      }
      free(pathStr);
    }
  }

  // Serial.printf("Writing file to: %s\r\n", path);
  File file = fs.open(path, FILE_WRITE);
  if (!file) {
    log_e("- failed to open file for writing");
    return;
  }
  if (file.print(message)) {
    log_d("- file written");
  } else {
    log_e("- write failed");
  }
  file.close();
}

// See:
// https://github.com/esp8266/Arduino/blob/master/libraries/LittleFS/src/LittleFS.h#L149
void deleteFile2(fs::FS& fs, const char* path) {
  // Serial.printf("Deleting file and empty folders on path: %s\r\n", path);

  if (fs.remove(path)) {
    log_d("- file deleted");
  } else {
    log_e("- delete failed");
  }

  char* pathStr = strdup(path);
  if (pathStr) {
    char* ptr = strrchr(pathStr, '/');
    if (ptr) {
      // Serial.printf("Removing all empty folders on path: %s\r\n", path);
    }
    while (ptr) {
      *ptr = 0;
      fs.rmdir(pathStr);
      ptr = strrchr(pathStr, '/');
    }
    free(pathStr);
  }
}

void testFileIO(fs::FS& fs, const char* path) {
  // Serial.printf("Testing file I/O with %s\r\n", path);

  static uint8_t buf[512];
  size_t len = 0;
  File file = fs.open(path, FILE_WRITE);
  if (!file) {
    log_e("- failed to open file for writing");
    return;
  }

  size_t i;
  log_d("- writing");
  uint32_t start = millis();
  for (i = 0; i < 2048; i++) {
    if ((i & 0x001F) == 0x001F) {
      log_v(".");
    }
    file.write(buf, 512);
  }
  log_d("");
  uint32_t end = millis() - start;
  // Serial.printf(" - %u bytes written in %u ms\r\n", 2048 * 512, end);
  file.close();

  file = fs.open(path);
  start = millis();
  end = start;
  i = 0;
  if (file && !file.isDirectory()) {
    len = file.size();
    size_t flen = len;
    start = millis();
    log_d("- reading");
    while (len) {
      size_t toRead = len;
      if (toRead > 512) {
        toRead = 512;
      }
      file.read(buf, toRead);
      if ((i++ & 0x001F) == 0x001F) {
        log_v(".");
      }
      len -= toRead;
    }
    log_d("");
    end = millis() - start;
    // Serial.printf("- %u bytes read in %u ms\r\n", flen, end);
    file.close();
  } else {
    log_e("- failed to open file for reading");
  }
}