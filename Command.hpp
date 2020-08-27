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
