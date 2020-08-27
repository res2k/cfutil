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
