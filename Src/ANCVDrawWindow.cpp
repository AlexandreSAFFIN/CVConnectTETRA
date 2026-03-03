#include "ANCVDrawWindow.hpp"
#include "Utils.hpp"
#include "AppResources.hpp"
#include "cib/disk/Disk.hpp"
#include "TileButton.hpp"
#include "HistoricDrawWindow.hpp"
#include "string.h"


bool ANCVDrawWindow::onClick(Message& msg)
{

	int id = msg.getWidget().getId();
	if(id == 1)
	{
		cib::json::Document jsonParam;
		loadDataAsJson(FIC_PARAM, jsonParam);
		bool val = (bool)jsonParam["ANCVOnly"].as_bool();
		jsonParam["ANCVOnly"] = !val;
		saveDataAsJson(FIC_PARAM, jsonParam);
		refreshInformation();
	}
	else if(id == 3)
	{
		canDispatch = false;
		Utils::ref().launchTransactionWithANCVParam(Utils::ref().getSaveTransac());
	}
	else
	{
		HistoricDrawWindow(Utils::ref().glib, "HISTORIQUE").drawing();
	}
	return true;
}



ANCVDrawWindow::ANCVDrawWindow(GraphicLib& glib, string text) :
    BaseDrawWindow(glib, text)
{

    createSnackBar();
    string bg_completion = "rounded_red";

    if(Utils::ref().getSaveTransac().orderId.empty())
    {
		btnCompletion = new TileButton(mainWindow,
			Utils::ref().getIconsPath(bg_completion), Utils::ref().getIconsPath("completion_icon"), "Complétion",
			8,150, 1, this, &BaseDrawWindow::onClick);

		new TileButton(mainWindow,
				Utils::ref().getIconsPath("rounded"), Utils::ref().getIconsPath("waiting_icon"), "Historique",
						165,150, 2,this, &BaseDrawWindow::onClick);
    }
    else
    {
        btnCompletion = new TileButton(mainWindow,
            Utils::ref().getIconsPath(bg_completion), Utils::ref().getIconsPath("completion_icon"), "Complétion",
            8,74, 1, this, &BaseDrawWindow::onClick);

        new TileButton(mainWindow,
        		Utils::ref().getIconsPath("rounded"), Utils::ref().getIconsPath("waiting_icon"), "Historique",
        		        165,74, 2,this, &BaseDrawWindow::onClick);

        new TileButton(mainWindow,
        		Utils::ref().getIconsPath("rounded"), Utils::ref().getIconsPath("ok_icon"), "Relancer",8,230,3, this, &BaseDrawWindow::onClick);

    }


    mainWindow.registerMethod(GL_EVENT_KEY_DOWN, this, &ANCVDrawWindow::onKeyPress);
}


void ANCVDrawWindow::refreshInformation()
{
    cib::json::Document jsonParam;
    loadDataAsJson(FIC_PARAM, jsonParam);
    string bg = "rounded_green";
    if((bool)jsonParam["ANCVOnly"].as_bool())
    {
    	bg = "rounded_red";
    }

    btnCompletion->getBack()->setSource(Utils::ref().getIconsPath(bg));
    mainWindow.show();
}

// Redéfinition de la méthode drawing
bool ANCVDrawWindow::drawing()
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

bool ANCVDrawWindow::onKeyPress(ingenico::graphics::Message &message) {
        // Handle key press event here
        int key = message.getKey();
        if (key == GL_KEY_CANCEL || key == GL_KEY_CORRECTION)
        {
        	canDispatch = false;
        }
        return true;
}
