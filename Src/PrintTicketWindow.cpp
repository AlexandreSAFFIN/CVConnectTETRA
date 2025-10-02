#include "PrintTicketWindow.hpp"
#include "Utils.hpp"
#include "AppResources.hpp"
#include "cib/disk/Disk.hpp"
#include "TileButton.hpp"
#include "string.h"


bool PrintTicketWindow::onClick(Message& msg)
{
	int id = msg.getWidget().getId();

	processId(id);

	return true;
}


void PrintTicketWindow::processId(int id)
{
	if(!isClient)
	{
		if(id == 1)
		{
			//TODO PRINT TICKET COMMERCANT
		}

		title->setText("TICKET CLIENT");
		isClient = true;
	}
	else
	{
		if(id == 1)
		{
			//TODO PRINT TICKET COMMERCANT
		}
		canDispatch = false;

	}
}

PrintTicketWindow::PrintTicketWindow(GraphicLib& glib, string text) :
    BaseDrawWindow(glib, text)
{

    createSnackBar();

    new TileButton(mainWindow,
    		Utils::ref().getIconsPath("rounded"), Utils::ref().getIconsPath("pay"), "OUI",
    		        8,150,1,this, &BaseDrawWindow::onClick);

    new TileButton(mainWindow,
    		Utils::ref().getIconsPath("rounded"), Utils::ref().getIconsPath("qrcode"), "NON",
    		        165,150,2,this, &BaseDrawWindow::onClick);


    mainWindow.registerMethod(GL_EVENT_KEY_DOWN, this, &PrintTicketWindow::onKeyPress);
}


void PrintTicketWindow::refreshInformation()
{
	isClient = true;
    mainWindow.show();
}

// Redéfinition de la méthode drawing
bool PrintTicketWindow::drawing()
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

    return true;
}

bool PrintTicketWindow::onKeyPress(ingenico::graphics::Message &message) {
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
