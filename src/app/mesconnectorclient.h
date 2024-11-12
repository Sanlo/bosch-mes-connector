#ifndef MESCONNECTORCLIENT_H
#define MESCONNECTORCLIENT_H

#include <QDialog>
#include <QTcpSocket>

QT_BEGIN_NAMESPACE
namespace Ui {
class MESConnectorClient;
}
class QTcpSocket;
class QDataStream;
QT_END_NAMESPACE

class MESConnectorClient : public QDialog
{
    Q_OBJECT

public:
    MESConnectorClient(QWidget *parent = nullptr);
    ~MESConnectorClient();

private slots:
    void requestPartRecevied();
    void readPartRecevied();
    void displayError(QAbstractSocket::SocketError socketError);

    void on_btn_settings_clicked();
    void on_btn_connect_mes_clicked();

private:
    Ui::MESConnectorClient *ui;

    void loadSettings();
    void updateSystemLog(const QString & msg);

    QTcpSocket *tcpSocket = nullptr;
    QDataStream in;
};
#endif // MESCONNECTORCLIENT_H
