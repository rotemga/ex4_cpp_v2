#ifndef ROBOT_H_
#define ROBOT_H_

#include "House.h"
#include "AbstractAlgorithm.h"
#include "AbstractSensor.h"
#include "OurSensor.h"
#include "Battery.h"
#include "Score.h"
#include <boost/filesystem.hpp>
#include "Montage.h"
#include "Encoder.h"

class Robot
{
	House *house;
	AbstractAlgorithm *algo;
	OurSensor *sensor;
	Point* position;
	Battery* battery;
	Score score;
	bool canRun;
	bool crashedWall;
	bool brokedDown;
	Direction prevStep;

public:

	Robot(House *house, AbstractAlgorithm *algo, Point* docking, Battery* battery);
	~Robot();

	void updateBattery(Point& point, Battery& battery);
	bool updateHouse(const Point& point);
	void updatePositionDirt(Point& point);
	void runRobot();
	bool isHouseClean() const;
	bool areWeInDocking() const;
	int DirtCollected();
	int sumDirtInHouse();
	void printHouse();
	void montage(const string& algoName, const string& houseName, int couner) const;
	void toVideo(const string& algoName, const string& houseName) const;
	//void createDirectoryIfNotExists(const string& dir_path);

	bool isCanRun() const {
		return canRun;
	}
	bool isBrokedDown() const {
		return brokedDown;
	}
	bool isCrashedWall() const {
		return crashedWall;
	}
	void setCanRun(bool canRun) {
		this->canRun = canRun;
	}

	const Score& getScore() const {
		return score;
	}

	void setScore(const Score& score) {
		this->score = score;
	}
	bool crashedToWall(const Point& point) {
		return house->findPointState(point) == WALL;
	}
	string DirectionToString(Direction d);
};
#endif /*ROBOT_H_*/