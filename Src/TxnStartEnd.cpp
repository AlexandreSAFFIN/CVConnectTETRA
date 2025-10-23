/**
 * \file    TxnStartEnd.cpp
 * \author  Kassovic
 * \date    31 June 2016
 * \brief   This file manages the start and the end of a transaction.
 *
 * \details 053116-BK : File created.
 */

#include <PaymentTransacWindow.hpp>
#include "TxnStartEnd.hpp"
#include "Transaction.hpp"
#include "AppResources.hpp"
#include "PaymentChoiceDrawWindow.hpp"
#include "PaymentQRWindow.hpp"
#include "PayIDWindow.hpp"
#include "Utils.hpp"
#include "PrintTicketWindow.hpp"
#include "YesNoWindow.hpp"
#include "AncvPrintTicket.hpp"
//                            #####################
//                            #   TXN START END   #
//                            #####################

//! \brief TxnStartEnd constructor.
//! \param[in] graphicLib A reference to the graphic library instance.
TxnStartEnd::TxnStartEnd()
{
}

//! \brief TxnStartEnd destructor.
TxnStartEnd::~TxnStartEnd()
{
}

//! \brief Indicates that the transaction is started.
//! \details At this step, transaction data can be updated.
//! \details Call type : broadcast.
//! \param[in] inputData May have been updated by a previously called application.
//! \details TAG_TXN_STRUCT_INPUT_DATA_LOG : present only if input data have been modified.
//! \details - list of TAG_TXN_STRUCT_LOG_ENTRY : contains the modification done bye one application.
//! \details -- TAG_TXN_SERVICE_CLASS_NAME : the application class name that has changed the input data.
//! \details -- TAG_TXN_STRUCT_PREVIOUS_TAGS : the application id that has changed the inpud data.
//! \details -- TAG_TXN_STRUCT_PREVIOUS_TAGS : modified tags with their previous value.
//! \details --- list of modified tags.
//! \details -- TAG_TXN_STRUCT_ADDED_TAGS : added tags (without value).
//! \details --- list of added tags.
//! \param[out] outputData No output data or list of updated input tags.
//! \return TXN_SR_OK.
int TxnStartEnd::start(const TLV_TREE_NODE inputData, TLV_TREE_NODE outputData)
{
	// Get transaction info
	TransactionInfo txn;
	getTransactionInfo(inputData, txn);
	m_transaction = new Transaction();
	m_transaction->isANCVTransac = false;
	m_transaction->isCB = true;

	cib::json::Document jsonParam;
	loadDataAsJson(FIC_PARAM, jsonParam);

	bool isParam = Utils::ref().isConnected;
	if(!isParam)
	{
		Utils::ref().checkLicense();
	}
	Utils::ref().timeout = 3000;

//	isParam = true;
	bool isANCV = false;
	int amountToComplete = 0;

	if(isParam)
	{
		isANCV = Utils::ref().paymentChoiceWindow->drawing();
	}

	long long int amount = atoll((txn.amount).c_str());
	if((!(atoll((txn.amount).c_str()) > 2147483647) && txn.txnType == TXN_TRANSACTION_TYPE_DEBIT && isANCV))
	{
		jsonParam["amountToPay"] = amount;
		jsonParam["toComplete"] = 0;
		saveDataAsJson(FIC_PARAM, jsonParam);
		PaymentQRWindow* pw;
		if(Utils::ref().paymentChoiceWindow->getPM() == QRCODE)
		{
			if(Utils::ref().initQrCodePayment(amount))
			{
				pw = new PaymentQRWindow(Utils::ref().glib,PadSGL::ref(), "MOYEN DE PAIEMENT", amount, txn.amount);
				if(pw->drawing())
				{
					loadDataAsJson(FIC_PARAM, jsonParam);
					amountToComplete = (int)jsonParam["toComplete"].as_int();
					m_transaction->isANCVTransac = true;
					m_transaction->isCB = false;
					m_transaction->isPre = 0;
				}
				free(pw);
			}
		}
		else
		{
			Utils::ref().payIdWindow = new PayIDWindow(Utils::ref().glib,PadSGL::ref(), "MOYEN DE PAIEMENT", amount);
			if(Utils::ref().payIdWindow->drawing())
			{
				PaymentPreTransacWindow* ppw = new PaymentPreTransacWindow(Utils::ref().glib, PadSGL::ref(), "PAIEMENT ANCV");
				if(ppw->drawing())
				{
					loadDataAsJson(FIC_PARAM, jsonParam);
					amountToComplete = (int)jsonParam["toComplete"].as_int();
					m_transaction->isANCVTransac = true;
					m_transaction->isCB = false;
					m_transaction->isPre = 1;
				}
			}
			free(Utils::ref().payIdWindow);
		}

		if(m_transaction->isANCVTransac)
		{
			if(!(bool)jsonParam["ANCVOnly"].as_bool() && amountToComplete > 0)
			{
				if(!YesNoWindow(Utils::ref().glib, "",amountToComplete).drawing())
				{
					amountToComplete = 0;
					m_transaction->isANCVTransac = false;
					Utils::ref().waitingWindow->drawing("Annulation en cours", Waiting);
					Utils::ref().terminateTransac(false, m_transaction->isPre, amountToComplete);
					Utils::ref().waitingWindow->hidding();
				}
				else
				{
					m_transaction->isCB = true;
				}
			}
		}

		if(amountToComplete > 0)
		{
			m_transaction->updateTransactionInfo(outputData,amountToComplete);
			updateTransactionInfo(outputData, amountToComplete, NULL, NULL, NULL);
		}
		else
		{
			unsigned long readerDetected = TXN_TECHNO_READER_DETECTED;
			updateTransactionInfo(outputData, 0, NULL, NULL, &readerDetected);
		}
	}

	return TXN_SR_OK;
}

//! \brief Asks if the application is ready to process the transaction.
//! \details If not implemented, the application is not involved in the transaction processing.
//! \details Call type : broadcast.
//! \param[in] inputData Provided by Transaction::start and may have been updated.
//! \details For each technology available and defined in TAG_TXN_REQUESTED_TECHNOLOGIES:
//! \details TAG_TXN_READER : constructed tag containing the reader information.
//! \details - TAG_TXN_READER_TECHNOLOGY : technology manager by the reader.
//! \details - TAG_TXN_READER_NAME : name of the reader.
//! \param[out] outputData The list of supported readers by the application with the given transaction conditions.
//! \details TAG_TXN_READER : constructed tag containing the reader information.
//! \details - TAG_TXN_READER_TECHNOLOGY : technology managed by the reader.
//! \details - TAG_TXN_READER_NAME : name of the reader.
//! \return May return the following values:
//! - \ref TXN_SR_OK if the application continues the transaction.
//! - \ref TXN_SR_NOT_SATISFIED if the application stops the transaction.
int TxnStartEnd::checkAndPrepare(const TLV_TREE_NODE inputData, TLV_TREE_NODE outputData)
{
	// Get the list of applications with their tags that have changed the transaction input data
	vector<LogApp> list;
	getLogInfo(inputData, list);

	// Get transaction info
	TransactionInfo txn;
	getTransactionInfo(inputData, txn);

	// Get requested technologies
	vector<ReaderInfo> listTechno;
	getRequestedTechno(inputData, listTechno);

	// *************************************************************
	// Check the amount and the currency here :                    *
	// 	  - Amount over a floor limit                              *
	//    - Currency supported by the application                  *
	//    If conditions not satisfied return :                     *
	//    - TXN_SR_NOT_SATISFIED                                   *
	//    This application does not participate to the transaction *
	// *************************************************************

	// All readers are supported by the application with the given transaction conditions
	// For example, the application can reject the transaction over the contactless reader if the amount exceeds a limit
	TLV_TREE_NODE nodeIn = TlvTree_GetFirstChild(inputData);
	while (nodeIn != NULL)
	{
		if (TlvTree_GetTag(nodeIn) == TAG_TXN_READER)
		{
			// Get reader technology & reader name
			unsigned long readerTechno;
			string readerName;
			getReaderInfo(nodeIn, readerTechno, readerName);

			if ((readerTechno & (TXN_TECHNO_CONTACT_CHIP | TXN_TECHNO_CONTACTLESS | TXN_TECHNO_READER_DETECTED)) != 0)
			{
				// Return the reader technology & name supported by the application
				TLV_TREE_NODE nodeOut = TlvTree_AddChild(outputData, TAG_TXN_READER, NULL, 0);
				if (nodeOut != NULL)
				{
					unsigned long readerTechnoMsb;
					readerTechnoMsb = SWAP32(readerTechno);
					TlvTree_AddChildInteger(nodeOut, TAG_TXN_READER_TECHNOLOGY, readerTechnoMsb, sizeof(unsigned long));
					// !!! Do not add the ending null of the readerName string to the tag (TlvTree_AddChildString will not work for cless)
					TlvTree_AddChild(nodeOut, TAG_TXN_READER_NAME, readerName.c_str(), readerName.length());
				}
			}
		}

		// Get next reader
		nodeIn = TlvTree_GetNext(nodeIn);
	}
	//	 Continue the transaction
	return TXN_SR_OK;
}

//! \brief Indicates that the transaction is completed (whatever the result).
//! \details Call type : broadcast.
//! \param[in] inputData Amount, currency, transaction type, etc...
//! \details TAG_TXN_PROCESSING_STATUS : the final transaction status.
//! \details TAG_TXN_USED_TECHNOLOGY : the technology used for the transaction, if available.
//! \param[out] outputData No output data.
//! \return TXN_SR_OK.
int TxnStartEnd::end(const TLV_TREE_NODE inputData, TLV_TREE_NODE outputData)
{
	// Get transaction final status
	unsigned long status, readerUsed, appId;
	string appName;
	AncvConnectData dataToPrint;
	getTransactionStatus(inputData, status, readerUsed, appName, appId);
	cib::json::Document jsonParam;
	loadDataAsJson(FIC_PARAM, jsonParam);

	if((status == TXN_STATUS_TXN_APPROVED && m_transaction->isCB && m_transaction->isANCVTransac) ||  (m_transaction->isANCVTransac && (int)jsonParam["toComplete"].as_int() == 0))
	{
		Utils::ref().waitingWindow->drawing("Validation en cours", Waiting);
		if(Utils::ref().terminateTransac(true, m_transaction->isPre, (int)jsonParam["toComplete"].as_int()))
		{
			Utils::ref().waitingWindow->hidding();
			Utils::ref().fillTicketTransacData(dataToPrint);
			PrintTicketWindow(Utils::ref().glib, "IMPRESSION TICKET", dataToPrint).drawing();
		}
		else
		{
			Utils::ref().waitingWindow->drawing("Transaction Annulé", Cancel);
		}
	}
	else if(m_transaction->isCB && m_transaction->isANCVTransac)
	{
		Utils::ref().waitingWindow->drawing("Annulation en cours", Waiting);
		Utils::ref().terminateTransac(false, m_transaction->isPre, 0);
		Utils::ref().waitingWindow->hidding();
		Utils::ref().waitingWindow->drawing("Transaction Annulé", Cancel);

	}

	return TXN_SR_OK;
}
