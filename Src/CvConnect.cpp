
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
#include "TlvTree.h"
#include <memory>   // important !

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

/** Callback called when the explorer menu clicked
@param link information on menu clicked
@return error code */
Error CvConnect::onMenuClicked(SoftwareSelectedEvent & link)
{
	// Instanciate event used in the decodage
	SoftwareSelectedEvent::Instance evt = SoftwareSelectedEvent::instantiate();
	return SUCCESS;
}

void CvConnect::initMenu()
{
	m_menu.setAppName("ANCV");
	m_menu.addItem("Initialisation", this, &CvConnect::goMenu, "", 0);
	m_menu.m_explorer.addIcon("ANCV", Utils::ptr()->getIconsPath("ancvLogo"), false, 1);
}

void CvConnect::goMenu()
{
	SGL::ref().openApplicationWindow();
	cib::json::Document jsonParam;
	Utils::ref().checkLicense();
	loadDataAsJson(FIC_PARAM, jsonParam);
//	Utils::ref().isConnected = true;
	MaintenanceDrawWindow(SGL::ref(), "PARAMETRAGE").drawing();
}

void CvConnect::initTransacInterfaces()
{
	events.push_back(std::make_pair(SE_START, TXN_SERVICES_HIGH_PRIORITY));
	events.push_back(std::make_pair(SE_CHECK_PREPARE, TXN_SERVICES_HIGH_PRIORITY));
	events.push_back(std::make_pair(SE_END, TXN_SERVICES_HIGH_PRIORITY));
	m_startEndInterface = new StartEndPayment(*m_service, *new TxnStartEnd());
	interfaces.push_back(m_startEndInterface);
	Utils::ref().m_transaction.attachToService(*m_service);
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
			jsonParam["shopId"] = "";
			jsonParam["host"] = HOST_PROD;
			SavedTransaction transac = SavedTransaction();
			Utils::ref().saveTransacInProgress(transac);
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
	Utils::ptr()->connectionWindow = new ConnectDrawWindow(SGL::ref(), "AUTHENTIFICATION");
	Utils::ptr()->paymentChoiceWindow = new PaymentChoiceDrawWindow(SGL::ref(), "CHOIX DU PAIEMENT");
	Utils::ptr()->waitingWindow = new WaitingWindow(SGL::ref(), "OPERATION EN COURS\nMERCI DE PATIENTER");
	Utils::ptr()->isSavedTransac = false;
}
