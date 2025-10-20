#include <PaymentQRWindow.hpp>
#include "ThreadRequest.hpp"
#include "PaymentTransacWindow.hpp"
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
			PaymentQRWindow::error = Utils::ptr()->pollingPreTransacResult();
		}
		else
		{
			PaymentPreTransacWindow::error = Utils::ptr()->pollingTransacResult();
		}
	}while(canDispatch && (PaymentPreTransacWindow::error != 202 && PaymentQRWindow::error != 202));
}

void ThreadRequest::stop()
{
	canDispatch = false;
}

