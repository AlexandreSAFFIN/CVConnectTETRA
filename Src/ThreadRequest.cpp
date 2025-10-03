#include <PaymentQRWindow.hpp>
#include "ThreadRequest.hpp"
#include "Utils.hpp"

// Constructeur
ThreadRequest::ThreadRequest() : Thread()  {
    canDispatch = true;
}

// Thread principal
void ThreadRequest::run(){
	do
	{
//		PaymentQRWindow::error = Utils::ptr()->getTransactionState();
	}while(canDispatch);
}

void ThreadRequest::stop()
{
	canDispatch = false;
}

