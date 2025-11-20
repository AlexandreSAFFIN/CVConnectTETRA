#ifndef YESNOWINDOW_HPP
#define YESNOWINDOW_HPP

#include "BaseDrawWindow.hpp"
#include "TileButton.hpp"


class YesNoWindow : public BaseDrawWindow {
protected:
	bool isOk;
public:
    // Constructeur
	YesNoWindow(GraphicLib& glib, string text, string title);

    // Redéfinition de la méthode drawing
    bool drawing();
    bool onClick(Message&);
    void refreshInformation();
    void processId(int id);
    bool onKeyPress(ingenico::graphics::Message &message);

};

#endif
