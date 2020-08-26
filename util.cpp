#include "util.hpp"

#include <sstream>

#include "wrap_Windows.h"

std::wstring ErrorString(uint32_t error)
{
  std::wstring result;
  wchar_t* message = nullptr;
  FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, nullptr, error, 0,
                (LPWSTR)&message, 0, nullptr);
  if (message) {
    result = message;
    LocalFree(message);

    while ((result.size() > 0) && (wcschr(L"\r\n", *(result.rbegin())) != nullptr))
      result.resize(result.size() - 1);
  } else {
    auto err = GetLastError();
    std::wostringstream stream;
    stream << L"<FormatMessage failed: " << err << L">";
    result = stream.str();
  }
  return result;
}
