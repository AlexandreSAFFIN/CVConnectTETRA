#ifndef PAYMENTQRWINDOW_HPP
#define PAYMENTQRWINDOW_HPP


#include "ThreadRequest.hpp"
#include "BaseDrawWindow.hpp"  // Inclusion de votre base de classe


enum StatePayment {
    WaitingScanning,   // En attente de scanner
    ProcessInProgress, // Processus en cours
    Finish,             // Transaction terminée
};

class PaymentQRWindow : public BaseDrawWindow {
public:

    static int error;

    PaymentQRWindow(GraphicLib& , GraphicLib&, string text, long long int amount, string qrContent);
    ~PaymentQRWindow();

    bool drawing();
    void displayQrCode(long long int amount);
    void refreshInformation();

    bool onKeyPress(ingenico::graphics::Message &message);
    bool onClick(Message& msg);

    void setQrCodeContent(string content);
    void onCancelClick(Message& msg);
    void generateQrCodeImage();

private:

    long long int amount;
    ThreadRequest *threadRequest;
    Window qrWindow;
	Window m_ppwindow;
    string qrCodeContent;

    Label transactionStatusLabel; // Label pour "État de la transaction"
    Picture pp_bgpicture;
    Label p_paymentLabel;
    Label p_transactionStatusLabel;
    ingenico::graphics::Picture* p_transactionImage;  // Image de la transaction
    ingenico::graphics::Picture transactionImage;  // Image de la transaction

    ingenico::graphics::Picture p_logoimg;  // Image de la transactionn

    ingenico::graphics::Barcode* qrCodeViewer;

    StatePayment statePayment;
	T_GL_SIZE size;
	T_GL_SIZE psize;
    bool transactionStatus;
};

#endif // PAYMENTWINDOW_HPP
