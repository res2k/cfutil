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
 * Functions dynamically retrieved from NTDLL
 */
#ifndef NTDLL_HPP_
#define NTDLL_HPP_

#include "wrap_Windows.h"

DWORD ntdllInit();

enum {
  PHCM_APPLICATION_DEFAULT = 0,
  PHCM_DISGUISE_PLACEHOLDER = 1,
  PHCM_EXPOSE_PLACEHOLDERS = 2,
  PHCM_MAX = 2,
  PHCM_ERROR_INVALID_PARAMETER = -1,
  PHCM_ERROR_NO_TEB = -2
};

CHAR RtlSetProcessPlaceholderCompatibilityMode(CHAR mode);
NTSTATUS NtDeleteFile(IN POBJECT_ATTRIBUTES ObjectAttributes);

#endif // NTDLL_HPP_
