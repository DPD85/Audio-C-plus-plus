#pragma once

// C++ standard library

#include <array>
#include <atomic>
#include <cassert>
#include <cctype>
#include <chrono>
#include <clocale>
#include <cmath>
#include <complex>
#include <fstream>
#include <functional>
#include <iomanip>
#include <ios>
#include <iostream>
#include <limits>
#include <mutex>
#include <numbers>
#include <ostream>
#include <stack>
#include <thread>
#include <vector>

// O.S. API ed altro

#define NOMINMAX

#include <Audioclient.h>
#include <ConsoleApi.h>
#include <Windows.h>
#include <conio.h>
#include <mmdeviceapi.h>
// Deve trovasi dopo mmdeviceapi.h
#include <Functiondiscoverykeys_devpkey.h>
