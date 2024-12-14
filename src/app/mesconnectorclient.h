#ifndef MESCONNECTORCLIENT_H
#define MESCONNECTORCLIENT_H

#include <QDialog>
#include <QNetworkAccessManager>
#include <QTcpSocket>

#include "xopconreader.h"
#include "xopconwriter.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MESConnectorClient;
}
class QTcpSocket;
class QDataStream;
class QNetworkAccessManager;
class DataLoop;
class PolyWorks;
class QTimer;
class QThread;
QT_END_NAMESPACE

class MESConnectorClient : public QDialog {
    Q_OBJECT

public:
    enum EventType {
        PartRecevied,
        PartProcessed,
        Waiting,
    };
    MESConnectorClient(QWidget *parent = nullptr);
    ~MESConnectorClient();

private slots:
    void onServerReply();
    void onDataloopReply();
    void onAutoTransmit();
    void displayError(QAbstractSocket::SocketError socketError);
    void displayPolyworksError(int errorCode, const QString &errorMsg);

    void on_btn_settings_clicked();
    void on_btn_validate_clicked();
    void on_edit_partID_textChanged(const QString &arg1);
    void on_btn_startInspect_clicked();
    void on_btn_transmit_clicked();
    void on_tab_inspection_currentChanged(int index);
    void on_btn_clearLog_clicked();
    void on_btn_copyLog_clicked();
    void on_checkAutoTransmit_checkStateChanged(const Qt::CheckState &arg1);

private:
    Ui::MESConnectorClient *ui;

    QString mesIP;
    quint32 mesPort;
    QString dataloopEntry;
    QString dataloopToken;
    QString dataloopUser;
    QString dataloopPwd;

    QString pathPartReceived;
    QString pathPartProcessed;
    EventType partStatus;
    QString currentUuid;
    QString partIdentifier;
    QString statNo{"620"};
    QString processNo;
    QString typeNo{"00"};
    QString typeVar{"00"};
    QString receivedDateTime;
    QList<Norminal> nornimalArray;

    QTcpSocket *tcpSocket = nullptr;
    const int HEADER_SIZE = sizeof(qint32);
    QByteArray buffer;
    qint32 bodySize;
    bool readingHeader;

    QNetworkAccessManager *networkManager = nullptr;
    DataLoop *dataloop = nullptr;
    PolyWorks *polyworks = nullptr;
    QTimer *pollingTimer = nullptr;
    QThread *pollingThread = nullptr;

    void loadSettings();
    void updateUI();
    void startInspection();
    void updateSystemLog(const QString &msg);
    void sendMessage(QAnyStringView msg = nullptr);
    bool sendRequest(QIODevice *buffer);
    void generateRequest(const QString &partId, const QString &statNo);
};
#endif // MESCONNECTORCLIENT_H
