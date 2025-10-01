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

    bool onClick(Message& msg);

    bool onClickNetwork();
    bool onClickMaintenance();
    bool onClickANCV();
    bool onClickUpdate();
};

#endif
