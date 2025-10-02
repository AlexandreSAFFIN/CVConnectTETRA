#include "PaymentChoiceDrawWindow.hpp"
#include "Utils.hpp"
#include "AppResources.hpp"
#include "cib/disk/Disk.hpp"
#include "TileButton.hpp"
#include "string.h"


bool PaymentChoiceDrawWindow::onClick(Message& msg)
{
	int id = msg.getWidget().getId();

	processId(id);

	return true;
}

PaymentChoice PaymentChoiceDrawWindow::getPM()
{
	return paymentChoice;
}



void PaymentChoiceDrawWindow::processId(int id)
{
	if(!isQRChoice)
	{
		if(id == 1)
		{
			canDispatch = false;
			paymentChoice = CB;
		}
		else
		{
			isQRChoice = true;
			button1->setTextAndIcon("QRCODE", Utils::ref().getIconsPath("pay"));
			button2->setTextAndIcon("ID CLIENT", Utils::ref().getIconsPath("pay"));
			title->setText("OPTION PAIEMENT");
		}
	}
	else
	{
		paymentChoice = ID;
		if(id == 1)
		{
			paymentChoice = QRCODE;
		}
		canDispatch = false;

	}
}

PaymentChoiceDrawWindow::PaymentChoiceDrawWindow(GraphicLib& glib, string text) :
    BaseDrawWindow(glib, text)
{

    createSnackBar();

    button1 = new TileButton(mainWindow,
    		Utils::ref().getIconsPath("rounded"), Utils::ref().getIconsPath("pay"), "CB",
    		        8,150,1,this, &BaseDrawWindow::onClick);

    button2 = new TileButton(mainWindow,
    		Utils::ref().getIconsPath("rounded"), Utils::ref().getIconsPath("qrcode"), "ANCV",
    		        165,150,2,this, &BaseDrawWindow::onClick);


    mainWindow.registerMethod(GL_EVENT_KEY_DOWN, this, &PaymentChoiceDrawWindow::onKeyPress);
}


void PaymentChoiceDrawWindow::refreshInformation()
{
	paymentChoice = CB;
	isQRChoice = false;
	button1->setTextAndIcon("CB", Utils::ref().getIconsPath("pay"));
	button2->setTextAndIcon("ANCV", Utils::ref().getIconsPath("pay"));
	title->setText("MOYEN DE PAIEMENT");
    mainWindow.show();
}

// Redéfinition de la méthode drawing
bool PaymentChoiceDrawWindow::drawing()
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

    return !paymentChoice == CB;
}

bool PaymentChoiceDrawWindow::onKeyPress(ingenico::graphics::Message &message) {
        // Handle key press event here
        int key = message.getKey();
        if (key == GL_KEY_CANCEL || key == GL_KEY_CORRECTION)
        {
        	paymentChoice = CB;
        	if(isQRChoice)
        	{
        		isQRChoice = false;
    			button1->setTextAndIcon("CB", Utils::ref().getIconsPath("pay"));
    			button2->setTextAndIcon("ANCV", Utils::ref().getIconsPath("pay"));
    			title->setText("MOYEN DE PAIEMENT");
        	}
        	else
        	{
        		canDispatch = false;
        	}
        }
        return true;
}
