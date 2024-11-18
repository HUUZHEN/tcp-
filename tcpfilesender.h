#ifndef TCPFILESENDER_H
#define TCPFILESENDER_H

#include <QDialog>
#include <QTcpSocket>
#include <QFile>
#include <QProgressBar>
#include <QLabel>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QHostAddress>
#include <QLineEdit>
#include <QDataStream>
#include <QDebug>

class TcpFileSender : public QDialog
{
    Q_OBJECT

public:
    explicit TcpFileSender(QWidget *parent = nullptr);
    ~TcpFileSender();

private slots:
    void openFile();
    void start();
    void startTransfer();
    void updateClientProgress(qint64 numBytes);
    void displayError(QAbstractSocket::SocketError socketError); // 添加這行

private:
    QProgressBar *clientProgressBar;
    QLabel *clientStatusLabel;
    QPushButton *startButton;
    QPushButton *quitButton;
    QPushButton *openButton;
    QDialogButtonBox *buttonBox;
    QTcpSocket tcpClient;
    QFile *localFile;
    QString fileName;
    QByteArray outBlock;
    qint64 totalBytes;
    qint64 bytesWritten;
    qint64 bytesToWrite;
    qint64 loadSize;

    // 新增的變量
    QLabel *ipAddressLabel;
    QLineEdit *ipAddressInput;
};

#endif // TCPFILESENDER_H
