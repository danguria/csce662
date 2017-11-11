/*!
 * \file param.h
 * \class Participant
 *
 * \brief Internal data structure for the participant
 */
#ifndef PARTICIPANT_H
#define PARTICIPANT_H

#include <iostream>
#include <string>
#include <ctime>
using namespace std;
namespace fbsd {
    class Participant {
        private:
            string _name;
            time_t _joinedTime;

        public:
            Participant(string name) : _name(name), _joinedTime(time(0)) {}
            Participant(string name, time_t time)
                : _name(name), _joinedTime(time) {}

            string GetName() { return _name; }
            time_t GetJoinedTime() { return _joinedTime; }
    };
}
#endif  // PARTICIPANT_H
