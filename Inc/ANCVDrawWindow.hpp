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
    bool onClick(Message& msg);
    void refreshInformation();

    bool onKeyPress(ingenico::graphics::Message &message);


};

#endif
