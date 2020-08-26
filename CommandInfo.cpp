#include "CommandInfo.hpp"

#include "util.hpp"

#include <algorithm>
#include <filesystem>
#include <iostream>
#include <queue>

#include <boost/program_options.hpp>

#include "wrap_Windows.h"
#include <cfapi.h>

static std::wstring PlaceholderStateString (const WIN32_FIND_DATA& findData)
{
  auto state = CfGetPlaceholderStateFromFindData(&findData);

  std::wstring result;

  if ((findData.dwFileAttributes & FILE_ATTRIBUTE_OFFLINE) != 0) {
    result.append(L"(offline)");
  }

  if (state == CF_PLACEHOLDER_STATE_NO_STATES) {
    if (!result.empty())
      result.append(L", ");
    result.append(L"NO_STATES");
    return result;
  }
  if (state == CF_PLACEHOLDER_STATE_INVALID) {
    if (!result.empty())
      result.append(L", ");
    result.append(L"INVALID");
    return result;
  }

  auto handle_state = [&](CF_PLACEHOLDER_STATE flag, const wchar_t* str) {
    if ((state & flag) != 0) {
      if (!result.empty())
        result.append(L", ");
      result.append(str);
      state &= ~flag;
    }
  };

  handle_state(CF_PLACEHOLDER_STATE_PLACEHOLDER, L"PLACEHOLDER");
  handle_state(CF_PLACEHOLDER_STATE_SYNC_ROOT, L"SYNC_ROOT");
  handle_state(CF_PLACEHOLDER_STATE_ESSENTIAL_PROP_PRESENT, L"ESSENTIAL_PROP_PRESENT");
  handle_state(CF_PLACEHOLDER_STATE_IN_SYNC, L"IN_SYNC");
  handle_state(CF_PLACEHOLDER_STATE_PARTIAL, L"PARTIAL");
  handle_state(CF_PLACEHOLDER_STATE_PARTIALLY_ON_DISK, L"PARTIALLY_ON_DISK");

  if (state != 0) {
    if (!result.empty())
      result.append(L", ");
    result.append(L"(CF_PLACEHOLDER_STATE ");
    result.append(std::to_wstring(static_cast<int>(state)));
    result.append(L")");
    }

    return result;
}

static LARGE_INTEGER make_LARGE_INTEGER (int64_t value)
{
    LARGE_INTEGER li;
    li.QuadPart = value;
    return li;
}

static void PrintCloudRanges(const std::filesystem::path& full_path, int64_t file_size, unsigned int level)
{
    std::wstring indent (level + 1, ' ');

    auto fileHandle = CreateFile (full_path.native ().c_str (), FILE_READ_ATTRIBUTES, 0, nullptr, OPEN_EXISTING, FILE_FLAG_OPEN_REPARSE_POINT, nullptr);
    auto openErr = GetLastError ();
    if (fileHandle == INVALID_HANDLE_VALUE)
    {
        std::wcout << indent << L"Error opening: " << ErrorString (openErr) << std::endl;
        return;
    }

    CF_FILE_RANGE stackResult[32];
    std::unique_ptr<CF_FILE_RANGE[]> heapResult;

    CF_FILE_RANGE* resultPtr = stackResult;
    DWORD resultSize = sizeof(stackResult)/sizeof(stackResult[0]);
    DWORD resultReturned = 0;
    HRESULT rangeInfoHR = HRESULT_FROM_WIN32 (ERROR_MORE_DATA);
    while(rangeInfoHR == HRESULT_FROM_WIN32 (ERROR_MORE_DATA))
    {
      rangeInfoHR = CfGetPlaceholderRangeInfo (fileHandle, CF_PLACEHOLDER_RANGE_INFO_ONDISK, make_LARGE_INTEGER (0), make_LARGE_INTEGER (file_size), resultPtr, resultSize * sizeof (CF_FILE_RANGE), &resultReturned);
      if (rangeInfoHR == HRESULT_FROM_WIN32 (ERROR_MORE_DATA))
      {
        resultSize = resultSize + std::min (resultSize, DWORD(1024));
        heapResult.reset (new CF_FILE_RANGE[resultSize]);
        resultPtr = heapResult.get ();
      }
    }
    if (FAILED (rangeInfoHR))
    {
        std::wcout << indent << L"Error obtaining range info: " << rangeInfoHR << std::endl;
    }
    else
    {
        std::wcout << indent << L"ONDISK: [";
        auto num_ui64 = resultReturned / sizeof (CF_FILE_RANGE);
        for (unsigned int i = 0; i < num_ui64; i++)
        {
            if (i > 0) std::wcout << L", ";
            std::wcout << resultPtr[i].StartingOffset.QuadPart << L", " << resultPtr[i].Length.QuadPart;
        }
        std::wcout << L"]" << std::endl;
    }

    CloseHandle (fileHandle);
}

static void HandleFile(const std::filesystem::path& path, const WIN32_FIND_DATA& findData, unsigned int level)
{
  bool is_dir = (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
  std::wstring indent (level, ' ');
  std::wcout << indent << (is_dir ? path : path.filename()) << L": ";

  if((findData.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) == 0) {
    std::wcout << L"not a reparse point" << std::endl;
    return;
  }

  std::wcout << PlaceholderStateString(findData) << std::endl;

  if((findData.dwFileAttributes & FILE_ATTRIBUTE_OFFLINE) != 0) {
    return;
  }

  if((findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) {
    WIN32_FIND_DATA findData;
    auto findHandle = FindFirstFileEx ((path / L"*").native().c_str(), FindExInfoBasic, &findData, FindExSearchNameMatch, nullptr, FIND_FIRST_EX_LARGE_FETCH);
    auto findErr = GetLastError ();
    if (findHandle == INVALID_HANDLE_VALUE)
    {
        std::wcerr << indent << L"Error scanning " << path << ": " << ErrorString (findErr) << std::endl;
        return;
    }

    do
    {
      if ((wcscmp(findData.cFileName, L".") == 0) || (wcscmp(findData.cFileName, L"..") == 0))
        continue;
      HandleFile(path / findData.cFileName, findData, level + 1);
    } while (FindNextFileW(findHandle, &findData));
    FindClose(findHandle);
  } else {
    PrintCloudRanges(path, findData.nFileSizeLow + (int64_t(findData.nFileSizeHigh) << 32), level + 1);
  }
}

static void HandleFile(const std::filesystem::path& initialPath)
{
  WIN32_FIND_DATA findData;
  auto findHandle = FindFirstFileEx(initialPath.c_str(), FindExInfoBasic, &findData,
                                    FindExSearchNameMatch, nullptr, 0);
  if (findHandle == INVALID_HANDLE_VALUE) {
    auto err = GetLastError();
    std::wcout << L"Error getting attributes for " << initialPath << ": " << ErrorString(err) << std::endl;
    return;
  }
  FindClose(findHandle);

  HandleFile(initialPath, findData, 0);
}

//---------------------------------------------------------------------------

const wchar_t* CommandInfo::id()
{
  return L"info";
}

const wchar_t* CommandInfo::description()
{
  return L"Display information on cloud file(s)";
}

namespace po = boost::program_options;

int CommandInfo::exec(const wchar_t* const* argBegin, const wchar_t* const* argEnd)
{
  po::options_description generalOpt("Command options");
  generalOpt.add_options()
      ("help,h", "show help message")
  ;
  po::options_description commandArgs("Command arguments");
  std::vector<std::wstring> files;
  commandArgs.add_options()
      ("file", po::wvalue(&files), "file to display info for")
  ;

  po::options_description allDesc;
  allDesc.add(generalOpt).add(commandArgs);

  po::positional_options_description positional;
  positional.add("file", -1);

  po::wcommand_line_parser parser(std::vector<std::wstring> (argBegin, argEnd));
  parser.options(allDesc).positional(positional);

  po::variables_map vm;
  try
  {
    po::store(parser.run(), vm);
    po::notify(vm);
  }
  catch(const std::exception& e)
  {
    std::wcerr << boost::from_local_8_bit(e.what()) << std::endl;
    return -1;
  }

  auto printHelp = [&]() {
    std::wcout << L"Command usage:" << std::endl;
    std::wcout << L"  " << id() << L" [options] <file> [<file>...]" << std::endl;

    po::options_description displayDesc;
    displayDesc.add(generalOpt);

    std::ostringstream descStream;
    descStream << displayDesc;

    std::wcout << boost::from_local_8_bit(descStream.str()) << std::endl;
  };

  if (vm.count("help") || files.empty()) {
    printHelp();
    return 0;
  }

  for (const auto& f : files) {
    HandleFile(f);
  }

  return 0;
}
