#include "mesconnectorclient.h"
#include "./ui_mesconnectorclient.h"

MESConnectorClient::MESConnectorClient(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::MESConnectorClient)
{
    ui->setupUi(this);
}

MESConnectorClient::~MESConnectorClient()
{
    delete ui;
}
