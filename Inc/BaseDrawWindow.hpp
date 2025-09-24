#ifndef BASEDRAWWINDOW_HPP
#define BASEDRAWWINDOW_HPP

#include "AppResources.hpp"  // Pour utiliser Label pour SnackBar

class Utils; // Déclaration anticipée de la classe Utils, si nécessaire

class BaseDrawWindow {
protected:
    Window mainWindow;  // Fenêtre principale
    Window* eventWindow;  // Fenêtre principale
    Label *snackBar;     // SnackBar pour afficher des messages
    bool canDispatch;

    bool isShowSnackbar;
    int timer;

public:
    // Constructeur
    BaseDrawWindow(GraphicLib& glib);

    // Afficher un message dans la SnackBar
    void showSnackBar(const std::string& message, bool isSuccess);
    void createSnackBar();
    // Cacher la SnackBar
    void hideSnackBar();

    // Méthode de dessin virtuelle (doit être redéfinie dans les classes dérivées)
    virtual bool drawing() = 0;

    virtual void refreshInformation() = 0;

    Window getMainWindow();

    // Destructeur virtuel
    virtual ~BaseDrawWindow();
};

#endif // BASEDRAWWINDOW_HPP
