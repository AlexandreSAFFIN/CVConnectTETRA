#ifndef PRINTTICKETWINDOW_HPP
#define PRINTTICKETWINDOW_HPP

#include "BaseDrawWindow.hpp"
#include "TileButton.hpp"
#include "AncvPrintTicket.hpp"


class PrintTicketWindow : public BaseDrawWindow {
protected:
	bool isClient;
	AncvConnectData dataToPrint;
public:
    // Constructeur
	PrintTicketWindow(GraphicLib& glib, string title, AncvConnectData& data);

    bool drawing();
    bool onClick(Message&);
    void refreshInformation();
    void processId(int id);
    bool onKeyPress(ingenico::graphics::Message &message);

};

#endif
