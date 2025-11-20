#include "YesNoWindow.hpp"
#include "Utils.hpp"
#include "AppResources.hpp"
#include "cib/disk/Disk.hpp"
#include "TileButton.hpp"
#include "string.h"


bool YesNoWindow::onClick(Message& msg)
{
	int id = msg.getWidget().getId();

	processId(id);

	return true;
}


void YesNoWindow::processId(int id)
{
		isOk = (id==1);
		canDispatch = false;
}

YesNoWindow::YesNoWindow(GraphicLib& glib, string text, string title):
    BaseDrawWindow(glib, text)
{
    createSnackBar();

	this->title->setText(title);

    new TileButton(mainWindow,
    		Utils::ref().getIconsPath("rounded"), Utils::ref().getIconsPath("ok_icon"), "OUI",
    		        8,150,1,this, &BaseDrawWindow::onClick);

    new TileButton(mainWindow,
    		Utils::ref().getIconsPath("rounded"), Utils::ref().getIconsPath("cancel_icon"), "NON",
    		        165,150,2,this, &BaseDrawWindow::onClick);


    mainWindow.registerMethod(GL_EVENT_KEY_DOWN, this, &YesNoWindow::onKeyPress);
}


void YesNoWindow::refreshInformation()
{
	isOk = false;
}

// Redéfinition de la méthode drawing
bool YesNoWindow::drawing()
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

    return isOk;
}

bool YesNoWindow::onKeyPress(ingenico::graphics::Message &message) {
        // Handle key press event here
        int key = message.getKey();
        if (key == GL_KEY_CANCEL)
        {
        	processId(2);
        }
        else if (key == GL_KEY_VALID)
        {
        	processId(1);
        }
        return true;
}
