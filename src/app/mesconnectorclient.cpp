#include "mesconnectorclient.h"

#include <QNetworkAccessManager>
#include <QNetworkProxy>
#include <QNetworkReply>
#include <QtConcurrent/QtConcurrent>
#include <QtCore>
#include <QtLogging>
#include <QtWidgets>

#include "./ui_mesconnectorclient.h"
#include "dataloop.h"
#include "polyworks.h"
#include "settings.h"
#include "xopconreader.h"
#include "xopconwriter.h"

using namespace Qt::StringLiterals;

MESConnectorClient::MESConnectorClient(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::MESConnectorClient)
    , tcpSocket(new QTcpSocket(this))
    , networkManager(new QNetworkAccessManager(this))

{
    // init client
    ui->setupUi(this);

    loadSettings();
    updateUI();
    updateSystemLog(QString("Client started!"));

    // setup socket to MES server
    tcpSocket->setProxy(QNetworkProxy::NoProxy);
    connect(tcpSocket, &QAbstractSocket::readyRead, this, &MESConnectorClient::onServerReply);
    connect(tcpSocket, &QAbstractSocket::errorOccurred, this, &MESConnectorClient::displayError);
    connect(tcpSocket, &QAbstractSocket::disconnected, this, [&] {
        ui->btn_connect_mes->setEnabled(true);
        ui->btn_disconnect_mes->setDisabled(true);
        ui->label_status_messerver->setText(tr("Mes Server is Disconnected."));
        updateSystemLog(QString("Client is disconnected from MES server (IP:%1, Port:%2).")
                            .arg(tcpSocket->peerAddress().toString())
                            .arg(tcpSocket->peerPort()));
    });
    // setup signal/slot of DataLoop API
    connect(networkManager, &QNetworkAccessManager::finished, this, &MESConnectorClient::onDataloopReply);
}

MESConnectorClient::~MESConnectorClient()
{
    delete ui;
    if (polyworks)
        delete polyworks;
}

void MESConnectorClient::onServerReply()
{
    // 3. recevie XML response file stream from mes server
    QBuffer buffer;
    buffer.setData(tcpSocket->readAll());
    buffer.open(QBuffer::ReadWrite);

    const quint32 bufferSize = qFromBigEndian<quint32>(buffer.read(sizeof(quint32)));
    updateSystemLog(QString("Client send a xml reply. Size: %1.").arg(bufferSize));

#if 0
    QDataStream in(tcpSocket);
    in.setVersion(QDataStream::Qt_6_8);
    in.startTransaction();
    in >> bufferSize;
    qDebug() << tcpSocket->bytesAvailable() << bufferSize;
    if (tcpSocket->bytesAvailable() < bufferSize) {
        qDebug() << tcpSocket->bytesAvailable();
        updateSystemLog(
            QString("Data size ERROR: size:%1, received:%2").arg(bufferSize).arg(tcpSocket->bytesAvailable()));
        // return;
    }
    in >> buffer;
    if (!in.commitTransaction()) {
        updateSystemLog("Data transaction error occured!");
        // return;
    }
#endif

    QFile file(QString("%1/Response_%2_%3_IO.xml")
                   .arg(MESConnectorClient::PartRecevied == partStatus ? pathPartReceived : pathPartProcessed,
                        tcpSocket->peerAddress().toString(),
                        currentUuid));
    if (!file.open(QFile::ReadWrite)) {
        QMessageBox::warning(this,
                             QString("MES Connector Client"),
                             tr("Cannot read file %1: %2")
                                 .arg(QDir::toNativeSeparators(pathPartReceived), file.errorString()));
        return;
    }
    file.write(buffer.data().mid(sizeof(quint32)));
    file.seek(0);

    // 4. parse and retrieve data from respone file
    XopconReader xopconReader;
    if (!xopconReader.read(&file)) {
        QMessageBox::warning(this,
                             QString("MES Connector Client"),
                             tr("Parse error in file %1:\n\n%2")
                                 .arg(QDir::toNativeSeparators(pathPartReceived), xopconReader.errorString()));
    }

    processNo = xopconReader.processNo();
    typeNo = xopconReader.typeNo();

    file.close();

    // 5. respond to the result
    // if (MESConnectorClient::PartRecevied == partStatus && "true" == xopconReader.partForStation()) {
    //     // post process for part recevied

    //     on_btn_startInspect_clicked();
    //     updateSystemLog(QString("server reply: partForStation: %1, typeNo: %2.")
    //                         .arg(xopconReader.partForStation(), xopconReader.typeNo()));
    // } else if (MESConnectorClient::PartProcessed == partStatus) {
    //     int item = ui->combo_partList->currentIndex();
    //     ui->combo_partList->setItemIcon(item, QIcon(":/img/Bosch-logo.png"));
    //     updateSystemLog(QString("server replied for part processed.(Code:%1)").arg(xopconReader.returnCode()));
    // }
}

void MESConnectorClient::onDataloopReply()
{
    QNetworkReply *reply = reinterpret_cast<QNetworkReply *>(sender());
    QJsonDocument json = QJsonDocument::fromJson(reply->readAll());
    QJsonArray controlArray = json.object()["value"].toArray();
    QStringList objNames, controlNames;
    QList<double> measuredList;
    for (const QJsonValueRef obj : controlArray) {
        if (obj.toObject()["type"].toString() == "Distance") {
            qDebug().noquote() << obj.toObject()["controls"].toArray()[0].toObject()["objectName"].toString()
                               << obj.toObject()["controls"].toArray()[0].toObject()["name"].toString()
                               << obj.toObject()["controls"].toArray()[0].toObject()["measured"].toDouble();
            objNames << obj.toObject()["controls"].toArray()[0].toObject()["objectName"].toString();
            controlNames << obj.toObject()["controls"].toArray()[0].toObject()["name"].toString();
            measuredList << obj.toObject()["controls"].toArray()[0].toObject()["measured"].toDouble();
        }
    }

    const QString fileName = QString("%1/Request_%2_%3.xml")
                                 .arg(pathPartProcessed, tcpSocket->peerAddress().toString(), currentUuid);
    QFile file(fileName);
    if (!file.open(QFile::ReadWrite | QFile::Text)) {
        QMessageBox::warning(this,
                             QString("MES Connector Client"),
                             tr("Cannot write file %1:\n%2").arg(QDir::toNativeSeparators(fileName), file.errorString()));
        return;
    }

    XopconWriter writer(partIndentifier, XopconWriter::PartProcessed);
    writer.setMeasureData(objNames, controlNames, measuredList);
    // set xml location info
    QSettings clientSettings("MesConnector", "Client");
    writer.setLineNo(clientSettings.value("connection/mes/lineNo").toString());
    writer.setStatNo(clientSettings.value("connection/mes/statNo").toString());
    writer.setStatIdx(clientSettings.value("connection/mes/statIdx").toString());
    writer.setFuNo(clientSettings.value("connection/mes/fuNo").toString());
    writer.setWorkPos(clientSettings.value("connection/mes/workPos").toString());
    writer.setToolPos(clientSettings.value("connection/mes/toolPos").toString());
    writer.setProcessNo(clientSettings.value("connection/mes/processNo").toString());
    writer.setProcessName(clientSettings.value("connection/mes/processName").toString());
    writer.setApplication(clientSettings.value("connection/mes/application").toString());

    writer.writeXmlData(&file);

    file.seek(0);
    sendRequest(&file);

    // update UI
    updateSystemLog(QString("Measurement data has been send to MES server, part identifier is %1").arg(partIndentifier));
}

void MESConnectorClient::displayError(QAbstractSocket::SocketError socketError)
{
    switch (socketError) {
    case QAbstractSocket::RemoteHostClosedError:
        break;
    case QAbstractSocket::HostNotFoundError:
        QMessageBox::information(
            this,
            QString("MES Connector Client"),
            tr(
                "The connection is refused by the peer" "Make sure the server is running, and " "che" "ck " "the ip " "and " "por" "t " "settings"));
        break;
    default:
        QMessageBox::information(this,
                                 tr("MES Connector Client"),
                                 tr("The following error occurred: %1").arg(tcpSocket->errorString()));
    }
}

void MESConnectorClient::on_btn_settings_clicked() {
    Settings settingsDlg(this);
    settingsDlg.setWindowModality(Qt::WindowModal);

    if (settingsDlg.exec()) {
        loadSettings();
        updateUI();
    } else {
        qDebug() << "User reject";
    }
}

void MESConnectorClient::loadSettings() {
    QSettings clientSettings("MesConnector", "Client");

    mesIP = clientSettings.value("connection/mesIP").toString();
    ui->label_mesIP->setText(mesIP);
    mesPort = clientSettings.value("connection/mesPort").toUInt();
    ui->label_mesPort->setText(QString::number(mesPort));
    dataloopEntry = clientSettings.value("connection/dlapi").toString();
    dataloopToken = clientSettings.value("connection/dlToken").toString();

    pathPartReceived = clientSettings.value("general/pathReceived").toString();
    pathPartProcessed = clientSettings.value("general/pathProcessed").toString();

    // Set language hot frash
    int lang = clientSettings.value("general/language").toInt();
    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    if (1 == lang && translator.load(":/MesConnector_zh_CN.qm")) {
        qApp->installTranslator(&translator);
        ui->retranslateUi(this);
    } else {
        for (const QString &locale : uiLanguages) {
            const QString baseName = "MesConnector_" + QLocale(locale).name();
            if (translator.load(":/i18n/" + baseName)) {
                qDebug() << baseName;
                qApp->installTranslator(&translator);
                break;
            }
        }
        ui->retranslateUi(this);
    }

    QString theme = clientSettings.value("general/theme").toString();
    if (theme == QString("light")) {
        qApp->styleHints()->setColorScheme(Qt::ColorScheme::Light);
    }
    if (theme == QString("dark")) {
        qApp->styleHints()->setColorScheme(Qt::ColorScheme::Dark);
    }
    if (theme == QString("auto")) {
        qApp->styleHints()->setColorScheme(Qt::ColorScheme::Unknown);
    }
}

void MESConnectorClient::updateUI()
{
    QPixmap bosch(":/img/bosch.svg");
    ui->link_bosch->setScaledContents(true);
    ui->link_bosch->setAutoFillBackground(true);
    ui->link_bosch->setPixmap(bosch);
    ui->label_status_messerver->setText("Mes Server is Disconnected.");
    ui->btn_connect_mes->setEnabled(true);
    ui->btn_disconnect_mes->setDisabled(true);
    ui->btn_validate->setDisabled(true);
    ui->label_mesIP->setText(mesIP);
    ui->label_mesPort->setText(QString::number(mesPort));
    ui->label_dlAPI->setText(dataloopEntry);
    ui->tab_connection->setCurrentIndex(0);
    ui->tab_inspection->setCurrentIndex(0);
    ui->label_status_partvalidation->setText(tr("Waiting for user input.\nPlease scan part ID with BAR scanner"));
}

void MESConnectorClient::sendMessage(QAnyStringView msg) {
    qDebug() << tcpSocket->state();
    qDebug() << tcpSocket->isOpen();
    qDebug() << tcpSocket->peerAddress().toString();
    qDebug() << msg;

    // check if socket available
    if (!tcpSocket) {
        QMessageBox::warning(this, tr("MES Connector Client"), tr("Server not connected!"));
        updateSystemLog("ERROR: Please connect to MES server first!");
        return;
    }

    if (!tcpSocket->isOpen()) {
        QMessageBox::warning(this, tr("MES Connector Client"), tr("Socket Error!"));
        updateSystemLog("ERROR: Please reconnect to MES server first!");
        return;
    }

    qDebug() << "Start transaction";

    QString id = ui->edit_partID->text();
    QDataStream out(tcpSocket);
    out.setVersion(QDataStream::Qt_6_8);

    QByteArray header;
    header.prepend(QString("fileType:message, fileSize:%1").arg(id.size()).toUtf8());
    header.resize(128);

    QByteArray byteArr = id.toUtf8();
    byteArr.prepend(header);

    out << byteArr;

    updateSystemLog("Send Data to server");
    ui->edit_partID->setText("");
    ui->btn_validate->setDisabled(true);
}

bool MESConnectorClient::sendRequest(QIODevice *buffer)
{
    // check if socket available
    if (!tcpSocket) {
        QMessageBox::warning(this, tr("MES Connector Client"), tr("Server has't been connected!"));
        return false;
    }

    if (!tcpSocket->isOpen()) {
        QMessageBox::warning(this, QString("MES Connector Client"), QString("Please reconnect to MES server first!"));
        return false;
    }

    // QFile file(filePath);
    if ((!buffer->isOpen()) && buffer->open(QFile::ReadOnly)) {
        return false;
    }

    QByteArray data;
    quint32 dataSize = buffer->size() + sizeof(quint32);
    QByteArray size = QByteArray::fromRawData(reinterpret_cast<const char *>(&dataSize), sizeof(dataSize));
    std::reverse(size.begin(), size.end());
    data.append(size);
    qDebug() << (quint32) (buffer->size() + 4);
    data.append(buffer->readAll());
    tcpSocket->write(data);

    buffer->close();

    return true;
}

void MESConnectorClient::updateSystemLog(const QString &msg)
{
    if (!msg.isEmpty()) {
        ui->clientLog->append(
            tr("%1 %2").arg(QDateTime::currentDateTime().toString(tr("yyyy-MM-dd hh:mm:ss")), -40, QChar('-')).arg(msg));
    }
}

void MESConnectorClient::on_btn_connect_mes_clicked() {
    tcpSocket->abort();
    tcpSocket->connectToHost(ui->label_mesIP->text(), ui->label_mesPort->text().toInt());

    if (tcpSocket->waitForConnected()) {
        ui->label_status_messerver->setText(tr("Mes Server is Connected."));
        ui->btn_connect_mes->setDisabled(true);
        ui->btn_disconnect_mes->setEnabled(true);
        updateSystemLog(QString("Successful connect to MES server %1:%2")
                            .arg(tcpSocket->peerAddress().toString())
                            .arg(tcpSocket->peerPort()));
    }
}

void MESConnectorClient::on_btn_disconnect_mes_clicked() {
    if (tcpSocket->isOpen()) {
        tcpSocket->close();
        ui->label_status_messerver->setText("Mes Server is Disconnected.");
        ui->btn_connect_mes->setEnabled(true);
        ui->btn_disconnect_mes->setDisabled(true);
        updateSystemLog(tr("Diconnected with MES server."));
    }
}

void MESConnectorClient::on_btn_validate_clicked() {

    // 1. generate XML request file with part id and system info
    partStatus = MESConnectorClient::PartRecevied;
    partIndentifier = ui->edit_partID->text();

    XopconWriter writer(partIndentifier, XopconWriter::PartRecevied);
    QSettings clientSettings("MesConnector", "Client");
    writer.setLineNo(clientSettings.value("connection/mes/lineNo").toString());
    writer.setStatNo(clientSettings.value("connection/mes/statNo").toString());
    writer.setStatIdx(clientSettings.value("connection/mes/statIdx").toString());
    writer.setFuNo(clientSettings.value("connection/mes/fuNo").toString());
    writer.setWorkPos(clientSettings.value("connection/mes/workPos").toString());
    writer.setToolPos(clientSettings.value("connection/mes/toolPos").toString());
    writer.setProcessNo(clientSettings.value("connection/mes/processNo").toString());
    writer.setProcessName(clientSettings.value("connection/mes/processName").toString());
    writer.setApplication(clientSettings.value("connection/mes/application").toString());

    QBuffer buffer;
    buffer.open(QBuffer::ReadWrite);
    if (writer.writeXmlData(&buffer)) {
        updateSystemLog(QObject::tr("XML file is writed!"));
    }

    // 2. send request file to mes server
    buffer.seek(0);
    if (!sendRequest(&buffer)) {
        updateSystemLog("Cannot build communications with MES server");
        return;
    }

    // write Request file to local for debug
    currentUuid = QUuid::createUuid().toString(QUuid::WithoutBraces);
    const QString fileName = QString("%1/Request_%2_%3.xml")
                                 .arg(pathPartReceived, tcpSocket->peerAddress().toString(), currentUuid);
    qDebug() << fileName;
    QFile file(fileName);
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        QMessageBox::warning(this,
                             tr("MES Connector Client"),
                             tr("Cannot write file %1:\n%2").arg(QDir::toNativeSeparators(fileName), file.errorString()));
        return;
    }
    file.write(buffer.buffer());

    // udpate UI
    updateSystemLog("Send XML request to MES server");
    updateSystemLog(tr("PartID = %1, start validate and start PolyWorks").arg(partIndentifier));

    ui->label_status_partvalidation->setText(tr("ID:%1 is recevied").arg(partIndentifier));
    ui->edit_partID->setText("");
    ui->btn_validate->setDisabled(true);

    updateGeometry();
}

void MESConnectorClient::on_edit_partID_textChanged(const QString &arg1)
{
    if (arg1.length() > 5) {
        ui->btn_validate->setEnabled(true);
    } else {
        ui->btn_validate->setDisabled(true);
    }
}

void MESConnectorClient::on_btn_startInspect_clicked()
{
    QtConcurrent::run([this] {

#ifdef NDEBUG
        QString macropath = QString("%1/mscl/FindInspectTemplate.pwmacro").arg(QDir::currentPath());
        qDebug() << macropath;
        QMessageBox::information(this, QString("Path"), macropath);
#else
        QDir dir(QDir::currentPath());
        dir.cd("../../../../");
        QString macropath = QString("%1/src/app/mscl/FindInspectTemplate.pwmacro").arg(dir.path());
        qDebug() << macropath;
#endif

        polyworks = new PolyWorks();
        QString argVar = QString("%1 %2 %3").arg(processNo, typeNo, partIndentifier);
        polyworks->scriptExecute(PolyWorks::MODULE_WORKSPACE,
                                 macropath.toStdWString().c_str(),
                                 argVar.toStdWString().c_str());
    });
    // polyworks = new PolyWorks();
    // QString argVar = QString("%1 %2 %3").arg(processNo, typeNo, partIndentifier);
    // polyworks->scriptExecute(PolyWorks::MODULE_WORKSPACE,
    //                          macropath.toStdWString().c_str(),
    //                          argVar.toStdWString().c_str());

    // qDebug() << argVar;
}

void MESConnectorClient::on_btn_transmit_clicked()
{
    loadSettings();

    if (dataloopEntry.isEmpty() || dataloopToken.isEmpty()) {
        QMessageBox::warning(this,
                             QString("MES Connector Client"),
                             QString("DataLoop entry or token don't setup correctly!"));
        return;
    }

    if (!dataloop)
        dataloop = new DataLoop(this, dataloopEntry, dataloopToken);

    partIndentifier = ui->combo_partList->currentText();
    partStatus = MESConnectorClient::PartProcessed;
    QNetworkReply *reply = dataloop->reqMeasureObjectByPieceID(ui->combo_partList->currentData().toString());
    connect(reply, &QNetworkReply::finished, this, &MESConnectorClient::onDataloopReply);
}

void MESConnectorClient::on_tab_inspection_currentChanged(int index)
{
    if (1 == index) {
        if (!dataloop)
            dataloop = new DataLoop(this, dataloopEntry, dataloopToken);

        QSettings clientSettings("MesConnector", "Client");

        QNetworkReply *reply = dataloop->reqPieceIDs(clientSettings.value("connection/mes/processNo").toString());
        connect(reply, &QNetworkReply::finished, this, [this, reply]() {
            QByteArray data = reply->readAll();
            QJsonDocument json = QJsonDocument::fromJson(data);
            if (json.isEmpty() || json.isNull())
                return;

            ui->combo_partList->clear();
            QJsonArray jsonArray = json.object()["value"].toArray()[0].toObject()["inspectorPieces"].toArray();
            for (const QJsonValueRef item : jsonArray) {
                ui->combo_partList->addItem(item.toObject()["name"].toString(), item.toObject()["id"].toString());
            }
        });
    }
}

void MESConnectorClient::on_tab_connection_currentChanged(int index)
{
    if (1 == index) {
        if (!dataloop)
            dataloop = new DataLoop(this, dataloopEntry, dataloopToken);

        QNetworkReply *reply = dataloop->testConnection();
        connect(reply, &QNetworkReply::finished, this, [this, reply]() {
            QString dataloopConnection = 200 == reply->attribute(QNetworkRequest::HttpStatusCodeAttribute)
                                             ? "Connected"
                                             : "Disconnected";
            ui->label_status_dataloop->setText(dataloopConnection);
            updateSystemLog(QString("DataLoop Connection Test Failed: code = %1")
                                .arg(reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt()));
        });
    }
}

