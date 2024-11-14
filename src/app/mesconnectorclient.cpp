#include <QtCore>
#include <QtWidgets>
#include <QNetworkProxy>
#include <QtLogging>

#include "./ui_mesconnectorclient.h"
#include "mesconnectorclient.h"
#include "settings.h"
#include "xopconreader.h"
#include "xopconwriter.h"

using namespace Qt::StringLiterals;

MESConnectorClient::MESConnectorClient(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::MESConnectorClient)
    , tcpSocket(new QTcpSocket(this))
{
    ui->setupUi(this);
    loadSettings();

    QPixmap bosch(":/img/bosch.png");
    ui->link_bosch->setScaledContents(true);
    ui->link_bosch->setPixmap(bosch);

    pathPartReceived = "C:/Users/sanlozhang/Documents/GitHub/boschData/partRecevied";
    pathPartProcessed = "C:/Users/sanlozhang/Documents/GitHub/boschData/partProcessed";


    // No Proxy should be used for Tcp Socket
    tcpSocket->setProxy(QNetworkProxy::NoProxy);

    connect(tcpSocket, &QAbstractSocket::readyRead, this, &MESConnectorClient::readPartRecevied);
    connect(tcpSocket, &QAbstractSocket::errorOccurred, this, &MESConnectorClient::displayError);

    in.setDevice(tcpSocket);
    in.setVersion(QDataStream::Qt_6_8);

    updateSystemLog(tr("client started!"));
}

MESConnectorClient::~MESConnectorClient()
{
    delete ui;
}

void MESConnectorClient::requestPartRecevied() {}

void MESConnectorClient::readPartRecevied() {
    in.startTransaction();

    qint64 blockSize = 0;

    if(blockSize == 0) {
        if(tcpSocket->bytesAvailable() < (int)sizeof(quint16))
            return;
        in >> blockSize;
    }

    if(tcpSocket->bytesAvailable() < blockSize)
        return;

    qDebug() << blockSize;

    QByteArray serverReply;
    in >> serverReply;

    pathPartReceived.append("/ServerReply_Test.xml");
    QFile file(pathPartReceived);
    file.open(QIODevice::WriteOnly);
    file.write(serverReply);
    file.close();


    if(!in.commitTransaction()) {
        updateSystemLog("Data transaction error occured!");
        return;
    }

    updateSystemLog("Server reply.");
}

void MESConnectorClient::displayError(QAbstractSocket::SocketError socketError) {

    switch (socketError) {
    case QAbstractSocket::RemoteHostClosedError:
        break;
    case QAbstractSocket::HostNotFoundError:
        QMessageBox::information(this, tr("MES Connector Client"), tr("The connection is refused by the peer"
                                                                      "Make sure the server is running, and check the ip and port settings"));
        break;
    default:
        QMessageBox::information(this, tr("MES Connector Client"), tr("The following error occurred: %1").arg(tcpSocket->errorString()));
    }

}

void MESConnectorClient::on_btn_settings_clicked() {
    Settings settingsDlg(this);

    if(settingsDlg.exec()){
        loadSettings();
    } else {
        qDebug()<<"User reject";

    }
}

void MESConnectorClient::loadSettings() {
    QSettings clientSettings("MesConnector", "Client");

    ui->label_mesIP->setText(clientSettings.value("connection/mesIP").toString());
    ui->label_mesPort->setText(clientSettings.value("connection/mesPort").toString());
    ui->label_dlAPI->setText(clientSettings.value("connection/dlapi").toString());
}

void MESConnectorClient::updateSystemLog(const QString &msg) {
    if(!msg.isEmpty()) {
        ui->clientLog->append(tr("%1: %2")
                                  .arg(QDateTime::currentDateTime().toString(tr("yyyy-MM-dd-hh:mm:ss.z")), msg));
    }
}

void MESConnectorClient::on_btn_connect_mes_clicked() {

    tcpSocket->abort();
    tcpSocket->connectToHost(ui->label_mesIP->text(),ui->label_mesPort->text().toInt());

}

void MESConnectorClient::on_btn_validate_clicked() {


    //===========================TEST XopconWriter================================
    const QString fileName = pathPartReceived + "/ServerRequest_Test.xml";
    QFile fileRequest(fileName);
    if(!fileRequest.open(QFile::WriteOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("MES Connector Client"), tr("Cannot write file %1:\n%2").arg(QDir::toNativeSeparators(fileName), fileRequest.errorString()));
        return;
    }

    XopconWriter writer;
    if(writer.writeXmlData(&fileRequest)) {
        updateSystemLog(QObject::tr("XML file is writed!"));
    }

    //===========================TEST XopconReader================================
    XopconReader xopconReader;

    qDebug() << pathPartReceived;

    QFile file(pathPartReceived + "/ServerReply_Test.xml" );
    if(!file.open(QFile::ReadOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("MES Connector Client"), tr("Cannot read file %1: %2")
                                 .arg(QDir::toNativeSeparators(pathPartReceived), file.errorString()));
        return;
    }

    if(!xopconReader.read(&file)) {
        QMessageBox::warning(this, tr("MES Connector Client"), tr("Parse error in file %1:\n\n%2").arg(QDir::toNativeSeparators(pathPartReceived), xopconReader.errorString()));
    }

}

