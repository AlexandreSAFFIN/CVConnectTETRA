#ifndef PAYMENTPREWINDOW_HPP
#define PAYMENTPREWINDOW_HPP


#include "ThreadRequest.hpp"
#include "BaseDrawWindow.hpp"  // Inclusion de votre base de classe
#include "PaymentQRWindow.hpp"


class PaymentPreTransacWindow : public BaseDrawWindow {
public:

    static int error;

    PaymentPreTransacWindow(GraphicLib& , GraphicLib&, string text);
    ~PaymentPreTransacWindow();

    bool drawing();
    void refreshInformation();

    bool onKeyPress(ingenico::graphics::Message &message);
    bool onClick(Message& msg);

    void onCancelClick(Message& msg);
    void treatPollingReturn();

private:

    long long int amount;
    ThreadRequest *threadRequest;

	Window m_ppwindow;

    Label transactionStatusLabel;
    Picture pp_bgpicture;
    Label p_paymentLabel;
    Label p_transactionStatusLabel;
    ingenico::graphics::Picture* p_transactionImage;  // Image de la transaction
    ingenico::graphics::Picture transactionImage;  // Image de la transaction

    ingenico::graphics::Picture p_logoimg;  // Image de la transactionn

    StatePayment statePayment;
	T_GL_SIZE size;
	T_GL_SIZE psize;
    bool transactionStatus;
};

#endif // PAYMENTWINDOW_HPP
