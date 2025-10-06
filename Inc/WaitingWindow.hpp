#ifndef WAITINGWINDOW_HPP
#define WAITINGWINDOW_HPP

#include "BaseDrawWindow.hpp"
#include "TileButton.hpp"

enum WaitingStep {
    Cancel,
    Waiting,
    Valid,
};


class WaitingWindow : public BaseDrawWindow {

private:
	Picture icon;
	Label text;
public:
    // Constructeur
	WaitingWindow(GraphicLib& glib, string title);

    // Redéfinition de la méthode drawing
    bool drawing();
    bool drawing(string, WaitingStep);
    bool hidding();
    bool onClick(Message&);
    void refreshInformation();
    void processId(int id);
    bool onKeyPress(ingenico::graphics::Message &message);

};

#endif
