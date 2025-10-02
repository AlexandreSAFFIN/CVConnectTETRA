#ifndef PRINTTICKETWINDOW_HPP
#define PRINTTICKETWINDOW_HPP

#include "BaseDrawWindow.hpp"
#include "TileButton.hpp"


class PrintTicketWindow : public BaseDrawWindow {
protected:
	bool isClient;
public:
    // Constructeur
	PrintTicketWindow(GraphicLib& glib, string title);

    // Redéfinition de la méthode drawing
    bool drawing();
    bool onClick(Message&);
    void refreshInformation();
    void processId(int id);
    bool onKeyPress(ingenico::graphics::Message &message);

};

#endif
