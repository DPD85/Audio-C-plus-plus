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
#include <cstdlib>
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

// Boost

#include <boost/circular_buffer.hpp>

// O.S. API ed altro

#define NOMINMAX

#include <Audioclient.h>
#include <ConsoleApi.h>
#include <Windows.h>
#include <mmdeviceapi.h>
// Deve trovasi dopo mmdeviceapi.h
#include <Functiondiscoverykeys_devpkey.h>

// Impostazioni warning specifici

#pragma warning(3:4061) // Enumerator 'identifier' in a switch of enum is not explicitly handled by a case label.
#pragma warning(3:4062) // Enumerator 'identifier' in a switch of enum 'enumeration' is not handled.
#pragma warning(3:4211) // Nonstandard extension used : redefined extern to static
#pragma warning(3:4263) // Member function does not override any base class virtual member function.
#pragma warning(3:4264) // No override available for virtual member function from base 'class'.
#pragma warning(3:4265) // 'class': class has virtual functions, but destructor is not virtual.
#pragma warning(3:4266) // No override available for one overload of virtual member function from base 'type'.
#pragma warning(3:4287) // 'operator': unsigned/negative constant mismatch.
#pragma warning(3:4296) // 'operator': expression is always false.
#pragma warning(3:4619) // #pragma warning: there is no warning number 'number'.
#pragma warning(3:4822) // 'member': local class member function does not have a body.
#pragma warning(3:4946) // Reinterpret_cast used between related classes: 'class1' and 'class2'.
#pragma warning(3:5038) // Data member 'member1' will be initialized after data member 'member2'.
#pragma warning(3:5240) // 'attribute-name': attribute is ignored in this syntactic position.
#pragma warning(3:5262) // Use [[fallthrough]] when a break statement is intentionally omitted between cases.
#pragma warning(3:5266) // 'const' qualifier on return type has no effect.
