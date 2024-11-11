#ifndef MESCONNECTORCLIENT_H
#define MESCONNECTORCLIENT_H

#include <QDialog>

QT_BEGIN_NAMESPACE
namespace Ui {
class MESConnectorClient;
}
QT_END_NAMESPACE

class MESConnectorClient : public QDialog
{
    Q_OBJECT

public:
    MESConnectorClient(QWidget *parent = nullptr);
    ~MESConnectorClient();

private:
    Ui::MESConnectorClient *ui;
};
#endif // MESCONNECTORCLIENT_H
