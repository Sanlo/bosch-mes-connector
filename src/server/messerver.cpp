#include "messerver.h"

#include <QTcpServer>
#include <QtCore>
#include <QtNetwork>
#include <QtWidgets>

#include "ui_messerver.h"
#include "xopconreader.h"

MesServer::MesServer(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::MesServer)
{
    ui->setupUi(this);

    PATH_PART_PROCESSED = QString("%1/Documents/GitHub/boschData/partProcessed").arg(QDir::homePath());
    PATH_PART_RECEVIED = QString("%1/Documents/GitHub/boschData/partRecevied").arg(QDir::homePath());
    qDebug() << PATH_PART_PROCESSED;
    qDebug() << PATH_PART_RECEVIED;
}

MesServer::~MesServer()
{
    delete ui;
    if (clientSocket)
        clientSocket->disconnectFromHost();
}

void MesServer::newConnection()
{
    clientSocket = tcpServer->nextPendingConnection();
    connect(clientSocket, &QAbstractSocket::readyRead, this, &MesServer::readSocket);
    connect(clientSocket, &QAbstractSocket::disconnected, this, [&] {
        updateSystemLog(QString("The client (IP:%1, Port:%2) disconnected from MES server.")
                            .arg(clientSocket->peerAddress().toString())
                            .arg(clientSocket->peerPort()));
    });
    connect(clientSocket, &QAbstractSocket::errorOccurred, this, &MesServer::displayError);
    updateSystemLog(QString("A client connect to MES server"));
}

void MesServer::displayError(QAbstractSocket::SocketError socketError)
{
    switch (socketError) {
    case QAbstractSocket::RemoteHostClosedError:
        break;
    case QAbstractSocket::HostNotFoundError:
        QMessageBox::information(
            this,
            QString("MES Server"),
            QString(
                "The connection is refused by the peer" "Make sure the server is running, and " "check " "the ip and " "port " "settings"));
        break;
    default:
        QMessageBox::information(this,
                                 QString("MES Server"),
                                 QString("The following error occurred: %1").arg(clientSocket->errorString()));
    }
}

void MesServer::sendReply()
{
    if (!clientSocket) {
        updateSystemLog("ERROR: Use connection is lost!");
        return;
    }
    if (!clientSocket->isOpen()) {
        updateSystemLog("ERROR: Use connection is closed!");
        return;
    }

    QString filePath
        = (int) XopconReader::PartRecevied == currentEvent
              ? QString("%1/Response_10.179.90.149_46635c0a-fabe-4bc4-a0ca-0184bd1496f5_IO.xml").arg(PATH_PART_RECEVIED)
              : QString("%1/Response_10.179.90.149_46635c0a-fabe-4bc4-a0ca-0184bd1496f5_IO.xml").arg(PATH_PART_PROCESSED);
    QFile file(filePath);

    if (!file.open(QIODevice::ReadOnly)) {
        updateSystemLog("ERROR: Server reply file cannot open!");
        return;
    }

    QByteArray data;
    quint32 dataSize = file.size() + sizeof(quint32);
    QByteArray size = QByteArray::fromRawData(reinterpret_cast<const char *>(&dataSize), sizeof(dataSize));
    std::reverse(size.begin(), size.end());
    data.append(size);
    data.append(file.read(file.size()));
    clientSocket->write(data);
}

void MesServer::readSocket()
{
    QBuffer buffer;
    buffer.setData(clientSocket->readAll());
    buffer.open(QBuffer::ReadWrite);

    const quint32 bufferSize = qFromBigEndian<quint32>(buffer.read(sizeof(quint32)));

    updateSystemLog(QString("Client send a xml file. Size: %1.").arg(bufferSize));

    // save buffer to local file for debug
    QString pathRequest = QString("%1/Documents/GitHub/boschData/serverData").arg(QDir::homePath());
    const QString fileName = QString("%1/Request_%2_%3.xml")
                                 .arg(pathRequest,
                                      clientSocket->peerAddress().toString(),
                                      QUuid::createUuid().toString(QUuid::WithoutBraces));
    QFile file(fileName);
    if (file.open(QIODevice::ReadWrite | QIODevice::Text)) {
        file.write(buffer.data().mid(sizeof(quint32)));
    }
    file.seek(0);

    XopconReader reader;
    if (!reader.read(&file)) {
        QMessageBox::warning(this,
                             QString("MES Server"),
                             QString("Parse error in file %1:\n\n%2")
                                 .arg(QDir::toNativeSeparators(pathRequest), reader.errorString()));
    }

    qDebug() << reader.eventName();

    currentEvent = (int) ("partReceived" == reader.eventName() ? XopconReader::PartRecevied
                                                               : XopconReader::PartProcessed);

    file.close();

    // send client the server reply
    sendReply();
}

void MesServer::on_btn_clear_serverLog_clicked()
{
    ui->edit_systemLog->clear();
}

void MesServer::on_btn_copy_severLog_clicked()
{
    ui->edit_systemLog->selectAll();
    ui->edit_systemLog->copy();
}

void MesServer::on_btn_startServer_clicked()
{
    tcpServer = new QTcpServer(this);

    QHostAddress ipAddress;
    const QList<QHostAddress> ipAddressesList = QNetworkInterface::allAddresses();
    for (const QHostAddress &entry : ipAddressesList) {
        if (entry != QHostAddress::LocalHost && entry.toIPv4Address()) {
            ipAddress = entry;
            break;
        }
    }

    if (ipAddress.isNull()) {
        ipAddress = QHostAddress(QHostAddress::LocalHost);
    }

    if (!tcpServer->listen(ipAddress, ui->edit_serverPort->text().toInt())) {
        QMessageBox::critical(this,
                              QString("MES Server"),
                              QString("Unable to start the sever: %1.").arg(tcpServer->errorString()));
        close();
        return;
    }

    ui->label_status->setText(
        QString("The Server is running on IP: %1, Port: %2").arg(ipAddress.toString(), ui->edit_serverPort->text()));

    connect(tcpServer, &QTcpServer::newConnection, this, &MesServer::newConnection);

    updateSystemLog(QString("Server Started successful!"));
}

void MesServer::updateSystemLog(const QString &msg)
{
    if (!msg.isEmpty()) {
        ui->edit_systemLog->append(
            QString("%1: %2").arg(QDateTime::currentDateTime().toString(QString("yyyy-MM-dd-hh:mm:ss")), msg));
    }
}
