#ifndef THREAD_H_
#define THREAD_H_
#include <thread>

namespace fbsd {
class Thread {
    public:
        Thread() : _finish(false) {}
        std::thread runThread() { return std::thread([=] { run(); }); }
        bool finished() { return _finish; }
        void finish() { _finish = true; }
        void restart() { _finish = false; }
        virtual void run() = 0;

    private:
        bool _finish;
};
}


#endif
