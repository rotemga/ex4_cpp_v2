
#include "FuncForMain.h"
#include <dlfcn.h>
#include "AlgorithmRegistration.h"
#include "AlgorithmRegistrar.h"
#include "Score.h"

typedef AbstractAlgorithm* (*algoCreator)();



int main(int argc, char* argv[]) {
	string config_path, house_path, algo_path, score_path;
	map<string, string> houses_mapName, algo_mapName, config_mapName, score_mapName;
	vector <fs::path> fileName_currDir;
	vector <House*> houses;
	vector <AbstractAlgorithm*> algos;
	map<string, string> configBeforeValid;
	map<string, int> config;
	map<string, string> ErrorMSGHouse;
	vector<unique_ptr<AbstractAlgorithm>> Algos;
	map<string, string> ErrorMSGAlgo;
	string ErrorMSGScore = "";
	bool makeVideo = false;
	int number_threads;
	vector<void*> handlersVector;
	void* scoreHandler;
	scoreFunc scorefunction = nullptr;
	bool house_exist, config_exist, algo_exist, score_exist;
	checkArguments(argc, argv, config_path, algo_path, house_path, score_path, number_threads, makeVideo);
	if (makeVideo && (number_threads > 1)) {
		cout << "Cannot create video with more than one thread" << endl;
		return 1;
	}
	house_exist = updateFilesFromDirectory(houses_mapName, ".house", house_path);
	config_exist = updateFilesFromDirectory(config_mapName, ".ini", config_path);
	algo_exist = updateFilesFromDirectory(algo_mapName, "_.so", algo_path);
	if (score_path != "") {
		score_exist = updateFilesFromDirectory(score_mapName, "score_formula.so", score_path);
		if (score_exist) {
			for (auto scoreFilesNames : score_mapName) {
				const char* tmpname = scoreFilesNames.first.c_str();
				scoreHandler = dlopen(tmpname, RTLD_NOW);
				if (scoreHandler == nullptr) {
					ErrorMSGScore = "score_formula.so exists in '" + score_path + "' but cannot be opened or is not a valid .so";
					break;
				}
				*(void **)(&scorefunction) = dlsym(scoreHandler, "calc_score");
				if (scorefunction == nullptr) {
					ErrorMSGScore = "score_formula.so is a valid.so but it does not have a valid score formula";
					break;
				}
			}
		}
	}
	else {
		scorefunction = nullptr;
		score_exist = true;
	}

	if (!house_exist || !algo_exist || !config_exist || !score_exist) {
		Usage(house_path, config_path, algo_path, score_path, house_exist, config_exist, algo_exist, score_exist);
		return 1;

	}
	if (ErrorMSGScore != "") {
		cout << ErrorMSGScore << endl;
		return 1;
	}

	int numberOfValidHouses = checkHouses(houses_mapName, &houses, &ErrorMSGHouse);
	if (!checkConfig(config_mapName.begin()->first, &configBeforeValid, config_path)) {
		return 1;
	}
	convertToMapInt(configBeforeValid, &config);
	for (auto algoSoFilesNames : algo_mapName) {
		int resNum = AlgorithmRegistrar::getInstance().loadAlgorithm(algoSoFilesNames.first, algoSoFilesNames.first);
		if (resNum == -1) {
			ErrorMSGAlgo[algoSoFilesNames.first] = "file cannot be loaded or is not a valid .so";
			continue;
		}
		else if (resNum == -2) {
			ErrorMSGAlgo[algoSoFilesNames.first] = "valid .so but no algorithm was registered after loading it";
			continue;
		}
	}

	Algos = AlgorithmRegistrar::getInstance().getAlgorithms();
	list<string> algorithmNames = AlgorithmRegistrar::getInstance().getAlgorithmNames();
	for (vector<unique_ptr<AbstractAlgorithm>>::size_type k = 0; k != Algos.size(); k++) {
		auto l_front = algorithmNames.begin();
		std::advance(l_front, k);
		string algoName = *l_front;
		if (ErrorMSGAlgo.find(algoName) == ErrorMSGAlgo.end())
			algos.push_back(Algos[k].release());
	}
	bool scoreError = true;
	int numberOfValidAlgo = algos.size();
	if (numberOfValidAlgo && numberOfValidHouses) {
		vector<string> algoOnlyNames, houseonlyNames;
		createVcetorFromMapValues(algoOnlyNames, algo_mapName, ErrorMSGAlgo);
		createVcetorFromMapValues(houseonlyNames, houses_mapName, ErrorMSGHouse);
		Simulator sim(houses, algos, config, algoOnlyNames, houseonlyNames, makeVideo);
		scoreError = sim.runSimulator(algoOnlyNames, houseonlyNames, number_threads, scorefunction);
	}
	if (ErrorMSGHouse.size() > 0 || ErrorMSGAlgo.size() > 0) {
		cout << "\n" << "Errors:" << endl;
		if (houses.size() == 0)
			cout << "All house files in target folder " << "'" << house_path << "'" << " cannot be opened or are invalid:" << endl;
		PrintErrors(ErrorMSGHouse, houses_mapName, "house");
		if (algos.size() == 0)
			cout << "All algorithm files in target folder " << "'" << algo_path << "'" << " cannot be opened or are invalid:" << endl;
		PrintErrors(ErrorMSGAlgo, algo_mapName, "so");
	}
	if (!scoreError)
		cout << "Score formula could not calculate some scores, see -1 in the results table" << endl;
	for (auto it = houses.begin(); it != houses.end(); it++)
		delete *it;


	AlgorithmRegistrar::getInstance().closeAlgorithms();

	houses.clear();
	Algos.clear();

	return 0;
}