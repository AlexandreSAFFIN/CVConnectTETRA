#ifndef PAYMENTCHOICEDRAWWINDOW_HPP
#define PAYMENTCHOICEDRAWWINDOW_HPP

#include "BaseDrawWindow.hpp"
#include "TileButton.hpp"

enum PaymentChoice {
    CB,
    QRCODE,
    ID,
};

class PaymentChoiceDrawWindow : public BaseDrawWindow {
protected:
	bool isQRChoice;
	PaymentChoice paymentChoice;
	TileButton *button1;
	TileButton *button2;
public:
    // Constructeur
	PaymentChoiceDrawWindow(GraphicLib& glib, string title);

    // Redéfinition de la méthode drawing
    bool drawing();
    bool onClick(Message&);
    void refreshInformation();
    void processId(int id);
    PaymentChoice getPM();
    bool onKeyPress(ingenico::graphics::Message &message);

};

#endif
