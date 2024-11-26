#include "settings.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QNetworkReply>
#include <QtCore>

#include "dataloop.h"
#include "ui_settings.h"

Settings::Settings(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Settings)
{
    ui->setupUi(this);

    loadSettings();
}

Settings::~Settings()
{
    delete ui;
}

void Settings::on_btn_settingsApply_clicked()
{
    QString theme;
    if (ui->radio_light->isChecked()) {
        theme = "light";
    }
    if (ui->radio_dark->isChecked())
        theme = "dark";
    if (ui->radio_auto->isChecked())
        theme = "auto";

    QSettings clientSettings("MesConnector", "Client");
    // general settings
    clientSettings.setValue("general/theme", theme);
    clientSettings.setValue("general/language", ui->combo_language->currentIndex());

    // connection settings
    clientSettings.setValue("connection/mesIP", ui->edit_mesIP->text());
    clientSettings.setValue("connection/mesPort", ui->edit_mesPort->text());
    clientSettings.setValue("connection/dlapi", ui->edit_dlAPI->text());
    clientSettings.setValue("connection/dlToken", ui->edit_dlToken->text());

    // workstation infomation
    clientSettings.setValue("connection/mes/lineNo", ui->edit_mes_lineNo->text());
    clientSettings.setValue("connection/mes/statNo", ui->edit_mes_statNo->text());
    clientSettings.setValue("connection/mes/statIdx", ui->edit_mes_statIdx->text());
    clientSettings.setValue("connection/mes/fuNo", ui->edit_mes_fuNo->text());
    clientSettings.setValue("connection/mes/workPos", ui->edit_mes_workPos->text());
    clientSettings.setValue("connection/mes/toolPos", ui->edit_mes_toolPos->text());
    clientSettings.setValue("connection/mes/processNo", ui->edit_mes_processNo->text());
    clientSettings.setValue("connection/mes/processName", ui->edit_mes_processName->text());
    clientSettings.setValue("connection/mes/application", ui->edit_mes_application->text());
}

void Settings::loadSettings()
{
    QSettings clientSettings("MesConnector", "Client");

    // retrieve general settings
    QString theme = clientSettings.value("general/theme").toString();
    if (theme == QString("light")) {
        ui->radio_light->setChecked(true);
    }
    if (theme == QString("dark")) {
        ui->radio_dark->setChecked(true);
    }
    if (theme == QString("auto")) {
        ui->radio_auto->setChecked(true);
    }
    ui->combo_language->setCurrentIndex(clientSettings.value("general/language").toInt());
    ui->edit_path_recevied->setText(clientSettings.value("general/pathReceived").toString());
    ui->edit_path_processed->setText(clientSettings.value("general/pathProcessed").toString());

    // retrieve connection settings
    ui->edit_mesIP->setText(clientSettings.value("connection/mesIP").toString());
    ui->edit_mesPort->setText(clientSettings.value("connection/mesPort").toString());
    ui->edit_dlAPI->setText(clientSettings.value("connection/dlapi").toString());
    ui->edit_dlToken->setText(clientSettings.value("connection/dlToken").toString());

    // retrieve workstation settings
    ui->edit_mes_lineNo->setText(clientSettings.value("connection/mes/lineNo").toString());
    ui->edit_mes_statNo->setText(clientSettings.value("connection/mes/statNo").toString());
    ui->edit_mes_statIdx->setText(clientSettings.value("connection/mes/statIdx").toString());
    ui->edit_mes_fuNo->setText(clientSettings.value("connection/mes/fuNo").toString());
    ui->edit_mes_workPos->setText(clientSettings.value("connection/mes/workPos").toString());
    ui->edit_mes_toolPos->setText(clientSettings.value("connection/mes/toolPos").toString());
    ui->edit_mes_processNo->setText(clientSettings.value("connection/mes/processNo").toString());
    ui->edit_mes_processName->setText(clientSettings.value("connection/mes/processName").toString());
    ui->edit_mes_application->setText(clientSettings.value("connection/mes/application").toString());
}

void Settings::on_btn_settingsOK_clicked()
{
    on_btn_settingsApply_clicked();
    accept();
}

void Settings::on_toolBtn_received_clicked()
{
    QString dir = QFileDialog::getExistingDirectory(this,
                                                    tr("Select Folder for Part Recevied"),
                                                    QDir::currentPath(),
                                                    QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if (dir.isEmpty())
        return;

    qDebug() << dir;

    ui->edit_path_recevied->setText(dir);

    QSettings clientSettings("MesConnector", "Client");
    clientSettings.setValue("general/pathReceived", dir);
}

void Settings::on_toolBtn_prcessed_clicked()
{
    QString dir = QFileDialog::getExistingDirectory(this,
                                                    tr("Select Folder for Part Processed"),
                                                    QDir::currentPath(),
                                                    QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if (dir.isEmpty())
        return;

    qDebug() << dir;

    ui->edit_path_processed->setText(dir);

    QSettings clientSettings("MesConnector", "Client");
    clientSettings.setValue("general/pathProcessed", dir);
}

void Settings::on_btn_testDL_clicked()
{
    if (ui->edit_dlAPI->text().isEmpty() || ui->edit_dlToken->text().isEmpty()) {
        QMessageBox::warning(this,
                             QString("MES Connector Settings"),
                             QString("DataLoop API entry or token don't setup correctly!"));
        return;
    }

    DataLoop *dataloop = new DataLoop(this, ui->edit_dlAPI->text(), ui->edit_dlToken->text());

    QNetworkReply *reply = dataloop->testConnection();
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        if (200 == reply->attribute(QNetworkRequest::HttpStatusCodeAttribute)) {
            QPixmap pixmap(":/img/check_green.svg");
            // pixmap.scaled(QSize(48, 48));
            ui->label_dlStatus->setPixmap(pixmap);
            ui->label_dlStatus->setScaledContents(true);
        }
    });
}

