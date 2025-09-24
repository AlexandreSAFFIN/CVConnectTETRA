#ifndef TXNSTARTEND_HPP
#define TXNSTARTEND_HPP

#include "AppResources.hpp"
#include "cib/transaction/Transaction.hpp"
#include "Transaction.hpp"

using namespace cib::transaction;

//! Class to manage the start/end of a transaction.
class TxnStartEnd : public StartEndHandler
{
public:
	//! \brief TxnStartEnd constructor.
	//! \param[in] graphicLib A reference to the graphic library instance.
	TxnStartEnd();
	//! \brief TxnStartEnd destructor.
	virtual ~TxnStartEnd();

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
	int start(const TLV_TREE_NODE inputData, TLV_TREE_NODE outputData);

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
	int checkAndPrepare(const TLV_TREE_NODE inputData, TLV_TREE_NODE outputData);

	//! \brief Indicates that the transaction is completed (whatever the result).
	//! \details Call type : broadcast.
	//! \param[in] inputData Amount, currency, transaction type, etc...
	//! \details TAG_TXN_PROCESSING_STATUS : the final transaction status.
	//! \details TAG_TXN_USED_TECHNOLOGY : the technology used for the transaction, if available.
	//! \param[out] outputData No output data.
	//! \return TXN_SR_OK.
	int end(const TLV_TREE_NODE inputData, TLV_TREE_NODE outputData);

	void resetData();

	Transaction *m_transaction;
};

#endif
