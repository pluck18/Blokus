#pragma once

#ifdef UNITTESTING_EXPORTS
#define UNITTESTING_API __declspec(dllexport)
#else
#define UNITTESTING_API __declspec(dllimport)
#endif
