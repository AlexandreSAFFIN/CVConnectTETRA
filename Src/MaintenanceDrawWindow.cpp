#include "MaintenanceDrawWindow.hpp"
#include "Utils.hpp"
#include "AppResources.hpp"
#include "cib/disk/Disk.hpp"
#include "TileButton.hpp"


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
    return true;  // Événement géré
}

bool MaintenanceDrawWindow::onClickANCV()
{
	Utils::ptr()->parameterOptionWindow->drawing();
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
		onClickANCV();
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

    new TileButton(mainWindow,
        Utils::ref().getIconsPath("rounded"), Utils::ref().getIconsPath("pay"), "Maintenance",
        8,74,1, this, &BaseDrawWindow::onClick);

    new TileButton(mainWindow,
    		Utils::ref().getIconsPath("rounded"), Utils::ref().getIconsPath("pay"), "Reseau",
    		        8,230,2, this, &BaseDrawWindow::onClick);

    new TileButton(mainWindow,
    		Utils::ref().getIconsPath("rounded"), Utils::ref().getIconsPath("pay"), "Option",160,74,3, this, &BaseDrawWindow::onClick);

    new TileButton(mainWindow,
    		Utils::ref().getIconsPath("rounded"), Utils::ref().getIconsPath("pay"), "Update",160,230,4, this, &BaseDrawWindow::onClick);


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
