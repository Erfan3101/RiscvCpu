#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "memory.h"
#include "assembler.h"
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
       updateMemoryView(); // مقداردهی اولیه جدول حافظه هنگام باز شدن برنامه
       updateStatus();
}


MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_addbutton_clicked()
{
    sim = Simulator(); // ریست کامل شبیه‌ساز

    QString filePath = QFileDialog::getOpenFileName(
        this,
        tr("Select Binary File"),
        QDir::homePath(),
        tr("Binary Files (*.bin);;All Files (*)")
    );

    if (filePath.isEmpty()) {
        return;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(this, tr("Error"), tr("Could not open file!"));
        return;
    }

    binaryData = file.readAll();
    currentFilePath = filePath;
    file.close();

    // ⚠️ این خط جدید: بارگذاری داده در حافظه شبیه‌ساز
    sim.loadFromRawData(binaryData);
    sim.setPC(0x1000); // نقطه شروع

    updateCurrentInstruction();
    updateRegisterView();
    updateMemoryView();
    updateStatus();

    QMessageBox::information(
        this,
        tr("File Loaded"),
        tr("Successfully loaded %1\nSize: %2 bytes")
            .arg(QFileInfo(filePath).fileName())
            .arg(binaryData.size())
    );
}



void MainWindow::on_btnStep_clicked(){
    sim.step();
      updateRegisterView();
      updateMemoryView();
      updateCurrentInstruction();

      updateStatus();

}


void MainWindow::on_btnReset_clicked()
{
    sim = Simulator();  // بازسازی کامل شبیه‌ساز

       ui->codeEditor->clear();              // خالی کردن ادیتور اسمبلی
       ui->clockLogBox->clear();            // پاک کردن لاگ کلاک‌ه

       currentFilePath.clear();             // ریست مسیر فایل باینری
       binaryData.clear();                  // ریست داده‌های باینری بارگذاری‌شده

       updateRegisterView();                // صفر شدن رجیسترها
       updateMemoryView();                  // حافظه ریست شده
       updateCurrentInstruction();          // پاک شدن نمایش دستور فعلی
       updateStatus();                      // ریست وضعیت PC و کلاک‌ها

}

void MainWindow::updateRegisterView() {
    for (int i = 0; i < 32; ++i) {
        QString val = QString::number(sim.getRegisterValue(i));
        ui->registerTable->item(i, 1)->setText(val);
    }
}

void MainWindow::updateStatus() {
    ui->labelPC->setText(QString("PC = 0x%1").arg(sim.getPC(), 8, 16, QLatin1Char('0')));
    QString logText;
    for (const auto& line : sim.getClockLog()) {
        logText += line + "\n";
    }
    ui->clockLogBox->setText(logText);

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
        updateMemoryView();
        updateStatus();
        updateCurrentInstruction();


        QMessageBox::information(this, "Run Complete", "This is the end...");
}

QString decodeInstruction(uint32_t instr) {
    uint32_t opcode = instr & 0x7F;
    uint32_t rd = (instr >> 7) & 0x1F;
    uint32_t funct3 = (instr >> 12) & 0x7;
    uint32_t rs1 = (instr >> 15) & 0x1F;
    uint32_t rs2 = (instr >> 20) & 0x1F;
    uint32_t funct7 = (instr >> 25) & 0x7F;

    switch (opcode) {
        case 0x33: // R-type
            if (funct3 == 0x0 && funct7 == 0x00) return QString("add x%1, x%2, x%3").arg(rd).arg(rs1).arg(rs2);
            if (funct3 == 0x0 && funct7 == 0x20) return QString("sub x%1, x%2, x%3").arg(rd).arg(rs1).arg(rs2);
            break;
        case 0x13:
            if (funct3 == 0x0) return QString("addi x%1, x%2, imm").arg(rd).arg(rs1);
            break;
    }
    return "-";
}

void MainWindow::updateMemoryView(uint32_t startAddr, int count) {
    ui->memoryTable->setRowCount(count);
    ui->memoryTable->setColumnCount(5);
    ui->memoryTable->setHorizontalHeaderLabels({"Address", "Hex", "Decimal", "ASCII", "Label"});

    for (int i = 0; i < count; ++i) {
        uint32_t addr = startAddr + i;
        uint8_t val = sim.memory1().load_byte(addr); // پیش‌فرض 0
        uint32_t word = sim.memory1().load_word(addr & ~0x3); // هم‌ترازی برای دستور 4 بایتی
        QString label = decodeInstruction(word);
        ui->memoryTable->setItem(i, 0, new QTableWidgetItem(QString("0x%1").arg(addr, 4, 16, QChar('0'))));
        ui->memoryTable->setItem(i, 1, new QTableWidgetItem(QString("0x%1").arg(val, 2, 16, QChar('0'))));
        ui->memoryTable->setItem(i, 2, new QTableWidgetItem(QString::number(val)));
        ui->memoryTable->setItem(i, 3, new QTableWidgetItem((val >= 32 && val <= 126) ? QString(QChar(val)) : "."));
        ui->memoryTable->setItem(i, 4, new QTableWidgetItem(label));

    }
}

QStringList MainWindow::assembleProgramFromText(const QString& asmCode) {
    QStringList lines = asmCode.split('\n', Qt::SkipEmptyParts);
    QMap<QString, int> labels = extractLabels(lines);
    QStringList pureLines = removeLabels(lines);

    QStringList binaryLines;
    for (int i = 0; i < pureLines.size(); ++i) {
        QString binary = assembleLine(pureLines[i], labels, i);
        if (!binary.isEmpty()) {
            binaryLines.append(binary);
        }
    }
    return binaryLines;
}

void MainWindow::loadBinaryLinesToMemory(const QStringList& binaryLines) {
    sim = Simulator(); // ریست
    uint32_t addr = 0x1000;
    for (const QString& bin : binaryLines) {
        bool ok;
        uint32_t instr = bin.toUInt(&ok, 2);
        if (ok) {
            sim.memory1().store_word(addr, instr);
            addr += 4;
        }
    }
    sim.setPC(0x1000); // شروع اجرا از 0x1000

    updateRegisterView();
    updateMemoryView();
    updateStatus();
    updateCurrentInstruction();
}



void MainWindow::on_btnAssembleAndRun_clicked()
{
    QString asmCode = ui->codeEditor->toPlainText();

     QStringList binLines = assembleFromPlainText(asmCode);
     if (binLines.isEmpty()) {
         QMessageBox::critical(this, "Assembly Error", "Could not assemble the code.");
         return;
     }
     sim = Simulator();
     uint32_t addr = 0x1000;
     for (const QString& bin : binLines) {
         bool ok;
         uint32_t instr = bin.toUInt(&ok, 2);
         if (ok) {
             sim.memory1().store_word(addr, instr);
             addr += 4;
         }
     }

     sim.setPC(0x1000);
     updateRegisterView();
     updateMemoryView();
     updateStatus();
     updateCurrentInstruction();

     QMessageBox::information(this, "Assembled!", "Code assembled and loaded into simulator.");
}

