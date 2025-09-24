#include "TransactionWindow.hpp"
#include <sstream>
#include <iostream>
#include "Utils.hpp"


TransactionWindow::TransactionWindow(ingenico::graphics::GraphicLib& lib, std::vector<CfPayTransactionInfo> transactionInfo, std::vector<CfPayHistoricTransac> transactionHisto)
    : BaseDrawWindow(lib),	lastTransacTab(mainWindow), HistoricsTab(mainWindow), printerButton(mainWindow)
{
	transactions = transactionInfo;
	transactionsHisto = transactionHisto;
	isLastTransacWindow = true;
	printerButton.setVisible(false);
	HistoricsTab.setVisible(false);
	lastTransacTab.setVisible(false);
//	printerButton.setSource(Utils::ptr()->getIconsPath("cfpaylogo"));  // Chemin vers l'image du logo
////	printerButton.setTransformation(GL_TRANSFORMATION_STRETCH_ALL);
//	printerButton.setSize(60, 50, GL_UNIT_PIXEL);
//	printerButton.setPosition(10, 40, GL_UNIT_PIXEL);
//	printerButton.registerMethod(GL_EVENT_STYLUS_CLICK, this, &TransactionWindow::onPrintButtonClick);
//	printerButton.setVisible(false);

//	lastTransacTab.setText("Derniéres\ntransactions");
//	lastTransacTab.setSize(110, 50, GL_UNIT_PIXEL);
//	lastTransacTab.setPosition(75, 40, GL_UNIT_PIXEL);
//	lastTransacTab.registerMethod(GL_EVENT_STYLUS_CLICK, this, &TransactionWindow::onLastTransacClick);
//	lastTransacTab.setVisible(false);

//    HistoricsTab.setText("Historique\n(2 jours)");
//    HistoricsTab.setSize(110, 50, GL_UNIT_PIXEL);
//    HistoricsTab.setPosition(200, 40, GL_UNIT_PIXEL);
//    HistoricsTab.registerMethod(GL_EVENT_STYLUS_CLICK, this, &TransactionWindow::onHistoricClick);
//    HistoricsTab.setVisible(false);

}

void TransactionWindow::refreshInformation()
{
	if(isLastTransacWindow)
	{
//		lastTransacTab.setFocusable(false);
//		lastTransacTab.setPressable(false);
//		lastTransacTab.setBackColor(GL_COLOR_WHITE);
//		lastTransacTab.setForeColor(GL_COLOR_BLACK);
//		lastTransacTab.setFocus();

//		HistoricsTab.setFocusable(true);
//		HistoricsTab.setPressable(true);
//		HistoricsTab.setClickable(true);
//		HistoricsTab.setCheck(false);
//		HistoricsTab.setBackColor(GL_COLOR_TRANSPARENT);
//		HistoricsTab.setForeColor(GL_COLOR_WHITE);
		printerButton.setVisible(false);
	}
	else
	{
//		HistoricsTab.setFocusable(false);
//		HistoricsTab.setPressable(false);
//		HistoricsTab.setBackColor(GL_COLOR_WHITE);
//		HistoricsTab.setForeColor(GL_COLOR_BLACK);
//		HistoricsTab.setFocus();

		lastTransacTab.setFocusable(true);
		lastTransacTab.setPressable(true);
		lastTransacTab.setClickable(true);
		lastTransacTab.setBackColor(GL_COLOR_TRANSPARENT);
		lastTransacTab.setForeColor(GL_COLOR_WHITE);

		printerButton.setFocusable(true);
		printerButton.setPressable(true);
		printerButton.setClickable(true);
		printerButton.setVisible(true);
	}
}

bool TransactionWindow::onHistoricClick(Message& msg)
{
	isLastTransacWindow = false;
	windowLayout[0]->setVisible(false);
	windowLayout[1]->setVisible(true);
	refreshInformation();
	return true;
}

bool TransactionWindow::onLastTransacClick(Message& msg)
{
	isLastTransacWindow = true;
	windowLayout[0]->setVisible(true);
	windowLayout[1]->setVisible(false);
	refreshInformation();
	return true;
}


bool TransactionWindow::onPrintButtonClick(Message& msg)
{
	Utils::ptr()->printTicketHistory(transactionsHisto);

	return true;
}

bool TransactionWindow::drawing()
{
	canDispatch = true;
	refreshInformation();
	while(canDispatch)
	{
		mainWindow.dispatch(0);
	}
	return true;
}

void TransactionWindow::initializeWindow() {
	isLastTransacWindow = true;
    // Configuration de la fenêtre principale
    mainWindow.setSize(320, 480, GL_UNIT_PIXEL);

    // Enregistrer le gestionnaire d'événements pour les touches
    mainWindow.registerMethod(GL_EVENT_KEY_DOWN, this, &TransactionWindow::onKeyPressed);

    //TODO IF 2 DAY HISTO GO TO 2 NOT 1
    for(int i = 0; i<1; i++)
    {
		windowLayout[i] = new Layout(mainWindow);
		windowLayout[i]->setItem(0,2);

		// Création de la barre de défilement verticale
		vScrollBar = new ingenico::graphics::VScrollBar(*windowLayout[i]);
		vScrollBar->setItem(1, 0);                      // Position dans le layout

		// Création de la vue défilable (ScrollView)
		scrollView[i] = new ingenico::graphics::ScrollView(*windowLayout[i]);
		scrollView[i]->setItem(0, 0);                      // Position dans le layout
		scrollView[i]->attachScrollbar(*vScrollBar);       // Associer la barre de défilement à la ScrollView
		scrollView[i]->setId(1);
		scrollView[i]->setSize(320, 320, GL_UNIT_PIXEL);
		scrollView[i]->setPosition(0, 40, GL_UNIT_PIXEL);

		if(i == 0)
		{
			for (size_t j = 0; j < transactions.size(); ++j)
			{
				CfPayTransactionInfo& transaction = transactions[j];

				// Création du texte pour chaque bouton
				std::ostringstream buttonText;
				buttonText << "Date: " << transaction.getDate() << "\n"
						   << "Montant: " <<  transaction.getAmount()
						   << " "+transaction.getTransactionNumber();

				// Création du bouton
				ingenico::graphics::Button* button = new ingenico::graphics::Button(*scrollView[i]);
				button->setSize(280, 70, GL_UNIT_PIXEL);
				button->setText(buttonText.str());
				button->setUserData(j);
				button->setItem(0,j);

				// Enregistrement de l'événement clic
				button->registerMethod(GL_EVENT_STYLUS_CLICK, this, &TransactionWindow::onTransactionClick);

				// Sauvegarder les boutons
				transactionButtons.push_back(button);
			}
		}
		else
		{
//			for (size_t j = 0; j < transactionsHisto.size(); ++j)
//			{
//				CfPayHistoricTransac& transaction = transactionsHisto[j];
//
//				// Création du texte pour chaque bouton
//				std::ostringstream buttonText;
//				buttonText << "Date: " << transaction.getDate() << "\n"
//						   << transaction.getCount()
//						   << " transaction enregistrées\nPour un total de "<<transaction.getAmount()<<" "+transaction.getTransactionNumber();
//
//				// Création du bouton
//				ingenico::graphics::Button* button = new ingenico::graphics::Button(*scrollView[i]);
//				button->setSize(280, 70, GL_UNIT_PIXEL);
//				button->setText(buttonText.str());
//				button->setUserData(j);
//				button->setItem(0,j);
//
//				// Enregistrement de l'événement clic
//				button->registerMethod(GL_EVENT_STYLUS_CLICK, this, &TransactionWindow::onTransactionClick);
//
//				// Sauvegarder les boutons
//				transactionButtons.push_back(button);
//			}
//			windowLayout[i]->setVisible(false);
		}

		if(transactions.size() == 0)
		{
			HistoricsTab.setClickable(false);
			text = new Label(mainWindow);
			text->setText("Pas de transaction(s)");
			text->setItem(0,0);
			text->setTextAlign(GL_ALIGN_CENTER);
			text->setVisible(true);
		}
    }
}


bool TransactionWindow::onKeyPressed(ingenico::graphics::Message& msg) {
    // Vérifier que la ScrollView est définie
    if (!scrollView[0]) return false;

    // Récupérer la plage de défilement verticale
    T_GL_RANGE range = scrollView[0]->getVerticalRange();

    // Récupérer la clé pressée
    switch (msg.getKey()) {
        case GL_VIRTUAL_KEY_PREVIOUS:
            scrollView[0]->move(0, range.value - range.page);
            break;

        case GL_VIRTUAL_KEY_NEXT:
            scrollView[0]->move(0, range.value + range.page);
            break;

        case GL_KEY_CANCEL:
        	canDispatch = false;
        	break;

        default:
            // Quitter si une autre touche est pressée
            msg.setResult(0);
            break;
    }

    return true;
}



bool TransactionWindow::onTransactionClick(ingenico::graphics::Message& msg) {
    RefWidget clickedWidget = msg.getWidget();
	long buttonId;
	clickedWidget.getUserData(buttonId);


    return true;
}


void TransactionWindow::show() {
    mainWindow.dispatch();
}

TransactionWindow::~TransactionWindow() {
    for (size_t i = 0; i < transactionButtons.size(); ++i) {
        delete transactionButtons[i];
    }
}
