/**\file
 * Command base class
 */
#ifndef COMMAND_HPP_
#define COMMAND_HPP_

class Command
{
public:
  virtual ~Command() {}

  //static const wchar_t* id();
  //static const wchar_t* description();

  virtual int exec(const wchar_t* const* argBegin, const wchar_t* const* argEnd) = 0;
};

#endif // COMMAND_HPP_
