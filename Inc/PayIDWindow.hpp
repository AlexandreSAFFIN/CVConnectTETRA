#ifndef PAYIDDRAWWINDOW_HPP
#define PAYIDTDRAWINDOW_HPP

#include "BaseDrawWindow.hpp"
#include "TileButton.hpp"


class PayIDWindow : public BaseDrawWindow {
protected:
	Edit* editText;
	long long int amount;
	bool transactionStatus;
	Window m_ppwindow;
	Picture pp_bgpicture;
    Label p_paymentLabel;
    Label p_transactionStatusLabel;
    ingenico::graphics::Picture p_logoimg;  // Image de la transactionn
    ingenico::graphics::Edit* p_edit;  // Image de la transaction
//    ingenico::graphics::Picture transactionImage;  // Image de la transaction




public:
    // Constructeur
	PayIDWindow(GraphicLib& glib,GraphicLib& pplib, string title, long long int amount);

    // Redéfinition de la méthode drawing
    bool drawing();
    bool onClick(Message&);
    void refreshInformation();

    bool onKeyPress(ingenico::graphics::Message &message);
    void onValidate();
};

#endif

