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
     void updateMemoryView(uint32_t startAddr = 0x1000, int numWords = 16);
     void on_btnSendInput_clicked();


private:
    Ui::MainWindow *ui;
    QByteArray binaryData;        // Store the binary data for future processing
    QString currentFilePath;
    Simulator sim;
    QTimer *autoRunTimer = nullptr;
    QString userInput;
       // Store the file path
};
#endif // MAINWINDOW_H
