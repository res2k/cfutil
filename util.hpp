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
