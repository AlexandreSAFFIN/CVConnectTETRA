#ifndef MYPINPADTHREAD_HPP
#define MYPINPADTHREAD_HPP

#include "AppResources.hpp"

class PinpadThread : public ingenico::os::Thread {
public:
    // Constructeur
	PinpadThread();

    // Thread principal
    void run();
    void stop();
private:
	bool canDispatch;
};



#endif // MYTHREAD_HPP
