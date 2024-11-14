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

    // QFile file("C:/Users/sanlozhang/Documents/GitHub/boschData/partRecevied/Response_10.179.90.149_46635c0a-fabe-4bc4-a0ca-0184bd1496f5_IO.xml");
    QFile file("C:/Users/sanlozhang/Documents/GitHub/boschData/partProcessed/Response_10.179.90.149_b19ce799-9499-4704-90cd-d1622aec227e_IO.xml");

    file.open(QIODevice::ReadOnly);

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << (quint64)0;
    out<< file.readAll();
    out.device()->seek(0);
    out << (quint64)(block.size()) - sizeof(quint64);

    // wireshark: 986, debug: 978, 0x03D2
    qDebug() << "Server file size" << (quint64)(block.size()) - sizeof(quint64);

    QTcpSocket *clientConnection= tcpServer->nextPendingConnection();

    connect(clientConnection, &QAbstractSocket::disconnected, clientConnection, &QObject::deleteLater);

    clientConnection->write(block);
    clientConnection->waitForBytesWritten();
    clientConnection->disconnectFromHost();

    updateSystemLog(tr("A client connected"));
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

