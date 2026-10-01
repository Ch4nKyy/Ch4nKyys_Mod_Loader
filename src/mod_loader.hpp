#pragma once

#include <windows.h>

bool IsNativeModLoaderPresent(HINSTANCE instance);
bool InstallInitTermEHook(HINSTANCE instance);
