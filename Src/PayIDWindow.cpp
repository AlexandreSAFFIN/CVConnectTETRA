#include "PayIDWindow.hpp"
#include "Utils.hpp"
#include "AppResources.hpp"
#include "cib/disk/Disk.hpp"
#include "TileButton.hpp"
#include "string.h"
#include "PinpadThread.hpp"

bool PayIDWindow::onClick(Message& msg)
{
	return true;
}


PayIDWindow::PayIDWindow(GraphicLib& glib,GraphicLib& pplib, string text, long long int amount) :
    BaseDrawWindow(glib, text),
	m_ppwindow(pplib),
	pp_bgpicture(m_ppwindow),
	p_paymentLabel(m_ppwindow),        // Initialisation des labels et de l'image
	p_transactionStatusLabel(m_ppwindow),
	p_logoimg(m_ppwindow)
{
    createSnackBar();

    // La fenêtre occupe tout l'écran
    mainWindow.setSize(100, 100, GL_UNIT_PERCENT);
    this->amount = amount;
    Layout root(mainWindow);


    Label spacerTop(root);
    spacerTop.setItem(0, 0);
    spacerTop.setText(""); // rien à afficher
    spacerTop.setExpand(GL_DIRECTION_HEIGHT);


    Layout content(root);
    content.setItem(0, 1);


    Label labelEdit(content);
    labelEdit.setItem(0, 0);
    labelEdit.setExpand(GL_DIRECTION_WIDTH);
    labelEdit.setText("RENTREZ VOTRE\nID CLIENT");
    labelEdit.setTextAlign(GL_ALIGN_CENTER);
    labelEdit.setForeColor(GL_COLOR_BLACK);


    Layout row(content);
    row.setItem(0, 1);

    Label spacerLeft(row);
    spacerLeft.setItem(0, 0);
    spacerLeft.setText("");
    spacerLeft.setExpand(GL_DIRECTION_WIDTH);

    editText = new Edit(row);
    editText->setItem(1, 0);

    editText->setSize(200, 45, GL_UNIT_PIXEL);
    editText->setBackColor(GL_COLOR_WHITE);
    editText->setBorders(2,2,2,2,GL_UNIT_PIXEL, GL_COLOR_BLACK);
    editText->setForeColor(GL_COLOR_BLACK);
    editText->setTextAlign(GL_ALIGN_CENTER);

    Label spacerRight(row);
    spacerRight.setItem(2, 0);
    spacerRight.setText("");
    spacerRight.setExpand(GL_DIRECTION_WIDTH);


    Label spacerBottom(root);
    spacerBottom.setItem(0, 2);
    spacerBottom.setText("");
    spacerBottom.setExpand(GL_DIRECTION_HEIGHT);

    //PPAD PART


	pp_bgpicture.setSize(100, 100, GL_UNIT_PERCENT);
	pp_bgpicture.setPosition(0, 0, GL_UNIT_PIXEL);
	pp_bgpicture.setSource("file://flash/HOST/background.png");
	pp_bgpicture.setTransformation(GL_TRANSFORMATION_STRETCH_ALL);

	p_paymentLabel.setText("");
	p_paymentLabel.setFontName("Arial");
	p_paymentLabel.setForeColor(GL_COLOR_BLACK);
	p_paymentLabel.setFontSize(15, 15, GL_UNIT_PIXEL);  // Taille ajustée pour lisibilité
	p_paymentLabel.setFontStyle(GL_FONT_STYLE_BOLD);
	p_paymentLabel.setSize(610, 50, GL_UNIT_PIXEL);  // Largeur quasi-complète de l'écran
	p_paymentLabel.setPosition(2, 2, GL_UNIT_PIXEL); // Position en haut avec marge de 5px
	p_paymentLabel.setTextAlign(GL_ALIGN_CENTER); // Centré

	// Label de statut de transaction
	p_transactionStatusLabel.setText(labelEdit.getText());
	p_transactionStatusLabel.setFontName("Arial");
	p_transactionStatusLabel.setFontSize(14, 14, GL_UNIT_PIXEL); // Plus petit pour statut
	p_transactionStatusLabel.setFontStyle(GL_FONT_STYLE_BOLD);
	p_transactionStatusLabel.setForeColor(GL_COLOR_BLACK);
	p_transactionStatusLabel.setSize(610, 80, GL_UNIT_PIXEL);  // Largeur quasi-complète de l'écran
	p_transactionStatusLabel.setPosition(8, 50, GL_UNIT_PIXEL); // Position en dessous de p_paymentLabel
	p_transactionStatusLabel.setTextAlign(GL_ALIGN_CENTER); // Centré

	p_logoimg.setSource("file://flash/HOST/logo.png");
	p_logoimg.setSize(50,50, GL_UNIT_PIXEL);
	p_logoimg.setPosition(5,105, GL_UNIT_PIXEL);
	p_logoimg.setTransformation(GL_TRANSFORMATION_STRETCH_ALL);

    p_edit = new Edit(m_ppwindow);
    p_edit->setPosition(80,120, GL_UNIT_PIXEL);
    p_edit->setSize(180, 45, GL_UNIT_PIXEL);
    p_edit->setBackColor(GL_COLOR_WHITE);
    p_edit->setForeColor(GL_COLOR_BLACK);
    p_edit->setTextAlign(GL_ALIGN_CENTER);
    p_edit->setBorders(2,2,2,2,GL_UNIT_PIXEL, GL_COLOR_BLACK);
}

void PayIDWindow::dispatchPpWindow()
{
	m_ppwindow.dispatch(100);  // Boucle pour gérer les événements
	m_ppwindow.show();
}


void PayIDWindow::refreshInformation()
{
	transactionStatus = false;
	double euros = amount / 100.0;
	std::ostringstream oss;
	oss << std::fixed << std::setprecision(2) << euros;
	std::string amountStr = oss.str();

	string paymentText = "A PAYER\n" + amountStr + " EUR";
	transactionStatus = false;
	// Affiche un message pour l'état "En attente de scanner"

	p_paymentLabel.setText(paymentText);
	title->setText(paymentText);

    mainWindow.show();
}

// Redéfinition de la méthode drawing
bool PayIDWindow::drawing()
{
	refreshInformation();
    canDispatch = true;
    PinpadThread* th = new PinpadThread();
	mainWindow.registerMethod(GL_EVENT_KEY_DOWN, this, &PayIDWindow::onKeyPress);
	m_ppwindow.registerMethod(GL_EVENT_KEY_DOWN, this, &PayIDWindow::onKeyPressPinpad);
    mainWindow.show();  // Afficher la fenêtre principale
//    topLayout.show();
    hideSnackBar();
    m_ppwindow.show();
	th->start();
    while (canDispatch) {
//    	topLayout.show();
        mainWindow.dispatch(100);  // Boucle pour gérer les événements

		mainWindow.show();
        if(isShowSnackbar)
        {
        	timer+=100;
        	if(timer == 1000)
        	{
        		hideSnackBar();
        	}
        }
    }
	th->stop();
	th->join();
	m_ppwindow.hide();
	m_ppwindow.dispatch(0);
	mainWindow.hide();
	mainWindow.dispatch(0);
	mainWindow.unregisterMethod(GL_EVENT_KEY_DOWN, this, &PayIDWindow::onKeyPress);
	m_ppwindow.unregisterMethod(GL_EVENT_KEY_DOWN, this, &PayIDWindow::onKeyPressPinpad);
    return transactionStatus;
}

void PayIDWindow::onValidate()
{
	cib::json::Document jsonParam;
	loadDataAsJson(FIC_PARAM, jsonParam);
	transactionStatus = Utils::ref().sendMiseEnPaiementTransac((string)jsonParam["beneficiaryId"].as_string(),amount);

	canDispatch = false;
}

bool PayIDWindow::onKeyPress(ingenico::graphics::Message &message) {
        // Handle key press event here
        int key = message.getKey();
        cib::json::Document jsonParam;
		loadDataAsJson(FIC_PARAM, jsonParam);
		jsonParam["beneficiaryId"] = editText->getText();
        saveDataAsJson(FIC_PARAM, jsonParam);

        if (key == GL_KEY_CANCEL)
        {
        	canDispatch = false;
        }
        else if(key == GL_KEY_VALID)
        {
        	onValidate();
        }
        return true;
}

bool PayIDWindow::onKeyPressPinpad(ingenico::graphics::Message &message) {
        // Handle key press event here
        int key = message.getKey();
        cib::json::Document jsonParam;
		loadDataAsJson(FIC_PARAM, jsonParam);
		jsonParam["beneficiaryId"] = p_edit->getText();
        saveDataAsJson(FIC_PARAM, jsonParam);

		if (key == GL_KEY_CANCEL)
        {
        	canDispatch = false;
        }
        else if(key == GL_KEY_VALID)
        {
        	onValidate();
        }
        return true;
}
