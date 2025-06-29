#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "simulator.h"
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
     void on_addButton_clicked();
private slots:
     void on_addbutton_clicked();

     void on_btnStep_clicked();

     void on_btnReset_clicked();

private:
    Ui::MainWindow *ui;
    QByteArray binaryData;        // Store the binary data for future processing
       QString currentFilePath;
       Simulator sim;

       void updateRegisterView();
       void updateStatus();
       // Store the file path
};
#endif // MAINWINDOW_H
