/*!
 * \file failure_detector.h
 * \class FailtureDetector
 *
 * \brief This class is responsible for the process failure.
 */
#ifndef FAILURE_DETECTOR_H
#define FAILURE_DETECTOR_H
#include <thread>
#include "ft/p2p_comm.h"
#include "thread.h"
namespace ft {
    class FailureDetector : public fbsd::Thread {
        private:
            void run();
            void split(const string& str, string& left, string& right) {
                int idx = str.find(":");
                string l, r;
                left = str.substr(0, idx);
                right = str.substr(idx + 1, str.size());
            }

            void takeOverMaster();
            void masterHeartbeat();
            void slaveWatchdog();
        public:
            static const string TAG;
            FailureDetector();
            std::thread runTakingOverMaster();
            std::thread runMasterHeartbeat();
            std::thread runSlaveWatchdog();

            bool _masterAlive;
    };
};

#endif // FAILURE_DETECTOR_H
