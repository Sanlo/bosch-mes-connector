#ifndef SETTINGS_H
#define SETTINGS_H

#include <QDialog>

namespace Ui {
class Settings;
}

class Settings : public QDialog
{
    Q_OBJECT

public:
    explicit Settings(QWidget *parent = nullptr);
    ~Settings();

private slots:

    void on_btn_settingsApply_clicked();

    void on_btn_settingsOK_clicked();

    void on_btn_settingsCancel_clicked();

private:
    Ui::Settings *ui;

    void loadSettings();
};

#endif // SETTINGS_H
