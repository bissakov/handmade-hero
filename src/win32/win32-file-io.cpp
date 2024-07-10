#include "../../src/win32/win32-file-io.h"

FileResult ReadEntireFileDebug(wchar_t *file_path) {
  FileResult result = {};

  HANDLE file_handle = CreateFileW(file_path, GENERIC_READ, FILE_SHARE_READ, 0,
                                   OPEN_EXISTING, 0, 0);
  if (file_handle == INVALID_HANDLE_VALUE) {
    CloseHandle(file_handle);
    return result;
  }

  LARGE_INTEGER file_size;

  if (!GetFileSizeEx(file_handle, &file_size)) {
    CloseHandle(file_handle);
    return result;
  }

  Assert(file_size.QuadPart <= 0xFF'FF'FF'FF);
  result.file_size = (uint32_t)(file_size.QuadPart);

  result.content = VirtualAlloc(0, result.file_size, MEM_RESERVE | MEM_COMMIT,
                                PAGE_READWRITE);
  if (!result.content) {
    FreeFileMemoryDebug(&result.content);
    CloseHandle(file_handle);
    return result;
  }

  DWORD bytes_read = 0;
  if (!(ReadFile(file_handle, result.content, result.file_size, &bytes_read,
                 0) &&
        result.file_size == bytes_read)) {
    FreeFileMemoryDebug(&result.content);
    CloseHandle(file_handle);
    return result;
  }

  CloseHandle(file_handle);

  return result;
}

void FreeFileMemoryDebug(void **memory) {
  if (!memory || !*memory) {
    return;
  }

  VirtualFree(*memory, 0, MEM_RELEASE);
  *memory = 0;

  Assert(!*memory);
}

bool WriteEntireFileDebug(wchar_t *file_path, uint32_t memory_size,
                          void *memory) {
  HANDLE file_handle =
      CreateFileW(file_path, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
  if (file_handle == INVALID_HANDLE_VALUE) {
    CloseHandle(file_handle);
    return false;
  }

  DWORD bytes_written = 0;
  if (!WriteFile(file_handle, memory, memory_size, &bytes_written, 0)) {
    CloseHandle(file_handle);
    return false;
  }

  CloseHandle(file_handle);
  return bytes_written == memory_size;
}
