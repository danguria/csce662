#ifndef VECTOR_CLCOK_H
#define VECTOR_CLCOK_H
#include <iostream>
#include <string>
#include <vector>
using namespace std;

namespace ft {
	class VectorClock {
		private:
			std::vector<int> Time_P1;
			std::vector<int> Time_P2;
			std::vector<int> Time_P3;
			std::vector<int> pid_List;

			std::string dataFileName;
		public:
            static const std::string TAG;
			VectorClock() {
                dataFileName = "clock.txt";
                init();
			}
			void init();
			std::vector<int> requestTime(int id);
			bool comparison(std::vector<int> rec_time, int pid); 

			std::vector<int>* getPsTimeId (int pid);
			void exchangeTimeId (int preId, int newerId);
			void notifyNewNetMaster(int dead_id, int new_id);

            std::string getClock();
            void setClock(int pid, std::string clock);
			std::string toString(std::vector<int> time);
			std::vector<int> toVector(std::string time);
			void saveToFile();
			void readFromFile();

	};
}

#endif
