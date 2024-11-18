#include "tcpfilesender.h"

TcpFileSender::TcpFileSender(QWidget *parent)
    : QDialog(parent)
{
    loadSize = 1024 * 4;
    totalBytes = 0;
    bytesWritten = 0;
    bytesToWrite = 0;

    ipAddressLabel = new QLabel(QStringLiteral("IP 地址："));
    ipAddressInput = new QLineEdit;

    clientProgressBar = new QProgressBar;
    clientStatusLabel = new QLabel(QStringLiteral("客戶端就緒"));
    startButton = new QPushButton(QStringLiteral("開始"));
    quitButton = new QPushButton(QStringLiteral("退出"));
    openButton = new QPushButton(QStringLiteral("開檔"));
    startButton->setEnabled(false);

    buttonBox = new QDialogButtonBox;
    buttonBox->addButton(startButton, QDialogButtonBox::ActionRole);
    buttonBox->addButton(openButton, QDialogButtonBox::ActionRole);
    buttonBox->addButton(quitButton, QDialogButtonBox::RejectRole);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(ipAddressLabel);
    mainLayout->addWidget(ipAddressInput); // 添加 IP 地址輸入框到佈局
    mainLayout->addWidget(clientProgressBar);
    mainLayout->addWidget(clientStatusLabel);
    mainLayout->addStretch(1);
    mainLayout->addSpacing(10);
    mainLayout->addWidget(buttonBox);

    setLayout(mainLayout);
    setWindowTitle(QStringLiteral("檔案傳送"));

    connect(openButton, SIGNAL(clicked()), this, SLOT(openFile()));
    connect(startButton, SIGNAL(clicked()), this, SLOT(start()));
    connect(&tcpClient, SIGNAL(connected()), this, SLOT(startTransfer()));
    connect(&tcpClient, SIGNAL(bytesWritten(qint64)), this, SLOT(updateClientProgress(qint64)));
    connect(&tcpClient, SIGNAL(errorOccurred(QAbstractSocket::SocketError)), this, SLOT(displayError(QAbstractSocket::SocketError))); // 確認信號和槽的連接
    connect(quitButton, SIGNAL(clicked()), this, SLOT(close()));
}

void TcpFileSender::openFile()
{
    fileName = QFileDialog::getOpenFileName(this);
    if (!fileName.isEmpty()) startButton->setEnabled(true);
}

void TcpFileSender::start()
{
    startButton->setEnabled(false);
    bytesWritten = 0;
    clientStatusLabel->setText(QStringLiteral("連接中..."));

    QString ipAddress = ipAddressInput->text(); // 獲取 IP 地址
    if (ipAddress.isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("錯誤"), QStringLiteral("請輸入有效的 IP 地址"));
        startButton->setEnabled(true);
        return;
    }

    tcpClient.connectToHost(QHostAddress(ipAddress), 16998);
}

void TcpFileSender::startTransfer()
{
    localFile = new QFile(fileName);
    if (!localFile->open(QFile::ReadOnly))
    {
        QMessageBox::warning(this, QStringLiteral("應用程式"),
                             QStringLiteral("無法讀取 %1:\n%2.").arg(fileName).arg(localFile->errorString()));
        startButton->setEnabled(true);
        return;
    }

    totalBytes = localFile->size();
    QDataStream sendOut(&outBlock, QIODevice::WriteOnly);
    sendOut.setVersion(QDataStream::Qt_4_6);
    QString currentFile = fileName.right(fileName.size() - fileName.lastIndexOf("/") - 1);
    sendOut << qint64(0) << qint64(0) << currentFile;
    totalBytes += outBlock.size();

    sendOut.device()->seek(0);
    sendOut << totalBytes << qint64((outBlock.size() - sizeof(qint64) * 2));
    bytesToWrite = totalBytes - tcpClient.write(outBlock);

    clientStatusLabel->setText(QStringLiteral("已連接"));
    qDebug() << "正在傳送文件: " << currentFile << " 總大小: " << totalBytes << " bytes";
    outBlock.resize(0);
}

void TcpFileSender::updateClientProgress(qint64 numBytes)
{
    bytesWritten += (int)numBytes;
    if (bytesToWrite > 0)
    {
        outBlock = localFile->read(qMin(bytesToWrite, loadSize));
        bytesToWrite -= (int)tcpClient.write(outBlock);
        outBlock.resize(0);
    }
    else
    {
        localFile->close();
    }

    clientProgressBar->setMaximum(totalBytes);
    clientProgressBar->setValue(bytesWritten);
    clientStatusLabel->setText(QStringLiteral("已傳送 %1 Bytes").arg(bytesWritten));
}

void TcpFileSender::displayError(QAbstractSocket::SocketError socketError)
{
    switch (socketError) {
    case QAbstractSocket::HostNotFoundError:
        QMessageBox::information(this, QStringLiteral("TCP 客戶端"),
                                 QStringLiteral("主機未找到。請檢查 IP 地址和端口設置。"));
        break;
    case QAbstractSocket::ConnectionRefusedError:
        QMessageBox::information(this, QStringLiteral("TCP 客戶端"),
                                 QStringLiteral("連接被拒絕。請確保服務器正在運行，並檢查 IP 地址和端口設置。"));
        break;
    case QAbstractSocket::RemoteHostClosedError:
        QMessageBox::information(this, QStringLiteral("TCP 客戶端"),
                                 QStringLiteral("遠程主機關閉連接。"));
        break;
    default:
        QMessageBox::information(this, QStringLiteral("TCP 客戶端"),
                                 QStringLiteral("發生錯誤：%1。").arg(tcpClient.errorString()));
    }

    startButton->setEnabled(true);
}

TcpFileSender::~TcpFileSender()
{
}
