#pragma once
#include <string>
#include <memory>
#include <thread>
#include <vector>
#include <set>
#include <map>
#include <unordered_set>
#include <unordered_map>
#include <cassert>
#include <functional>

#include "EasyGuiExport.h"
#include "Queue.h"
#include "ThreadTaskQueue.h"
#include "Reflect/Reflect.h"

#ifdef _WIN32
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif