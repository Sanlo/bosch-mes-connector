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

using namespace Qt::StringLiterals;

/*
    MES Connector Client Work Procedure:
1: Setting
    MES ip & port
    Dataloop api and token, optional username and password
    Workstation information
    Optional: language and theme, local debug file path

2: Work flow
    1. Scan Part identifier with SmartScan
    2. Message indicate the status of this part, auto start inspector or warning message for part not for the station
    3. Manually or Automaticly transmit measurement data to MES server for this part
    4. 

*/

MESConnectorClient::MESConnectorClient(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::MESConnectorClient)
    , tcpSocket(new QTcpSocket(this))
    , bodySize(0)
    , readingHeader(true)
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
        updateSystemLog(QString("Client is disconnected from MES server (IP:%1, Port:%2).")
                            .arg(tcpSocket->peerAddress().toString())
                            .arg(tcpSocket->peerPort()));
    });
    // setup signal/slot of DataLoop API
    connect(networkManager, &QNetworkAccessManager::finished, this, &MESConnectorClient::onDataloopReply);
    connect(networkManager, &QNetworkAccessManager::sslErrors, this, [=]() {
        updateSystemLog(QString("DL connection SSL Errror"));
    });
}

MESConnectorClient::~MESConnectorClient()
{
    if (pollingThread && pollingThread->isRunning()) {
        pollingThread->quit();
        pollingThread->wait();
    }

    if (tcpSocket) {
        delete tcpSocket;
        tcpSocket = Q_NULLPTR;
    }

    if (polyworks)
        delete polyworks;

    delete ui;
}

void MESConnectorClient::onServerReply()
{
    // 3. recevie XML response from mes server
    while (tcpSocket->bytesAvailable() > 0) {
        if (readingHeader) {
            if (tcpSocket->bytesAvailable() < HEADER_SIZE) {
                return; // wait until it has enough bytes for the header
            }
            bodySize = qFromBigEndian<quint32>(tcpSocket->read(HEADER_SIZE)) - HEADER_SIZE;
            readingHeader = false;
            buffer.clear();
        }

        if (!readingHeader) {
            // read data body based on the specified bodySize
            if (tcpSocket->bytesAvailable() < bodySize) {
                return; // wait until it has the complete body
            }

            qDebug() << "Server finished data send.";
            buffer = tcpSocket->read(bodySize);
            readingHeader = true;
            break;
        }
    }

    updateSystemLog(QString("Received server reply a xml reply. Size: %1.").arg(bodySize));
    qDebug() << buffer;

    // write Reponse file to local for debug
    if (!pathPartReceived.isEmpty() && !pathPartProcessed.isEmpty()) {
        QFile file(QString("%1/Response_%2_%3_IO.xml")
                       .arg(MESConnectorClient::PartRecevied == partStatus ? pathPartReceived : pathPartProcessed,
                            tcpSocket->peerAddress().toString(),
                            currentUuid));
        if (!file.open(QFile::ReadWrite)) {
            QMessageBox::warning(this,
                                 QString("MES Connector Client"),
                                 QString("Cannot read file %1: %2")
                                     .arg(QDir::toNativeSeparators(pathPartReceived), file.errorString()));
            return;
        }

        file.write(buffer);
    }

    // 4. parse and retrieve data from respone file
    XopconReader xopconReader;
    QBuffer in(&buffer);
    in.open(QBuffer::ReadOnly);
    if (!xopconReader.read(&in)) {
        QMessageBox::warning(this,
                             QString("MES Connector Client"),
                             tr("Parse error in file %1:\n\n%2")
                                 .arg(QDir::toNativeSeparators(pathPartReceived), xopconReader.errorString()));

        return;
    }

    // Retrive response data from server reply
    if (0 != xopconReader.returnCode()) {
        updateSystemLog(QString("Mes Server return error for request. Code:-1"));
        return;
    }

    // 5. respond to the result
    switch (partStatus) {
    case MESConnectorClient::PartRecevied:
        // post process for part recevied
        processNo = xopconReader.nextProcessNo();
        if (processNo.isEmpty() || statNo != processNo.right(3)) {
            statNo = processNo.right(3);
            generateRequest(partIdentifier, statNo);
            break;
        }

        typeNo = xopconReader.typeNo();
        typeVar = xopconReader.typeVar();
        nornimalArray = xopconReader.norminalArray();
        receivedDateTime = xopconReader.timeStamp();

        if (!xopconReader.isValidProcess(processNo)) {
            QString content = tr("Part ID:%1, Bosch MES server reply process ID:%2 isn't suitable for measurement here")
                                  .arg(partIdentifier, processNo);
            QMessageBox::warning(this, QString("MES Connector Client"), content);
            break;
        }

        updateSystemLog(
            tr("Server reply for PartID: %1: partForStation: %2, typeNo: %3, typeVar: %4.\nValidate part " "and start " "Polyworks")
                .arg(partIdentifier, xopconReader.partForStation(), typeNo, typeVar));

        startInspection();
        break;
    default:
        // post process for part inspection
        partStatus = MESConnectorClient::Waiting;

        int item = ui->combo_partList->currentIndex();
        ui->combo_partList->setItemIcon(item, QIcon(":/img/Bosch-logo.png"));
        updateSystemLog(QString("server replied for part processed.(Code:%1)").arg(xopconReader.returnCode()));
        break;
    }
}

void MESConnectorClient::onDataloopReply()
{
    QNetworkReply *reply = reinterpret_cast<QNetworkReply *>(sender());

    QJsonDocument json = QJsonDocument::fromJson(reply->readAll());

    qDebug() << json;

    if (json.isEmpty()) {
        updateSystemLog(QString("DataLoop measurement value is empty"));
        return;
    }
    QJsonArray controlArray = json.object()["value"].toArray();
    if (controlArray.isEmpty())
        return;
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

    generateRequest(partIdentifier, statNo, MESConnectorClient::PartProcessed, objNames, controlNames, measuredList);

    // update UI
    updateSystemLog(QString("Measurement data has been send to MES server, part identifier is %1").arg(partIdentifier));
}

void MESConnectorClient::onAutoTransmit()
{
    qDebug() << "Checking data for upload";
    // check part is upload to dataloop, if not, continue polling
    if (!dataloop)
        dataloop = new DataLoop(this, dataloopEntry, dataloopToken);

    if (MESConnectorClient::PartRecevied != partStatus || statNo.isEmpty()) {
        return;
    }

    // find the part, start to upload xml to Bosch MES server
    QNetworkReply *reply = dataloop->reqPieceIDs(partIdentifier);
    reply->ignoreSslErrors();
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        QByteArray data = reply->readAll();
        QJsonDocument json = QJsonDocument::fromJson(data);
        if (json.isEmpty() || json.isNull())
            return;

        auto value = json.object()["value"].toArray();
        if (value.isEmpty()) {
            qDebug() << "DataLoop don't find this piece. Wait for inspection uploaded.";
            return;
        }

        QDateTime pieceDateTime = QDateTime::fromString(value.at(0).toObject()["creationDateUser"].toString(),
                                                        Qt::ISODateWithMs);
        pieceDateTime.setTimeZone(QTimeZone::utc());
        QDateTime pieceDateTimeLocal = pieceDateTime.toLocalTime();

        if (pieceDateTimeLocal < QDateTime::fromString(receivedDateTime, Qt::ISODateWithMs)) {
            return;
        }

        // set part status for send xml to bosch server
        partStatus = MESConnectorClient::PartProcessed;

        // retreive piece id and filled to combolist
        QString pieceID = value.at(0).toObject()["id"].toString();
        ui->combo_partList->clear();
        ui->combo_partList->addItem(partIdentifier, pieceID);

        // retreive measurement object with current piece id
        QNetworkReply *reply = dataloop->reqMeasureObjectByPieceID(pieceID);
        reply->ignoreSslErrors();
        connect(reply, &QNetworkReply::finished, this, &MESConnectorClient::onDataloopReply);
    });
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
            tr("The connection is refused by the peer.\n" "Make sure the server is running, and " "check the ip and " "port settings"));
        break;
    default:
        QMessageBox::information(this,
                                 tr("MES Connector Client"),
                                 tr("The following error occurred: %1").arg(tcpSocket->errorString()));
    }
}

void MESConnectorClient::displayPolyworksError(int errorCode, const QString &errorMsg)
{
    QMessageBox::critical(this,
                          QString("MES Connector Client"),
                          tr("ERROR: Code:%1.\n%2").arg(QString::number(errorCode), errorMsg));
    updateSystemLog(QString::fromStdWString(polyworks->returnMessage()));
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
    mesPort = clientSettings.value("connection/mesPort").toUInt();
    if (mesIP.isEmpty() || mesPort < 1024) {
        QMessageBox::warning(
            this,
            QString(" MES Connector Client"),
            tr("The IP address and port must be filled before any communication with Bosch MES server"));
    }

    dataloopEntry = clientSettings.value("connection/dlapi").toString();
    dataloopToken = clientSettings.value("connection/dlToken").toString();
    if (dataloopEntry.isEmpty() || dataloopToken.isEmpty()) {
        QMessageBox::warning(this,
                             QString(" MES Connector Client"),
                             tr("The DataLoop API entry and token must be filled."));
    }

    dataloopUser = clientSettings.value("connection/dluser").toString();
    dataloopPwd = clientSettings.value("connection/dlpwd").toString();

    pathPartReceived = clientSettings.value("general/pathReceived").toString();
    pathPartProcessed = clientSettings.value("general/pathProcessed").toString();
    if (!clientSettings.value("connection/mes/statNo").toString().isEmpty()) {
        statNo = clientSettings.value("connection/mes/statNo").toString();
    }
    if (!clientSettings.value("connection/mes/processNo").toString().isEmpty()) {
        processNo = clientSettings.value("connection/mes/processNo").toString();
    }

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

    updateGeometry();
}

void MESConnectorClient::updateUI()
{
    QPixmap bosch(":/img/bosch.png");
    ui->link_bosch->setScaledContents(true);
    ui->link_bosch->setAutoFillBackground(true);
    ui->link_bosch->setPixmap(bosch);
    ui->btn_validate->setDisabled(true);
    ui->btn_transmit->setDisabled(true);
    ui->edit_partID->setFocus();
    ui->tab_inspection->setCurrentIndex(0);
    ui->label_status_partvalidation->setText(tr("Waiting for user input.\nPlease scan part ID with BAR scanner"));
}

void MESConnectorClient::startInspection()
{
    auto future = QtConcurrent::run([this] {

#ifdef NDEBUG
        QString macropathLogin = QString("%1/mscl/LoginDataloop.pwmacro").arg(QDir::currentPath());
        QString macropathInspection = QString("%1/mscl/FindInspectTemplate.pwmacro")
                                          .arg(QDir::currentPath());

#else
        QDir dir(QDir::currentPath());
        dir.cd("../../../../");
        QString macropathLogin = QString("%1/src/app/mscl/LoginDataloop.pwmacro").arg(dir.path());
        QString macropathInspection = QString("%1/src/app/mscl/FindInspectTemplate.pwmacro").arg(dir.path());
        qDebug() << macropathInspection;
#endif
        QApplication::setOverrideCursor(Qt::WaitCursor);
        polyworks = new PolyWorks();

        // Try to login to Dataloop
        if (!dataloopUser.isEmpty() && !dataloopPwd.isEmpty()) {
            QString argLogin = QString("%1 %2").arg(dataloopUser, dataloopPwd);
            polyworks->scriptExecute(PolyWorks::MODULE_WORKSPACE,
                                     macropathLogin.toStdWString().c_str(),
                                     argLogin.toStdWString().c_str());
            if (200 != polyworks->returnCode()) {
                updateSystemLog(QString::fromStdWString(polyworks->returnMessage()));
            }
        }

        // Open Inspector to metrology
        QString argInspection = QString("%1 %2 %3 %4 %5 %6")
                                    .arg(processNo.right(3), partIdentifier, statNo, processNo, typeNo, typeVar);
        polyworks->scriptExecute(PolyWorks::MODULE_WORKSPACE,
                                 macropathInspection.toStdWString().c_str(),
                                 argInspection.toStdWString().c_str());

        QApplication::restoreOverrideCursor();

        if (200 != polyworks->returnCode()) {
            QMetaObject::invokeMethod(this,
                                      "displayPolyworksError",
                                      Qt::QueuedConnection,
                                      Q_ARG(int, polyworks->returnCode()),
                                      Q_ARG(QString, QString::fromStdWString(polyworks->returnMessage())));
        }

    });
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
    if ((!buffer->isOpen()) && buffer->open(QFile::ReadOnly)) {
        return false;
    }

    QByteArray data;
    quint32 dataSize = buffer->size() + sizeof(quint32);
    QByteArray size = QByteArray::fromRawData(reinterpret_cast<const char *>(&dataSize), sizeof(dataSize));
    std::reverse(size.begin(), size.end());
    data.append(size);
    data.append(buffer->readAll());
    tcpSocket->write(data);

#ifdef DEBUG
    qDebug() << (quint32) (buffer->size() + 4);
    updateSystemLog(QString("Current data size is %1, real data size is %2").arg(dataSize).arg(data.size()));
#endif

    return true;
}

void MESConnectorClient::generateRequest(
    const QString &partId,
    const QString &statNo,
    EventType event,
    const QStringList &objNames_,
    const QStringList &controlNames_,
    const QList<double> &measuredList_)
{
    XopconWriter writer(partId, (XopconWriter::EventType) event);
    QSettings clientSettings("MesConnector", "Client");
    writer.setLineNo(clientSettings.value("connection/mes/lineNo").toString());
    writer.setStatIdx(clientSettings.value("connection/mes/statIdx").toString());
    writer.setFuNo(clientSettings.value("connection/mes/fuNo").toString());
    writer.setWorkPos(clientSettings.value("connection/mes/workPos").toString());
    writer.setToolPos(clientSettings.value("connection/mes/toolPos").toString());
    writer.setProcessName(clientSettings.value("connection/mes/processName").toString());
    writer.setApplication(clientSettings.value("connection/mes/application").toString());

    writer.setStatNo(statNo);
    if (processNo.isEmpty()) {
        processNo = clientSettings.value("connection/mes/statNo").toString();
    }
    writer.setProcessNo(processNo);
    writer.setTypeNo(typeNo);
    writer.setTypeVar(typeVar);

    if (MESConnectorClient::PartProcessed == event) {
        writer.setMeasureData(objNames_, controlNames_, measuredList_);
        if (objNames_.size() == (nornimalArray.size() - 2)) {
            writer.setNorminalArray(nornimalArray);
        }
    }

    QBuffer buffer;
    buffer.open(QBuffer::ReadWrite);
    if (writer.writeXmlData(&buffer)) {
        updateSystemLog(tr("Request xml is generated!"));
    }

    // 2. send request file to mes server
    buffer.seek(0);
    if (!sendRequest(&buffer)) {
        updateSystemLog("Cannot build communications with MES server");
        return;
    }

    // write Request file to local for debug
    if (!pathPartReceived.isEmpty() && !pathPartProcessed.isEmpty()) {
        QString filePath;
        if (MESConnectorClient::PartRecevied == event) {
            currentUuid = QUuid::createUuid().toString(QUuid::WithoutBraces);
            filePath = pathPartReceived;
        } else {
            filePath = pathPartProcessed;
        }
        const QString fileName = QString("%1/Request_%2_%3.xml")
                                     .arg(filePath, tcpSocket->peerAddress().toString(), currentUuid);

        QFile file(fileName);
        if (!file.open(QFile::WriteOnly | QFile::Text)) {
            QMessageBox::warning(this,
                                 QString("MES Connector Client"),
                                 tr("Cannot write file %1:\n%2")
                                     .arg(QDir::toNativeSeparators(fileName), file.errorString()));
            return;
        }
        file.write(buffer.buffer());
    }
}

void MESConnectorClient::updateSystemLog(const QString &msg)
{
    if (!msg.isEmpty()) {
        ui->clientLog->append(
            QString("%1 %2").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"), -40, QChar('-')).arg(msg));
        ui->clientLog->verticalScrollBar()->setValue(ui->clientLog->verticalScrollBar()->maximum());
    }
}

void MESConnectorClient::on_btn_validate_clicked() {
    if (mesIP.isEmpty() || mesPort < 1024) {
        QMessageBox::warning(
            this,
            QString(" MES Connector Client"),
            tr("The IP address and port must be filled before any communication with Bosch MES server"));

        return;
    }

    // check if socket available
    if (QAbstractSocket::ConnectedState != tcpSocket->state()) {
        tcpSocket->abort();
        tcpSocket->connectToHost(mesIP, mesPort);

        QApplication::setOverrideCursor(Qt::WaitCursor);
        bool isConnect = tcpSocket->waitForConnected();
        QApplication::restoreOverrideCursor();
        if (!isConnect) {
            return;
        }
        updateSystemLog(QString("Successful connect to MES server %1:%2")
                            .arg(tcpSocket->peerAddress().toString())
                            .arg(tcpSocket->peerPort()));
    }

    partStatus = MESConnectorClient::PartRecevied;
    partIdentifier = ui->edit_partID->text();
    generateRequest(partIdentifier, statNo);

    // udpate UI
    ui->label_status_partvalidation->setText(tr("ID:%1 is recevied").arg(partIdentifier));

    ui->edit_partID->setText("");
    ui->btn_validate->setDisabled(true);
    ui->combo_partList->clear();
    ui->btn_transmit->setDefault(true);

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
    if (partIdentifier.isEmpty() || typeNo.isEmpty()) {
        QMessageBox::warning(this,
                             QString("MES Connector Client"),
                             tr("Part identifier and type number is empty"));

        return;
    }
    startInspection();
}

void MESConnectorClient::on_btn_transmit_clicked()
{
    // loadSettings();

    if (dataloopEntry.isEmpty() || dataloopToken.isEmpty()) {
        QMessageBox::warning(this,
                             QString("MES Connector Client"),
                             QString("DataLoop entry or token don't setup correctly!"));
        return;
    }
    if (!dataloop)
        dataloop = new DataLoop(this, dataloopEntry, dataloopToken);

    partIdentifier = ui->combo_partList->currentText();
    partStatus = MESConnectorClient::PartProcessed;
    QNetworkReply *reply = dataloop->reqMeasureObjectByPieceID(ui->combo_partList->currentData().toString());
    reply->ignoreSslErrors();
    connect(reply, &QNetworkReply::finished, this, &MESConnectorClient::onDataloopReply);
}

void MESConnectorClient::on_tab_inspection_currentChanged(int index)
{
    if (1 != index)
        return;

    if (!dataloop)
        dataloop = new DataLoop(this, dataloopEntry, dataloopToken);

    QNetworkReply *reply = dataloop->reqPieceIDs(partIdentifier);
    reply->ignoreSslErrors();
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        QByteArray data = reply->readAll();
        QJsonDocument json = QJsonDocument::fromJson(data);
        if (json.isEmpty() || json.isNull())
            return;

        // qDebug() << data;

        auto value = json.object()["value"].toArray();
        if (value.isEmpty()) {
            return;
        }

        ui->combo_partList->clear();

        QDateTime pieceDateTime = QDateTime::fromString(value.at(0).toObject()["creationDateUser"].toString(),
                                                        Qt::ISODateWithMs);
        pieceDateTime.setTimeZone(QTimeZone::utc());
        QDateTime pieceDateTimeLocal = pieceDateTime.toLocalTime();
        if (pieceDateTimeLocal < QDateTime::fromString(receivedDateTime, Qt::ISODateWithMs)) {
            return;
        }

        QString pieceID = value.at(0).toObject()["id"].toString();
        ui->combo_partList->addItem(partIdentifier, pieceID);
        ui->btn_transmit->setEnabled(true);

    });
}

void MESConnectorClient::on_btn_clearLog_clicked()
{
    ui->clientLog->clear();
}

void MESConnectorClient::on_btn_copyLog_clicked()
{
    ui->clientLog->selectAll();
    ui->clientLog->copy();
}

void MESConnectorClient::on_checkAutoTransmit_checkStateChanged(const Qt::CheckState &arg1)
{
    bool isAutoTransmit = ui->checkAutoTransmit->isChecked();
    ui->combo_partList->setDisabled(isAutoTransmit);
    ui->btn_transmit->setDisabled(isAutoTransmit);

    if (Qt::Unchecked == arg1) {
        pollingThread->quit();
        pollingThread->wait();
        return;
    }
    auto ok = QMessageBox::question(this,
                                    QString("MES Connector Client"),
                                    tr("Auto Transmit will auto upload measurement result to BOSCH "
                                       "MES Server.\nAre you sure to do that?"),
                                    QMessageBox::Ok | QMessageBox::Cancel,
                                    QMessageBox::Ok);
    if (QMessageBox::Ok != ok) {
        ui->checkAutoTransmit->setCheckState(Qt::Unchecked);
        return;
    }    


    if (isAutoTransmit) {
        // Polling Thread
        pollingThread = new QThread(this);
        pollingTimer = new QTimer(nullptr);
        pollingTimer->setInterval(5000);
        pollingTimer->moveToThread(pollingThread);
        connect(pollingThread, &QThread::started, pollingTimer, [&] { pollingTimer->start(); });
        connect(pollingTimer, &QTimer::timeout, this, &MESConnectorClient::onAutoTransmit);
        pollingThread->start();
    }
}
