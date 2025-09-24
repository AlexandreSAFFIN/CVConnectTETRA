#ifndef Transaction_hpp_INCLUDED
#define Transaction_hpp_INCLUDED

#include "AppResources.hpp"

class Transaction
{
public:
	//! \brief Transaction constructor.
	Transaction();
	//! \brief Transaction destructor.
	virtual ~Transaction();

	void updateTransactionInfo(TLV_TREE_NODE outputData, const unsigned long long amount);
	void prepareTransaction();
	bool startTransaction(TLV_TREE_NODE outputData);
	void endTransaction();
	ulong paymentChoice();
	bool showQRCode(long long int amount);
	void transactionType();


};

#endif // Transaction_hpp_INCLUDED
