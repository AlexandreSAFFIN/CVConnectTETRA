#include <PaymentQRWindow.hpp>
#include "ThreadRequest.hpp"
#include "Utils.hpp"

// Constructeur
ThreadRequest::ThreadRequest(bool isPreTransac) : Thread()  {
    canDispatch = true;
    this->isPreTransac = isPreTransac;
    PaymentQRWindow::error = 0;
}

// Thread principal
void ThreadRequest::run(){
	do
	{
		if(isPreTransac)
		{
			PaymentQRWindow::error = Utils::ptr()->pollingTransacResult();
		}
		else
		{
			PaymentQRWindow::error = Utils::ptr()->pollingPreTransacResult();
		}
	}while(canDispatch);
}

void ThreadRequest::stop()
{
	canDispatch = false;
}

