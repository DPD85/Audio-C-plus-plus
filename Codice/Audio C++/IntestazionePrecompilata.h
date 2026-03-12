#pragma once

// C++ standard library

#include <algorithm>
#include <array>
#include <atomic>
#include <cassert>
#include <cctype>
#include <chrono>
#include <clocale>
#include <cmath>
#include <complex>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iomanip>
#include <ios>
#include <iostream>
#include <limits>
#include <mutex>
#include <numbers>
#include <numeric>
#include <ostream>
#include <sstream>
#include <stack>
#include <string>
#include <thread>
#include <vector>

// Boost

#include <boost/circular_buffer.hpp>
#include <boost/locale.hpp>

// PortAudio

#include <portaudio.h>

// ALSA

#ifndef WIN32
#    include <alsa/asoundlib.h>
#endif

// Vulkan

#include <vulkan/vk_enum_string_helper.h>
#include <vulkan/vulkan.h>

// vk-bootstrap

#include <VkBootstrap.h>

// SLD3

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

// ImGui

#include <imgui.h>
#include <imgui_freetype.h>
#include <imgui_internal.h>

// ImGui back-end

#include <imgui_impl_sdl3.h>
#include <imgui_impl_vulkan.h>

// ImPlot

#define IMPLOT_DISABLE_OBSOLETE_FUNCTIONS

#include <implot.h>

// IconFontCppHeaders

#include <IconsFontAwesome7.h>

// platform_folders

#include <sago/platform_folders.h>

// JSON for Modern C++ (nlohmann JSON)

#include <nlohmann/json.hpp>

// O.S. API ed altro

#ifdef WIN32

#    define NOMINMAX

#    include <Windows.h>
// Deve stare dopo Windows.h
#    include <ConsoleApi.h>

// Impostazioni avvisi specifici

#    pragma warning(3:4'061) // Enumerator 'identifier' in a switch of enum is not explicitly handled by a case label.
#    pragma warning(3:4'062) // Enumerator 'identifier' in a switch of enum 'enumeration' is not handled.
#    pragma warning(3:4'211) // Nonstandard extension used : redefined extern to static
#    pragma warning(3:4'263) // Member function does not override any base class virtual member function.
#    pragma warning(3:4'264) // No override available for virtual member function from base 'class'.
#    pragma warning(3:4'265) // 'class': class has virtual functions, but destructor is not virtual.
#    pragma warning(3:4'266) // No override available for one overload of virtual member function from base 'type'.
#    pragma warning(3:4'287) // 'operator': unsigned/negative constant mismatch.
#    pragma warning(3:4'296) // 'operator': expression is always false.
#    pragma warning(3:4'619) // #pragma warning: there is no warning number 'number'.
#    pragma warning(3:4'822) // 'member': local class member function does not have a body.
#    pragma warning(3:4'946) // Reinterpret_cast used between related classes: 'class1' and 'class2'.
#    pragma warning(3:5'038) // Data member 'member1' will be initialized after data member 'member2'.
#    pragma warning(3:5'233) // explicit lambda capture 'identifier' is not used.
#    pragma warning(3:5'240) // 'attribute-name': attribute is ignored in this syntactic position.
#    pragma warning(3:5'262) // Use [[fallthrough]] when a break statement is intentionally omitted between cases.
#    pragma warning(3:5'264) // 'variable-name': 'const' variable is not used.
#    pragma warning(3:5'266) // 'const' qualifier on return type has no effect.

#else

#    include <signal.h>
#    include <unistd.h>

#endif
