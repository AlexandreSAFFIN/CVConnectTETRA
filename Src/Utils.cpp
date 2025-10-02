/*
 * Utils.c
 *
 *  Created on: Aug 12, 2020
 *      Author: Tanguy
 */

#include "Utils.hpp"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <ctime>

using namespace cib;
using namespace cib::communication::http;
using namespace cib::json;

Utils::Utils()
{
}


int Utils::connect()
{
	int iRet = LL_ERROR_OK;

	if(LL_GetStatus(m_session.getSessionHandle()) != LL_STATUS_CONNECTED)
	{
		m_session.setSessionData(m_host, 443, m_cntType, "CIB", m_gprs);
		if ((iRet = m_session.configAndConnect()) != LL_ERROR_OK)
		{
			SGL::ref().dialogMessage("Erreur", "Connexion impossible", GL_ICON_ERROR, GL_BUTTON_VALID, GL_TIME_SECOND);
		}
	}


	return iRet;
}


void Utils::resetTerminal(const string &host)
{
	cib::json::Document jsonParam;

	jsonParam["connectionType"] = (int)(connectionType::IP);
	jsonParam["token"] = "";
	jsonParam["host"] = host;
    jsonParam["Acceptor"]["clientID"] = "";
    jsonParam["Acceptor"]["clientSecret"] = "";
    jsonParam["Acceptor"]["isParam"] = false;
    jsonParam["Domain"]["isParam"] = false;
    jsonParam["Acceptor"]["token"] = "Bearer 1234";
    jsonParam["Domain"]["token"] = "Bearer 1234";
    jsonParam["Domain"]["clientID"] = "";
    jsonParam["Domain"]["clientSecret"] = "";
	saveDataAsJson(FIC_PARAM, jsonParam);

	deleteFile(FIC_LICENSE);

	this->loadData();

	SGL::ref().dialogMessage("Paramétrage", "Application remise à zéro", GL_ICON_INFORMATION, GL_BUTTON_VALID, GL_TIME_INFINITE);
}

bool Utils::getCertificate()
{
	return settings::certificate::UpdateCertificate(&m_session);
}

void Utils::disconnect()
{
	if (LL_GetStatus(m_session.getSessionHandle()) == LL_STATUS_CONNECTED)
	{
		m_session.disconnect();
	}
}

void Utils::loadData()
{
	cib::json::Document jsonParam;
	loadDataAsJson(FIC_PARAM, jsonParam);

	m_host = (string)jsonParam["host"].as_string();;
	m_cntType = (connectionType::EconnectionType)(int)jsonParam["connectionType"];
	m_gprs.apn = (string)jsonParam["gprs"]["apn"].as_string();
	m_gprs.login = (string)jsonParam["gprs"]["login"].as_string();
	m_gprs.pwd = (string)jsonParam["gprs"]["password"].as_string();
}


bool Utils::getNewToken()
{
	Response response;
	string path;
	cib::json::Document jsonBody;
	cib::json::Document jsonParam;
	cib::json::Document jsonResponse;

	bool bRet = false;
	loadDataAsJson(FIC_PARAM, jsonParam);

	// Construction du corps de la requête avec les informations d'authentification
	jsonBody["grant_type"] = "client_credentials";



	// Effectuer la requête POST pour obtenir un nouveau token
	response = createRequest(path, _POST, jsonBody.serialize());
	if (response.getStatusCode() == 200)
	{
		// Parser la réponse JSON pour extraire le token
		jsonResponse.parse(response.getContent().data());


		bRet = true;
		saveDataAsJson(FIC_PARAM, jsonParam);
	}
	return bRet;
}

std::string Utils::formatDateTime(const std::string& input) {
    int year, month, day, hour, minute, second;

    // Parse manuel de la date ISO 8601
    if (sscanf(input.c_str(), "%4d-%2d-%2dT%2d:%2d:%2d", &year, &month, &day, &hour, &minute, &second) != 6) {
        return "Erreur : Format invalide";
    }

    // Construire la chaîne formatée
    char formatted[50];
    sprintf(formatted, "%02d/%02d/%04d à %02d:%02d:%02d", day, month, year, hour, minute, second);

    return std::string(formatted);
}


bool Utils::launchDispatch()
{
	Response response;
	bool bRet = false;
	cib::json::Document jsonBody;
	cib::json::Document jsonParam;
	cib::json::Document jsonResponse;

	loadDataAsJson(FIC_PARAM, jsonParam);
	response = createRequest("/tpe/private/transaction/"+(string)jsonParam["tr_token"].as_string()+"/dispatch", _POST, jsonBody.serialize());

	if (response.getStatusCode() == 200)
	{
		bRet = true;
	}

	return bRet;
}

int Utils::getTransactionState()
{
	Response response;
	int ret = 0;
	cib::json::Document jsonBody;
	cib::json::Document jsonParam;
	cib::json::Document jsonResponse;

	loadDataAsJson(FIC_PARAM, jsonParam);
	string trToken = (string)jsonParam["tr_token"].as_string();
	// Effectuer la requête POST pour obtenir un nouveau token
	string path = "/tpe/private/transaction/"+(string)jsonParam["tr_token"].as_string()+"/verify";
	response = createRequest(path, _GET, jsonBody.serialize());

	return ret;

}

bool Utils::checkQrCodeHook()
{
	Response response;
	bool bRet = false;
	cib::json::Document jsonBody;
	cib::json::Document jsonParam;
	cib::json::Document jsonResponse;

	loadDataAsJson(FIC_PARAM, jsonParam);

	response = createRequest("/tpe/private/transaction/"+(string)jsonParam["tr_token"].as_string()+"/confirm", _POST, jsonBody.serialize());

	if (response.getStatusCode() == 200)
	{
		bRet = true;
	}

	return bRet;
}

string Utils::getQrCodeTransaction(long long int price)
{
	Response response;
	cib::json::Document jsonBody;
	cib::json::Document jsonParam;
	cib::json::Document jsonResponse;

	string qrCodeContent ="";

	loadDataAsJson(FIC_PARAM, jsonParam);

	// Construction du corps de la requête avec les informations d'authentification
	jsonBody["amount"] = json::Document(json::VALUE_IS_OBJECT);
	jsonBody["amount"]["value"] = price;
	jsonBody["amount"]["currency"] = (string)jsonParam["Acceptor"]["devise"].as_string();
	jsonBody["externalId"]= Terminal::ref().TerminalType + Terminal::ref().SerialNumber;
	// Effectuer la requête POST pour obtenir un nouveau token
	response = createRequest("/tpe/private/transaction", _POST, jsonBody.serialize());
	if (response.getStatusCode() == 200)
	{
		jsonResponse.parse(response.getContent().data());
		qrCodeContent = (string)jsonResponse["qr_code"].as_string();
		jsonParam["tr_token"] = (string)jsonResponse["tr_token"].as_string();
		saveDataAsJson(FIC_PARAM, jsonParam);
	}

	return qrCodeContent;
}


Response Utils::createRequest(string path, eMethod method, const string &body)
{
	string token;


	Request request = m_session.getRequest();

	if (this->connect() == LL_ERROR_OK)
	{
		request.editRequest(method, path, body);
		if (!token.empty())
		{
			request.addHeader("Authorization", token);
		}
		if (method == _POST)
		{
			request.addHeader("Content-Type", "application/json");
		}

		m_session.setRequest(request);
		m_session.sendReceive();

		switch (m_session.getResponse().getStatusCode())
		{
		case 200:
			break;
		case 401:

			if (path.find("login") == std::string::npos && getNewToken())
			{
				this->createRequest(path, method, body);
			}
			break;
		default:
			if(path.find("verify") == std::string::npos)
			{
				SGL::ref().dialogMessage("Communication", "Une erreur est survenue", GL_ICON_WARNING, GL_BUTTON_VALID, GL_TIME_SECOND);
			}
			break;
		}
	}
	return m_session.getResponse();
}


ulong Utils::displayDualScreen(const std::string & title, const std::string & text, T_GL_DIALOG_ICON icon, T_GL_BUTTONS buttons, T_GL_DURATION duration)
{
	ulong ulRet;

	SGL::ref().openApplicationWindow();
	if (getClientGL().getViewer() == GL_VIEWER_PINPAD)
	{
		PadSGL::ref().openApplicationWindow();
		PadSGL::ref().dialogMessage(title, text, icon, buttons, duration);
		PadSGL::ref().closeApplicationWindow();
	}
	ulRet = SGL::ref().dialogMessage(title, text, icon, buttons, duration);
	SGL::ref().closeApplicationWindow();

	return ulRet;
}



string Utils::getIconsPath(string name)
{
	std::ostringstream iconPath;
	iconPath << "file://param/" << "DATA54F9250"<< ".tar/Icons/" << name << ".png";
	return iconPath.str();
}



void Utils::copyLogoToPinPad()
{
//	GL_File_Copy(Utils::ptr()->getIconsPath("cfpaylogo").c_str(), "file://flash/HOST/cfpaylogo.png");
//	GL_File_Copy(Utils::ptr()->getIconsPath("cancelicon").c_str(), "file://flash/HOST/cancelicon.png");
//	GL_File_Copy(Utils::ptr()->getIconsPath("okicon").c_str(), "file://flash/HOST/okicon.png");

	if(Ppad_IsConnected(PPAD_ID_0))
	{
		SGL::ref().injectResource("file://flash/HOST/okicon.png");
		PadSGL::ref().injectResource("file://flash/HOST/okicon.png");

		SGL::ref().injectResource("file://flash/HOST/cancelicon.png");
		PadSGL::ref().injectResource("file://flash/HOST/cancelicon.png");

		SGL::ref().injectResource("file://flash/HOST/cfpaylogo.png");
		PadSGL::ref().injectResource("file://flash/HOST/cfpaylogo.png");
	}
}

