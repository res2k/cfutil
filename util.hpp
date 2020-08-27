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
 * Utility functions
 */
#ifndef UTIL_HPP_
#define UTIL_HPP_

#include <cstdint>
#include <string>

/// Return string for Windows error code
std::wstring ErrorString(uint32_t error);

#endif // UTIL_HPP_
