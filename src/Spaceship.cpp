#include "Spaceship.h"
#include "Texture.h"

Spaceship::Spaceship() {
	hp = 100;
	fuel = 10000;
	model = obj::loadModelFromFile("models/spaceship.obj");
}

void Spaceship::setModel(obj::Model model_)
{
	model = model_;
}

obj::Model Spaceship::getModel() const
{
	return model;
}

void Spaceship::setFuel(float fuel_)
{
	fuel = fuel_;
}

float Spaceship::getFuel() const
{
	return fuel;
}

void Spaceship::setHp(int hp_)
{
	hp = hp_;
}

int Spaceship::getHp() const
{
	return hp;
}
