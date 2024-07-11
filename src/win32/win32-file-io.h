#include "../../src/win32/win32-handmade-hero.h"

#ifndef WIN32_FILE_IO_H_

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

#define WIN32_FILE_IO_H_
#endif  // WIN32_FILE_IO_H_
