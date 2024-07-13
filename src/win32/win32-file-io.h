#ifndef SRC_WIN32_WIN32_FILE_IO_H_
#define SRC_WIN32_WIN32_FILE_IO_H_

#include "../../src/win32/win32-handmade-hero.h"

#if DEV
struct FileResult {
  uint32_t file_size;
  void *content;
};

FileResult ReadEntireFileDebug(wchar_t *file_path);
void FreeFileMemoryDebug(void **memory);
bool WriteEntireFileDebug(wchar_t *file_path, uint32_t memory_size,
                          void *memory);
#endif

#endif  // SRC_WIN32_WIN32_FILE_IO_H_
