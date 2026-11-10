#pragma once

#include "Portability.h"

#ifdef _WIN32

#include <sql.h>
#include <sqltypes.h>
#include <sqlext.h>

#else

#define DWORD unixodbc_DWORD
#define LPDWORD unixodbc_LPDWORD
#define ULONG unixodbc_ULONG

#include <sql.h>
#include <sqltypes.h>
#include <sqlext.h>

#undef DWORD
#undef LPDWORD
#undef ULONG

#endif
