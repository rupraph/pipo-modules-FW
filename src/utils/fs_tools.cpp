#include <utils/fs_tools.h>

/* You only need to format LittleFS the first time you run a
   test or else use the LITTLEFS plugin to create a partition
   https://github.com/lorol/arduino-esp32littlefs-plugin */

// please don't use printf

void init_filesystem() {
  // Init LittleFS
  if (!LittleFS.begin(FORMAT_LITTLEFS_IF_FAILED)) {
    Serial.println("LittleFS Mount Failed");
    return;
  }
  Serial.println("LittleFS Mount Success");
  // listDir(LittleFS, "/config", 2);
  // listDir(LittleFS, "/webpage", 2);
  Serial.print("Used filesystem bytes: ");
  Serial.println(LittleFS.usedBytes());
#ifdef DEBUG_HEAP
  Serial.println("Remaining Heap:" + String(ESP.getFreeHeap()));
#endif
}

void listDir(fs::FS& fs, const char* dirname, uint8_t levels) {
  Serial.print("Listing directory:");
  Serial.println(dirname);

  File root = fs.open(dirname);
  if (!root) {
    Serial.println("- failed to open directory");
    return;
  }
  if (!root.isDirectory()) {
    Serial.println(" - not a directory");
    return;
  }

  File file = root.openNextFile();
  while (file) {
    if (file.isDirectory()) {
      Serial.print("  DIR : ");

      Serial.print(file.name());
      time_t t = file.getLastWrite();
      struct tm* tmstruct = localtime(&t);
      //Serial.printf("  LAST WRITE: %d-%02d-%02d %02d:%02d:%02d\n",(tmstruct->tm_year)+1900,( tmstruct->tm_mon)+1, tmstruct->tm_mday,tmstruct->tm_hour , tmstruct->tm_min, tmstruct->tm_sec);

      if (levels) {
        listDir(fs, file.name(), levels - 1);
      }
    } else {
      Serial.print("  FILE: ");
      Serial.print(file.name());
      Serial.print("  SIZE: ");

      Serial.print(file.size());
      time_t t = file.getLastWrite();
      struct tm* tmstruct = localtime(&t);
      //Serial.printf("  LAST WRITE: %d-%02d-%02d %02d:%02d:%02d\n",(tmstruct->tm_year)+1900,( tmstruct->tm_mon)+1, tmstruct->tm_mday,tmstruct->tm_hour , tmstruct->tm_min, tmstruct->tm_sec);
    }
    file = root.openNextFile();
  }
  file.close();
  root.close();
}

void createDir(fs::FS& fs, const char* path) {
  Serial.print("Creating Dir:");
  Serial.println(path);
  if (fs.mkdir(path)) {
    Serial.println("Dir created");
  } else {
    Serial.println("mkdir failed");
  }
}

void removeDir(fs::FS& fs, const char* path) {
  //Serial.printf("Removing Dir: %s\n", path);
  Serial.print("Removing Dir:");
  Serial.println(path);
  if (fs.rmdir(path)) {
    Serial.println("Dir removed");
  } else {
    Serial.println("rmdir failed");
  }
}

// issue with the pipo analog might also be related to the file itself.
std::string readFile(fs::FS& fs, const char* path) {
  Serial.print("Start reading file: ");
  Serial.println(path);

  File file = fs.open(path, "r");
  if (!file || file.isDirectory()) {
    Serial.println("- failed to open file for reading");
    return std::string();
  }

  Serial.println("- read file:");
  std::string fileContents;
  char buffer[128];
  while (file.available()) {
    size_t bytesRead = file.readBytes(buffer, 128);
    fileContents.append(buffer, bytesRead);
  }
  file.close();
  Serial.println("- file read done");

  return fileContents;
}

void writeFile(fs::FS& fs, const char* path, const char* message) {
  Serial.print("Writing file: ");
  Serial.println(path);

  File file = fs.open(path, FILE_WRITE);
  if (!file) {
    Serial.println("- failed to open file for writing");
    return;
  }
  if (file.print(message)) {
    Serial.println("- file written");
  } else {
    Serial.println("- write failed");
  }
  file.close();
}

void appendFile(fs::FS& fs, const char* path, const char* message) {
  Serial.print("Appending to file:");
  Serial.println(path);

  File file = fs.open(path, FILE_APPEND);
  if (!file) {
    Serial.println("- failed to open file for appending");
    return;
  }
  if (file.print(message)) {
    Serial.println("- message appended");
  } else {
    Serial.println("- append failed");
  }
  file.close();
}

void renameFile(fs::FS& fs, const char* path1, const char* path2) {
  Serial.print("Renaming file");
  Serial.print(path1);
  Serial.print("to");
  Serial.println(path2);
  if (fs.rename(path1, path2)) {
    Serial.println("- file renamed");
  } else {
    Serial.println("- rename failed");
  }
}

void deleteFile(fs::FS& fs, const char* path) {
  Serial.println("Deleting file:" + String(path));
  if (fs.remove(path)) {
    Serial.println("- file deleted");
  } else {
    Serial.println("- delete failed");
  }
}

void copyFile(fs::FS& fs, const char* path1, const char* path2) {
  Serial.println("Copying file from:" + String(path1) + String(path2));

  File file = fs.open(path1, "r");
  if (!file || file.isDirectory()) {
    Serial.println("- failed to open file for reading");
    return;
  }

  File file2 = fs.open(path2, "w");
  if (!file2) {
    Serial.println("- failed to open file for writing");
    return;
  }

  while (file.available()) {
    file2.write(file.read());
  }

  file.close();
  file2.close();
  Serial.println("- file copied");
}

// SPIFFS-like write and delete file

// See: https://github.com/esp8266/Arduino/blob/master/libraries/LittleFS/src/LittleFS.cpp#L60
void writeFile2(fs::FS& fs, const char* path, const char* message) {
  if (!fs.exists(path)) {
    if (strchr(path, '/')) {
      //Serial.printf("Create missing folders of: %s\r\n", path);
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

  //Serial.printf("Writing file to: %s\r\n", path);
  File file = fs.open(path, FILE_WRITE);
  if (!file) {
    Serial.println("- failed to open file for writing");
    return;
  }
  if (file.print(message)) {
    Serial.println("- file written");
  } else {
    Serial.println("- write failed");
  }
  file.close();
}

// See:  https://github.com/esp8266/Arduino/blob/master/libraries/LittleFS/src/LittleFS.h#L149
void deleteFile2(fs::FS& fs, const char* path) {
  //Serial.printf("Deleting file and empty folders on path: %s\r\n", path);

  if (fs.remove(path)) {
    Serial.println("- file deleted");
  } else {
    Serial.println("- delete failed");
  }

  char* pathStr = strdup(path);
  if (pathStr) {
    char* ptr = strrchr(pathStr, '/');
    if (ptr) {
      //Serial.printf("Removing all empty folders on path: %s\r\n", path);
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
  //Serial.printf("Testing file I/O with %s\r\n", path);

  static uint8_t buf[512];
  size_t len = 0;
  File file = fs.open(path, FILE_WRITE);
  if (!file) {
    Serial.println("- failed to open file for writing");
    return;
  }

  size_t i;
  Serial.print("- writing");
  uint32_t start = millis();
  for (i = 0; i < 2048; i++) {
    if ((i & 0x001F) == 0x001F) {
      Serial.print(".");
    }
    file.write(buf, 512);
  }
  Serial.println("");
  uint32_t end = millis() - start;
  //Serial.printf(" - %u bytes written in %u ms\r\n", 2048 * 512, end);
  file.close();

  file = fs.open(path);
  start = millis();
  end = start;
  i = 0;
  if (file && !file.isDirectory()) {
    len = file.size();
    size_t flen = len;
    start = millis();
    Serial.print("- reading");
    while (len) {
      size_t toRead = len;
      if (toRead > 512) {
        toRead = 512;
      }
      file.read(buf, toRead);
      if ((i++ & 0x001F) == 0x001F) {
        Serial.print(".");
      }
      len -= toRead;
    }
    Serial.println("");
    end = millis() - start;
    //Serial.printf("- %u bytes read in %u ms\r\n", flen, end);
    file.close();
  } else {
    Serial.println("- failed to open file for reading");
  }
}