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
#include <string>
#include <vector>
#include <cctype>
#include <cstring>
#include <sys/time.h>

using namespace cib;
using namespace cib::communication::http;
using namespace cib::json;

//// --- helper: base64 char -> valeur 0..63, -1 sinon
static int b64idx(unsigned char c)
{
    if (c >= 'A' && c <= 'Z') return c - 'A';
    if (c >= 'a' && c <= 'z') return c - 'a' + 26;
    if (c >= '0' && c <= '9') return c - '0' + 52;
    if (c == '+') return 62;
    if (c == '/') return 63;
    return -1;
}

// --- base64 decode (C++98)
static std::vector<unsigned char> base64Decode(const std::string& in)
{
    std::vector<unsigned char> out;
    out.reserve(in.size() * 3 / 4);

    int val = 0;       // accumulateur 24 bits
    int valb = -8;     // combien de bits valides dans val (-8, -2, 4, ...)
    int pad = 0;

    for (size_t i = 0; i < in.size(); ++i)
    {
        unsigned char c = (unsigned char)in[i];
        if (isspace(c)) continue; // ignore espaces/retours ligne

        if (c == '=') { pad++; continue; }

        int d = b64idx(c);
        if (d < 0) {
            // ignore les caractères non base64 (utile si header data:...)
            continue;
        }

        val = (val << 6) | d;
        valb += 6;
        if (valb >= 0) {
            unsigned char byte = (unsigned char)((val >> valb) & 0xFF);
            out.push_back(byte);
            valb -= 8;
        }
    }

    // gérer padding (=) : retirer 1 ou 2 bytes si nécessaire
    if (pad) {
        // Le padding signifie que les derniers octets sortis sont en trop
        // pad==1 -> enlever 1 octet ; pad==2 -> enlever 2 octets (si présents)
        while (pad-- > 0 && !out.empty()) out.pop_back();
    }
    return out;
}
//
//// --- enlève un éventuel préfixe "data:image/png;base64,..." ---
static std::string stripDataUrlPrefix(const std::string& b64)
{
    // Cherche "base64," et renvoie la partie après
    const std::string key = "base64,";
    std::string::size_type pos = b64.find(key);
    if (pos != std::string::npos) {
        return b64.substr(pos + key.size());
    }
    return b64;
}

// --- Sauvegarde le PNG à partir d'un base64 (retourne true si OK) ---
static bool saveImageBase64ToPng(const std::string& imageBase64, const char* outPath)
{
	std::vector<unsigned char> png = base64Decode(imageBase64);

	T_GL_HFILE myfile = GL_File_Open(outPath, GL_FILE_CREATE_ALWAYS, GL_FILE_ACCESS_READ_WRITE);
	if (!myfile) return false;

	// 3) écrire (attention si l’API peut écrire partiellement)
	const unsigned char* p = png.data();
	int remaining = static_cast<int>(png.size());
	int totalWritten = 0;

	while (remaining > 0) {
		int n = GL_File_Write(myfile, p + totalWritten, remaining);
		if (n <= 0) break; // erreur d'écriture
		totalWritten += n;
		remaining    -= n;
	}

	// 4) flush & close
	if (totalWritten > 0) {
		GL_File_Flush(myfile);
	}
	GL_File_Close(myfile);

	return totalWritten > 0;
}

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



std::string Utils::makeId(int length)
{
    static const char characters[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789";

    const int charCount = sizeof(characters) - 1; // -1 pour ignorer '\0'
    std::string result;
    result.reserve(length);

    for (int i = 0; i < length; ++i)
    {
        int index = rand() % charCount;
        result += characters[index];
    }

    return result;
}

std::string Utils::makePaymentIdWithMs()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);

    // conversion en date/heure locale
    struct tm* tm_info = localtime(&tv.tv_sec);

    std::ostringstream oss;
    oss << "PAY-"
        << std::setfill('0') << std::setw(4) << (tm_info->tm_year + 1900)
        << std::setfill('0') << std::setw(2) << (tm_info->tm_mon + 1)
        << std::setfill('0') << std::setw(2) << tm_info->tm_mday
        << std::setfill('0') << std::setw(2) << tm_info->tm_hour
        << std::setfill('0') << std::setw(2) << tm_info->tm_min
        << std::setfill('0') << std::setw(2) << tm_info->tm_sec
        << std::setfill('0') << std::setw(3) << (tv.tv_usec / 1000); // ms

    return oss.str();
}

std::string Utils::getCaptureDate()
{
    // Obtenir le temps actuel
    std::time_t now = std::time(NULL);

    // Ajouter 1 jour (24h * 3600s)
    now += 24 * 60 * 60;

    // Convertir en UTC
    struct tm gmt;
#if defined(_WIN32)
    gmtime_s(&gmt, &now); // Windows
#else
    gmtime_r(&now, &gmt); // POSIX
#endif

    // Formatter manuellement
    static const char* DAYS[]   = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
    static const char* MONTHS[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
                                   "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

    std::ostringstream oss;
    oss << DAYS[gmt.tm_wday] << ", "
        << std::setw(2) << std::setfill('0') << gmt.tm_mday << " "
        << MONTHS[gmt.tm_mon] << " "
        << (1900 + gmt.tm_year) << " "
        << std::setw(2) << std::setfill('0') << gmt.tm_hour << ":"
        << std::setw(2) << std::setfill('0') << gmt.tm_min  << ":"
        << std::setw(2) << std::setfill('0') << gmt.tm_sec
        << " GMT";

    return oss.str();
}

bool Utils::checkIfStringBullshitApiIsValid(string value)
{
	return !value.empty()
	        && value != "null"
	        && value != "error";
}

bool Utils::checkIfIntBullshitApiIsValid(int value)
{
	return value;
}


Response Utils::createRequest(string path, eMethod method, const string &body)
{
	Request request = m_session.getRequest();
	waitingWindow->drawing();
	if (this->connect() == LL_ERROR_OK)
	{
		request.editRequest(method, path, body);
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
			break;
		default:
			SGL::ref().dialogMessage("Communication", "Une erreur est survenue", GL_ICON_WARNING, GL_BUTTON_VALID, GL_TIME_SECOND);
			break;
		}
	}
	waitingWindow->hidding();
	return m_session.getResponse();
}

bool Utils::initQrCodePayment(long long int amount)
{
	bool bRet = false;
	Response response;
	cib::json::Document jsonBody;
	cib::json::Document jsonParam;
	cib::json::Document jsonResponse;

	loadDataAsJson(FIC_PARAM, jsonParam);
	if(jsonParam["shopId"] && jsonParam["shopId"] != "")
	{
		string id = makeId();
		// Construction du corps de la requête avec les informations d'authentification
		jsonBody["ParamInitAndGetQRPreTransac"] = json::Document(json::VALUE_IS_OBJECT);
		jsonBody["ParamInitAndGetQRPreTransac"]["shopId"] = (string)jsonParam["shopId"].as_string();
		jsonBody["ParamInitAndGetQRPreTransac"]["expirationDate"] = getCaptureDate();
		jsonBody["ParamInitAndGetQRPreTransac"]["order"] = json::Document(json::VALUE_IS_OBJECT);
		jsonBody["ParamInitAndGetQRPreTransac"]["order"]["ClaANCVReturnedOrder"] = json::Document(json::VALUE_IS_OBJECT);
		jsonBody["ParamInitAndGetQRPreTransac"]["order"]["ClaANCVReturnedOrder"]["id"] = id;
		jsonBody["ParamInitAndGetQRPreTransac"]["order"]["ClaANCVReturnedOrder"]["paymentId"] = makePaymentIdWithMs();
		jsonBody["ParamInitAndGetQRPreTransac"]["order"]["ClaANCVReturnedOrder"]["amount"] = json::Document(json::VALUE_IS_OBJECT);
		jsonBody["ParamInitAndGetQRPreTransac"]["order"]["ClaANCVReturnedOrder"]["amount"]["ClaANCVReturnedAmount"] = json::Document(json::VALUE_IS_OBJECT);
		jsonBody["ParamInitAndGetQRPreTransac"]["order"]["ClaANCVReturnedOrder"]["amount"]["ClaANCVReturnedAmount"]["total"] = amount;
		jsonBody["ParamInitAndGetQRPreTransac"]["order"]["ClaANCVReturnedOrder"]["amount"]["ClaANCVReturnedAmount"]["currency"] = "978";
		jsonBody["ParamInitAndGetQRPreTransac"]["paymentMethod"] = json::Document(json::VALUE_IS_OBJECT);
		jsonBody["ParamInitAndGetQRPreTransac"]["paymentMethod"]["ClaANCVReturnedPaymentMethod"] = json::Document(json::VALUE_IS_OBJECT);
		jsonBody["ParamInitAndGetQRPreTransac"]["paymentMethod"]["ClaANCVReturnedPaymentMethod"]["tspdMode"] = "001";
		jsonBody["ParamInitAndGetQRPreTransac"]["paymentMethod"]["ClaANCVReturnedPaymentMethod"]["captureMode"] = "NORMAL";
		jsonBody["ParamInitAndGetQRPreTransac"]["paymentMethod"]["ClaANCVReturnedPaymentMethod"]["captureTerm"] = 1;
		// Effectuer la requête POST pour obtenir un nouveau token
		response = createRequest("/InitAndGetQRPreTransac", _POST, jsonBody.serialize());
		if (response.getStatusCode() == 200)
		{
			jsonResponse.parse(response.getContent().data());
			if(jsonResponse["QRCodeReturned"] && jsonResponse["QRCodeReturned"]["InitAndGetQRPreTransacResult"])
			{
				bRet = saveImageBase64ToPng((string)jsonResponse["QRCodeReturned"]["InitAndGetQRPreTransacResult"]["ImageBase64"].as_string(), "file://flash/HOST/QRCODE.png");
			}
		}
	}

	return bRet;
}

bool Utils::sendMiseEnPaiementTransac(string beneficiaryId, long long int amount)
{
	bool bRet = false;
	Response response;
	cib::json::Document jsonBody;
	cib::json::Document jsonParam;
	cib::json::Document jsonResponse;

	loadDataAsJson(FIC_PARAM, jsonParam);
	if(jsonParam["shopId"] && jsonParam["shopId"] != "")
	{
		string id = makeId();
		// Construction du corps de la requête avec les informations d'authentification
		jsonBody["ParamInitAndMiseEnPaiement"] = json::Document(json::VALUE_IS_OBJECT);
		jsonBody["ParamInitAndMiseEnPaiement"]["beneficiaryId"] = beneficiaryId;
		jsonBody["ParamInitAndMiseEnPaiement"]["shopId"] = (string)jsonParam["shopId"].as_string();
		jsonBody["ParamInitAndMiseEnPaiement"]["order"] = json::Document(json::VALUE_IS_OBJECT);
		jsonBody["ParamInitAndMiseEnPaiement"]["order"]["ClaANCVReturnedOrder"] = json::Document(json::VALUE_IS_OBJECT);
		jsonBody["ParamInitAndMiseEnPaiement"]["order"]["ClaANCVReturnedOrder"]["id"] = id;
		jsonBody["ParamInitAndMiseEnPaiement"]["order"]["ClaANCVReturnedOrder"]["paymentId"] = makePaymentIdWithMs();
		jsonBody["ParamInitAndMiseEnPaiement"]["order"]["ClaANCVReturnedOrder"]["amount"] = json::Document(json::VALUE_IS_OBJECT);
		jsonBody["ParamInitAndMiseEnPaiement"]["order"]["ClaANCVReturnedOrder"]["amount"]["ClaANCVReturnedAmount"] = json::Document(json::VALUE_IS_OBJECT);
		jsonBody["ParamInitAndMiseEnPaiement"]["order"]["ClaANCVReturnedOrder"]["amount"]["ClaANCVReturnedAmount"]["total"] = amount;
		jsonBody["ParamInitAndMiseEnPaiement"]["order"]["ClaANCVReturnedOrder"]["amount"]["ClaANCVReturnedAmount"]["currency"] = "978";
		jsonBody["ParamInitAndMiseEnPaiement"]["paymentMethod"] = json::Document(json::VALUE_IS_OBJECT);
		jsonBody["ParamInitAndMiseEnPaiement"]["paymentMethod"]["ClaANCVReturnedPaymentMethod"] = json::Document(json::VALUE_IS_OBJECT);
		jsonBody["ParamInitAndMiseEnPaiement"]["paymentMethod"]["ClaANCVReturnedPaymentMethod"]["tspdMode"] = "001";
		jsonBody["ParamInitAndMiseEnPaiement"]["paymentMethod"]["ClaANCVReturnedPaymentMethod"]["captureMode"] = "NORMAL";
		jsonBody["ParamInitAndMiseEnPaiement"]["paymentMethod"]["ClaANCVReturnedPaymentMethod"]["captureDate"] = getCaptureDate();
		// Effectuer la requête POST pour obtenir un nouveau token
		response = createRequest("/InitAndMiseEnPaiementTransac", _POST, jsonBody.serialize());
		if (response.getStatusCode() == 200)
		{
			jsonResponse.parse(response.getContent().data());
			if(jsonResponse["MitEnPaiement"] && (string)jsonResponse["MitEnPaiement"]["InitAndMiseEnPaiementTransacResult"].as_string() == "OK")
			{
				jsonParam["orderId"] = id;
				saveDataAsJson(FIC_PARAM, jsonParam);
				bRet = true;
			}
		}
	}

	return bRet;
}

bool Utils::pollingPreTransacResult(string orderId)
{
	bool bRet = false;
	Response response;
	cib::json::Document jsonBody;
	cib::json::Document jsonParam;
	cib::json::Document jsonResponse;

	loadDataAsJson(FIC_PARAM, jsonParam);
	if(jsonParam["shopId"] && jsonParam["shopId"] != "")
	{

		jsonBody["id"] = orderId;

		// Effectuer la requête POST pour obtenir un nouveau token
		response = createRequest("/PollingPreTransac", _POST, jsonBody.serialize());
		if (response.getStatusCode() == 200)
		{
			jsonResponse.parse(response.getContent().data());
			if(jsonResponse["PollingPreTransacResult"])
			{
				string etat = (string)jsonResponse["PollingPreTransacResult"]["etat"].as_string();
				string beneficiaryId = (string)jsonResponse["PollingPreTransacResult"]["beneficiary_id"].as_string();

				if(jsonResponse["PollingPreTransacResult"]["total"])
				{
					if(checkIfStringBullshitApiIsValid(etat) && checkIfStringBullshitApiIsValid(beneficiaryId))
					{
						jsonParam["beneficiaryId"] = beneficiaryId;
						jsonParam["lastState"] = etat;
						saveDataAsJson(FIC_PARAM, jsonParam);
						bRet = true;
					}
				}

			}
		}
	}

	return bRet;
}

bool Utils::pollingTransacResult(string orderId)
{
	bool bRet = false;
	Response response;
	cib::json::Document jsonBody;
	cib::json::Document jsonParam;
	cib::json::Document jsonResponse;

	loadDataAsJson(FIC_PARAM, jsonParam);
	if(jsonParam["shopId"] && jsonParam["shopId"] != "")
	{

		jsonBody["id"] = orderId;

		// Effectuer la requête POST pour obtenir un nouveau token
		response = createRequest("/PollingTransac", _POST, jsonBody.serialize());
		if (response.getStatusCode() == 200)
		{
			jsonResponse.parse(response.getContent().data());
			if(jsonResponse["ResponsePollingTransac"] && jsonResponse["ResponsePollingTransac"]["PollingTransacResult"])
			{
				string etat = (string)jsonResponse["ResponsePollingTransac"]["PollingTransacResult"]["etat"].as_string();
				string beneficiaryId = (string)jsonResponse["ResponsePollingTransac"]["PollingTransacResult"]["beneficiary_id"].as_string();

				if(jsonResponse["ResponsePollingTransac"]["PollingTransacResult"]["total"])
				{
					if(checkIfStringBullshitApiIsValid(etat) && checkIfStringBullshitApiIsValid(beneficiaryId))
					{
						jsonParam["beneficiaryId"] = beneficiaryId;
						jsonParam["lastState"] = etat;
						saveDataAsJson(FIC_PARAM, jsonParam);
						bRet = true;
					}
				}

			}
		}
	}

	return bRet;
}

bool Utils::terminateTransac(string orderId, bool isValid)
{
	bool bRet = false;
	Response response;
	cib::json::Document jsonBody;
	cib::json::Document jsonParam;
	cib::json::Document jsonResponse;

	loadDataAsJson(FIC_PARAM, jsonParam);
	if(jsonParam["shopId"] && jsonParam["shopId"] != "")
	{
		if(isValid)
		{
			jsonBody["ParamValidateTransaction"] = json::Document(json::VALUE_IS_OBJECT);
			jsonBody["ParamValidateTransaction"]["id"] = jsonParam["orderId"];
			jsonBody["ParamValidateTransaction"]["pre_or_transac"] = 1;
			jsonBody["ParamValidateTransaction"]["paidWithCB"] = 0;

			response = createRequest("/ValidateTransaction", _POST, jsonBody.serialize());
		}
		else
		{
			jsonBody["ParamValidateTransaction"] = json::Document(json::VALUE_IS_OBJECT);
			jsonBody["ParamValidateTransaction"]["id"] = jsonParam["orderId"];
			jsonBody["ParamValidateTransaction"]["pre_or_transac"] = 1;
			jsonBody["ParamValidateTransaction"]["reason"] = "ABORTED_MERCHANT";
			jsonBody["ParamValidateTransaction"]["cancelOrigine"] = 0;

			response = createRequest("/AnnuleTransacPreTransac", _POST, jsonBody.serialize());
		}

		if (response.getStatusCode() == 200)
		{
			jsonResponse.parse(response.getContent().data());
			if(isValid)
			{
				if(jsonResponse["BodyValidateTransaction"])
				{
					if(jsonResponse["BodyValidateTransaction"]["ValidateTransactionResult"] && jsonResponse["BodyValidateTransaction"]["beneficiary_id"] == (string)jsonParam["beneficiaryId"].as_string())
					{
						bRet = (bool)jsonResponse["BodyValidateTransaction"]["ValidateTransactionResult"].as_bool();
					}
				}

			}
			else
			{

				if(jsonResponse["BodyAnnulationPreTransac"])
				{
					if(jsonResponse["BodyAnnulationPreTransac"]["AnnuleTransacPreTransacResult"])
					{
						bRet = (bool)jsonResponse["BodyAnnulationPreTransac"]["AnnuleTransacPreTransacResult"].as_bool();
					}
				}

			}
		}

	}
	return bRet;
}

bool Utils::checkLicense()
{
	bool bRet = false;
	Response response;
	cib::json::Document jsonBody;
	cib::json::Document jsonParam;
	cib::json::Document jsonResponse;

	loadDataAsJson(FIC_PARAM, jsonParam);
	if(jsonParam["shopId"] && jsonParam["shopId"] != "")
	{
		// Construction du corps de la requête avec les informations d'authentification
		jsonBody["RequestHasLicense"] = json::Document(json::VALUE_IS_OBJECT);
		jsonBody["RequestHasLicense"]["serialNumber"] = Terminal::ref().SerialNumber;
		jsonBody["RequestHasLicense"]["typeAppli"] = 0;
		jsonBody["RequestHasLicense"]["typePeriph"] = Terminal::ref().TerminalType;
		jsonBody["RequestHasLicense"]["shopId"] = (string)jsonParam["shopId"].as_string();

		// Effectuer la requête POST pour obtenir un nouveau token
		response = createRequest("/checkLicence", _POST, jsonBody.serialize());
		if (response.getStatusCode() == 200)
		{
			jsonResponse.parse(response.getContent().data());
			if(jsonResponse["ResponseHasValidLicense"] && (string)jsonResponse["ResponseHasValidLicense"]["checkLicenceResult"].as_string() == "VALID")
			{
				bRet = true;
			}
		}
	}

	return bRet;
}

bool Utils::connectWithShopId(string shopId)
{
	bool bRet = false;
	Response response;
	cib::json::Document jsonBody;
	cib::json::Document jsonParam;
	cib::json::Document jsonResponse;

	loadDataAsJson(FIC_PARAM, jsonParam);
		// Effectuer la requête POST pour obtenir un nouveau token
	response = createRequest("/GetShopId/"+Terminal::ref().SerialNumber, _GET, "");
	if (response.getStatusCode() == 200)
	{
		jsonResponse.parse(response.getContent().data());
		if(jsonResponse["ResponseGetShopId"] && jsonResponse["ResponseGetShopId"]["GetShopIdResult"])
		{
			jsonParam["shopId"] = (string)jsonResponse["ResponseGetShopId"]["GetShopIdResult"].as_string();
			bRet = true;
		}
	}
	saveDataAsJson(FIC_PARAM, jsonParam);
	return bRet;
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
	GL_File_Copy(Utils::ptr()->getIconsPath("valid").c_str(), "file://flash/HOST/valid.png");
	GL_File_Copy(Utils::ptr()->getIconsPath("cancelicon").c_str(), "file://flash/HOST/cancel.png");
	GL_File_Copy(Utils::ptr()->getIconsPath("ancvlogomini").c_str(), "file://flash/HOST/logo.png");
	GL_File_Copy(Utils::ptr()->getIconsPath("backgroundtpe").c_str(), "file://flash/HOST/background.png");
//	GL_File_Copy("/CVCONNECT/QRCODE.png", "file://flash/HOST/QRCODE.png");

	if(Ppad_IsConnected(PPAD_ID_0))
	{
		SGL::ref().injectResource("file://flash/HOST/valid.png");
		PadSGL::ref().injectResource("file://flash/HOST/valid.png");

		SGL::ref().injectResource("file://flash/HOST/cancel.png");
		PadSGL::ref().injectResource("file://flash/HOST/cancel.png");

		SGL::ref().injectResource("file://flash/HOST/logo.png");
		PadSGL::ref().injectResource("file://flash/HOST/logo.png");

		SGL::ref().injectResource("file://flash/HOST/background.png");
		PadSGL::ref().injectResource("file://flash/HOST/background.png");
	}
}

