#include "Robot.h"


Robot::Robot(House *house, AbstractAlgorithm *algo, Point* docking,
	Battery* battery) :
	house(house), algo(algo), position(docking), battery(battery) {
	sensor = new OurSensor(house, docking);
	score.setSumDirtInHouse(house->sumDirt());
	score.setPosition(10);
	canRun = true;
	brokedDown = false;
	crashedWall = false;
	algo->setSensor(*sensor);
	prevStep = Direction::Stay;
}

Robot::~Robot() {
	delete sensor;
	delete battery;
	delete(house);
	delete(position);
}

void Robot::runRobot() {
	if (canRun) {
		if (updateHouse(*position))
			score.setDirtCollected(score.getDirtCollected() + 1);
		updateBattery(*position, *battery);
		if (battery->isEmpry()) {
			canRun = false;
			brokedDown = true;
			score.setPosition(10);
			return;
		}
		//	algo->setSensor(*sensor);

		Direction direction = algo->step(prevStep); //should check if direction is legal
		prevStep = direction;
		//cout << "Direction step is: " << DirectionToString(direction) << endl;
		position->move(direction);
		sensor->setPoint(position);

		if (crashedToWall(*position)) {
			canRun = false;
			crashedWall = true;
			brokedDown = true;
			return;
		}
		//house->output();
		sensor->getInfoFromPoint(house, position);
		score.setNumSteps(score.getNumSteps() + 1);
	}
}

bool Robot::updateHouse(const Point& point) {
	if (house->findPointState(point) == DIRTY) {
		house->cleanDirtyPoint(point);
		return true;
	}
	return false;
}
void Robot::updateBattery(Point& point, Battery& battery) {
	switch (house->findPointState(point)) {
	case DOCKING:
		battery.recharge();
		break;
	default:
		battery.reduce();
	}
}

void Robot::updatePositionDirt(Point& point) {
	int dirt = house->currentValue(point.getX(), point.getY());
	if (dirt > 0) {
		house->cleanDirtyPoint(point);
	}
}

bool Robot::isHouseClean() const {
	return house->isClean();
}
bool Robot::areWeInDocking() const {
	return house->findPointState(*position) == DOCKING;
}

int Robot::DirtCollected() {
	return (score.getSumDirtInHouse() - house->sumDirt());
}
int Robot::sumDirtInHouse() {
	return(house->sumDirt());
}

void Robot::printHouse() {
	string* matrix = house->getMatrix();
	cout << "Printing house from instance into standard output" << endl;
	for (int i = 0; i < house->getR(); ++i)
	{
		for (int j = 0; j < house->getC(); ++j)
		{
			cout << matrix[i][j];
		}
		cout << endl;
	}
	cout << endl;
}



string Robot::DirectionToString(Direction d) {
	switch (d) {
	case Direction::East:
		return "East";

	case Direction::West:
		return "West";

	case Direction::South:
		return "South";

	case Direction::North:
		return "North";

	case Direction::Stay:
		return "Stay";

	default: return "Stay";
	}
	return "Stay";
}

//static int counter = 0;

void createDirectoryIfNotExists(const string& dir_path){
	boost::filesystem::path dir(dir_path);
	if (!boost::filesystem::exists(boost::filesystem::status(dir))){
		if (boost::filesystem::create_directories(dir)) {
		}
	}
}

void Robot::montage(const string& algoName, const string& houseName,int counter) const
{
	size_t C = house->getC();
	size_t R = house->getR();
	size_t x = position->getX(), y = position->getY();
	vector<string> tiles;
	for (size_t row = 0; row < R; ++row)
	{
		for (size_t col = 0; col < C; ++col)
		{

			if (row == x && col == y){
				tiles.push_back("R");
			}

			else if (house->getMatrix()[row][col] == ' ')
				tiles.push_back("0");
			else
				tiles.push_back(string() + house->getMatrix()[row][col]);
		}
	}
	string imagesDirPath = "simulations/" + algoName + "_" + houseName;
	createDirectoryIfNotExists(imagesDirPath);
	string counterStr = to_string(counter++);
	string composedImage = imagesDirPath + "/image" + string(5 - counterStr.length(), '0') + counterStr + ".jpg";
	Montage::compose(tiles, C, R, composedImage);
}


void Robot::toVideo(const string& algoName, const string& houseName) const
{
	//.... To Encode images into video:
	string simulationDir = "simulations/" + algoName + "_" + houseName + "/";
	string imagesExpression = simulationDir + "image%5d.jpg";
	Encoder::encode(imagesExpression, algoName + "_" + houseName + ".mpg");
	//don't forget to remove the images after the video is created...
	if (boost::filesystem::exists("simulations/" + algoName + "_" + houseName))
	{

		boost::filesystem::remove_all(simulationDir);
	}

	//if (boost::filesystem::exists("simulations"))
	//{

	//	boost::filesystem::remove_all("simulations");
	//}


}