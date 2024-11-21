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
    if (clientConnection)
        clientConnection->disconnectFromHost();
}

void MesServer::newConnection()
{
    clientConnection = tcpServer->nextPendingConnection();
    connect(clientConnection, &QAbstractSocket::readyRead, this, &MesServer::readSocket);
    connect(clientConnection, &QAbstractSocket::disconnected, this, [&] {
        updateSystemLog(QString("The client (IP:%1, Port:%2) disconnected from MES server.")
                            .arg(clientConnection->peerAddress().toString())
                            .arg(clientConnection->peerPort()));
    });
    connect(clientConnection, &QAbstractSocket::errorOccurred, this, &MesServer::displayError);
    updateSystemLog(tr("A client connect to MES server"));
}

void MesServer::displayError(QAbstractSocket::SocketError socketError)
{
    switch (socketError) {
    case QAbstractSocket::RemoteHostClosedError:
        break;
    case QAbstractSocket::HostNotFoundError:
        QMessageBox::information(
            this,
            tr("MES Server"),
            QString(
                "The connection is refused by the peer" "Make sure the server is running, and " "check " "the ip and " "port " "settings"));
        break;
    default:
        QMessageBox::information(this,
                                 tr("MES Server"),
                                 QString("The following error occurred: %1").arg(clientConnection->errorString()));
    }
}

void MesServer::sendReply()
{
    if (!clientConnection) {
        updateSystemLog("ERROR: Use connection is lost!");
        return;
    }
    if (!clientConnection->isOpen()) {
        updateSystemLog("ERROR: Use connection is closed!");
        return;
    }

    QString filePath = PATH_PART_RECEVIED + "/Response_10.179.90.149_46635c0a-fabe-4bc4-a0ca-0184bd1496f5_IO.xml";
    QFile file(filePath);

    if (!file.open(QIODevice::ReadOnly)) {
        updateSystemLog("ERROR: Server reply file cannot open!");
        return;
    }

    QDataStream out(clientConnection);
    out.setVersion(QDataStream::Qt_6_8);
    // add message header
    out << (quint32) (file.size() - sizeof(qint32));
    // add message body
    out << file.readAll();
}

void MesServer::readSocket()
{
    QByteArray buffer;
    quint32 bufferSize = 0;

    QDataStream in(clientConnection);
    in.setVersion(QDataStream::Qt_6_8);

    // read buffer from socket
    in.startTransaction();
    in >> bufferSize;
    if (clientConnection->bytesAvailable() > bufferSize) {
        qDebug() << clientConnection->bytesAvailable();
        updateSystemLog("Data size ERROR: no enough size to read data!");
        return;
    }
    in >> buffer;
    if (!in.commitTransaction()) {
        updateSystemLog("Data transaction error occured!");
        return;
    }

    // save buffer to local file
    updateSystemLog(QString("Client send a xml file. Size: %1").arg(bufferSize));

    QString pathRequest = QString("%1/Documents/GitHub/boschData/serverData").arg(QDir::homePath());
    const QString fileReq = QString("%1/Request_%2_%3.xml")
                                .arg(pathRequest,
                                     clientConnection->peerAddress().toString(),
                                     QUuid::createUuid().toString(QUuid::WithoutBraces));
    QFile file(fileReq);
    if (file.open(QIODevice::ReadWrite | QIODevice::Text)) {
        file.write(buffer);
    }
    file.seek(0);

    XopconReader reader;
    if (!reader.read(&file)) {
        QMessageBox::warning(this,
                             tr("MES Server"),
                             tr("Parse error in file %1:\n\n%2")
                                 .arg(QDir::toNativeSeparators(pathRequest), reader.errorString()));
    }

    qDebug() << reader.eventName();

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
                              tr("MES Server"),
                              tr("Unable to start the sever: %1.").arg(tcpServer->errorString()));
        close();
        return;
    }

    ui->label_status->setText(
        tr("The Server is running on IP: %1, Port: %2").arg(ipAddress.toString(), ui->edit_serverPort->text()));

    connect(tcpServer, &QTcpServer::newConnection, this, &MesServer::newConnection);

    updateSystemLog(tr("Server Started successful!"));
}

void MesServer::updateSystemLog(const QString &msg)
{
    if (!msg.isEmpty()) {
        ui->edit_systemLog->append(
            tr("%1: %2").arg(QDateTime::currentDateTime().toString(tr("yyyy-MM-dd-hh:mm:ss")), msg));
    }
}
