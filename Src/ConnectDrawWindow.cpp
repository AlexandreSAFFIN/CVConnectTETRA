#include "ConnectDrawWindow.hpp"
#include "Utils.hpp"
#include "AppResources.hpp"
#include "cib/disk/Disk.hpp"
#include "TileButton.hpp"
#include "string.h"
#include <unistd.h>

bool ConnectDrawWindow::onClick(Message& msg)
{
	return true;
}

ConnectDrawWindow::ConnectDrawWindow(GraphicLib& glib, string text) :
    BaseDrawWindow(glib, text)
{
    createSnackBar();

    // La fenêtre occupe tout l'écran
    mainWindow.setSize(100, 100, GL_UNIT_PERCENT);

    Layout root(mainWindow);


    Label spacerTop(root);
    spacerTop.setItem(0, 0);
    spacerTop.setText(""); // rien à afficher
    spacerTop.setExpand(GL_DIRECTION_HEIGHT);


    Layout content(root);
    content.setItem(0, 1);


    Label title(content);
    title.setItem(0, 0);
    title.setExpand(GL_DIRECTION_WIDTH);
    title.setText("RENTREZ VOTRE\nSHOP ID");
    title.setTextAlign(GL_ALIGN_CENTER);
    title.setForeColor(GL_COLOR_BLACK);


    Layout row(content);
    row.setItem(0, 1);

    Label spacerLeft(row);
    spacerLeft.setItem(0, 0);
    spacerLeft.setText("");
    spacerLeft.setExpand(GL_DIRECTION_WIDTH);

    editText = new Edit(row);
    editText->setItem(1, 0);

    editText->setSize(200, 45, GL_UNIT_PIXEL);
    editText->setBackColor(GL_COLOR_WHITE);
    editText->setForeColor(GL_COLOR_BLACK);
    editText->setTextAlign(GL_ALIGN_CENTER);

    Label spacerRight(row);
    spacerRight.setItem(2, 0);
    spacerRight.setText("");
    spacerRight.setExpand(GL_DIRECTION_WIDTH);


    Label spacerBottom(root);
    spacerBottom.setItem(0, 2);
    spacerBottom.setText("");
    spacerBottom.setExpand(GL_DIRECTION_HEIGHT);

    // Callback clavier (si tu en as besoin)
    mainWindow.registerMethod(GL_EVENT_KEY_DOWN, this, &ConnectDrawWindow::onKeyPress);
}


void ConnectDrawWindow::refreshInformation()
{

    mainWindow.show();
}

// Redéfinition de la méthode drawing
bool ConnectDrawWindow::drawing()
{
	refreshInformation();
    canDispatch = true;

    mainWindow.show();  // Afficher la fenêtre principale
//    topLayout.show();
    hideSnackBar();

    while (canDispatch) {
//    	topLayout.show();
        mainWindow.dispatch(100);  // Boucle pour gérer les événements
        if(isShowSnackbar)
        {
        	timer+=100;
        	if(timer == 1000)
        	{
        		hideSnackBar();
        	}
        }
    }

    mainWindow.hide();  // Fermer la fenêtre
    mainWindow.dispatch();


    return Utils::ref().isConnected;
}

void ConnectDrawWindow::onValidate()
{

	Utils::ref().isConnected = false;

	Utils::ref().isConnected = Utils::ref().connectWithShopId(editText->getText());

	canDispatch = false;
}

bool ConnectDrawWindow::onKeyPress(ingenico::graphics::Message &message) {
        // Handle key press event here
        int key = message.getKey();
        if (key == GL_KEY_CANCEL)
        {
        	canDispatch = false;
        }
        else if(key == GL_KEY_VALID)
        {
        	onValidate();
        }
        return true;
}
