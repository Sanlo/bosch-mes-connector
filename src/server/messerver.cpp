#include <QtNetwork>
#include <QtCore>
#include <QtWidgets>
#include <QTcpServer>

#include "messerver.h"
#include "ui_messerver.h"

MesServer::MesServer(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MesServer)
{
    ui->setupUi(this);    
}

MesServer::~MesServer()
{
    delete ui;
}

void MesServer::sendReply() {
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_6_8);

    out<< tr("Server reply: 1112");
    updateSystemLog(tr("A client connected"));

    QTcpSocket *clientConnection= tcpServer->nextPendingConnection();

    connect(clientConnection, &QAbstractSocket::disconnected, clientConnection, &QObject::deleteLater);

    clientConnection->write(block);
    clientConnection->disconnectFromHost();
}

void MesServer::on_btn_clear_serverLog_clicked() {
    ui->edit_systemLog->clear();
}

void MesServer::on_btn_copy_severLog_clicked() {
    ui->edit_systemLog->selectAll();
    ui->edit_systemLog->copy();
}

void MesServer::on_btn_startServer_clicked() {
    tcpServer=new QTcpServer(this);

    QHostAddress ipAddress;
    const QList<QHostAddress> ipAddressesList = QNetworkInterface::allAddresses();
    for (const QHostAddress &entry : ipAddressesList) {
        if(entry != QHostAddress::LocalHost && entry.toIPv4Address()){
            ipAddress = entry;
            break;
        }
    }

    if(ipAddress.isNull()) {
        ipAddress=QHostAddress(QHostAddress::LocalHost);
    }

    if(!tcpServer->listen(ipAddress, ui->edit_serverPort->text().toInt())) {
        QMessageBox::critical(this, tr("MES Server"), tr("Unable to start the sever: %1.").arg(tcpServer->errorString()));
        close();
        return;
    }

    ui->label_status->setText(tr("The Server is running on IP: %1, Port: %2")
                                  .arg(ipAddress.toString(), ui->edit_serverPort->text()));

    connect(tcpServer, &QTcpServer::newConnection, this, &MesServer::sendReply);

    updateSystemLog(tr("Server Started successful!"));
}

void MesServer::updateSystemLog(const QString &msg) {
    if(!msg.isEmpty()) {
        ui->edit_systemLog->append(tr("%1: %2")
                                       .arg(QDateTime::currentDateTime().toString(tr("yyyy-MM-dd-hh:mm:ss.z")), msg));
    }
}

