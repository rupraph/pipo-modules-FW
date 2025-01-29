#ifndef FS_TOOLS_H
#define FS_TOOLS_H

#include <Arduino.h>
#include "FS.h"
#include <LittleFS.h>
#include "HW_CONFIG.h"
#include "utils/debug.h"

void init_filesystem();
void createDir(fs::FS& fs, const char* path);
void writeFile(fs::FS& fs, const char* path, const char* message);
void writeFile2(fs::FS& fs, const char* path, const char* message);
void deleteFile(fs::FS& fs, const char* path);
void copyFile(fs::FS& fs, const char* path1, const char* path2);
void deleteFile2(fs::FS& fs, const char* path);
void removeDir(fs::FS& fs, const char* path);
std::string readFile(fs::FS& fs, const char* path);
void appendFile(fs::FS& fs, const char* path, const char* message);
void renameFile(fs::FS& fs, const char* path1, const char* path2);
void testFileIO(fs::FS& fs, const char* path);
void listDir(fs::FS& fs, const char* dirname, uint8_t levels);

#endif  //FS_TOOLS_H