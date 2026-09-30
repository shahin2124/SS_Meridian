#pragma once
#ifdef _WIN64
#error SS Meridian Mission 3 must be built for Win32/x86.
#endif
#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <cstdlib>
#include <windows.h>
#include "glut.h"
#else
// Host-only QA build. The delivered Visual Studio solution targets Win32 exclusively.
#include <GLUT/glut.h>
#endif
