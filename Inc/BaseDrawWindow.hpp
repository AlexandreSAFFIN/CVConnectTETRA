#ifndef BASEDRAWWINDOW_HPP
#define BASEDRAWWINDOW_HPP

#include "AppResources.hpp"  // Pour utiliser Label pour SnackBar

class Utils; // Déclaration anticipée de la classe Utils, si nécessaire

class BaseDrawWindow {

#define GL_COLOR_ANCV                  ((T_GL_COLOR)0xFFFF271A)

protected:
    Window mainWindow;

    Picture picture;
    Layout topLayout;

    Window* eventWindow;  // Fenêtre principale
    Label *title;
    Label *snackBar;     // SnackBar pour afficher des messages
    bool canDispatch;

    bool isShowSnackbar;
    int timer;

public:
    // Constructeur
    BaseDrawWindow(GraphicLib& glib, string topText);

    // Afficher un message dans la SnackBar
    void showSnackBar(const std::string& message, bool isSuccess);
    void createSnackBar();
    // Cacher la SnackBar
    void hideSnackBar();

    virtual bool drawing() = 0;
    virtual bool onClick(Message& msg) = 0;
    virtual void refreshInformation() = 0;

    // Destructeur virtuel
    virtual ~BaseDrawWindow();
};

#endif // BASEDRAWWINDOW_HPP
