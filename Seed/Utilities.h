#pragma once
#ifndef UTILITIES_H
#define UTILITIES_H

#ifndef XELLANIX_STRING_UTILS_H
#if USE_XSTRING_UTILS == 1
#include "../../../Libraries/utilities/string_utils.h"
#endif // USE_XSTRING_UTILS
#endif // !XELLANIX_STRING_UTILS_H

#ifndef XELLANIX_URI_NORMALIZE_H
#if USE_XURI_NORMALIZE == 1
#include "../../../Libraries/utilities/uri_normalize.h"
#endif // USE_URI_NORMALIZE
#endif // !#ifndef XELLANIX_URI_NORMALIZE_H

#ifndef XELLANIX_FILE_INFO_H
#if USE_XFILE_INFO == 1
#include "../../../Libraries/utilities/file_info.h"
#endif // USE_FILE_INFO
#endif // !XELLANIX_FILE_INFO_H

#ifndef XELLANIX_TIME_UTILS_H
#if USE_XTIME_UTILS == 1
#include "../../../Libraries/utilities/time_utils.h"
#endif // USE_XTIME_UTILS
#endif

#ifndef XELLANIX_APP_DIR_H
#if USE_XAPP_DIR == 1
#include "../../../Libraries/utilities/app_dir.h"
#endif
#endif // !XELLANIX_APP_DIR_H

#ifndef XELLANIX_SEARCH_H
#if USE_XSEARCH == 1
#include "../../../Libraries/utilities/search.h"
#endif // USE_XSEARCH
#endif // !XELLANIX_SEARCH_H

#ifndef XELLANIX_SORT_H
#if USE_XSORT == 1
#include "../../../Libraries/utilities/sort.h"
#endif // USE_XSORT
#endif // !XELLANIX_SORT_H

#ifndef XELLANIX_ARRAY_UTILS_H
#if USE_XARRAY_UTILS == 1
#include "../../../Libraries/utilities/array_utils.h"
#endif // USE_XARRAY_UTILS
#endif // !XELLANIX_ARRAY_UTILS_H

#ifndef XELLANIX_SWAP_UTILS_H
#if USE_XSWAP_UTILS == 1
#include "../../../Libraries/utilities/swap_utils.h"
#endif // USE_XSWAP_UTILS
#endif // !XELLANIX_SWAP_UTILS_H

#ifndef XELLANIX_WINDOW_UTILS_H
#if USE_XWINDOW_UTILS == 1
#include "../../../Libraries/utilities/window_utils.h"
#endif // USE_XWINDOW_UTILS
#endif // !XELLANIX_WINDOW_UTILS_H

#ifndef XELLANIX_PROCESS_UTILS_H
#if USE_XPROCESS_UTILS == 1
#include "../../../Libraries/utilities/process_utils.h"
#endif // USE_XPROCESS_UTILS
#endif // !XELLANIX_PROCESS_UTILS_H

#ifndef XELLANIX_NUMBER_UTILS_H
#if USE_XNUMBER_UTILS == 1
#include "../../../Libraries/utilities/number_utils.h"
#endif // USE_XNUMBER_UTILS
#endif // !XELLANIX_NUMBER_UTILS_H

#endif // !UTILITIES_H
