#include "BaseDrawWindow.hpp"
#include "Utils.hpp"
BaseDrawWindow::BaseDrawWindow(GraphicLib& glib, string topText) : mainWindow(glib), picture(mainWindow),topLayout(mainWindow),
centerLayout(mainWindow), bottomLayout(mainWindow)
{
	mainWindow.setSize(100, 100, GL_UNIT_PERCENT);

	picture.setSize(100, 100, GL_UNIT_PERCENT);
	picture.setPosition(0, 0, GL_UNIT_PIXEL);
	picture.setSource(Utils::ptr()->getIconsPath("backgroundtpe"));
	picture.setTransformation(GL_TRANSFORMATION_STRETCH_ALL);

	// --- TOP --- (10%)
	topLayout.setPosition(0, 0, GL_UNIT_PERCENT);
	topLayout.setSize(100, 15, GL_UNIT_PERCENT);
	topLayout.setBackColor(GL_COLOR_WHITE);
	// --- CENTER --- (73%)
	centerLayout.setPosition(0, 15, GL_UNIT_PERCENT);
	centerLayout.setSize(100, 75, GL_UNIT_PERCENT);
	centerLayout.setForeColor(GL_COLOR_WHITE);
	// --- BOTTOM --- (17%)
	bottomLayout.setPosition(0, 90, GL_UNIT_PERCENT);
	bottomLayout.setSize(100, 10, GL_UNIT_PERCENT);
//	bottomLayout.setBackColor(GL_COLOR_WHITE);

    title = new Label(topLayout);
    title->setText(topText);
    title->setForeColor(GL_COLOR_BLACK);

    // Taille de police (seule la hauteur compte)
    title->setFontSize(0, 24, GL_UNIT_PIXEL);
    // ou: title->setFontScale(GL_SCALE_LARGE);

    // Alignement du texte à l'intérieur du label
    title->setTextAlign(GL_ALIGN_CENTER);

    // Placement/occupation dans la cellule du layout
    title->setItem(0, 0);                          // cellule (col=0, row=0) si nécessaire
    title->setMargins(8, 8, 8, 8, GL_UNIT_PIXEL);  // marges dans la cellule
    title->setForeAlign(GL_ALIGN_CENTER); // centre le widget dans la cellule
    title->setExpand(GL_DIRECTION_ALL);           // occupe tout l'espace disponible
    title->setGrow(GL_DIRECTION_ALL);             // autorise l'étirement si besoin
    title->setFontStyle(GL_FONT_STYLE_BOLD);
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
