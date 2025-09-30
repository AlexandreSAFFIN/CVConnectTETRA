#ifndef MAINTENANCEDRAWWINDOW_HPP
#define MAINTENANCEDRAWWINDOW_HPP

#include "BaseDrawWindow.hpp"



class MaintenanceDrawWindow : public BaseDrawWindow {

public:
    // Constructeur
    MaintenanceDrawWindow(GraphicLib& glib, string title);

    // Redéfinition de la méthode drawing
    bool drawing();

    void refreshInformation();

    bool onKeyPress(ingenico::graphics::Message &message);

    bool onClickNetwork(Message& msg);
    bool onClickMaintenance(Message& msg);
    bool onClickANCV(Message& msg);
    bool onClickUpdate(Message& msg);
};

#endif
