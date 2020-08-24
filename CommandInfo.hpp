/**\file
 * Implementation of 'info' command
 */
#ifndef COMMANDINFO_HPP_
#define COMMANDINFO_HPP_

#include "Command.hpp"

class CommandInfo : public Command
{
public:
  static const wchar_t* id();
  static const wchar_t* description();

  int exec(const wchar_t* const* argBegin, const wchar_t* const* argEnd) override;
};


#endif // COMMANDINFO_HPP_
