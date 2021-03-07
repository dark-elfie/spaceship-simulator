#pragma once
#include "Spaceship.h"
#include "HUD.h"

class Player : public Spaceship
{
protected:
	float angleSpeed;
	float moveSpeed;
	int maxFuel;
	int lastSec;	// latest second HP was subtracted

public:
	Player();

	void fuelUpdate();

	void shipPosition(glm::vec3 cameraPos, float t);

	bool checkStation(glm::vec3 cameraPos) const;

	void setAngleSpeed(float angleSpeed_);
	float getAngleSpeed() const;

	void setMoveSpeed(float moveSpeed_);
	float getMoveSpeed() const;

	void refuel();

	int getMaxFuel() const;
};
