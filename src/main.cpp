#include "glew.h"
#include "freeglut.h"
#include "glm.hpp"
#include "ext.hpp"
#include <iostream>
#include <cmath>
#include <gl/gl.h>
#include <stdlib.h>
#include <thread>
#include <string>

#include "Texture.h"		// files given by lecturer to use in project
#include "Shader_Loader.h"	// (after independent implementation of methods created in those files)
#include "Render_Utils.h"	//
#include "Camera.h"			//

GLuint program;
GLuint programSun;
GLuint programTex;
GLuint programTexN;
GLuint programTexShip;
GLuint programProcTex;
GLuint programAtmo;
GLuint programSkybox;
GLuint programParticles;
Core::Shader_Loader shaderLoader;

GLuint textureShip;
GLuint textureEarth;
GLuint textureSun;
GLuint textureMoon;
GLuint textureMars;
GLuint textureJupiter;
GLuint textureSaturn;
GLuint textureHaumea;
GLuint textureVenus;
GLuint textureNeptune;
GLuint textureMilkyWay;
GLuint textureAsteroid;
GLuint textureStar;

GLuint textureShipN;
GLuint textureEarthN;
GLuint textureAsteroidN;

obj::Model shipModel;
obj::Model sphereModel;
obj::Model spaceStationModel;

float t = 0.0f;
int whichSec = 0, whichSec2 = 0;
float appLoadingTime;
float frustumScale = 1.1f;
float cameraAngle = 0;
float shipAngle;
glm::vec3 cameraPos = glm::vec3(-30, 0, 0);
glm::vec3 cameraDir; // Wektor "do przodu" kamery
glm::vec3 cameraSide; // Wektor "w bok" kamery

glm::mat4 cameraMatrix, perspectiveMatrix;

glm::vec3 lightPos1 = glm::vec3(0, 0, 0);
glm::vec3 lightPos2 = glm::vec3(-100, 0, 0);

glm::vec3 playerShipPos;

glm::quat rotation = glm::quat(1, 0, 0, 0);

glm::vec2 mouseLast = glm::vec2(300, 300);
glm::vec2 mouseChange;

glm::quat odwrRotation;

struct Particle {
	glm::vec3 pos, speed;
	float size, angle, weight;
	float life; // Remaining life of the particle. if <0 : dead and unused.
	float cameradistance; // *Squared* distance to the camera. if dead : -1.0f

	bool operator<(const Particle& that) const {
		// Sort in reverse order : far particles drawn first.
		return this->cameradistance > that.cameradistance;
	}
};
double lastTime = 0;

const int MaxParticles = 1;
Particle ParticlesContainer[MaxParticles];
int ParticlesCount = 0;
int LastUsedParticle = 0;

int FindUnusedParticle() {

	for (int i = LastUsedParticle; i < MaxParticles; i++) {
		if (ParticlesContainer[i].life < 0) {
			LastUsedParticle = i;
			return i;
		}
	}

	for (int i = 0; i < LastUsedParticle; i++) {
		if (ParticlesContainer[i].life < 0) {
			LastUsedParticle = i;
			return i;
		}
	}
	return 0;
}

void SortParticles() {
	std::sort(&ParticlesContainer[0], &ParticlesContainer[MaxParticles]);
}

static const int NUM_ASTEROIDS = 10;
glm::vec3 asteroidsPositions[NUM_ASTEROIDS];
static const int NUM_CAMERA_POINTS = 10;
glm::vec3 cameraKeyPoints[NUM_CAMERA_POINTS];

void drawOnScreen(std::string str, GLfloat x, GLfloat y, GLfloat one=1.0f, GLfloat two=0.0f, GLfloat three=0.0f);
void reset();
void refuel();
bool checkStation();

class Spaceship
{
private:
	int fuel;
	int hp;
	obj::Model model;

public:
	Spaceship() {
		hp = 100;
		fuel = 10000;
		model = obj::loadModelFromFile("models/spaceship.obj");
	}

	void setModel(obj::Model model_)
	{
		model = model_;
	}

	obj::Model getModel()
	{
		return model;
	}

	void setFuel(int fuel_)
	{
		fuel = fuel_;
	}

	int getFuel()
	{
		return fuel;
	}

	void setHp(int hp_)
	{
		hp = hp_;
	}

	int getHp()
	{
		return hp;
	}
};

class PlayerSpaceship : public Spaceship
{
private:
	GLuint* textureId;
	GLuint* normalmapId;
	float angleSpeed = 0.1f;
	float moveSpeed = 0.2f;

public:
	PlayerSpaceship()
	{
		textureId = &textureShip;
		normalmapId = &textureShipN;
	}

	void movement(unsigned char key, int x, int y)
	{
		int checkHp = getHp();
		int checkFuel = getFuel();
		//std::cout << key << std::endl;
		if (checkHp <= 0 || checkFuel <= 0)
		{
			angleSpeed = 0;
			moveSpeed = 0;
			if (key == 'r')
			{
				reset();
				angleSpeed = 0.1f;
				moveSpeed = 0.2f;
			}
			if (key == 'q')
			{
				exit(EXIT_SUCCESS);
			}
		}
		if (checkStation())
		{
			if (key == 'r')
			{
				refuel();
			}
		}
		switch (key)
		{
		case 'z': rotation = glm::angleAxis(angleSpeed, glm::vec3(0, 0, -1)) * rotation; break;
		case 'x': rotation = glm::angleAxis(angleSpeed, glm::vec3(0, 0, 1)) * rotation; break;
		case 'w': cameraPos += cameraDir * moveSpeed; fuelUpdate(); break;
		case 's': cameraPos -= cameraDir * moveSpeed; fuelUpdate(); break;
		case 'd': cameraPos += cameraSide * moveSpeed; fuelUpdate(); break;
		case 'a': cameraPos -= cameraSide * moveSpeed; fuelUpdate(); break;
		}
	}

	void setTextureId(GLuint textureId_)
	{
		textureId = &textureId_;
	}

	GLuint getTextureId()
	{
		return *textureId;
	}

	void setNormalmapId(GLuint normalmapId_)
	{
		normalmapId = &normalmapId_;
	}

	GLuint getNormalmapId()
	{
		return *normalmapId;
	}

	double fuelUpdate()
	{
		double heatHydrogen = 141.9*1000000, speed = 1.0, mass = 45000000, speeding = 1.0, howMuch, a, b;
		float h = getHp();
		float efficiency = glm::normalize(h);
		int fuel = getFuel();
		int fuel2 = fuel - 1;
		setFuel(fuel2);
		b = efficiency * heatHydrogen;
		a = speed * mass * speeding;
		howMuch = (a / b);
		return fuel2; //pierwsze oddanie wersja uproszczona przy naciśnięciu 'w' paliwo się zmniejsza o 1
	}

	void shipPosition()
	{
		float root = cameraPos.x * cameraPos.x + cameraPos.y * cameraPos.y + cameraPos.z * cameraPos.z;
		float root2= (cameraPos.x + 100)*(cameraPos.x + 100) + cameraPos.y * cameraPos.y + cameraPos.z * cameraPos.z;
		if (sqrt(root)<=15 || sqrt(root2) <= 15)
		{
			int h = getHp(), h2;
			int timeForFuel = round(t);
			if ((timeForFuel % 1 == 0) && (whichSec != timeForFuel))
			{
				if (h == 0)
				{
					setHp(0);
				}
				else
				{
					h2 = h - 5;
					setHp(h2);
				}
				whichSec = timeForFuel;
			}
			int h3 = getHp();
			if (h3 <= 0)
			{
				std::string x = std::to_string(0);
				drawOnScreen("GAMEOVER HP = 0", -0.2, 0.9);
				drawOnScreen("Press R to restart or Q to close the game.", -0.5, 0.85);
				drawOnScreen("HP: ", -0.5, -0.9);
				drawOnScreen(x, -0.4, -0.9);
			}
			else
			{
				std::string x = std::to_string(h3);
				drawOnScreen("THE SHIP IS TOO CLOSE TO THE SUN!", -0.45, 0.9);
				drawOnScreen("HP: ", -0.5, -0.9);
				drawOnScreen(x, -0.4, -0.9);
			}
		}
		else if (sqrt(root) <= 25 || sqrt(root2) <= 25)
		{
			int h = getHp(), h2;
			int timeForFuel = round(t);

			if ((timeForFuel % 1 == 0) && (whichSec != timeForFuel))
			{
				if (h == 0)
				{
					setHp(0);
				}
				else 
				{
					h2 = h - 1;
					setHp(h2);
				}
				whichSec = timeForFuel;
			}

			int h3 = getHp();

			if (h3 <= 0)
			{
				std::string x = std::to_string(0);
				drawOnScreen("GAMEOVER HP = 0", -0.2, 0.9);
				drawOnScreen("Press R to restart or Q to close the game.", -0.4, 0.8);
				drawOnScreen("HP: ", -0.5, -0.9);
				drawOnScreen(x, -0.4, -0.9);
			}
			else
			{
				std::string x = std::to_string(h3);
				drawOnScreen("THE SHIP IS TOO CLOSE TO THE SUN!", -0.45, 0.9);
				drawOnScreen("HP: ", -0.5, -0.9);
				drawOnScreen(x, -0.4, -0.9);
			}
		}
		else
		{
			int h = getHp();
			if (h == 0)
			{
				std::string x = std::to_string(0);
				drawOnScreen("HP: ", -0.5, -0.9);
				drawOnScreen(x, -0.4, -0.9);
			}
			else
			{
				std::string x = std::to_string(h);
				drawOnScreen("HP: ", -0.5, -0.9);
				drawOnScreen(x, -0.4, -0.9);
			}
		}
	}
};

PlayerSpaceship player;
PlayerSpaceship botShip;

void refuel()
{
	player.setFuel(10000);
}

bool checkStation()
{
	float root = (cameraPos.x + 50) * (cameraPos.x + 50) + cameraPos.y * cameraPos.y + cameraPos.z * cameraPos.z;
	//std::cout << sqrt(root) << std::endl;
	if (sqrt(root) <= 4)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void drawOnScreen(std::string str, GLfloat x, GLfloat y, GLfloat one, GLfloat two, GLfloat three) //string napis do wyświetlenia
													//x=0 y=0  środek układu współrzędnych/ekranu
{                                                    //one, two, three floaty do kolorów
	int len, i;
	glColor3f(one, two, three); //kolor napisu
	glRasterPos2f(x, y); //pozycja napisu
	len = str.length();
	for (i = 0; i < len; i++) {
		glutBitmapCharacter(GLUT_BITMAP_9_BY_15, str[i]);
	}
}

void drawOnScreenDigit(int digit, float x, float y)
{
	std::string s = std::to_string(digit);
	drawOnScreen(s, x, y);
}

void keyboard(unsigned char key, int x, int y)
{
	player.movement(key, x, y);
}

void mouse(int x, int y)
{
	mouseChange = glm::vec2(x - mouseLast.x, y - mouseLast.y);
	mouseLast = glm::vec2(x, y);
}

glm::mat4 getCatmullRomPath()
{
	float time = (glutGet(GLUT_ELAPSED_TIME) / 1000.f - appLoadingTime) / 8;
	float dec;
	float s = std::modf(time, &dec);
	int j = (int)dec;
	glm::vec3 v1, v2, v3, v4;
	glm::vec3 shipPos;
	glm::vec3 shipDir;

	int n = NUM_CAMERA_POINTS;
	v1 = cameraKeyPoints[(j - 1) % n];
	v2 = cameraKeyPoints[(j) % n];
	v3 = cameraKeyPoints[(j + 1) % n];
	v4 = cameraKeyPoints[(j + 2) % n];

	shipPos = glm::catmullRom(v1, v2, v3, v4, s);
	shipDir = glm::normalize(glm::catmullRom(v1, v2, v3, v4, s + 0.001) - glm::catmullRom(v1, v2, v3, v4, s - 0.001));
	shipAngle = atan2f(shipDir.z, shipDir.x);

	glm::mat4 shipMatrix = glm::translate(shipPos) * glm::rotate(-shipAngle + glm::radians(90.0f), glm::vec3(0, 1, 0));
	return shipMatrix;
}

glm::mat4 createCameraMatrix()
{
	mouseChange = mouseChange * 0.01;

	float angleX, angleY;
	angleX = mouseChange.x;
	angleY = mouseChange.y;

	glm::quat obrX = glm::angleAxis(angleX, glm::vec3(0, 1, 0));
	glm::quat obrY = glm::angleAxis(angleY, glm::vec3(1, 0, 0));
	glm::quat obr = obrX * obrY;

	mouseChange = glm::vec2(0, 0);
	glutPassiveMotionFunc(mouse);

	rotation = obr * rotation;
	rotation = glm::normalize(rotation);

	odwrRotation = glm::inverse(rotation);
	cameraDir = odwrRotation * glm::vec3(0, 0, -1);
	glm::vec3 up = odwrRotation * glm::vec3(0, 1, 0);
	cameraSide = glm::cross(cameraDir, up);

	return Core::createViewMatrixQuat(cameraPos, rotation);
}

void setUpUniforms(GLuint program, glm::mat4 modelMatrix)
{
	glUniform3f(glGetUniformLocation(program, "lightPos1"), lightPos1.x, lightPos1.y, lightPos1.z);
	glUniform3f(glGetUniformLocation(program, "lightPos2"), lightPos2.x, lightPos2.y, lightPos2.z);
	glUniform3f(glGetUniformLocation(program, "cameraPos"), cameraPos.x, cameraPos.y, cameraPos.z);

	glm::mat4 transformation = perspectiveMatrix * cameraMatrix * modelMatrix;
	glUniformMatrix4fv(glGetUniformLocation(program, "modelViewProjectionMatrix"), 1, GL_FALSE, (float*)&transformation);
	glUniformMatrix4fv(glGetUniformLocation(program, "modelMatrix"), 1, GL_FALSE, (float*)&modelMatrix);
}

void setUpUniforms2(GLuint program, glm::mat4 modelMatrix, float alpha)
{
	glUniform3f(glGetUniformLocation(program, "lightPos1"), lightPos1.x, lightPos1.y, lightPos1.z);
	glUniform3f(glGetUniformLocation(program, "lightPos2"), lightPos2.x, lightPos2.y, lightPos2.z);
	glUniform3f(glGetUniformLocation(program, "cameraPos"), cameraPos.x, cameraPos.y, cameraPos.z);
	glUniform1f(glGetUniformLocation(program, "alpha"), alpha);

	glm::mat4 transformation = perspectiveMatrix * cameraMatrix * modelMatrix;
	glUniformMatrix4fv(glGetUniformLocation(program, "modelViewProjectionMatrix"), 1, GL_FALSE, (float*)&transformation);
	glUniformMatrix4fv(glGetUniformLocation(program, "modelMatrix"), 1, GL_FALSE, (float*)&modelMatrix);
}

void drawObject(obj::Model* model, glm::mat4 modelMatrix, glm::vec3 color)
{
	GLuint prog = program;
	glUseProgram(prog);

	setUpUniforms(program, modelMatrix);
	glUniform3f(glGetUniformLocation(program, "objectColor"), color.x, color.y, color.z);

	Core::DrawModel(model);

	glUseProgram(0);
}

void drawObjectAtmo(obj::Model* model, glm::mat4 modelMatrix, glm::vec3 color, float alpha)
{
	GLuint prog = programAtmo;
	glUseProgram(prog);

	setUpUniforms2(prog, modelMatrix, alpha);
	glUniform3f(glGetUniformLocation(program, "objectColor"), color.x, color.y, color.z);

	Core::DrawModel(model);

	glUseProgram(0);
}

void drawObjectTexture(obj::Model* model, glm::mat4 modelMatrix, GLuint textureId)
{
	GLuint program = programTex;

	glUseProgram(program);

	setUpUniforms(program, modelMatrix);
	Core::SetActiveTexture(textureId, "textureSampler", program, 0);

	Core::DrawModel(model);

	glUseProgram(0);
}

void drawObjectTextureN(obj::Model* model, glm::mat4 modelMatrix, GLuint textureId, GLuint normalmapId)
{
	GLuint program = programTexN;

	glUseProgram(program);

	setUpUniforms(program, modelMatrix);
	Core::SetActiveTexture(textureId, "textureSampler", program, 0);
	Core::SetActiveTexture(normalmapId, "normalSampler", program, 1);

	Core::DrawModel(model);

	glUseProgram(0);
}

void drawShip(obj::Model* model, glm::mat4 modelMatrix, GLuint textureId, GLuint normalmapId)
{
	GLuint program = programTexShip;

	glUseProgram(program);

	setUpUniforms(program, modelMatrix);
	Core::SetActiveTexture(textureId, "textureSampler", program, 0);
	Core::SetActiveTexture(normalmapId, "normalSampler", program, 1);

	Core::DrawModel(model);

	glUseProgram(0);
}

void drawSkybox(obj::Model* model)
{
	GLuint program = programSkybox;

	glUseProgram(program);

	setUpUniforms(program, glm::translate(glm::vec3(cameraPos)) * glm::scale(glm::vec3(100.0f)));
	glUniform3f(glGetUniformLocation(program, "cameraPos"), cameraPos.x, cameraPos.y, cameraPos.z);
	glm::mat4 modelMatrix = glm::translate(glm::vec3(cameraPos)) * glm::scale(glm::vec3(190.0f));

	glm::mat4 transformation = perspectiveMatrix * cameraMatrix * modelMatrix;
	glUniformMatrix4fv(glGetUniformLocation(program, "modelViewProjectionMatrix"), 1, GL_FALSE, (float*)&transformation);
	glUniformMatrix4fv(glGetUniformLocation(program, "modelMatrix"), 1, GL_FALSE, (float*)&modelMatrix);
	Core::SetActiveTexture(textureMilkyWay, "textureSampler", program, 0);

	Core::DrawModel(model);

	glUseProgram(0);
}

void drawParticles()
{
	for (int i = 0; i < MaxParticles; i++) {
		ParticlesContainer[i].life = -1.0f;
		ParticlesContainer[i].cameradistance = -1.0f;
	}

	double delta = t - lastTime;
	delta = 0.5;
	lastTime = t;
	int newparticles = (int)(1);

	std::cout << ParticlesCount << std::endl;
	for (int i = 0; i < newparticles; i++) {
		if (ParticlesCount < MaxParticles)
		{
			int particleIndex = FindUnusedParticle();
			ParticlesContainer[particleIndex].life = 2.0f;
			ParticlesContainer[particleIndex].pos = glm::vec3(0);

			float spread = 1.5f;
			glm::vec3 maindir = glm::vec3(0.0f, 0.0f, 1.0f);
			glm::vec3 randomdir = glm::vec3((rand() % 10 - 1.0f) / 50.0f, (rand() % 10 - 1.0f) / 100.0f, (rand() % 10 - 1.0f) / 50.0f);

			//ParticlesContainer[particleIndex].speed = maindir + randomdir * spread;
			ParticlesContainer[particleIndex].speed = maindir;

			//ParticlesContainer[particleIndex].size = (rand() % 10) / 20000.0f + 0.05f;
			ParticlesContainer[particleIndex].size = 0.05f;
			std::cout << "Tworze czastke!" << std::endl;
		}
	}

	// Simulate all particles
	glUseProgram(programParticles);
	ParticlesCount = 0;
	for (int i = 0; i < MaxParticles; i++) {

		Particle& p = ParticlesContainer[i]; // shortcut

		if (p.life > 0.0f) {
			p.life = p.life - delta;
			//std::cout << p.life << std::endl;
			if (p.life > 0.0f) {
				//p.speed += glm::vec3(0.0f, 0.0f, 1.0f);
				p.pos += p.speed;
				std::cout << "x: " << p.pos.x << ", y: " << p.pos.y << ", z: " << p.pos.z << std::endl;
				p.cameradistance = glm::length2(p.pos - cameraPos);

				glm::mat4 modelMatrix = glm::translate(glm::vec3(p.pos.x, p.pos.y, p.pos.z)) * glm::scale(glm::vec3(p.size));

				glm::mat4 transformation = perspectiveMatrix * cameraMatrix * modelMatrix;
				glUniformMatrix4fv(glGetUniformLocation(program, "modelViewProjectionMatrix"), 1, GL_FALSE, (float*)&transformation);
				
				Core::DrawModel(&sphereModel);
				ParticlesCount++;
				//printf("Rysuje");
			}
			else {
				// Particles that just died will be put at the end of the buffer in SortParticles();
				p.cameradistance = -1.0f;
			}
			//ParticlesCount++;
		}
		ParticlesContainer[i] = p;
	}
	//std::cout << ParticlesCount << std::endl;
	//SortParticles();

	glUseProgram(0);
}

void renderScene()
{
	cameraMatrix = createCameraMatrix();
	perspectiveMatrix = Core::createPerspectiveMatrix(0.1, 200, frustumScale);

	t = glutGet(GLUT_ELAPSED_TIME) / 3500.f;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.0f, 0.0f, 0.15f, 1.0f);

	drawSkybox(&sphereModel);

	glm::mat4 shipRotation = glm::mat4_cast(odwrRotation);
	glm::mat4 shipInitialTransformation = glm::translate(glm::vec3(0, -0.25f, 0)) * glm::rotate(glm::radians(180.0f), glm::vec3(0, 1, 0)) * glm::scale(glm::vec3(0.25f));
	glm::mat4 shipModelMatrix = glm::translate(cameraPos + cameraDir * 0.5f) * shipRotation * shipInitialTransformation;
	playerShipPos = cameraPos + cameraDir * 0.5f * glm::vec3(0, -0.25f, 0);

	// Player's ship
	drawShip(&player.getModel(), shipModelMatrix, player.getTextureId(), player.getNormalmapId());
	//drawParticles();
	
	// Bot ship	jego trzeba zmienić na te nadrzędną klasę, ale tam w niej nie ma pola tekstury więc dałąm na razie PlayerSpaceship
	glm::mat4 shipModelMatrixBot = getCatmullRomPath();
	drawShip(&botShip.getModel(), shipModelMatrixBot, botShip.getTextureId(), botShip.getNormalmapId());

	// Space Station
	drawShip(&spaceStationModel, glm::eulerAngleY(t / 8) * glm::translate(glm::vec3(-25, 0, 0)) * glm::eulerAngleY(t) * glm::translate(glm::vec3(-5, 0, 0)) * glm::scale(glm::vec3(0.3f)), textureShip, textureShipN);
	drawShip(&spaceStationModel, glm::translate(glm::vec3(-50, 0, 0)) * glm::eulerAngleY(t / 2) * glm::scale(glm::vec3(0.3f)), textureShip, textureShipN);

	// Earth
	drawObjectTextureN(&sphereModel, glm::eulerAngleY(t / 8) * glm::translate(glm::vec3(-25, 0, 0)) * glm::scale(glm::vec3(1.2f)), textureEarth, textureEarthN);
	
	// Moon
	drawObjectTextureN(&sphereModel, glm::eulerAngleY(t / 8) * glm::translate(glm::vec3(-25, 0, 0)) * glm::eulerAngleXZ(0.4f, 0.6f) * glm::eulerAngleY(t * 2) * glm::translate(glm::vec3(-2, 0, 0)) * glm::scale(glm::vec3(0.4f)), textureMoon, textureEarthN);

	// Mars
	drawObjectTextureN(&sphereModel, glm::eulerAngleXZ(-0.2f, 0.1f) * glm::eulerAngleY(t / 4) * glm::translate(glm::vec3(-17, 0, 0)) * glm::scale(glm::vec3(0.7f)), textureMars, textureEarthN);

	// Jupiter
	drawObjectTextureN(&sphereModel, glm::eulerAngleXZ(0.1f, -0.1f) * glm::eulerAngleY(t / 8) * glm::translate(glm::vec3(-60, 0, 0)) * glm::scale(glm::vec3(3.5f)), textureJupiter, textureEarthN);
	// Jupiter's moons
	drawObjectTextureN(&sphereModel, glm::eulerAngleXZ(0.1f, -0.1f) * glm::eulerAngleY(t / 8) * glm::translate(glm::vec3(-60, 0, 0)) * glm::eulerAngleXZ(-0.4f, 0.6f) * glm::eulerAngleY(t) * glm::translate(glm::vec3(-4.5, 0, 0)) * glm::scale(glm::vec3(0.35f)), textureMoon, textureEarthN);
	drawObjectTextureN(&sphereModel, glm::eulerAngleXZ(0.1f, -0.1f) * glm::eulerAngleY(t / 8) * glm::translate(glm::vec3(-60, 0, 0)) * glm::eulerAngleXZ(0.4f, 0.2f) * glm::eulerAngleY(t) * glm::translate(glm::vec3(-4.8, 0, 0)) * glm::scale(glm::vec3(0.5f)), textureVenus, textureEarthN);
	drawObjectTextureN(&sphereModel, glm::eulerAngleXZ(0.1f, -0.1f) * glm::eulerAngleY(t / 8) * glm::translate(glm::vec3(-60, 0, 0)) * glm::eulerAngleXZ(0.2f, -0.2f) * glm::eulerAngleY(t / 2) * glm::translate(glm::vec3(-4.2, 0, 0)) * glm::scale(glm::vec3(0.25f)), textureNeptune, textureEarthN);
	drawObjectTextureN(&sphereModel, glm::eulerAngleXZ(0.1f, -0.1f) * glm::eulerAngleY(t / 8) * glm::translate(glm::vec3(-60, 0, 0)) * glm::eulerAngleXZ(-0.1f, -0.2f) * glm::eulerAngleY(t / 2) * glm::translate(glm::vec3(-5.2, 0, 0)) * glm::scale(glm::vec3(0.65f)), textureMars, textureEarthN);

	// Saturn
	drawObjectTextureN(&sphereModel, glm::eulerAngleXZ(0.15f, 0.15f) * glm::eulerAngleY(t / 12) * glm::translate(glm::vec3(-78, 0, 0)) * glm::scale(glm::vec3(2.7f)), textureSaturn, textureEarthN);

	// Haumea
	drawObjectTextureN(&sphereModel, glm::eulerAngleXZ(-0.1f, -0.1f) * glm::eulerAngleY(t / 16) * glm::translate(glm::vec3(-135, 0, 0)) * glm::scale(glm::vec3(2.0f)), textureHaumea, textureEarthN);

	// Sun
	drawObjectTexture(&sphereModel, glm::translate(lightPos1) * glm::rotate((t / 32) * glm::radians(360.0f), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::scale(glm::vec3(7.0f)), textureSun);
	
	// Second source of light aka another star; feel free to name it STEFCIO
	drawObjectTexture(&sphereModel, glm::translate(lightPos2) * glm::rotate((t / 32) * glm::radians(360.0f), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::scale(glm::vec3(6.0f)), textureStar);

	// Asteroids
	for (int i = 0; i < NUM_ASTEROIDS; i++)
	{
		if (i % 2 == 0)
		{
			drawObjectTextureN(&sphereModel, glm::eulerAngleXZ(-0.2f, 0.1f) * glm::eulerAngleY(t / 100) * glm::translate(asteroidsPositions[i]) * glm::scale(glm::vec3(0.5f)), textureAsteroid, textureAsteroidN);
		}
		else 
		{
			drawObjectTextureN(&sphereModel, glm::eulerAngleXZ(+0.2f, -0.1f) * glm::eulerAngleY(t / 100) * glm::translate(asteroidsPositions[i]) * glm::scale(glm::vec3(0.25f)), textureAsteroid, textureAsteroidN);
			//nie wiem czy nie za ma�e te asteroidki ale wydaje mi si� �e s� do�� proporcjonalne do tych planet
		}
	}
	
	// Cokolwiek rysujecie, te atmosfery zawsze muszą być na samym końcu
	// Earth's atmosphere
	drawObjectAtmo(&sphereModel, glm::eulerAngleY(t / 8) * glm::translate(glm::vec3(-25, 0, 0)) * glm::scale(glm::vec3(1.4f)), glm::vec3(1.0, 1.0, 1.0), 0.8);

	drawOnScreen("fuel: ", -0.98, -0.9);
	int f = player.getFuel();
	std::string fuelString = std::to_string(f);
	drawOnScreen(fuelString, -0.8, -0.9); //wysiwetlanie ilosci paliwa na ekranie
	player.shipPosition();
	int timeFuel = round(t);
	if ((timeFuel % 1 == 0) && (whichSec2 != timeFuel))
	{
		whichSec2 = timeFuel;
		player.fuelUpdate();
	}
	if (checkStation())
	{
		drawOnScreen("Press 'r' to refuel", 0.5, -0.9);
	}

	glutSwapBuffers();
}

void reset()
{
	cameraPos = glm::vec3(-30, 0, 0);
	player.setFuel(10000);
	player.setHp(100);
}

void init()
{
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	// shaders
	program = shaderLoader.CreateProgram("shaders/shader_light.vert", "shaders/shader_light.frag");
	programTex = shaderLoader.CreateProgram("shaders/shader_tex.vert", "shaders/shader_tex.frag");
	programTexN = shaderLoader.CreateProgram("shaders/shader_texMultipleLightsN.vert", "shaders/shader_texMultipleLightsN.frag");
	programTexShip = shaderLoader.CreateProgram("shaders/shader_texShip.vert", "shaders/shader_texShip.frag");
	programAtmo = shaderLoader.CreateProgram("shaders/shader_atmosphere.vert", "shaders/shader_atmosphere.frag");
	programSkybox = shaderLoader.CreateProgram("shaders/shader_skybox.vert", "shaders/shader_skybox.frag");
	programParticles = shaderLoader.CreateProgram("shaders/shader_particle.vert", "shaders/shader_particle.frag");

	// textures
	textureEarth = Core::LoadTexture("textures/earth2.png");
	textureSun = Core::LoadTexture("textures/sun.png");		
	textureMoon = Core::LoadTexture("textures/moon.png");
	textureMars = Core::LoadTexture("textures/mars.png");
	textureJupiter = Core::LoadTexture("textures/jupiter.png");
	textureSaturn = Core::LoadTexture("textures/saturn.png");
	textureHaumea = Core::LoadTexture("textures/haumea.png");
	textureVenus = Core::LoadTexture("textures/venus.png");
	textureNeptune = Core::LoadTexture("textures/neptune.png");
	textureMilkyWay = Core::LoadTexture("textures/s-milky-way.png");
	textureShip = Core::LoadTexture("textures/spaceship.png");
	textureAsteroid = Core::LoadTexture("textures/asteroid.png");
	textureStar = Core::LoadTexture("textures/lava_star.png");

	// normals
	textureShipN = Core::LoadTexture("textures/spaceship_normals.png");
	textureEarthN = Core::LoadTexture("textures/earth2_normals.png");
    textureAsteroidN = Core::LoadTexture("textures/asteroid_normals.png");

	// models
	sphereModel = obj::loadModelFromFile("models/sphere.obj");
	shipModel = obj::loadModelFromFile("models/spaceship.obj");
	spaceStationModel = obj::loadModelFromFile("models/spaceStattion.obj");

	// asteroids positions
	float R = 150.0;
	for (int i = 0; i < NUM_ASTEROIDS; i++)
	{
		asteroidsPositions[i] = glm::ballRand(R);
	}
	
	for (int i = 0; i < NUM_CAMERA_POINTS; i++)
	{
		cameraKeyPoints[i] = glm::sphericalRand(40.0);
		//std::cout << cameraKeyPoints[i].x << " " << cameraKeyPoints[i].y << std::endl;
	}
	appLoadingTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
}

void shutdown()
{
	shaderLoader.DeleteProgram(program);
	shaderLoader.DeleteProgram(programTexN);
}

void idle()
{
	glutPostRedisplay();
}

void onReshape(int width, int height)
{
	glViewport(0, 0, width, height);
	frustumScale = (float)width / (float)height;
}

int main(int argc, char ** argv)
{
	glutInit(&argc, argv);
	glutSetOption(GLUT_MULTISAMPLE, 8);										// MSAA x8
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA | GLUT_MULTISAMPLE);
	glEnable(GL_MULTISAMPLE);
	glutInitWindowPosition(100, 10);
	glutInitWindowSize(800, 800);
	glutCreateWindow("Spaceship simulator");
	glewInit();

	init();
	glutKeyboardFunc(keyboard);
	glutDisplayFunc(renderScene);
	glutIdleFunc(idle);
	glutReshapeFunc(onReshape);

	glutMainLoop();

	shutdown();

	return 0;
}
