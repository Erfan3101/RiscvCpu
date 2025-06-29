#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "memory.h"
#include "registerfile.h"
#include <QLabel>
#include <QPixmap>
#include <QFileDialog>
#include <QMessageBox>
#include <QFile>
#include <QDataStream>
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->registerTable->setRowCount(32);
       ui->registerTable->setColumnCount(2);
       ui->registerTable->setHorizontalHeaderLabels({"Register", "Value"});
       for (int i = 0; i < 32; ++i) {
           ui->registerTable->setItem(i, 0, new QTableWidgetItem(QString("x%1").arg(i)));
           ui->registerTable->setItem(i, 1, new QTableWidgetItem("0"));
       }

       updateStatus();
}


MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_addbutton_clicked()
{
    // Open file dialog to select .bin file
    QString filePath = QFileDialog::getOpenFileName(
        this,
        tr("Select Binary File"),
        QDir::homePath(),
        tr("Binary Files (*.bin);;All Files (*)")
    );

    if (filePath.isEmpty()) {
        return;  // User cancelled
    }

    // Open and read the file
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(this, tr("Error"), tr("Could not open file!"));
        return;
    }

    // Store the data and path for future processing
    binaryData = file.readAll();
    currentFilePath = filePath;
    file.close();

    // Optional: Show success message
    QMessageBox::information(
        this,
        tr("File Loaded"),
        tr("Successfully loaded %1\nSize: %2 bytes")
            .arg(QFileInfo(filePath).fileName())
            .arg(binaryData.size())
    );

    // Now you can access binaryData and currentFilePath from other functions
}


void MainWindow::on_btnStep_clicked(){
    sim.step();
      updateRegisterView();
      updateStatus();
}


void MainWindow::on_btnReset_clicked()
{
    sim = Simulator();
    updateRegisterView();
    updateStatus();
}

void MainWindow::updateRegisterView() {
    for (int i = 0; i < 32; ++i) {
        QString val = QString::number(sim.getRegisterValue(i));
        ui->registerTable->item(i, 1)->setText(val);
    }
}

void MainWindow::updateStatus() {
    ui->labelPC->setText(QString("PC = 0x%1").arg(sim.getPC(), 8, 16, QLatin1Char('0')));
}
