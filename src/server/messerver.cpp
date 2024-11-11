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
