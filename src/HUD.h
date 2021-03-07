#pragma once

#include <freeglut_std.h>
#include <string>

void writeOnScreen(std::string str, float x, float y, float r = 1.0f, float g = 0.0f, float b = 0.0f);

void writeOnScreen(int digit, float x, float y, float r = 1.0f, float g = 0.0f, float b = 0.0f);

void writeOnScreen(float digit, float x, float y, float r = 1.0f, float g = 0.0f, float b = 0.0f);
