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
   // sim = Simulator(); // ریست

    // سه دستور ساده:
   // sim.memory1().store_word(0x1000, 0x00A00093); // addi x1, x0, 10
    //sim.memory1().store_word(0x1004, 0x01400113); // addi x2, x0, 20
   // sim.memory1().store_word(0x1008, 0x002081B3); // add x3, x1, x2

    //updateRegisterView();
   //updateStatus();
   // updateCurrentInstruction();

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
    updateCurrentInstruction();
    updateMemoryView();

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
      updateCurrentInstruction();
      updateMemoryView();
      updateStatus();

}


void MainWindow::on_btnReset_clicked()
{
    sim = Simulator();
    updateRegisterView();
    updateCurrentInstruction();
    updateMemoryView();
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
void MainWindow::updateCurrentInstruction() {
    uint32_t pc = sim.getPC();
    uint32_t instr = sim.memory1().load_word(pc);

    ui->labelInstr->setText(QString("Instr @ 0x%1: 0x%2")
                            .arg(pc, 8, 16, QLatin1Char('0'))
                            .arg(instr, 8, 16, QLatin1Char('0')));
}

void MainWindow::on_btnAutoRun_clicked()
{
    if (!autoRunTimer) {
           autoRunTimer = new QTimer(this);
           connect(autoRunTimer, &QTimer::timeout, this, &MainWindow::handleAutoRunStep);
       }
       updateMemoryView();
       autoRunTimer->start(2000);// هر2 میلی‌ثانیه یک گام اجرا می‌شود
}

void MainWindow::handleAutoRunStep() {
    uint32_t instr = sim.memory1().load_word(sim.getPC());
    if (instr == 0x00000000) {
        autoRunTimer->stop();
        QMessageBox::information(this, "Auto Run", "This is the end...!");
        return;
    }

    sim.step();
    updateRegisterView();
    updateStatus();
    updateCurrentInstruction();
}

void MainWindow::on_btnRunAll_clicked()
{
    int steps = 0;
        while (true) {
            uint32_t instr = sim.memory1().load_word(sim.getPC());
            if (instr == 0x00000000 || steps > 1000) break; // پایان یا جلوگیری از loop بی‌نهایت

            sim.step();
            steps++;
        }

        updateRegisterView();
        updateStatus();
        updateCurrentInstruction();
        updateMemoryView();

        QMessageBox::information(this, "Run Complete", "This is the end...");
}

void MainWindow::updateMemoryView(uint32_t start, int count) {
    ui->memoryTable->setRowCount(count);
    ui->memoryTable->setColumnCount(2);
    ui->memoryTable->setHorizontalHeaderLabels({"Address", "Value"});

    for (int i = 0; i < count; ++i) {
        uint32_t addr = start + i * 4;
        uint32_t val = sim.memory1().load_word(addr);

        auto *addrItem = new QTableWidgetItem(QString("0x%1").arg(addr, 8, 16, QLatin1Char('0')));
        auto *valItem  = new QTableWidgetItem(QString("0x%1").arg(val, 8, 16, QLatin1Char('0')));

        ui->memoryTable->setItem(i, 0, addrItem);
        ui->memoryTable->setItem(i, 1, valItem);
    }
}

