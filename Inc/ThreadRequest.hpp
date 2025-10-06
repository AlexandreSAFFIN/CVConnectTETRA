#ifndef MYTHREAD_HPP
#define MYTHREAD_HPP

#include "AppResources.hpp"

class ThreadRequest : public ingenico::os::Thread {
public:
    // Constructeur
	ThreadRequest(bool isPreTransac);

    // Thread principal
    void run();
    void stop();
private:
	bool canDispatch;
	bool isPreTransac;
};



#endif // MYTHREAD_HPP
