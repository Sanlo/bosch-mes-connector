#ifndef MESSERVER_H
#define MESSERVER_H

#include <QDialog>

namespace Ui {
class MesServer;
}

class MesServer : public QDialog
{
    Q_OBJECT

public:
    explicit MesServer(QWidget *parent = nullptr);
    ~MesServer();

private:
    Ui::MesServer *ui;
};

#endif // MESSERVER_H
