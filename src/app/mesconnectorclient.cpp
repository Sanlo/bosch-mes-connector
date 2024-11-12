#include <QtCore>
#include <QtWidgets>
#include <QtLogging>

#include "mesconnectorclient.h"
#include "./ui_mesconnectorclient.h"
#include "settings.h"

MESConnectorClient::MESConnectorClient(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::MESConnectorClient)
    , tcpSocket(new QTcpSocket(this))
{
    ui->setupUi(this);
    loadSettings();

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

    QString serverReply;
    in >> serverReply;

    if(!in.commitTransaction()) {
        return;
    }

    updateSystemLog(serverReply);
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

