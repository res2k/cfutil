#include <iostream>

#include <boost/optional.hpp>
#include <boost/program_options.hpp>

namespace po = boost::program_options;

int wmain(int argc, const wchar_t* const argv[])
{
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

  // TODO: Handle command

  return 0;
}
