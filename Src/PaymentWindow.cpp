#include <PaymentWindow.hpp>
#include "Utils.hpp"
#include <sstream>
#include <unistd.h>


int PaymentWindow::error = 0;

PaymentWindow::PaymentWindow(GraphicLib& glib, GraphicLib& pLib) :
    BaseDrawWindow(glib),
	qrWindow(glib),
	m_ppwindow(pLib),
    paymentLabel(mainWindow),        // Initialisation des labels et de l'image
    transactionStatusLabel(mainWindow),

    p_paymentLabel(m_ppwindow),        // Initialisation des labels et de l'image
    p_transactionStatusLabel(m_ppwindow),
	p_transactionImage(m_ppwindow),
	transactionImage(mainWindow),
	p_logoimg(m_ppwindow),
	logoimg(mainWindow)
{

	T_GL_SIZE size = glib.getScreenSize();
	T_GL_SIZE psize = pLib.getScreenSize();
	paymentLabel.setText("A PAYER\n2.00 XPF");
	paymentLabel.setFontName("Arial");
	paymentLabel.setFontSize(30, 30, GL_UNIT_PIXEL);  // Taille en hauteur
	paymentLabel.setFontStyle(GL_FONT_STYLE_BOLD);
	paymentLabel.setSize(320, 80, GL_UNIT_PIXEL);
	paymentLabel.setPosition(5, 30, GL_UNIT_PIXEL);
	paymentLabel.setTextAlign(GL_ALIGN_CENTER); // Alignement centré

	transactionStatusLabel.setText("Etat de la transaction");
	transactionStatusLabel.setFontName("Arial");
	transactionStatusLabel.setFontSize(20, 20, GL_UNIT_PIXEL);
	transactionStatusLabel.setFontStyle(GL_FONT_STYLE_ITALIC);

	transactionStatusLabel.setSize(320, 80, GL_UNIT_PIXEL);
	transactionStatusLabel.setPosition(10, 100, GL_UNIT_PIXEL);
	transactionStatusLabel.setTextAlign(GL_ALIGN_CENTER);

	// Label de paiement
	p_paymentLabel.setText("A PAYER\n2.00 XPF");
	p_paymentLabel.setFontName("Arial");
	p_paymentLabel.setFontSize(13, 13, GL_UNIT_PIXEL);  // Taille ajustée pour lisibilité
	p_paymentLabel.setFontStyle(GL_FONT_STYLE_BOLD);

	p_paymentLabel.setSize(610, 20, GL_UNIT_PIXEL);  // Largeur quasi-complète de l'écran
	p_paymentLabel.setPosition(2, 2, GL_UNIT_PIXEL); // Position en haut avec marge de 5px
	p_paymentLabel.setTextAlign(GL_ALIGN_CENTER); // Centré

	// Label de statut de transaction
	p_transactionStatusLabel.setText("Etat de la transaction");
	p_transactionStatusLabel.setFontName("Arial");
	p_transactionStatusLabel.setFontSize(14, 14, GL_UNIT_PIXEL); // Plus petit pour statut
	p_transactionStatusLabel.setFontStyle(GL_FONT_STYLE_ITALIC);

	p_transactionStatusLabel.setSize(610, 30, GL_UNIT_PIXEL);  // Largeur quasi-complète de l'écran
	p_transactionStatusLabel.setPosition(5, 25, GL_UNIT_PIXEL); // Position en dessous de p_paymentLabel
	p_transactionStatusLabel.setTextAlign(GL_ALIGN_CENTER); // Centré

	// Image de transaction
	p_transactionImage.setSize(150, 150, GL_UNIT_PIXEL); // Taille suffisante pour le QR code ou autre image
	p_transactionImage.setPosition((psize.width-150)/2, (psize.height-150)/2-15, GL_UNIT_PIXEL); // Centré en dessous des labels
	p_transactionImage.setTransformation(GL_TRANSFORMATION_STRETCH_ALL);
	p_transactionImage.setTextAlign(GL_ALIGN_CENTER); // Centré

	p_logoimg.setSize(55, 55, GL_UNIT_PIXEL); // Taille suffisante pour le QR code ou autre image
	p_logoimg.setPosition(5, 10, GL_UNIT_PIXEL); // Centré en dessous des labels
	p_logoimg.setTransformation(GL_TRANSFORMATION_STRETCH_ALL);
	p_logoimg.setTextAlign(GL_ALIGN_CENTER); // Centré
	p_logoimg.setSource("file://flash/HOST/cfpaylogo.png");

	logoimg.setSize(80, 80, GL_UNIT_PIXEL); // Taille suffisante pour le QR code ou autre image
	logoimg.setPosition(10, 35, GL_UNIT_PIXEL); // Centré en dessous des labels
	logoimg.setTransformation(GL_TRANSFORMATION_STRETCH_ALL);
	logoimg.setTextAlign(GL_ALIGN_CENTER); // Centré
	logoimg.setSource("file://flash/HOST/cfpaylogo.png");

	transactionImage.setSize(160, 160, GL_UNIT_PIXEL);  // Taille du logo
	transactionImage.setPosition((size.width-160)/2, (size.height-160)/2+35, GL_UNIT_PIXEL);  // Position du logo à gauche
	transactionImage.setSource(Utils::ptr()->getIconsPath("cfpaylogo"));  // Chemin vers l'image du logo
	transactionImage.setTransformation(GL_TRANSFORMATION_STRETCH_ALL);


	qrCodeViewer = new Barcode(qrWindow);

    createSnackBar();
}

void PaymentWindow::generateQrCodeImage()
{
	qrCodeViewer->setValue(qrCodeContent.c_str(), qrCodeContent.length());
	qrCodeViewer->setMimeType(GL_MIME_BARCODE_QR);
	qrCodeViewer->setParam(GL_BARCODE_QR_MARGIN, 1);
	qrCodeViewer->setPosition(0, 0, GL_UNIT_PIXEL);
	qrCodeViewer->setTransformation(GL_TRANSFORMATION_STRETCH_ALL);
	qrCodeViewer->setSize(290, 290, GL_UNIT_PIXEL);
	qrWindow.setSize(290,290, GL_UNIT_PIXEL);
	qrWindow.setPosition(15, 120, GL_UNIT_PIXEL);
	qrWindow.setTextAlign(GL_ALIGN_CENTER); // Centré
}

bool PaymentWindow::drawing()
{
	return drawing(0);
}
// Redéfinition de la méthode drawing
bool PaymentWindow::drawing(long long int amount)
{
	transactionImage.setVisible(false);
	threadRequest=NULL;
	error = 0;
	eventWindow = new Window(SGL::ref());
    eventWindow->registerMethod(GL_EVENT_KEY_DOWN, this, &PaymentWindow::onKeyPress);
    eventWindow->setSize(1,1,GL_UNIT_PIXEL);
	timer = 0;
	canDispatch = true;
	statePayment = WaitingScanning;
	hideSnackBar();
	generateQrCodeImage();
	m_ppwindow.show();

	displayQrCode(amount);
	refreshInformation();

    while(canDispatch)
    {
    	mainWindow.dispatch(0);
    	qrWindow.show();
    	eventWindow->dispatch(0);
    	refreshInformation();
    	eventWindow->dispatch(0);
    }

    m_ppwindow.hide();
    m_ppwindow.dispatch(0);
    mainWindow.hide();
    mainWindow.dispatch(0);
    eventWindow->destroy();
    return transactionStatus;

}

void PaymentWindow::displayQrCode(long long int amount)
{
	cib::json::Document jsonParam;
	loadDataAsJson(FIC_PARAM, jsonParam);


	std::ostringstream oss;
	oss << amount;
	std::string amountStr = oss.str();

	string paymentText = "A PAYER\n" + amountStr + " " + (string)jsonParam["Acceptor"]["devise"].as_string();
	string paymentTextppad = "A PAYER " + amountStr + " " + (string)jsonParam["Acceptor"]["devise"].as_string();
	transactionStatus = false;
	// Affiche un message pour l'état "En attente de scanner"
	transactionStatusLabel.setText("");
	p_transactionStatusLabel.setText("");
	p_paymentLabel.setText(paymentTextppad);
	paymentLabel.setText(paymentText);

	mainWindow.dispatch(0);
	qrWindow.dispatch(0);
	mainWindow.dispatch(0);
	qrWindow.saveImage("file://flash/HOST/QRCODE.png", GL_MIME_IMAGE_PNG);

	sleep(1);

	if(Ppad_IsConnected(PPAD_ID_0))
	{
		SGL::ref().injectResource("file://flash/HOST/QRCODE.png");
		m_ppwindow.getGraphicLib().injectResource("file://flash/HOST/QRCODE.png");
		p_transactionImage.setSource("file://flash/HOST/QRCODE.png");
		m_ppwindow.dispatch(0);
	}
}

void PaymentWindow::refreshInformation()
{
	T_GL_SIZE psize = m_ppwindow.getGraphicLib().getScreenSize();
	switch (statePayment) {
		case WaitingScanning:
			if(threadRequest == NULL)
			{
				threadRequest = new ThreadRequest();
				threadRequest->start();
			}
			usleep(150000);
			timer+=15;
			if(timer >= 100000 || error >= 400)
			{
				transactionStatus = false;
				statePayment = Finish;
				threadRequest->stop();
				threadRequest->join();
				threadRequest = NULL;
				timer=0;
			}
			if(error == 202 || error == 200)
			{
				if(error == 200)
				{
					threadRequest->stop();
					threadRequest->join();
					threadRequest = NULL;
				}
				statePayment = ProcessInProgress;
				timer=0;
			}

			break;

		case ProcessInProgress:
			if(timer == 0)
			{
//				p_transactionImage.setVisible(false);
				qrWindow.setVisible(false);
				qrWindow.dispatch(0);
				transactionImage.setVisible(true);
				transactionStatusLabel.setText("Transaction en cours...");
				if(Ppad_IsConnected(PPAD_ID_0))
				{
					p_transactionImage.setSize(100, 100, GL_UNIT_PIXEL); // Taille suffisante pour le QR code ou autre image
					p_transactionImage.setPosition((psize.width-100)/2, (psize.height-100)/2, GL_UNIT_PIXEL); // Centré en dessous des labels

					p_transactionImage.setSource("file://flash/HOST/cfpaylogo.png");
					p_transactionStatusLabel.setText("Transaction en cours...");
					m_ppwindow.dispatch(0);
//					PadSGL::ref().dialogMessage("CF-PAY", "Transaction en cours...", GL_ICON_INFORMATION, GL_BUTTON_NONE, GL_TIME_SECOND);
				}
			}
			usleep(150000);
			timer+=15;
			if(timer >= 100000  || error >= 400)
			{
				threadRequest->stop();
				threadRequest = NULL;
				transactionStatus = false;
				statePayment = Finish;
				timer = 0;
			}
			else if(error == 200)
			{
				if(threadRequest != NULL)
				{
					threadRequest->stop();
					threadRequest->join();
					threadRequest = NULL;
				}
				transactionStatus = true;
				statePayment = Finish;
				timer=0;
			}
			break;

		case Finish:

			qrWindow.setVisible(false);
			qrWindow.dispatch(0);
			transactionImage.setVisible(true);

			if(transactionStatus && Utils::ref().checkQrCodeHook())
			{
				p_transactionStatusLabel.setText("Transaction OK");
				p_transactionImage.setSource("file://flash/HOST/okicon.png");

				p_transactionImage.setSize(100, 100, GL_UNIT_PIXEL); // Taille suffisante pour le QR code ou autre image
				p_transactionImage.setPosition((psize.width-100)/2, (psize.height-100)/2, GL_UNIT_PIXEL); // Centré en dessous des labels

				transactionStatusLabel.setText("Transaction OK");
				transactionImage.setSource("file://flash/HOST/okicon.png");
				mainWindow.dispatch(0);
				m_ppwindow.dispatch(0);
			}
			else
			{
			    p_transactionStatusLabel.setText("Echec de la transaction");
			    p_transactionImage.setSource("file://flash/HOST/cancelicon.png");

			    p_transactionImage.setSize(100, 100, GL_UNIT_PIXEL); // Taille suffisante pour le QR code ou autre image
			    p_transactionImage.setPosition((psize.width - 100)/2, (psize.height - 100)/2, GL_UNIT_PIXEL); // Centré en dessous des labels

			    transactionStatusLabel.setText("Echec de la transaction");
			    transactionImage.setSource("file://flash/HOST/cancelicon.png");
			    mainWindow.dispatch(0);
			    m_ppwindow.dispatch(0);
			}

			if(threadRequest)
			{
				threadRequest->stop();
				threadRequest->join();
				threadRequest = NULL;
			}

			statePayment = WaitInput;

			break;
		case WaitInput:
		    mainWindow.dispatch(0);
		    m_ppwindow.dispatch(0);
			break;

		default:
			// Gérer un cas inattendu si nécessaire
			transactionStatusLabel.setText("État inconnu.");
			break;
	    }
}

void PaymentWindow::setQrCodeContent(string content)
{
	qrCodeContent = content;
}

bool PaymentWindow::onKeyPress(ingenico::graphics::Message &message) {
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
	else if ((key == GL_KEY_CANCEL || key == 0xf850) && statePayment == WaitInput)
	{
		if(threadRequest)
		{
			threadRequest->stop();
			threadRequest->join();
			threadRequest = NULL;
		}

		canDispatch = false;
	}
	return true;

}

bool PaymentWindow::onCancelClick(Message& msg)
{
	if(statePayment == WaitingScanning || statePayment == ProcessInProgress)
	{
		transactionStatus = false;
		statePayment = Finish;
	}
    return true;  // Événement géré
}

// Destructeur
PaymentWindow::~PaymentWindow() {}
