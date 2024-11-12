#ifndef MESSERVER_H
#define MESSERVER_H

#include <QDialog>

QT_BEGIN_NAMESPACE
class QTcpServer;
QT_END_NAMESPACE

namespace Ui {
class MesServer;
}

class MesServer : public QDialog
{
    Q_OBJECT

public:
    explicit MesServer(QWidget *parent = nullptr);
    ~MesServer();


private slots:
    void sendReply();

    void on_btn_clear_serverLog_clicked();

    void on_btn_copy_severLog_clicked();

    void on_btn_startServer_clicked();

private:
    Ui::MesServer *ui;

    void updateSystemLog(const QString& msg);

    QTcpServer *tcpServer = nullptr;
};

#endif // MESSERVER_H
