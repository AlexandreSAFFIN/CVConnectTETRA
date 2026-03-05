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
#include "WaitingWindow.hpp"
#include "PayIDWindow.hpp"
#include "AncvPrintTicket.hpp"
#include "SavedTransaction.hpp"


class Utils : public Singleton<Utils>
{
public:
	//! \brief Constructor.
	Utils();
	//! \brief Destructor.
	virtual ~Utils(){};

	int connect(bool isANCV = false);
	bool getCertificate();
	void disconnect();
	void loadData();
	void resetTerminal(const string &host);

	Response createRequest(string path, eMethod method, const string &body, bool isANCVRequest = false);
	string makePaymentIdWithMs();

	std::string getCaptureDate();

	void copyLogoToPinPad();
	ulong displayDualScreen(const std::string & title, const std::string & text, T_GL_DIALOG_ICON icon, T_GL_BUTTONS buttons, T_GL_DURATION duration);
	string getIconsPath(string name);
	bool checkLicense();
	bool connectWithShopId(string shopId);
	bool getHistoric(string dateFrom, string dateTo);
	string makeId(int length = 10);
	bool sendMiseEnPaiementTransac(string beneficiaryId, long long int amount);
	bool terminateTransac(bool isValid, int isPre, int paidWithCB);
	bool initQrCodePayment(long long int amount);
	int pollingTransacResult();
	int pollingPreTransacResult();
	void fillTicketTransacData(AncvConnectData &data);
	void copyFromResources(string filepathResources, string filepathToCopy);
	int createSSLProfile(string nameProfile,int count, ... );
	string getTodayDate();
	void saveTransacInProgress(SavedTransaction data);
	SavedTransaction getSaveTransac();
	void launchTransactionWithANCVParam(SavedTransaction savedTransaction);

    GraphicLib glib;

	cib::communication::l_layer::LLSession m_session;
	connectionType::EconnectionType m_cntType;
	string m_host;

	bool isConnected;
	long timeout;
	bool isSavedTransac;

	ingenico::service::Service m_serviceTransac;
	cib::transaction::TransactionLauncher m_transaction;

	BaseDrawWindow* connectionWindow;
	PaymentChoiceDrawWindow* paymentChoiceWindow;
	BaseDrawWindow* qrWindow;
	WaitingWindow* waitingWindow;
	PayIDWindow* payIdWindow;
	gprs::GprsConfig m_gprs;

};

#endif /* INC_UTILS_HPP_ */
