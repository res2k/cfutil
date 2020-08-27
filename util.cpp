/*
    cfutil
    Copyright (c) 2020 Frank Richter

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

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
