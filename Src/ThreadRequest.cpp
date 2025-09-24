#include "ThreadRequest.hpp"
#include "Utils.hpp"
#include "PaymentWindow.hpp"

// Constructeur
ThreadRequest::ThreadRequest() : Thread()  {
    canDispatch = true;
}

// Thread principal
void ThreadRequest::run(){
	do
	{
		PaymentWindow::error = Utils::ptr()->getTransactionState();
	}while(canDispatch);
}

void ThreadRequest::stop()
{
	canDispatch = false;
}

