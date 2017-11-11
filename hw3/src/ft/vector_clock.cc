#include "ft/vector_clock.h"
#include "ft/p2p_comm.h"
#include "utils/log.h"
#include <fstream>
extern ft::P2PComm _comm;

const std::string ft::VectorClock::TAG = "VectorClock";
void ft::VectorClock::exchangeTimeId (int preId, int newerId) {
	for(int i=0; i<3; i++){
		if (pid_List[i] == preId){
			pid_List[i] = newerId;
			break;
		}
	}
    saveToFile();
}

void ft::VectorClock::init() {
    std::ifstream file(dataFileName);
    if (file.good()) {
        fbsd::Log::d(TAG, "read file clock.txt");
        readFromFile();
    } else {
        Time_P1 = {0,0,0};
        Time_P2 = {0,0,0};
        Time_P3 = {0,0,0};
        pid_List = {-1,-1,-1};
    }
}

std::vector<int>* ft::VectorClock::getPsTimeId (int pid) {
	int index=0;

	for(index=0;index<3;index++){
		if (pid_List[index] == pid)
			break;
		if (pid_List[index] == -1){	
			pid_List[index] = pid;
			break;
		}
	}

	if (index == 0)
		return &Time_P1;
	else if (index == 1)
		return &Time_P2;
	else if (index == 2)
		return &Time_P3;

    return NULL; // never happends
}

std::vector<int> ft::VectorClock::requestTime(int pid) {

	std::vector<int>* requestorPs = getPsTimeId(pid);

	int index = 0;
	for(int i=0; i<3; i++){
		if(pid_List[i] == pid)
			break;
		index++;
	}

	requestorPs->at(index)++;

    saveToFile();
	return *requestorPs;
}

bool ft::VectorClock::comparison(std::vector<int> rec_time, int pid) {

	int same=0;
	int little=0;
	int large=0;

	std::vector<int>* server_time = getPsTimeId(pid);
	int index = 0;

    fbsd::Log::d(TAG, fbsd::Log::string_format(
                "compare (%s) vs (%s)",
                toString(rec_time).c_str(),
                toString(*server_time).c_str()));
	for(index=0; index<3; index++){
		if(pid_List[index] == pid)
			break;
	}


	for(int i=0 ; i<3 ; i++){
		if(rec_time[i] == server_time->at(i))
			same++;
		else if(rec_time[i] < server_time->at(i))
			little++;
		else if(rec_time[i] > server_time->at(i))
			large++;
	}

	// after comparison the server's Time will be incremented automatically.
	for(int i=0; i<3; i++){
		if(i==index){
			(server_time->at(i))++;//(server_time->at(index))++;
		}
		else{
			if(rec_time[i] > server_time->at(i))
				server_time->at(i) = rec_time[i];			    		}	
	}


    bool result;
	if (same == 3)
		result = false;
	else if (large >= 1 && little==0)
		result = true;
	else 
		result = false;
    if (result) 
        fbsd::Log::d(TAG, "My time is earlier than requester");
    else
        fbsd::Log::d(TAG, "Requester time is earlier than My Time");
    saveToFile();
    return result;
}	

std::string ft::VectorClock::toString(std::vector<int> time) {
    std::string str;
    str = std::to_string(time[0]);
    str = str + " " + std::to_string(time[1]);
    str = str + " " + std::to_string(time[2]);
    return str;
}
                
std::vector<int> ft::VectorClock::toVector(std::string time) {
    std::vector<int> vtime;
    if (time.empty()) return vtime;

    int idx = time.find(" ");
    std::string first, remain;
    first = time.substr(0, idx);
    remain = time.substr(idx + 1, time.size());

    idx = remain.find(" ");
    std::string second, third;
    second = remain.substr(0, idx);
    third = remain.substr(idx + 1, remain.size());

    vtime.push_back(std::stoi(first));

    vtime.push_back(std::stoi(second));
    vtime.push_back(std::stoi(third));

    return vtime;
}

void ft::VectorClock::notifyNewNetMaster(int dead_id, int new_id) {
	exchangeTimeId(dead_id, new_id);
}

void ft::VectorClock::saveToFile() {
    std::ofstream dataFile;
    dataFile.open(dataFileName);

    dataFile << toString(Time_P1) << std::endl;
    dataFile << toString(Time_P2) << std::endl;
    dataFile << toString(Time_P2) << std::endl;
    dataFile << toString(pid_List) << std::endl;
    dataFile.close();
}

void ft::VectorClock::readFromFile() {
    std::string line;
    std::ifstream dataFile(dataFileName);
    if (dataFile.is_open()) {
        getline(dataFile, line);
        Time_P1 = toVector(line);
        getline(dataFile, line);
        Time_P2 = toVector(line);
        getline(dataFile, line);
        Time_P3 = toVector(line);
        getline(dataFile, line);
        pid_List = toVector(line);
    }
}

std::string ft::VectorClock::getClock() {
	std::vector<int>* requestorPs = getPsTimeId(_comm.getP2PServer()->getID());
    return toString(*requestorPs);
}

void ft::VectorClock::setClock(int pid, std::string clock) {
    if (clock.empty()) return;
	std::vector<int> *requestorPs = getPsTimeId(pid);
    std::vector<int> tmp = toVector(clock);
    (*requestorPs) = tmp;




    std::vector<int> *mytime = getPsTimeId(_comm.getP2PServer()->getID());

    int index = 0;
    for (; index < 3; index++) {
        if (pid_List[index] == pid)
            break;
    }
    (*mytime)[index] = requestorPs->at(index);

    saveToFile();
}
