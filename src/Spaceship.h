#pragma once
#include "objload.h"

class Spaceship
{
protected:
	float fuel;
	int hp;
	obj::Model model;


public:
	Spaceship();

	void setModel(obj::Model model_);
	obj::Model getModel() const;

	void setFuel(float fuel_);
	float getFuel() const;

	void setHp(int hp_);
	int getHp() const;
};

