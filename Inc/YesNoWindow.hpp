#ifndef YESNOWINDOW_HPP
#define YESNOWINDOW_HPP

#include "BaseDrawWindow.hpp"
#include "TileButton.hpp"


class YesNoWindow : public BaseDrawWindow {
protected:
	bool isOk;
	string textToDisplay;
public:
    // Constructeur
	YesNoWindow(GraphicLib& glib, string title, unsigned long long);

    // Redéfinition de la méthode drawing
    bool drawing();
    bool onClick(Message&);
    void refreshInformation();
    void processId(int id);
    bool onKeyPress(ingenico::graphics::Message &message);

};

#endif
