
#include "PinpadThread.hpp"
#include "Utils.hpp"

// Constructeur
PinpadThread::PinpadThread() : Thread()  {
    canDispatch = true;
}

// Thread principal
void PinpadThread::run(){
	do
	{
		Utils::ref().payIdWindow->dispatchPpWindow();
	}while(canDispatch);
}

void PinpadThread::stop()
{
	canDispatch = false;
}

