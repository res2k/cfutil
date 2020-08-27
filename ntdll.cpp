#include "ntdll.hpp"

#include <iostream>

#include <boost/program_options.hpp>

#include "util.hpp"

using FnRtlSetProcessPlaceholderCompatibilityMode = CHAR(NTAPI*)(CHAR);
static FnRtlSetProcessPlaceholderCompatibilityMode pfnRtlSetProcessPlaceholderCompatibilityMode =
  nullptr;
using FnNtDeleteFile = NTSTATUS(NTAPI*)(IN POBJECT_ATTRIBUTES);
static FnNtDeleteFile pfnNtDeleteFile = nullptr;

template<typename PFN>
static DWORD TryGetFunction(HMODULE ntdll, PFN& func, const char* name)
{
  DWORD err = ERROR_SUCCESS;
  func = reinterpret_cast<PFN>(GetProcAddress(ntdll, name));
  if (func == nullptr) {
    err = GetLastError();
    std::wcerr << "Error retrieving " << boost::from_local_8_bit(name) << " handle: " << ErrorString(err) << std::endl;
  }
  return err;
}

DWORD ntdllInit()
{
  auto ntdll = GetModuleHandle(L"ntdll.dll");
  if (ntdll == nullptr) {
    DWORD err = GetLastError();
    std::wcerr << "Error obtaining NTDLL handle: " << ErrorString(err) << std::endl;
    return err;
  }

  if (auto funcErr = TryGetFunction(ntdll, pfnRtlSetProcessPlaceholderCompatibilityMode,
                                    "RtlSetProcessPlaceholderCompatibilityMode");
      funcErr != ERROR_SUCCESS) {
    return funcErr;
  }
  if (auto funcErr = TryGetFunction(ntdll, pfnNtDeleteFile, "NtDeleteFile"); funcErr != ERROR_SUCCESS) {
    return funcErr;
  }

  return ERROR_SUCCESS;
}

CHAR RtlSetProcessPlaceholderCompatibilityMode (CHAR mode)
{
  return pfnRtlSetProcessPlaceholderCompatibilityMode (mode);
}

NTSTATUS NtDeleteFile(IN POBJECT_ATTRIBUTES ObjectAttributes)
{
  return pfnNtDeleteFile(ObjectAttributes);
}
