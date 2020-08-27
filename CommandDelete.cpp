#include "CommandDelete.hpp"

#include "ntdll.hpp"
#include "util.hpp"

#include <algorithm>
#include <filesystem>
#include <iostream>
#include <queue>

#include <boost/program_options.hpp>

#include "wrap_Windows.h"

static NTSTATUS CloudDeleteNt(const wchar_t* filename)
{
  std::wstring nt_path(L"\\??\\");
  nt_path.append(filename);
  UNICODE_STRING uniName;
  RtlInitUnicodeString(&uniName, nt_path.c_str());

  HANDLE fileHandle = INVALID_HANDLE_VALUE;
  OBJECT_ATTRIBUTES objAttr;
  InitializeObjectAttributes(&objAttr, &uniName, OBJ_CASE_INSENSITIVE, nullptr, nullptr);

  auto status = NtDeleteFile(&objAttr);
  if (!NT_SUCCESS(status)) {
    if (status == STATUS_CLOUD_FILE_METADATA_CORRUPT) {
      // Even if status is STATUS_CLOUD_FILE_METADATA_CORRUPT file might have been deleted!
      std::error_code ec;
      if (!std::filesystem::exists(filename, ec))
        return STATUS_SUCCESS;
    }
  }
  return status;
}

static void CloudDelete(const std::filesystem::path& fullpath)
{
  std::deque<std::filesystem::path> pathQueue;
  std::deque<std::filesystem::path> deleteQueue;

  pathQueue.push_back(fullpath);
  while (!pathQueue.empty ()) {
    auto path = std::move(pathQueue.front());
    pathQueue.pop_front();

    /* Fetch attributes using FindFirstFile. Other approaches (GetFileAttributes,
     * GetFileInformationByHandle) apparently already trigger the "cloud file" path. */
    WIN32_FIND_DATA findData;
    auto findHandle = FindFirstFileEx(path.c_str(), FindExInfoBasic, &findData,
                                      FindExSearchNameMatch, nullptr, 0);
    if (findHandle == INVALID_HANDLE_VALUE) {
      auto err = GetLastError();
      if (err != ERROR_CLOUD_FILE_METADATA_CORRUPT) {
        std::wcerr << L"Error getting attributes for " << path << ": " << ErrorString(err)
                   << std::endl;
      }
      continue;
    }

    auto parent = path.parent_path();
    do {
      if ((wcscmp(findData.cFileName, L".") == 0) || (wcscmp(findData.cFileName, L"..") == 0))
        continue;
      std::filesystem::path fullFile = parent / findData.cFileName;
      if ((findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) {
        pathQueue.push_back(fullFile / L"*");
        deleteQueue.push_back(fullFile);
      } else {
        auto status = CloudDeleteNt(fullFile.c_str());
        if (status != STATUS_SUCCESS) {
          std::wcerr << L"Error deleting " << fullFile << ": "
                     << ErrorString(RtlNtStatusToDosError(status)) << std::endl;
        }
      }
    } while (FindNextFile(findHandle, &findData));

    FindClose(findHandle);
  }

  while (!deleteQueue.empty ()) {
    auto delPath = std::move(deleteQueue.back());
    deleteQueue.pop_back();

    auto status = CloudDeleteNt(delPath.c_str());
    if (status != STATUS_SUCCESS) {
      std::wcerr << L"Error deleting " << delPath << ": "
                 << ErrorString(RtlNtStatusToDosError(status)) << std::endl;
    }
  }
}

//---------------------------------------------------------------------------

const wchar_t* CommandDelete::id()
{
  return L"del";
}

const wchar_t* CommandDelete::description()
{
  return L"Delete (possibly corrupted) cloud file(s)";
}

namespace po = boost::program_options;

int CommandDelete::exec(const wchar_t* const* argBegin, const wchar_t* const* argEnd)
{
  po::options_description generalOpt("Command options");
  generalOpt.add_options()
      ("help,h", "show help message")
  ;
  po::options_description commandArgs("Command arguments");
  std::vector<std::wstring> files;
  commandArgs.add_options()
      ("file", po::wvalue(&files), "file to delete")
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
    CloudDelete(f);
  }

  return 0;
}
