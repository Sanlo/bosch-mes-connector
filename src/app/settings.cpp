#include <QtCore>

#include "settings.h"
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


void Settings::on_btn_settingsApply_clicked() {

    QSettings clientSettings("MesConnector", "Client");

    QString theme;

    if(ui->radio_light->isChecked()){
        theme=tr("light");
    }
    if(ui->radio_dark->isChecked())
        theme=tr("dark");
    if(ui->radio_auto->isChecked())
        theme=tr("auto");

    // general settings
    clientSettings.setValue("general/theme", theme);
    clientSettings.setValue("general/language", ui->combo_language->currentIndex());

    // connection settings
    clientSettings.setValue("connection/mesIP", ui->edit_mesIP->text());
    clientSettings.setValue("connection/mesPort", ui->edit_mesPort->text());
    clientSettings.setValue("connection/dlapi", ui->edit_dlAPI->text());

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

    accept();
}

void Settings::loadSettings() {
    QSettings clientSettings("MesConnector", "Client");

    // retrieve general settings
    QString theme = clientSettings.value("general/theme").toString();
    if(theme == tr("light")){
        ui->radio_light->setChecked(true);
    }
    if(theme == tr("dark")){
        ui->radio_dark->setChecked(true);
    }
    if(theme == tr("auto")){
        ui->radio_auto->setChecked(true);
    }
    ui->combo_language->setCurrentIndex(clientSettings.value("general/language").toInt());

    // retrieve connection settings
    ui->edit_mesIP->setText(clientSettings.value("connection/mesIP").toString());
    ui->edit_mesPort->setText(clientSettings.value("connection/mesPort").toString());
    ui->edit_dlAPI->setText(clientSettings.value("connection/dlapi").toString());

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

void Settings::on_btn_settingsOK_clicked() {
    on_btn_settingsApply_clicked();
    accept();
}

void Settings::on_btn_settingsCancel_clicked() {
    reject();
}

