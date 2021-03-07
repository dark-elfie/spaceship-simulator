#include "HUD.h"

void writeOnScreen(std::string str, float x, float y, float r, float g, float b)
{
	int len = str.length();
	glColor3f(r, g, b);
	glRasterPos2f(x, y);
	for (int i = 0; i < len; i++) {
		glutBitmapCharacter(GLUT_BITMAP_9_BY_15, str[i]);
	}
}

void writeOnScreen(int digit, float x, float y, float r, float g, float b)
{
	std::string str = std::to_string(digit);
	int len = str.length();
	glColor3f(r, g, b);
	glRasterPos2f(x, y);
	for (int i = 0; i < len; i++) {
		glutBitmapCharacter(GLUT_BITMAP_9_BY_15, str[i]);
	}
}

void writeOnScreen(float digit, float x, float y, float r, float g, float b)
{
	std::string str = std::to_string(digit);
	int len = str.length();
	glColor3f(r, g, b);
	glRasterPos2f(x, y);
	for (int i = 0; i < len; i++) {
		glutBitmapCharacter(GLUT_BITMAP_9_BY_15, str[i]);
	}
}