#include "MaintenanceDrawWindow.hpp"
#include "Utils.hpp"
#include "AppResources.hpp"
#include "cib/disk/Disk.hpp"
#include "TileButton.hpp"
#include "ANCVDrawWindow.hpp"

bool MaintenanceDrawWindow::onClickNetwork()
{
	int choice;
	cib::json::Document jsonParam;

	SGL::ref().openApplicationWindow();

	loadDataAsJson(FIC_PARAM, jsonParam);

	// creating menu
	ingenico::graphics::StringList editCntType;
	editCntType.add("IP");
	editCntType.add("GPRS");
	editCntType.add("WIFI");
	choice = SGL::ref().dialogChoice("CHOIX TYPE CONNEXION", editCntType, Utils::ptr()->m_cntType - 1, GL_BUTTON_VALID_CANCEL, GL_KEY_1, GL_TIME_MINUTE);

	if (choice == 1)
	{
		choice = GL_KEY_CANCEL;

		if (SGL::ref().dialogText("Configuration GPRS", "APN", fill("/c", 30), Utils::ptr()->m_gprs.apn) == GL_KEY_VALID)
		{
			if (SGL::ref().dialogText("Configuration GPRS", "Login", fill("/c", 20), Utils::ptr()->m_gprs.login) == GL_KEY_VALID)
			{
				if (SGL::ref().dialogText("Configuration GPRS", "Mot de passe", fill("/c", 20), Utils::ptr()->m_gprs.pwd) == GL_KEY_VALID)
				{
					jsonParam["gprs"]["apn"] = Utils::ptr()->m_gprs.apn;
					jsonParam["gprs"]["login"] = Utils::ptr()->m_gprs.login;
					jsonParam["gprs"]["password"] = Utils::ptr()->m_gprs.pwd;
					choice = 1;
				}
			}
		}
	}
	if (choice != GL_KEY_CANCEL && choice != GL_RESULT_INACTIVITY)
	{
		jsonParam["connectionType"] = choice + 1; // CommunicationTools::eConnectionType
		Utils::ptr()->m_cntType = (connectionType::EconnectionType)(choice + 1);
		saveDataAsJson(FIC_PARAM, jsonParam);
		Utils::ptr()->m_session.setSessionData(Utils::ptr()->m_host, 443, Utils::ptr()->m_cntType, "CIB", Utils::ptr()->m_gprs);
		SGL::ref().dialogMessage("Paramétrage", "Modifications enregistrées", GL_ICON_NONE, GL_BUTTON_NONE, GL_TIME_SECOND * 3);
	}
	SGL::ref().closeApplicationWindow();

	return true;
}

bool MaintenanceDrawWindow::onClickMaintenance()
{
	string code;
	string package;

//	SGL::ref().openApplicationWindow();
	SGL::ref().dialogText("Maintenance", "Code maintenance", "/d/d/d/d/d", code, GL_TIME_INFINITE);
	switch (atoi(code.c_str()))
	{
	case 3383: // DEVE
		SSL_DeleteProfile("ANCV");
		Utils::ref().resetTerminal(HOST_DEV);
		SGL::ref().dialogMessage("Maintenance", "Environnement de DEV", GL_ICON_INFORMATION, GL_BUTTON_VALID, GL_TIME_INFINITE);
		canDispatch = false;
		break;

	case 7763: // PROD
		SSL_DeleteProfile("ANCV");
		Utils::ref().resetTerminal(HOST_PROD);
		SGL::ref().dialogMessage("Maintenance", "Environnement de PROD", GL_ICON_INFORMATION, GL_BUTTON_VALID, GL_TIME_INFINITE);
		canDispatch = false;
		break;
	case 7753: // SSLD
		ssllib_open();
		if (SSL_DeleteProfile("ANCV") == SSL_PROFILE_EOK)
		{
			SSL_DeleteProfile("ANCV");
			SGL::ref().dialogMessage("Maintenance", "Certificat SSL supprime avec succes", GL_ICON_INFORMATION, GL_BUTTON_VALID, GL_TIME_INFINITE);
		}
		else
		{
			SGL::ref().dialogMessage("Maintenance", "Echec de suppression du certificat SSL", GL_ICON_ERROR, GL_BUTTON_VALID, GL_TIME_INFINITE);
		}
		canDispatch = false;
		ssllib_close();
		break;
	}
//	SGL::ref().closeApplicationWindow();
    return true;  // Événement géré
}

bool MaintenanceDrawWindow::onClickANCV()
{
	ANCVDrawWindow(SGL::ref(), "OPTION ANCV").drawing();
	canDispatch = !Utils::ref().isSavedTransac;
    return true;
}

bool MaintenanceDrawWindow::onClickUpdate()
{

	if (SGL::ref().dialogMessage("Mise à jour", "Souhaitez-vous mettre à jour le terminal ?", GL_ICON_QUESTION, GL_BUTTON_VALID_CANCEL, GL_TIME_INFINITE) == GL_KEY_VALID)
	{
		Utils::ref().getCertificate();
		cib::communication::l_layer::LLSession *session = new cib::communication::l_layer::LLSession();
		cib::disk::deleteFile(FIC_UPDATE);
		session->setSessionData(HOSTNAME_CIBCORE, PORTNUMBER_CIBCORE, Utils::ptr()->m_cntType, "", Utils::ptr()->m_gprs);

		cib::settings::update::UpdatePackages(_ING_APPLI_FAMILY, FIC_UPDATE, *session);
	}
	return true;
}

bool MaintenanceDrawWindow::onClick(Message& msg)
{
	cib::json::Document jsonParam;
	loadDataAsJson(FIC_PARAM, jsonParam);
	int id = msg.getWidget().getId();

	if(id == 1)
	{
		onClickMaintenance();
	}
	else if(id == 2)
	{
		onClickNetwork();
	}
	else if(id == 3)
	{
		if((string)jsonParam["shopId"].as_string() == "" || !Utils::ref().isConnected)
		{
			Utils::ptr()->connectionWindow->drawing();
			if(Utils::ref().isConnected)
			{
				canDispatch = false;
			}
		}
		else if(Utils::ref().isConnected)
		{
			onClickANCV();
		}
	}
	else if(id == 4)
	{
		onClickUpdate();
	}

	return true;
}

MaintenanceDrawWindow::MaintenanceDrawWindow(GraphicLib& glib, string text) :
    BaseDrawWindow(glib, text)
{

    createSnackBar();
    cib::json::Document jsonParam;
    loadDataAsJson(FIC_PARAM, jsonParam);

    new TileButton(mainWindow,
        Utils::ref().getIconsPath("rounded"), Utils::ref().getIconsPath("maintenance_icon"), "Maintenance",
        8,74,1, this, &BaseDrawWindow::onClick);

    new TileButton(mainWindow,
    		Utils::ref().getIconsPath("rounded"), Utils::ref().getIconsPath("reseau_icon"), "Reseau",
    		        8,230,2, this, &BaseDrawWindow::onClick);

	if(!Utils::ref().isConnected)
	{
	    new TileButton(mainWindow,
	    		Utils::ref().getIconsPath("rounded_red"), Utils::ref().getIconsPath("parametre_icon"), "Connexion",160,74,3, this, &BaseDrawWindow::onClick);
	}
	else
	{
		new TileButton(mainWindow,
    		Utils::ref().getIconsPath("rounded_green"), Utils::ref().getIconsPath("parametre_icon"), "Option",160,74,3, this, &BaseDrawWindow::onClick);
	}

    new TileButton(mainWindow,
    		Utils::ref().getIconsPath("rounded"), Utils::ref().getIconsPath("ok_icon"), "Update",160,230,4, this, &BaseDrawWindow::onClick);


    mainWindow.registerMethod(GL_EVENT_KEY_DOWN, this, &MaintenanceDrawWindow::onKeyPress);
}


void MaintenanceDrawWindow::refreshInformation()
{
    cib::json::Document jsonParam;
    loadDataAsJson(FIC_PARAM, jsonParam);
    mainWindow.show();
}
// Redéfinition de la méthode drawing
bool MaintenanceDrawWindow::drawing()
{
	refreshInformation();
    canDispatch = true;

    mainWindow.show();  // Afficher la fenêtre principale
//    topLayout.show();
    hideSnackBar();

    while (canDispatch) {
//    	topLayout.show();
    	mainWindow.show();
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

bool MaintenanceDrawWindow::onKeyPress(ingenico::graphics::Message &message) {
        // Handle key press event here
        int key = message.getKey();
        if (key == GL_KEY_CANCEL || key == GL_KEY_CORRECTION)
        {
        	canDispatch = false;
        }
        return true;
}
