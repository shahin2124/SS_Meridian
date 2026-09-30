#pragma once
#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
// VS2013 must see the CRT's noreturn declaration of exit before legacy GLUT.
// Keep this ordering here so every OpenGL translation unit receives the fix.
#include <cstdlib>
#include <windows.h>
#include "../../vendor/glut.h"
#else
#include <GLUT/glut.h>
#endif
