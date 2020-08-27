/**\file
 * Implementation of 'delete' command
 */
#ifndef COMMANDDELETE_HPP_
#define COMMANDDELETE_HPP_

#include "Command.hpp"

class CommandDelete : public Command
{
public:
  static const wchar_t* id();
  static const wchar_t* description();

  int exec(const wchar_t* const* argBegin, const wchar_t* const* argEnd) override;
};


#endif // COMMANDDELETE_HPP_
