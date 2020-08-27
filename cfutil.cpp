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

#include "Command.hpp"
#include "CommandDelete.hpp"
#include "CommandInfo.hpp"
#include "ntdll.hpp"

#include <functional>
#include <iostream>
#include <unordered_map>

#include <boost/algorithm/string/case_conv.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/key.hpp>
#include <boost/multi_index/sequenced_index.hpp>
#include <boost/optional.hpp>
#include <boost/program_options.hpp>
#include <boost/utility/string_view.hpp>

namespace multi_index = boost::multi_index;

class CommandManager
{
public:
  template<typename Impl>
  void add();

  std::unique_ptr<Command> resolve(const wchar_t* id) const;

  struct AvailableCommand
  {
    const wchar_t* id;
    const wchar_t* descr;
  };
  std::vector<AvailableCommand> all() const;

private:
  struct RegisteredCommand
  {
    boost::wstring_view id;
    const wchar_t* descr;
    std::function<std::unique_ptr<Command>()> factoryFunc;
  };

  struct Seq;
  struct Id;
  boost::multi_index_container<
    RegisteredCommand,
    multi_index::indexed_by<multi_index::sequenced<multi_index::tag<Seq>>,
                            multi_index::hashed_unique<multi_index::tag<Id>, multi_index::key<&RegisteredCommand::id>>>>
    commands;
};

template<typename Impl>
void CommandManager::add()
{
  commands.push_back(RegisteredCommand { Impl::id(), Impl::description(),
                                         []() -> std::unique_ptr<Command> { return std::make_unique<Impl>(); } });
}

std::unique_ptr<Command> CommandManager::resolve(const wchar_t* id) const
{
  std::wstring idStr(id);
  boost::algorithm::to_lower(idStr);
  auto cmdByKey = commands.get<Id>().find(idStr);
  if(cmdByKey == commands.get<Id>().end())
    return nullptr;
  return cmdByKey->factoryFunc();
}

std::vector<CommandManager::AvailableCommand> CommandManager::all() const
{
  std::vector<AvailableCommand> cmds;
  for(const auto& cmd : commands.get<Id>())
  {
    cmds.emplace_back(AvailableCommand { cmd.id.data(), cmd.descr });
  }
  return cmds;
}

//---------------------------------------------------------------------------

namespace po = boost::program_options;

int wmain(int argc, const wchar_t* const argv[])
{
  if (auto ntdll_err = ntdllInit(); ntdll_err != ERROR_SUCCESS)
    return static_cast<int>(ntdll_err);
  RtlSetProcessPlaceholderCompatibilityMode(PHCM_EXPOSE_PLACEHOLDERS);

  CommandManager cmds;
  cmds.add<CommandInfo>();
  cmds.add<CommandDelete>();

  /* Arguments order:
    <general options> <command> <command options>
    ...so look for index with command by skipping all (initial)
    args starting with '-' */
  int commandOptIndex = argc;
  for (int i = 1; i < argc; i++) {
    if(*argv[i] != '-') {
      commandOptIndex = i + 1;
      break;
    }
  }

  po::options_description generalOpt("General options");
  generalOpt.add_options()
      ("help,h", "show help message")
  ;
  po::options_description commandDesc("Command option");
  boost::optional<std::wstring> command;
  commandDesc.add_options()
      ("command", po::wvalue(&command), "command to execute")
  ;

  po::options_description allDesc;
  allDesc.add(generalOpt).add(commandDesc);

  po::positional_options_description commandPositional;
  commandPositional.add("command", 1);

  po::wcommand_line_parser parser(std::vector<std::wstring> (argv + 1, argv + commandOptIndex));
  parser.options(allDesc).positional(commandPositional);

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
    std::wcout << L"Usage:" << std::endl;
    std::wcout << L"  " << argv[0] << L" [general options] <command>" << std::endl;

    po::options_description displayDesc;
    displayDesc.add(generalOpt);

    std::ostringstream descStream;
    descStream << displayDesc;

    std::wcout << boost::from_local_8_bit(descStream.str()) << std::endl;

    std::wcout << L"Available commands:" << std::endl;
    for(const auto& cmd : cmds.all()) {
      std::wcout << L"  " << cmd.id << L"\t" << cmd.descr << std::endl;
    }
  };

  if (vm.count("help")) {
    printHelp();
    return 0;
  }

  if(!command) {
    std::wcerr << L"Missing 'command' argument!" << std::endl;
    printHelp();
    return 1;
  }

  // Handle command
  auto cmdObj = cmds.resolve(command->c_str());
  if(!cmdObj) {
    std::wcerr << L"Invalid 'command' argument: " << *command << std::endl;
    return 1;
  }

  return cmdObj->exec(&argv[commandOptIndex], &argv[argc]);
}
