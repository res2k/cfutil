#include "CommandInfo.hpp"

const wchar_t* CommandInfo::id()
{
  return L"info";
}

const wchar_t* CommandInfo::description()
{
  return L"Display information on cloud file(s)";
}

int CommandInfo::exec(const wchar_t* const* argBegin, const wchar_t* const* argEnd)
{
  return -1;
}
