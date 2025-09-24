#include "BaseDrawWindow.hpp"

BaseDrawWindow::BaseDrawWindow(GraphicLib& glib) : mainWindow(glib) {
    mainWindow.setSize(320, 480, GL_UNIT_PIXEL);  // Taille par défaut de la fenêtre
}

// Retourner la fenêtre principale
Window BaseDrawWindow::getMainWindow() {
    return mainWindow;
}

// Afficher un message dans la SnackBar en la repositionnant sur l'écran
void BaseDrawWindow::showSnackBar(const std::string& message, bool isSuccess) {

	timer = 0;
    snackBar->setText(message);
    snackBar->setPosition(0, 395, GL_UNIT_PIXEL);  // Position visible sur l'écran
	snackBar->setBackColor(GL_COLOR_DARK_RED);
    if(isSuccess)
    {
    	snackBar->setBackColor(GL_COLOR_DARK_GREEN);
    }
	mainWindow.show();
    isShowSnackbar = true;
}

void BaseDrawWindow::createSnackBar() {
    snackBar = new Label(mainWindow);  // Création dynamique de la SnackBar
    snackBar->setText("");              // Texte initial vide
    snackBar->setSize(320, 60, GL_UNIT_PIXEL);     // Taille de la SnackBar
    snackBar->setPosition(0, 500, GL_UNIT_PIXEL);  // Position initiale hors de l'écran
    hideSnackBar();
}

// Cacher la SnackBar en la déplaçant hors de l'écran
void BaseDrawWindow::hideSnackBar() {
	timer = 0;
    snackBar->setText("");
    snackBar->setPosition(0, 500, GL_UNIT_PIXEL);  // Hors de la vue (en bas de l'écran)
    mainWindow.show();
    isShowSnackbar = false;

}

// Destructeur virtuel
BaseDrawWindow::~BaseDrawWindow() {}
