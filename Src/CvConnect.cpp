
#include <CvConnect.hpp>
#include "emulation.hpp"
#include "Utils.hpp"
#include "GL_GraphicLib.h"
#include "MaintenanceDrawWindow.hpp"
#include "ANCVDrawWindow.hpp"
#include "ConnectDrawWindow.hpp"
#include "PayIDWindow.hpp"
#include "WaitingWindow.hpp"
#include "PaymentChoiceDrawWindow.hpp"
#include "SSL_.h"

#define APP_NAME "CvConnect"


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
		SSL_DeleteProfile("ANCV");
		Utils::ref().resetTerminal(HOST_DEV);
		SGL::ref().dialogMessage("Maintenance", "Environnement de DEV", GL_ICON_INFORMATION, GL_BUTTON_VALID, GL_TIME_INFINITE);
		break;

	case 7763: // PROD
		SSL_DeleteProfile("CIB");
		SSL_DeleteProfile("ANCV");
		Utils::ref().resetTerminal(HOST_PROD);
		SGL::ref().dialogMessage("Maintenance", "Environnement de PROD", GL_ICON_INFORMATION, GL_BUTTON_VALID, GL_TIME_INFINITE);
		break;
	case 7753: // SSLD
		ssllib_open();
		if (SSL_DeleteProfile("CIB") == SSL_PROFILE_EOK)
		{
			SSL_DeleteProfile("ANCV");
			SGL::ref().dialogMessage("Maintenance", "Certificat SSL supprime avec succes", GL_ICON_INFORMATION, GL_BUTTON_VALID, GL_TIME_INFINITE);
		}
		else
		{
			SGL::ref().dialogMessage("Maintenance", "Echec de suppression du certificat SSL", GL_ICON_ERROR, GL_BUTTON_VALID, GL_TIME_INFINITE);
		}
		ssllib_close();
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

	m_menu.setAppName("ANCV");
//	m_menu.addItem("MAJ version", this, &CfPay::updateApp, "", 1);
//	m_menu.addItem("Initialisation/Maintenance", this, &CfPay::maintenance, "", 0);
	m_menu.addItem("Initialisation", this, &CvConnect::goMenu, "", 0);
//	m_menu.addItem("Initialisation/Associer par USB", this, &CfPay::mountUSB, "", 2);
//	m_menu.addItem("Initialisation/Dissocier marchand", this, &CfPay::reset, "", 3);
//	m_menu.addItem("Initialisation/Type connexion", this, &CfPay::connectionType, "", 4);
//	m_menu.addItem("Historique", this, &CfPay::initTransacWindow, "", 1);
//
//	m_menu.m_explorer.addIcon(APP_NAME, Utils::ptr()->getIconsPath("cfpaylogo"), true, 0);
	m_menu.m_explorer.addIcon("ANCV", Utils::ptr()->getIconsPath("ancvlogomini"), false, 0);


}

void CvConnect::goMenu()
{
	SGL::ref().openApplicationWindow();
	if(!Utils::ref().isConnected)
	{
		Utils::ptr()->connectionWindow->drawing();
	}

	if(Utils::ref().isConnected)
	{
		Utils::ptr()->parameterWindow->drawing();
	}
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

	if (disk::diskCreate(DISK_PATH) == DISK_CREATED)
	{
		if (disk::getFileSize(FIC_PARAM) <= 0)
		{
			cib::json::Document jsonParam;
			jsonParam["connectionType"] = (int)(connectionType::IP);
			jsonParam["ANCVOnly"] = false;
			jsonParam["shopid"] = "";
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
	Utils::ptr()->m_session.setSessionData("boancv.alpigreen.com", 443, Utils::ptr()->m_cntType, "CIB", Utils::ptr()->m_gprs);
	Utils::ptr()->parameterWindow = new MaintenanceDrawWindow(SGL::ref(), "PARAMETRAGE");
	Utils::ptr()->parameterOptionWindow = new ANCVDrawWindow(SGL::ref(), "OPTION ANCV");
	Utils::ptr()->connectionWindow = new ConnectDrawWindow(SGL::ref(), "AUTHENTIFICATION");
	Utils::ptr()->paymentChoiceWindow = new PaymentChoiceDrawWindow(SGL::ref(), "CHOIX DU PAIEMENT");
	Utils::ptr()->waitingWindow = new WaitingWindow(SGL::ref(), "OPERATION EN COURS\nMERCI DE PATIENTER");
	Utils::ptr()->isConnected = Utils::ref().checkLicense();


//	std::ostringstream iconPath2;
//		iconPath2 << "file://param/" << "DATA54F9250"<< ".tar/Icons/" << "ca1.pem";
//
//		int sizeCsr = File::getSizeFile(iconPath2.str().c_str());
//		if (sizeCsr > 0) {
//			std::vector<char> bufferCsr(sizeCsr);
//			loadData(iconPath2.str().c_str(), bufferCsr.data(), sizeCsr);
//
//			if (disk::saveData("/CVCONNECT/ca1.pem", bufferCsr.data(), sizeCsr, 1) > 0) {
//				int a = 0; // succès
//			}
//		}
//
//		std::ostringstream iconPath3;
//		iconPath3 << "file://param/" << "DATA54F9250"<< ".tar/Icons/" << "ca2.pem";
//
//		sizeCsr = File::getSizeFile(iconPath3.str().c_str());
//		if (sizeCsr > 0) {
//			std::vector<char> bufferCsr(sizeCsr);
//			loadData(iconPath3.str().c_str(), bufferCsr.data(), sizeCsr);
//			int t = 0;
//			if (t = disk::saveData("/CVCONNECT/ca2.pem", bufferCsr.data(), sizeCsr, 1) > 0) {
//				int a = 0; // succès
//			}
//		}
//
//	SSL_PROFILE_HANDLE h = SSL_LoadProfile("CIB");
//	Utils::ptr()->m_session.setSessionData("boancv.alpigreen.com", 443, Utils::ptr()->m_cntType, "CIB", Utils::ptr()->m_gprs);
//	Utils::ref().createRequest("/Webservices/rest/FO//GetShopId/123456789", _POST, "" );

//	addProfileCaOnly("ANCVE", "/CVCONNECT/client.pem",  "/CVCONNECT/ca1.pem","/CVCONNECT/caC.pem");


}
