
#include <CvConnect.hpp>
#include "emulation.hpp"
#include "Utils.hpp"
#include "GL_GraphicLib.h"


#define APP_NAME "CF-Pay"


using namespace cib;

//! \brief Training constructor.
//! \param[in] serviceClass The service class name.
CvConnect::CvConnect(const string &serviceClass) : Application(serviceClass, _ING_APPLI_FAMILY, _ING_APPLI_TELIUM_TETRA_PACKAGE_VERSION)
{

}

//! \brief Training destructor.
CvConnect::~CvConnect()
{
}

///////////////////////////////////////////////////////////
///////////////////////// Menu ////////////////////////////
///////////////////////////////////////////////////////////

void CvConnect::connectionType()
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
}



void CvConnect::updateApp()
{
	this->update(_ING_APPLI_FAMILY);
}

void CvConnect::update(string familyName)
{
	Utils::ref().getCertificate();
	cib::communication::l_layer::LLSession *session = new cib::communication::l_layer::LLSession();
	disk::deleteFile(FIC_UPDATE);
	if (SGL::ref().dialogMessage("Mise à jour", "Souhaitez-vous mettre à jour le terminal ?", GL_ICON_QUESTION, GL_BUTTON_VALID_CANCEL, GL_TIME_INFINITE) == GL_KEY_VALID)
	{
		session->setSessionData(HOSTNAME_CIBCORE, PORTNUMBER_CIBCORE, Utils::ptr()->m_cntType, "", Utils::ptr()->m_gprs);

		settings::update::UpdatePackages(familyName.c_str(), FIC_UPDATE, *session);
	}

}

void CvConnect::maintenance()
{
	string code;
	string package;

	SGL::ref().openApplicationWindow();
	SGL::ref().dialogText("Maintenance", "Code maintenance", "/d/d/d/d/d", code, GL_TIME_INFINITE);
	switch (atoi(code.c_str()))
	{
	case 3383: // DEVE
		SSL_DeleteProfile("CIB");
		Utils::ref().resetTerminal(HOST_DEV);
		SGL::ref().dialogMessage("Maintenance", "Environnement de DEV", GL_ICON_INFORMATION, GL_BUTTON_VALID, GL_TIME_INFINITE);
		break;

	case 7763: // PROD
		SSL_DeleteProfile("CIB");
		Utils::ref().resetTerminal(HOST_PROD);
		SGL::ref().dialogMessage("Maintenance", "Environnement de PROD", GL_ICON_INFORMATION, GL_BUTTON_VALID, GL_TIME_INFINITE);
		break;
	case 7753: // SSLD
		ssllib_open();
		if (SSL_DeleteProfile("CIB") == SSL_PROFILE_EOK)
		{
			SGL::ref().dialogMessage("Maintenance", "Certificat SSL supprime avec succes", GL_ICON_INFORMATION, GL_BUTTON_VALID, GL_TIME_INFINITE);
		}
		else
		{
			SGL::ref().dialogMessage("Maintenance", "Echec de suppression du certificat SSL", GL_ICON_ERROR, GL_BUTTON_VALID, GL_TIME_INFINITE);
		}
		ssllib_close();
		break;
	case 8737: // UPDT
		SGL::ref().dialogText("Maintenance", "Numero de package", fill("/d", 6), package, GL_TIME_INFINITE);
		this->update(package);
		break;
	case 24328:
		cib::json::Document jsonParam;
		loadDataAsJson(FIC_PARAM, jsonParam);
		jsonParam["Acceptor"]["clientID"] = "86aaa14f2d44da401056f95969589824";
		jsonParam["Acceptor"]["clientSecret"] = "519a0bc0fd3baa5b37b961cdc39e20b4865170bbf1c60bed1764db591d62f41b";
		jsonParam["Acceptor"]["devise"] = "XPF";
		jsonParam["Domain"]["clientID"] = "cf6502b4c74afccb099ec941546dca3c";
		jsonParam["Domain"]["clientSecret"] = "10d296b4f127e1998408609855a47a3633dcc5312f442a8d39abd65079522672";
		saveDataAsJson(FIC_PARAM, jsonParam);
		Utils::ref().getCertificate();
		SGL::ptr()->dialogMessage("PARAMETRAGE", "PARAMETRAGE ACCEPTEUR", GL_ICON_NONE, GL_BUTTON_NONE, GL_TIME_SECOND);
		if(Utils::ptr()->getNewToken(false))
		{
			SGL::ptr()->dialogMessage("PARAMETRAGE", "PARAMETRAGE DOMAIN", GL_ICON_NONE, GL_BUTTON_NONE, GL_TIME_SECOND);
			if(Utils::ptr()->getNewToken(true))
			{
				SGL::ptr()->dialogMessage("PARAMETRAGE", "PARAMETRAGE OK", GL_ICON_NONE, GL_BUTTON_NONE, GL_TIME_SECOND);
			}
		}
		break;
	}
	SGL::ref().closeApplicationWindow();
}

void CvConnect::addTransacDescriptors()
{
	StartEndInterface::add_descriptor(*manifest);
}

void CvConnect::reset()
{
	string code;
		SGL::ref().openApplicationWindow();
		SGL::ref().dialogText("Maintenance", "Code maintenance", "/d/d/d/d/d", code, GL_TIME_INFINITE);
		switch (atoi(code.c_str()))
		{
		case 23729:
			SGL::ref().closeApplicationWindow();
			Utils::ptr()->resetTerminal(HOST_PROD);
			break;
		default:
			SGL::ref().dialogMessage("Maintenance", "Code invalide", GL_ICON_ERROR, GL_BUTTON_VALID, GL_TIME_INFINITE);
			SGL::ref().closeApplicationWindow();
			break;

		}

}

void CvConnect::initMenu()
{
//	m_menu.setAppName(APP_NAME);
//	m_menu.addFolder("Initialisation", "", 0);
//
//	m_menu.addItem("MAJ version", this, &CfPay::updateApp, "", 1);
//	m_menu.addItem("Initialisation/Maintenance", this, &CfPay::maintenance, "", 0);
//	m_menu.addItem("Initialisation/Associer marchand", this, &CfPay::goAcceptor, "", 1);
//	m_menu.addItem("Initialisation/Associer par USB", this, &CfPay::mountUSB, "", 2);
//	m_menu.addItem("Initialisation/Dissocier marchand", this, &CfPay::reset, "", 3);
//	m_menu.addItem("Initialisation/Type connexion", this, &CfPay::connectionType, "", 4);
//	m_menu.addItem("Historique", this, &CfPay::initTransacWindow, "", 1);
//
//	m_menu.m_explorer.addIcon("CF-Pay", Utils::ptr()->getIconsPath("cfpaylogo"), false, 0);


}

void CvConnect::initTransacInterfaces()
{
	events.push_back(std::make_pair(SE_START, TXN_SERVICES_HIGH_PRIORITY));
	events.push_back(std::make_pair(SE_CHECK_PREPARE, TXN_SERVICES_HIGH_PRIORITY));
	events.push_back(std::make_pair(SE_END, TXN_SERVICES_HIGH_PRIORITY));
	m_startEndInterface = new StartEndPayment(*m_service, *new TxnStartEnd());
	interfaces.push_back(m_startEndInterface);

	this->registerInterface(interfaces, events);
}

void CvConnect::initDisk()
{
	Utils::ptr()->m_token_acceptor="Bearer 1234";
	Utils::ptr()->m_token_domain= "Bearer 1234";

	if (disk::diskCreate(DISK_PATH) == DISK_CREATED)
	{
		if (disk::getFileSize(FIC_PARAM) <= 0)
		{
			cib::json::Document jsonParam;
			jsonParam["connectionType"] = (int)(connectionType::IP);
			jsonParam["host"] = HOST_PROD;
			saveDataAsJson(FIC_PARAM, jsonParam);
		}
	}
}

void CvConnect::initApp()
{
	Utils::ptr()->glib.setCharset(GL_ENCODING_ISO_8859_1);
	cib::json::Document jsonParam;
	initDisk();
	Utils::ptr()->copyLogoToPinPad();
	Utils::ptr()->loadData();
	Utils::ptr()->m_session.setSessionData(Utils::ptr()->m_host, 443, Utils::ptr()->m_cntType, "CIB", Utils::ptr()->m_gprs);
//	Utils::ptr()->acceptorWindow = new DrawWindowInitialisation(Utils::ptr()->glib);
//	Utils::ptr()->paymentWindow = new PaymentWindow(SGL::ref(), PadSGL::ref());
}
