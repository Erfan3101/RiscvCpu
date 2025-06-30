#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
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
     void updateCurrentInstruction();
     void updateRegisterView();
     void updateStatus();
     void on_btnAutoRun_clicked();
     void handleAutoRunStep();
     void on_btnRunAll_clicked();

private:
    Ui::MainWindow *ui;
    QByteArray binaryData;        // Store the binary data for future processing
    QString currentFilePath;
    Simulator sim;
    QTimer *autoRunTimer = nullptr;

       // Store the file path
};
#endif // MAINWINDOW_H
