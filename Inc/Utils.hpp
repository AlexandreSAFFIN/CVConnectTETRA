/*
 * Utils.hpp
 *
 *  Created on: Aug 12, 2020
 *      Author: Tanguy
 */

#ifndef INC_UTILS_HPP_
#define INC_UTILS_HPP_

#include <AppResources.hpp>
#include "GL_Types.h"

#include "GL_File.h"
#include <string.h>
#include <cstring>
#include "BaseDrawWindow.hpp"
#include "PaymentChoiceDrawWindow.hpp"


class Utils : public Singleton<Utils>
{
public:
	//! \brief Constructor.
	Utils();
	//! \brief Destructor.
	virtual ~Utils(){};

	int connect();
	bool getCertificate();
	void disconnect();
	void loadData();
	void resetTerminal(const string &host);
	void mountAndRetrieveParamFile();
	bool getNewToken();
	int getTransactionState();
	bool checkQrCodeHook();
	Response createRequest(string path, eMethod method, const string &body);
	string formatDateTime(const std::string& input);

	bool launchDispatch();
	void drawActiveWindow();
	string getQrCodeTransaction(long long int price);

	void copyLogoToPinPad();
	ulong displayDualScreen(const std::string & title, const std::string & text, T_GL_DIALOG_ICON icon, T_GL_BUTTONS buttons, T_GL_DURATION duration);
	string getIconsPath(string name);


    GraphicLib glib;

	cib::communication::l_layer::LLSession m_session;
	connectionType::EconnectionType m_cntType;
	string m_host;

	bool isConnected;
	bool isFirstConnection;

	BaseDrawWindow* parameterWindow;
	BaseDrawWindow* connectionWindow;
	BaseDrawWindow* parameterOptionWindow;
	PaymentChoiceDrawWindow* paymentChoiceWindow;
	BaseDrawWindow* qrWindow;

	gprs::GprsConfig m_gprs;

};

#endif /* INC_UTILS_HPP_ */
