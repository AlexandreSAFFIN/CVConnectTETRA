#include "AppResources.hpp"
#include "Transaction.hpp"
#include "Utils.hpp"

//                            ###################
//                            #   TRANSACTION   #
//                            ###################

//! \brief Transaction constructor.
Transaction::Transaction()
{
}

//! \brief Transaction destructor.
Transaction::~Transaction()
{
}


void Transaction::updateTransactionInfo(TLV_TREE_NODE outputData, const unsigned long long amount)
{
	// Update amount (Bcd 6bytes)
	if (amount != 0)
	{
		unsigned char tucAmount[6];
		GTL_Convert_UllToDcdNumber(amount, tucAmount, sizeof(tucAmount));
		TlvTree_AddChild(outputData, TAG_EMV_AMOUNT_AUTH_NUM, tucAmount, sizeof(tucAmount));
	}
}

ulong Transaction::paymentChoice()
{

}

bool Transaction::showQRCode(long long int amount)
{
	bool ret = 0;
//	SGL::ref().dialogMessage("CF-Pay", "Veuillez patienter\nLa transaction est en cours de création.", GL_ICON_INFORMATION, GL_BUTTON_NONE, 1);
//	string qrCodeContent = Utils::ptr()->getQrCodeTransaction(amount);
//	if((qrCodeContent != "" && Utils::ptr()->launchDispatch()))
//	{
//		Utils::ptr()->paymentWindow->setQrCodeContent(qrCodeContent);
//		ret = Utils::ptr()->paymentWindow->drawing(amount);
//		delete Utils::ptr()->paymentWindow;
//		Utils::ptr()->paymentWindow = new PaymentWindow(SGL::ref(), PadSGL::ref());
//	}

	return ret;
}
