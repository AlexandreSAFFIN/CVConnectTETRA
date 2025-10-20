#include "WaitingWindow.hpp"
#include "Utils.hpp"
#include "AppResources.hpp"
#include "cib/disk/Disk.hpp"
#include "TileButton.hpp"
#include "string.h"
#include "unistd.h"

bool WaitingWindow::onClick(Message& msg)
{
	int id = msg.getWidget().getId();

	processId(id);

	return true;
}


void WaitingWindow::processId(int id)
{

}

WaitingWindow::WaitingWindow(GraphicLib& glib, string text) :
    BaseDrawWindow(glib, text),
	icon(mainWindow),
	text(mainWindow)
{
	T_GL_SIZE size = glib.getScreenSize();
	icon.setSize(160, 160, GL_UNIT_PIXEL);  // Taille du logo
	icon.setPosition((size.width-160)/2, (size.height-160)/2-45, GL_UNIT_PIXEL);  // Position du logo à gauche
	icon.setSource("file://flash/HOST/QRCODE.png");  // Chemin vers l'image du logo
	icon.setTransformation(GL_TRANSFORMATION_STRETCH_ALL);

	this->text.setText("Etat de la transaction");
	this->text.setFontName("Arial");
	this->text.setFontSize(20, 20, GL_UNIT_PIXEL);
	this->text.setFontStyle(GL_FONT_STYLE_ITALIC);

	this->text.setSize(size.width, 80, GL_UNIT_PIXEL);
	this->text.setPosition(0, 280, GL_UNIT_PIXEL);
	this->text.setTextAlign(GL_ALIGN_CENTER);
	this->text.setForeColor(GL_COLOR_BLACK);

    createSnackBar();

}


void WaitingWindow::refreshInformation()
{

    mainWindow.show();
}

// Redéfinition de la méthode drawing
bool WaitingWindow::drawing()
{
	refreshInformation();
	mainWindow.dispatch(0);
    mainWindow.show();  // Afficher la fenêtre principale
//    topLayout.show();
    hideSnackBar();
    return true;
}


bool WaitingWindow::drawing(string msg, WaitingStep step)
{
	refreshInformation();
	text.setText(msg);

    if(step == Cancel)
    {
    	icon.setSource(Utils::ref().getIconsPath("cancelicon"));
    	mainWindow.dispatch(0);
        mainWindow.show();  // Afficher la fenêtre principale
    	sleep(1);
        mainWindow.hide();
        mainWindow.dispatch(0);
    }
    else if(step == Valid)
    {
    	icon.setSource(Utils::ref().getIconsPath("okicon"));
    	mainWindow.dispatch(0);
        mainWindow.show();  // Afficher la fenêtre principale
    	sleep(1);
        mainWindow.hide();
        mainWindow.dispatch(0);
    }
    else if(step == Waiting)
    {
    	icon.setSource(Utils::ref().getIconsPath("waiting_icon"));
    	mainWindow.dispatch(0);
        mainWindow.show();  // Afficher la fenêtre principale
    }

    hideSnackBar();

    return true;
}

bool WaitingWindow::hidding()
{
    mainWindow.hide();  // Afficher la fenêtre principale
    mainWindow.dispatch(0);
//    topLayout.show();
    hideSnackBar();

    return true;
}

