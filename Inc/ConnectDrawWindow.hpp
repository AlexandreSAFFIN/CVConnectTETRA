#ifndef CONNECTDRAWWINDOW_HPP
#define CONNECTDRAWWINDOW_HPP

#include "BaseDrawWindow.hpp"
#include "TileButton.hpp"


class ConnectDrawWindow : public BaseDrawWindow {
private:
	Edit* editText;

public:
    // Constructeur
	ConnectDrawWindow(GraphicLib& glib, string title);

    // Redéfinition de la méthode drawing
    bool drawing();
    bool onClick(Message&);
    void refreshInformation();

    bool onKeyPress(ingenico::graphics::Message &message);
    void onValidate();
};

#endif
