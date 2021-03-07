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

#include "Spaceship.h"
#include "Player.h"
#include "HUD.h"

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
glm::vec3 cameraDir;
glm::vec3 cameraSide;

glm::mat4 cameraMatrix, perspectiveMatrix;

glm::vec3 lightPos1 = glm::vec3(0, 0, 0);
glm::vec3 lightPos2 = glm::vec3(-100, 0, 0);

glm::vec3 playerShipPos;

glm::quat rotation = glm::quat(1, 0, 0, 0);

glm::vec2 mouseLast = glm::vec2(300, 300);
glm::vec2 mouseChange;

glm::quat odwrRotation;

void reset();

Player player;
//Spaceship botShip;

void keyboard(unsigned char key, int x, int y)
{
	int checkHp = player.getHp();
	float checkFuel = player.getFuel();

	float angleSpeed = player.getAngleSpeed(), moveSpeed = player.getMoveSpeed();
	if (checkHp <= 0 || checkFuel <= 0)
	{
		player.setAngleSpeed(0);
		player.setMoveSpeed(0);
		if (key == 'r')
		{
			reset();
			player.setAngleSpeed(0.1f);
			player.setMoveSpeed(0.2f);
		}
		if (key == 'q')
		{
			exit(EXIT_SUCCESS);
		}
	}
	if (player.checkStation(cameraPos))
	{
		if (key == 'r')
		{
			player.refuel();
		}
	}
	
	switch (key)
	{
	case 'z': rotation = glm::angleAxis(angleSpeed, glm::vec3(0, 0, -1)) * rotation; break;
	case 'x': rotation = glm::angleAxis(angleSpeed, glm::vec3(0, 0, 1)) * rotation; break;
	case 'w': cameraPos += cameraDir * moveSpeed; player.fuelUpdate(); break;
	case 's': cameraPos -= cameraDir * moveSpeed; player.fuelUpdate(); break;
	case 'd': cameraPos += cameraSide * moveSpeed; player.fuelUpdate(); break;
	case 'a': cameraPos -= cameraSide * moveSpeed; player.fuelUpdate(); break;
	}
}

void mouse(int x, int y)
{
	mouseChange = glm::vec2(x - mouseLast.x, y - mouseLast.y);
	mouseLast = glm::vec2(x, y);
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
	//playerShipPos = cameraPos + cameraDir * 0.5f * glm::vec3(0, -0.25f, 0);

	// Player's ship
	drawShip(&player.getModel(), shipModelMatrix, textureShip, textureShipN);

	// Space Station
	drawShip(&spaceStationModel, glm::eulerAngleY(t / 8) * glm::translate(glm::vec3(-25, 0, 0)) * glm::eulerAngleY(t) * glm::translate(glm::vec3(-5, 0, 0)) * glm::scale(glm::vec3(0.3f)), textureShip, textureShipN);

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
	
	// Second source of light - another star
	drawObjectTexture(&sphereModel, glm::translate(lightPos2) * glm::rotate((t / 32) * glm::radians(360.0f), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::scale(glm::vec3(6.0f)), textureStar);

	// Earth's atmosphere
	//drawObjectAtmo(&sphereModel, glm::eulerAngleY(t / 8) * glm::translate(glm::vec3(-25, 0, 0)) * glm::scale(glm::vec3(1.3f)), glm::vec3(1.0, 1.0, 1.0), 0.8);

	// HUD
	writeOnScreen("Fuel: ", -0.98, -0.9);
	float f = player.getFuel() / player.getMaxFuel();
	std::string fuel = std::to_string(f * 100) + "%";
	writeOnScreen(fuel, -0.85, -0.9);
	
	player.shipPosition(cameraPos, t);
	
	int timeFuel = round(t);
	if ((timeFuel % 1 == 0) && (whichSec2 != timeFuel))
	{
		whichSec2 = timeFuel;
		player.fuelUpdate();
	}
	if (player.checkStation(cameraPos))
	{
		writeOnScreen("Press 'r' to refuel", 0.5, -0.9);
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
