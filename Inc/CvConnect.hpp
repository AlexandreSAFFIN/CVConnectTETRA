#ifndef Walleety_hpp_INCLUDED
#define Walleety_hpp_INCLUDED

#include "AppResources.hpp"
#include "cib/transaction/Transaction.hpp"
#include "TxnStartEnd.hpp"
#include "SavedTransaction.hpp"
//! Class to create the training application
class CvConnect : public Application
{
public:
	//! \brief Constructor.
	//! \param[in] serviceClass The service class name.
	CvConnect(const string &serviceClass);
	//! \brief Destructor.
	virtual ~CvConnect();

	void connectionType();
	void getLicence();
	void updateApp();
	void update(string familyName);
	void launchTransaction();
	void getTransactionResult(int status, TLV_TREE_NODE outputData);
	void addTransacDescriptors();
	void goMenu();
	void initMenu();
	Error onMenuClicked(SoftwareSelectedEvent& link);
	void reset();
	void initTransacInterfaces();
	void initTransacWindow();
	void initDisk();
	void initApp();


protected:
	// Transaction interface
	std::vector<std::pair<int, int> > events;
	std::vector<IPayment *> interfaces;
	IPayment *m_startEndInterface;

	// Manage application info
	string appFamily;
	string appName;

};

#endif // Walleety_hpp_INCLUDED
