#include "Player.h"

Player::Player()
{
	angleSpeed = 0.1f;
	moveSpeed = 0.2f;
	maxFuel = 10000;
	lastSec = 0;
}

void Player::fuelUpdate()
{
	float heat = 141.9 * 1000000, speed = 2.0, mass = 45000000, efficiency = 1, newFuel;

	if (hp != 0)
	{
		newFuel = (speed * mass) / (efficiency * heat);
		fuel = fuel - newFuel;
	}
}

void Player::shipPosition(glm::vec3 cameraPos, float t)		// t - time
{
	float distanceToSun = cameraPos.x * cameraPos.x + cameraPos.y * cameraPos.y + cameraPos.z * cameraPos.z;
	float distanceToStar = (cameraPos.x + 100) * (cameraPos.x + 100) + cameraPos.y * cameraPos.y + cameraPos.z * cameraPos.z;
	if (sqrt(distanceToSun) <= 15 || sqrt(distanceToStar) <= 15)
	{
		int timeForFuel = round(t);
		if (lastSec != timeForFuel)
		{
			if (hp > 0)
			{
				hp = hp - 5;
			}
			
			lastSec = timeForFuel;
		}
		
		if (hp <= 0)
		{
			std::string x = std::to_string(0);
			writeOnScreen("GAMEOVER HP = 0", -0.2, 0.9);
			writeOnScreen("Press R to restart or Q to close the game.", -0.5, 0.85);
			writeOnScreen("HP: ", -0.5, -0.9);
			writeOnScreen(x, -0.42, -0.9);
		}
		else
		{
			std::string x = std::to_string(hp);
			writeOnScreen("THE SHIP IS TOO CLOSE TO THE SUN!", -0.45, 0.9);
			writeOnScreen("HP: ", -0.5, -0.9);
			writeOnScreen(x, -0.42, -0.9);
		}
	}
	else if (sqrt(distanceToSun) <= 25 || sqrt(distanceToStar) <= 25)
	{
		int timeForFuel = round(t);

		if (lastSec != timeForFuel)
		{
			if (hp > 0)
			{
				hp = hp - 1;
			}
			lastSec = timeForFuel;
		}

		if (hp <= 0)
		{
			writeOnScreen("GAMEOVER HP = 0", -0.2, 0.9);
			writeOnScreen("Press R to restart or Q to close the game.", -0.4, 0.8);
			writeOnScreen("HP: ", -0.5, -0.9);
			writeOnScreen(0, -0.42, -0.9);
		}
		else
		{
			writeOnScreen("THE SHIP IS TOO CLOSE TO THE SUN!", -0.45, 0.9);
			writeOnScreen("HP: ", -0.5, -0.9);
			writeOnScreen(hp, -0.42, -0.9);
		}
	}
	else
	{
		if (hp == 0)
		{
			writeOnScreen("HP: ", -0.5, -0.9);
			writeOnScreen(0, -0.42, -0.9);
		}
		else
		{
			writeOnScreen("HP: ", -0.5, -0.9);
			writeOnScreen(hp, -0.42, -0.9);
		}
	}
}

bool Player::checkStation(glm::vec3 cameraPos) const
{
	float distanceToStation = (cameraPos.x + 50) * (cameraPos.x + 50) + cameraPos.y * cameraPos.y + cameraPos.z * cameraPos.z;
	if (sqrt(distanceToStation) <= 4)
	{
		return true;
	}
	
	return false;
}

void Player::setAngleSpeed(float angleSpeed_)
	{
		angleSpeed = angleSpeed_;
	}
	
float Player::getAngleSpeed() const
	{
		return angleSpeed;
	}

void Player::setMoveSpeed(float moveSpeed_)
	{
		moveSpeed = moveSpeed_;
	}

float Player::getMoveSpeed() const
	{
		return moveSpeed;
	}

void Player::refuel()
	{
		fuel = maxFuel;
	}

int Player::getMaxFuel() const
{
	return maxFuel;
}
