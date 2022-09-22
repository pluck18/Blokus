#pragma once

#ifdef EXPORT_UNITTESING_API
#define UNITTESTING_API __declspec(dllexport)
#else
#define UNITTESTING_API __declspec(dllimport)
#endif
