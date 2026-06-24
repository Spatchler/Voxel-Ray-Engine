#pragma once

#include <filesystem>
#include <string>

#if defined(_WINDOWS)
  #include <windows.h>
#elif defined(_MACOSX)
  #include <mach-o/dyld.h>
#elif defined(_LINUX)
  #include <unistd.h>
  #include <limits.h>
#endif

namespace RTVE {
  static std::filesystem::path getExePath() {
#if defined(_WINDOWS)
    wchar_t buffer[MAX_PATH];
    GetModuleFileNameW(NULL, buffer, MAX_PATH);
    return std::filesystem::path(buffer).parent_path();
  
#elif defined(_MACOSX)
    char buffer[PATH_MAX];
    uint32_t size = sizeof(buffer);
    if (_NSGetExecutablePath(buffer, &size) == 0)
      return std::filesystem::canonical(std::filesystem::path(buffer)).parent_path();
    return std::filesystem::current_path(); // Fallback
  
#elif defined(_LINUX)
    char buffer[PATH_MAX];
    ssize_t len = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
    if (len != -1) {
      buffer[len] = '\0';
      return std::filesystem::path(buffer).parent_path();
    }
    return std::filesystem::current_path(); // Fallback
#else
    return std::filesystem::current_path(); // Generic fallback
#endif
  }

  static const std::filesystem::path exePath = getExePath();

  class Path {
  public:
    Path(std::filesystem::path pPath, bool pRelativeToExe = true) {
      if (pRelativeToExe) {
        mPath = exePath;
        mPath /= pPath;
      }
      else mPath = pPath;
    }

    std::string string() {
      return mPath.string();
    }

    const char* c_str() {
      return mPath.c_str();
    }

    std::filesystem::path mPath;
  private:
  };
}
