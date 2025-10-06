#include <PaymentQRWindow.hpp>
#include "Utils.hpp"
#include <sstream>
#include <unistd.h>


int PaymentQRWindow::error = 0;

bool PaymentQRWindow::onClick(Message& msg)
{
	return true;
}

PaymentQRWindow::PaymentQRWindow(GraphicLib& glib, GraphicLib& pLib, string text, long long int amount, string qrCodeContent) :
    BaseDrawWindow(glib, text),
//	qrWindow(glib),
	m_ppwindow(pLib),
    transactionStatusLabel(mainWindow),
	pp_bgpicture(m_ppwindow),
    p_paymentLabel(m_ppwindow),        // Initialisation des labels et de l'image
    p_transactionStatusLabel(m_ppwindow),
	transactionImage(mainWindow),
	p_logoimg(m_ppwindow)
{
	pp_bgpicture.setSize(100, 100, GL_UNIT_PERCENT);
	pp_bgpicture.setPosition(0, 0, GL_UNIT_PIXEL);
	pp_bgpicture.setSource("file://flash/HOST/background.png");
	pp_bgpicture.setTransformation(GL_TRANSFORMATION_STRETCH_ALL);
	size = glib.getScreenSize();
	psize = pLib.getScreenSize();
	this->amount = amount;
	this->qrCodeContent = qrCodeContent;

	transactionStatusLabel.setText("Etat de la transaction");
	transactionStatusLabel.setFontName("Arial");
	transactionStatusLabel.setFontSize(20, 20, GL_UNIT_PIXEL);
	transactionStatusLabel.setFontStyle(GL_FONT_STYLE_ITALIC);

	transactionStatusLabel.setSize(size.width, 80, GL_UNIT_PIXEL);
	transactionStatusLabel.setPosition(0, 280, GL_UNIT_PIXEL);
	transactionStatusLabel.setTextAlign(GL_ALIGN_CENTER);
	transactionStatusLabel.setForeColor(GL_COLOR_BLACK);
	// Label de paiement
	p_paymentLabel.setText(title->getText());
	p_paymentLabel.setFontName("Arial");
	p_paymentLabel.setFontSize(15, 15, GL_UNIT_PIXEL);  // Taille ajustée pour lisibilité
	p_paymentLabel.setFontStyle(GL_FONT_STYLE_BOLD);
	p_paymentLabel.setForeColor(GL_COLOR_BLACK);
	p_paymentLabel.setSize(610, 40, GL_UNIT_PIXEL);  // Largeur quasi-complète de l'écran
	p_paymentLabel.setPosition(2, 10, GL_UNIT_PIXEL); // Position en haut avec marge de 5px
	p_paymentLabel.setTextAlign(GL_ALIGN_CENTER); // Centré

	// Label de statut de transaction
	p_transactionStatusLabel.setText("Etat de la transaction");
	p_transactionStatusLabel.setForeColor(GL_COLOR_BLACK);
	p_transactionStatusLabel.setFontName("Arial");
	p_transactionStatusLabel.setFontSize(14, 14, GL_UNIT_PIXEL); // Plus petit pour statut
	p_transactionStatusLabel.setFontStyle(GL_FONT_STYLE_ITALIC);

	p_transactionStatusLabel.setSize(psize.width, 60, GL_UNIT_PIXEL);  // Largeur quasi-complète de l'écran
	p_transactionStatusLabel.setPosition(0, 140, GL_UNIT_PIXEL); // Position en dessous de p_paymentLabel
	p_transactionStatusLabel.setTextAlign(GL_ALIGN_CENTER); // Centré

	// Image de transaction
	p_transactionImage = new Picture(m_ppwindow);
	p_transactionImage->setSize(100, 100, GL_UNIT_PIXEL); // Taille suffisante pour le QR code ou autre image
	p_transactionImage->setPosition((psize.width-100)/2, (psize.height-100)/2-20, GL_UNIT_PIXEL); // Centré en dessous des labels
	p_transactionImage->setTransformation(GL_TRANSFORMATION_STRETCH_ALL);
	p_transactionImage->setTextAlign(GL_ALIGN_CENTER); // Centré

	transactionImage.setSize(160, 160, GL_UNIT_PIXEL);  // Taille du logo
	transactionImage.setPosition((size.width-160)/2, (size.height-160)/2-45, GL_UNIT_PIXEL);  // Position du logo à gauche
	transactionImage.setSource("file://flash/HOST/QRCODE.png");  // Chemin vers l'image du logo
	transactionImage.setTransformation(GL_TRANSFORMATION_STRETCH_ALL);
	p_logoimg.setSource("file://flash/HOST/logo.png");
	p_logoimg.setSize(65,65, GL_UNIT_PIXEL);
	p_logoimg.setPosition(5,115, GL_UNIT_PIXEL);
	p_logoimg.setTransformation(GL_TRANSFORMATION_NONE);
//	qrCodeViewer = new Barcode(qrWindow);



    createSnackBar();
}

// Redéfinition de la méthode drawing
bool PaymentQRWindow::drawing()
{
	transactionImage.setVisible(true);
	threadRequest=NULL;
	error = 0;
	eventWindow = new Window(SGL::ref());
    eventWindow->registerMethod(GL_EVENT_KEY_DOWN, this, &PaymentQRWindow::onKeyPress);
    m_ppwindow.registerMethod(GL_EVENT_KEY_DOWN, this, &PaymentQRWindow::onKeyPress);
    eventWindow->setSize(1,1,GL_UNIT_PIXEL);
	timer = 0;
	canDispatch = true;
	statePayment = WaitingScanning;
	hideSnackBar();
	transactionStatusLabel.setText("Scannez le QrCode");
	p_transactionStatusLabel.setText("Scannez le QrCode");
	m_ppwindow.show();

	displayQrCode(amount);

	threadRequest = new ThreadRequest(true);
	threadRequest->start();

	refreshInformation();

    while(canDispatch)
    {
    	m_ppwindow.dispatch(0);
    	mainWindow.dispatch(0);
//    	qrWindow.show();
    	eventWindow->dispatch(0);
    	refreshInformation();
    	eventWindow->dispatch(0);
    }

    m_ppwindow.hide();
    m_ppwindow.dispatch(0);
    mainWindow.hide();
    mainWindow.dispatch(0);
    eventWindow->destroy();
    m_ppwindow.unregisterMethod(GL_EVENT_KEY_DOWN, this, &PaymentQRWindow::onKeyPress);
    return transactionStatus;

}

void PaymentQRWindow::displayQrCode(long long int amount)
{
	cib::json::Document jsonParam;
	loadDataAsJson(FIC_PARAM, jsonParam);


	double euros = amount / 100.0;
	std::ostringstream oss;
	oss << std::fixed << std::setprecision(2) << euros;
	std::string amountStr = oss.str();

	string paymentText = "A PAYER\n" + amountStr + " EUR";
	transactionStatus = false;
	// Affiche un message pour l'état "En attente de scanner"

	p_paymentLabel.setText(paymentText);
	title->setText(paymentText);

	mainWindow.dispatch(0);
//	qrWindow.dispatch(0);
	mainWindow.dispatch(0);

	transactionImage.setSource("file://flash/HOST/QRCODE.png");
//	qrWindow.saveImage("file://flash/HOST/QRCODE.png", GL_MIME_IMAGE_PNG);
//	qrWindow.saveImage("/CVCONNECT/QRCODE.png", GL_MIME_IMAGE_PNG);
	sleep(1);

	if(Ppad_IsConnected(PPAD_ID_0))
	{
		SGL::ref().injectResource("file://flash/HOST/QRCODE.png");
		m_ppwindow.getGraphicLib().injectResource("file://flash/HOST/QRCODE.png");
		p_transactionImage->setSource("file://flash/HOST/QRCODE.png");
		m_ppwindow.dispatch(0);
	}
}

void PaymentQRWindow::treatPollingReturn()
{
	if(error > 0 || timer >= 100000)
	{
		if(error == 201)
		{
			statePayment = ProcessInProgress;
			timer = 0;
		}
		else if(error == 202)
		{
			transactionStatus = true;
			statePayment = Finish;
			timer = 0;
		}
		else if(error == 203 || timer >= 100000)
		{
			transactionStatus = false;
			statePayment = CanclByPinpad;
			timer = 0;
		}
		else if(error == 204)
		{
			transactionStatus = false;
			statePayment = Finish;
			timer = 0;
		}
		else if(error == 205)
		{
			transactionStatus = false;
			statePayment = Finish;
			timer = 0;
		}

		usleep(150000);
		timer+=15;
	}
}
void PaymentQRWindow::refreshInformation()
{
	T_GL_SIZE psize = m_ppwindow.getGraphicLib().getScreenSize();
	treatPollingReturn();
	switch (statePayment) {
		case WaitingScanning:
			p_transactionImage->setVisible(true);
			break;
		case ProcessInProgress:
			transactionStatusLabel.setText("Transaction en cours...");
			if(Ppad_IsConnected(PPAD_ID_0))
			{
				p_transactionImage->setSize(100, 100, GL_UNIT_PIXEL); // Taille suffisante pour le QR code ou autre image
				p_transactionImage->setPosition((psize.width-100)/2, (psize.height-100)/2, GL_UNIT_PIXEL); // Centré en dessous des labels
				p_transactionImage->setSource("file://flash/HOST/waiting.png");
				p_transactionStatusLabel.setText("Transaction en cours...");
				m_ppwindow.dispatch(0);
			}
			break;

		case Finish:
			if(transactionStatus)
			{
				p_transactionStatusLabel.setText("Transaction OK");
				p_transactionImage->setSource("file://flash/HOST/valid.png");

				p_transactionImage->setSize(100, 100, GL_UNIT_PIXEL); // Taille suffisante pour le QR code ou autre image
				p_transactionImage->setPosition((psize.width-100)/2, (psize.height-15)/2, GL_UNIT_PIXEL); // Centré en dessous des labels

				transactionStatusLabel.setText("Transaction OK");
				transactionImage.setSource("file://flash/HOST/valid.png");
				mainWindow.dispatch(0);
				m_ppwindow.dispatch(0);
			}
			else
			{
			    p_transactionStatusLabel.setText("Echec de la transaction");
			    p_transactionImage->setSource("file://flash/HOST/cancel.png");
			    p_transactionImage->setSize(100, 100, GL_UNIT_PIXEL); // Taille suffisante pour le QR code ou autre image
			    p_transactionImage->setPosition((psize.width - 100)/2, (psize.height)/2-60, GL_UNIT_PIXEL); // Centré en dessous des labels

			    transactionStatusLabel.setText("Echec de la transaction");
			    transactionImage.setSource("file://flash/HOST/cancel.png");
			    mainWindow.dispatch(0);
			    m_ppwindow.dispatch(0);
			}

			if(threadRequest)
			{
				threadRequest->stop();
				threadRequest->join();
				threadRequest = NULL;
			}

			sleep(1);
			canDispatch = false;

			break;
		case CanclByPinpad:
			p_transactionStatusLabel.setText("Transaction Annulé");
			p_transactionImage->setSource("file://flash/HOST/cancel.png");
			p_transactionImage->setSize(100, 100, GL_UNIT_PIXEL); // Taille suffisante pour le QR code ou autre image
			p_transactionImage->setPosition((psize.width - 100)/2, (psize.height)/2-60, GL_UNIT_PIXEL); // Centré en dessous des labels

			transactionStatusLabel.setText("Echec de la transaction");
			transactionImage.setSource("file://flash/HOST/cancel.png");
			mainWindow.dispatch(0);
			m_ppwindow.dispatch(0);


			if(threadRequest)
			{
				threadRequest->stop();
				threadRequest->join();
				threadRequest = NULL;
			}
			Utils::ref().terminateTransac(false, 0);
			canDispatch = false;

			break;

		default:
			transactionStatusLabel.setText("État inconnu.");
			break;
	    }
}

bool PaymentQRWindow::onKeyPress(ingenico::graphics::Message &message) {
	// Handle key press event here
	int key = message.getKey();

	if (key == GL_KEY_CANCEL && (statePayment == WaitingScanning || statePayment == ProcessInProgress))
	{
		if(threadRequest)
		{
			threadRequest->stop();
			threadRequest->join();
			threadRequest = NULL;
		}
		onCancelClick(message);
	}
	return true;

}

void PaymentQRWindow::onCancelClick(Message& msg)
{
	if(statePayment == WaitingScanning || statePayment == ProcessInProgress)
	{
		transactionStatus = false;
		statePayment = Finish;
	}

}

// Destructeur
PaymentQRWindow::~PaymentQRWindow() {}
