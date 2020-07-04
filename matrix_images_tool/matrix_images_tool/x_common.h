#ifndef _X_COMMON_H_
#define _X_COMMON_H_

#include <stdio.h>

#include <chrono>
#include <string>

#define xInfoPrintln(format,...) printf("[LOG]  [INFO] "##format##"\n",##__VA_ARGS__)
#define xErrorPrintln(format,...) printf("[LOG] [ERROR] "##format##"\n",##__VA_ARGS__)

#endif // !_X_COMMON_H_