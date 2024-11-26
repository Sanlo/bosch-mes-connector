#ifndef MESCONNECTORCLIENT_H
#define MESCONNECTORCLIENT_H

#include <QDialog>
#include <QNetworkAccessManager>
#include <QTcpSocket>

QT_BEGIN_NAMESPACE
namespace Ui {
class MESConnectorClient;
}
class QTcpSocket;
class QDataStream;
class QNetworkAccessManager;
class DataLoop;
class PolyWorks;
QT_END_NAMESPACE

class MESConnectorClient : public QDialog {
    Q_OBJECT

public:
    enum EventType {
        PartRecevied,
        PartProcessed,
    };
    MESConnectorClient(QWidget *parent = nullptr);
    ~MESConnectorClient();

private slots:
    void onServerReply();
    void onDataloopReply();
    void displayError(QAbstractSocket::SocketError socketError);

    void on_btn_settings_clicked();
    void on_btn_connect_mes_clicked();
    void on_btn_validate_clicked();
    void on_btn_disconnect_mes_clicked();
    void on_edit_partID_textChanged(const QString &arg1);
    void on_btn_startInspect_clicked();
    void on_btn_transmit_clicked();
    void on_tab_inspection_currentChanged(int index);
    void on_tab_connection_currentChanged(int index);
    void on_btn_clearLog_clicked();
    void on_btn_copyLog_clicked();

    void on_btn_connect_dataloop_clicked();

private:
    Ui::MESConnectorClient *ui;

    QString mesIP;
    quint32 mesPort;
    QString dataloopEntry;
    QString dataloopToken;

    QString pathPartReceived;
    QString pathPartProcessed;
    EventType partStatus;
    QString currentUuid;
    QString partIndentifier;
    QString processNo;
    QString typeNo = "A";

    QTcpSocket *tcpSocket = nullptr;
    QNetworkAccessManager *networkManager = nullptr;
    DataLoop *dataloop = nullptr;
    PolyWorks *polyworks = nullptr;

    void loadSettings();
    void updateUI();
    void startInspection();
    bool testDataloopConnection();
    void updateSystemLog(const QString &msg);
    void sendMessage(QAnyStringView msg = nullptr);
    bool sendRequest(QIODevice *buffer);
};
#endif // MESCONNECTORCLIENT_H
