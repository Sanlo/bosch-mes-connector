#ifndef SETTINGS_H
#define SETTINGS_H

#include <QDialog>

namespace Ui {
class Settings;
}

class Settings : public QDialog {
  Q_OBJECT

 public:
  explicit Settings(QWidget *parent = nullptr);
  ~Settings();

 private slots:

  void on_btn_settingsApply_clicked();

  void on_btn_settingsOK_clicked();

  void on_toolBtn_received_clicked();

  void on_toolBtn_prcessed_clicked();

  void on_btn_testDL_clicked();

  private:
  Ui::Settings *ui;

  void loadSettings();
};

#endif  // SETTINGS_H
