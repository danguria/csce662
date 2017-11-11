#ifndef ELECTION_H_
#define ELECTION_H_
#include "ft/p2p_comm.h"
#include "ft/p2p_client.h"
#include "thread.h"
#include "utils/log.h"


namespace ft {
    class Election : public fbsd::Thread {
        private:
            int _vote;
            P2PClient* getNextClient(int id);
        public:
            Election();
            void forward();
            int vote(int id);
            void run() { forward(); }
            void init();

            bool _participated;
            static const string TAG;
    };
}
#endif
