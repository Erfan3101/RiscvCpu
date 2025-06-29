#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "memory.h"
#include "registerfile.h"
#include "simulator.h"
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

