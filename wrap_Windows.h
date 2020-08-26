// Wrapper for Windows.h, avoiding double definition warnings for NT status codes

#define WIN32_NO_STATUS
#include <Windows.h>
#include <winternl.h>
#undef WIN32_NO_STATUS
#include <ntstatus.h>
