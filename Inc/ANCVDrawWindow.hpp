#ifndef ANCVDRAWWINDOW_HPP
#define ANCVDRAWWINDOW_HPP

#include "BaseDrawWindow.hpp"
#include "TileButton.hpp"


class ANCVDrawWindow : public BaseDrawWindow {
private:
	TileButton* btnCompletion;

public:
    // Constructeur
	ANCVDrawWindow(GraphicLib& glib, string title);

    // Redéfinition de la méthode drawing
    bool drawing();

    void refreshInformation();

    bool onKeyPress(ingenico::graphics::Message &message);

    bool onClickPrint(Message& msg);
    bool onClickCompletion(Message& msg);
};

#endif
