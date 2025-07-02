#include "assembler.h"
#include <QDebug>
#include <QRegularExpression>
#include <QFile>
#include <QIODevice>
#include <QDir>

QMap<QString, InstrFormat> instrMap = {
    {"add",   {"R", "0110011", "000", "0000000"}},
    {"sub",   {"R", "0110011", "000", "0100000"}},
    {"and",   {"R", "0110011", "111", "0000000"}},
    {"or",    {"R", "0110011", "110", "0000000"}},
    {"xor",   {"R", "0110011", "100", "0000000"}},
    {"sll",   {"R", "0110011", "001", "0000000"}},
    {"srl",   {"R", "0110011", "101", "0000000"}},
    {"sra",   {"R", "0110011", "101", "0100000"}},
    {"slt",   {"R", "0110011", "010", "0000000"}},
    {"sltu",  {"R", "0110011", "011", "0000000"}},
    {"addi",  {"I", "0010011", "000", ""}},
    {"lw",    {"I", "0000011", "010", ""}},
    {"lh",    {"I", "0000011", "001", ""}},
    {"sw",    {"S", "0100011", "010", ""}},
    {"sh",    {"S", "0100011", "001", ""}},
    {"beq",   {"B", "1100011", "000", ""}},
    {"bne",   {"B", "1100011", "001", ""}},
    {"blt",   {"B", "1100011", "100", ""}},
    {"bge",   {"B", "1100011", "101", ""}},
    {"bltu",  {"B", "1100011", "110", ""}},
    {"bgeu",  {"B", "1100011", "111", ""}},
    {"jal",   {"J", "1101111", "", ""}},
    {"jalr",  {"I", "1100111", "000", ""}},
    {"lui",   {"U", "0110111", "", ""}},
    {"auipc", {"U", "0010111", "", ""}},
    {"mul",   {"R", "0110011", "000", "0000001"}},
    {"mulh",  {"R", "0110011", "001", "0000001"}},
    {"div",   {"R", "0110011", "100", "0000001"}},
    {"rem",   {"R", "0110011", "110", "0000001"}}
};

// ... اینجا باقی توابع مانند intToBinary، encodeR، encodeI، assembleLine و ... قرار می‌گیرند که از اسمبلر اصلی کپی می‌کنی ...
int regToNum(const QString& reg) {
    static QMap<QString, int> regMap = {
        {"R0",0},{"R1",1},{"R2",2},{"R3",3},
        {"R4",4},{"R5",5},{"R6",6},{"R7",7}
    };
    return regMap.value(reg.toUpper(), -1);
}

// ØªØ¨Ø¯ÛŒÙ„ Ø¹Ø¯Ø¯ ØµØ­ÛŒØ­ Ø¨Ù‡ Ø±Ø´ØªÙ‡ Ø¨Ø§ÛŒÙ†Ø±ÛŒ Ø¨Ø§ Ø·ÙˆÙ„ bits (Ù…Ú©Ù…Ù„ Ø¯Ùˆ Ø¨Ø±Ø§ÛŒ Ø§Ø¹Ø¯Ø§Ø¯ Ù…Ù†ÙÛŒ)
QString intToBinary(int num, int bits) {
    if (bits <= 0) return "";
    unsigned int mask = (1u << bits) - 1;
    unsigned int val = static_cast<unsigned int>(num) & mask;
    QString bin;
    for (int i = bits - 1; i >= 0; --i) {
        bin += ((val >> i) & 1) ? '1' : '0';
    }
    return bin;
}

// ØªÙˆØ§Ø¨Ø¹ Ø±Ù…Ø²Ú¯Ø°Ø§Ø±ÛŒ Ø§Ù†ÙˆØ§Ø¹ Ø¯Ø³ØªÙˆØ±Ù‡Ø§:
QString encodeR(int rd, int rs1, int rs2, const InstrFormat& fmt) {
    // funct7(7) | rs2(3) | rs1(3) | funct3(3) | rd(3) | opcode(7)
    return fmt.funct7 +
        intToBinary(rs2, 5) +
        intToBinary(rs1, 5) +
        fmt.funct3 +
        intToBinary(rd, 5) +
        fmt.opcode;
}

QString encodeI(int rd, int rs1, int imm, const InstrFormat& fmt) {
    // imm(6) | rs1(3) | funct3(3) | rd(3) | opcode(7)
    return intToBinary(imm, 12) +
        intToBinary(rs1, 5) +
        fmt.funct3 +
        intToBinary(rd, 5) +
        fmt.opcode;
}

QString encodeS(int rs1, int rs2, int imm, const InstrFormat& fmt) {
    // imm(6) split: imm[5:3], imm[2:0]
    QString imm_bin = intToBinary(imm, 12);
    QString imm_11_5 = imm_bin.mid(0, 7);
    QString imm_4_0 = imm_bin.mid(7, 5);
    // imm[5:3] | rs2(3) | rs1(3) | funct3(3) | imm[2:0] | opcode(7)
    return imm_11_5 +
        intToBinary(rs2, 5) +
        intToBinary(rs1, 5) +
        fmt.funct3 +
        imm_4_0 +
        fmt.opcode;
}

QString encodeB(int rs1, int rs2, int imm, const InstrFormat& fmt) {
    // imm(6) split as needed â€” Ø¨Ø±Ø§ÛŒ Ù¾Ø±ÙˆÚ˜Ù‡ ÙØ±Ø¶ Ù…ÛŒâ€ŒÚ©Ù†ÛŒÙ… Ù‡Ù…ÛŒÙ† 6 Ø¨ÛŒØª Ø¨Ø§ Ø¬Ø§Ú¯Ø°Ø§Ø±ÛŒ Ø³Ø§Ø¯Ù‡
    QString imm_bin = intToBinary(imm, 13);
    QString imm12 = imm_bin.mid(0, 1);
    QString imm10_5 = imm_bin.mid(1, 6);
    QString imm4_1 = imm_bin.mid(7, 4);
    QString imm11 = imm_bin.mid(11, 1);
    return imm12 +
        imm10_5 +
        intToBinary(rs2, 5) +
        intToBinary(rs1, 5) +
        fmt.funct3 +
        imm4_1 +
        imm11 +
        "000" + // placeholder for Ø¨ÛŒØªâ€ŒÙ‡Ø§ÛŒ imm Ø§Ø¶Ø§ÙÛŒ Ø§Ú¯Ø± Ù†ÛŒØ§Ø² Ø¨Ø§Ø´Ù‡
        fmt.opcode;
}

QString encodeU(int rd, int imm, const InstrFormat& fmt) {
    // imm(9) | rd(3) | opcode(7)
    return intToBinary(imm, 20) +
        intToBinary(rd, 5) +
        fmt.opcode;
}

QString encodeJ(int rd, int imm, const InstrFormat& fmt) {
    // imm(11) | rd(3) | opcode(7)
    return intToBinary(imm, 20) +
        intToBinary(rd, 5) +
        fmt.opcode;
}

// Ù…Ø±Ø­Ù„Ù‡ Ø§ÙˆÙ„: Ø§Ø³ØªØ®Ø±Ø§Ø¬ LabelÙ‡Ø§ (Ø¢Ø¯Ø±Ø³ Ø®Ø·) Ø§Ø² Ù„ÛŒØ³Øª Ø¯Ø³ØªÙˆØ±Ø§Øª
QMap<QString, int> extractLabels(const QStringList& lines) {
    QMap<QString, int> labels;
    for (int i = 0; i < lines.size(); ++i) {
        QString line = lines[i].trimmed();
        if (line.endsWith(":")) {
            QString label = line.left(line.length() - 1);
            labels[label] = i; // Ø¢Ø¯Ø±Ø³ Ø®Ø·
        }
    }
    return labels;
}

// Ø­Ø°Ù Ø®Ø·ÙˆØ· Label Ø§Ø² Ù„ÛŒØ³Øª Ø¯Ø³ØªÙˆØ±Ø§Øª (Ø¨Ø±Ø§ÛŒ Ù…Ø±Ø­Ù„Ù‡ Ø¯ÙˆÙ…)
QStringList removeLabels(const QStringList& lines) {
    QStringList result;
    for (const QString& line : lines) {
        if (!line.trimmed().endsWith(":"))
            result.append(line);
    }
    return result;
}

// Ø§Ø³ØªØ®Ø±Ø§Ø¬ Ù‚Ø³Ù…Øª Ø§ÙˆÙ„ (Ù†Ø§Ù… Ø¯Ø³ØªÙˆØ±) Ø§Ø² Ø®Ø·
QString getInstrName(const QString& line) {
    QString trimmed = line.trimmed();
    int spaceIdx = trimmed.indexOf(' ');
    if (spaceIdx == -1)
        return trimmed.toLower();
    return trimmed.left(spaceIdx).toLower();
}

// ØªØ¨Ø¯ÛŒÙ„ ÛŒÚ© Ø¯Ø³ØªÙˆØ± Ø¨Ù‡ Ø¨Ø§ÛŒÙ†Ø±ÛŒ Ø¨Ø§ Ø¬Ø§ÛŒÚ¯Ø°Ø§Ø±ÛŒ Ø¨Ø±Ú†Ø³Ø¨â€ŒÙ‡Ø§ (offset)
QString assembleLine(const QString& line, const QMap<QString, int>& labels, int currentLine) {
    if (line.trimmed().isEmpty()) return "";

    QStringList parts = line.split(QRegularExpression("[ ,()\t]+"), Qt::SkipEmptyParts);
    if (parts.isEmpty()) return "";

    QString instr = parts[0].toLower();

    if (!instrMap.contains(instr)) {
        qDebug() << "Unknown command:" << instr;
        return "";
    }

    InstrFormat fmt = instrMap[instr];

    // Ø¨Ø³ØªÙ‡ Ø¨Ù‡ Ù†ÙˆØ¹ Ø¯Ø³ØªÙˆØ±ØŒ Ù¾Ø§Ø±Ø§Ù…ØªØ±Ù‡Ø§ Ø±Ø§ Ù¾Ø±Ø¯Ø§Ø²Ø´ Ú©Ù†
    if (fmt.type == "R") {
        if (parts.size() != 4) {
            qDebug() << "The R command format is incorrect.:" << line;
            return "";
        }
        int rd = regToNum(parts[1]);
        int rs1 = regToNum(parts[2]);
        int rs2 = regToNum(parts[3]);
        if (rd < 0 || rs1 < 0 || rs2 < 0) {
            qDebug() << "Invalid register in:" << line;
            return "";
        }
        return encodeR(rd, rs1, rs2, fmt);
    }
    else if (fmt.type == "I") {
        if (parts.size() < 3) {
            qDebug() << "The format of the I command is incorrect:" << line;
            return "";
        }
        int rd = regToNum(parts[1]);
        int rs1 = 0;
        int imm = 0;
        bool ok = false;

        // Ø¯Ø³ØªÙˆØ± jalr Ø­Ø§Ù„Øª Ø®Ø§ØµÛŒ Ø¯Ø§Ø±Ø¯ (Ù…Ø«Ù„Ø§Ù‹ jalr rd, imm(rs1))
        if (instr == "jalr" && parts.size() == 4) {
            imm = parts[2].toInt(&ok);
            rs1 = regToNum(parts[3]);
            if (!ok || rd < 0 || rs1 < 0) {
                qDebug() << "Invalid username or register:" << line;
                return "";
            }
        }
        else {
            rs1 = regToNum(parts[2]);
            imm = parts[3].toInt(&ok);
            if (!ok || rd < 0 || rs1 < 0) {
                qDebug() << "Invalid ID or register:" << line;
                return "";
            }
        }
        return encodeI(rd, rs1, imm, fmt);
    }
    else if (fmt.type == "S") {
        if (parts.size() != 4) {
            qDebug() << "S command format is incorrect:" << line;
            return "";
        }
        int rs2 = regToNum(parts[1]);
        int imm = parts[2].toInt();
        int rs1 = regToNum(parts[3]);
        if (rs1 < 0 || rs2 < 0) {
            qDebug() << "Invalid register in:" << line;
            return "";
        }
        return encodeS(rs1, rs2, imm, fmt);
    }
    else if (fmt.type == "B") {
        if (parts.size() != 4) {
            qDebug() << "The format of the command B is incorrect:" << line;
            return "";
        }
        int rs1 = regToNum(parts[1]);
        int rs2 = regToNum(parts[2]);

        QString labelOrImm = parts[3];
        int imm = 0;

        // Ø§Ú¯Ø± Ø¢Ø®Ø±ÛŒÙ† Ù¾Ø§Ø±Ø§Ù…ØªØ± Ø¨Ø±Ú†Ø³Ø¨ Ø¨ÙˆØ¯
        if (labels.contains(labelOrImm)) {
            int targetLine = labels[labelOrImm];
            imm = (targetLine - currentLine - 1) * 2; // offset (ÙØ±Ø¶ÛŒ)
        }
        else {
            bool ok;
            imm = labelOrImm.toInt(&ok);
            if (!ok) {
                qDebug() << "Invalid label or prompt address:" << line;
                return "";
            }
        }
        if (rs1 < 0 || rs2 < 0) {
            qDebug() << "Invalid register:" << line;
            return "";
        }
        return encodeB(rs1, rs2, imm, fmt);
    }
    else if (fmt.type == "U") {
        if (parts.size() != 3) {
            qDebug() << "The format of the U command is incorrect:" << line;
            return "";
        }
        int rd = regToNum(parts[1]);
        int imm = parts[2].toInt();
        if (rd < 0) {
            qDebug() << "Invalid register:" << line;
            return "";
        }
        return encodeU(rd, imm, fmt);
    }
    else if (fmt.type == "J") {
        if (parts.size() != 3) {
            qDebug() << "The format of the J command is incorrect:" << line;
            return "";
        }
        int rd = regToNum(parts[1]);
        if (rd < 0) {
            qDebug() << "Invalid register:" << line;
            return "";
        }

        QString labelOrImm = parts[2];
        int imm = 0;

        // Ø§Ú¯Ø± Ù¾Ø§Ø±Ø§Ù…ØªØ± Ø³ÙˆÙ… Ø¨Ø±Ú†Ø³Ø¨ Ø§Ø³ØªØŒ Ø¢Ø¯Ø±Ø³ Ø¢Ù† Ø±Ø§ Ù¾ÛŒØ¯Ø§ Ú©Ù†
        if (labels.contains(labelOrImm)) {
            int targetLine = labels[labelOrImm];
            imm = (targetLine - currentLine - 1) * 2; // offset ÙØ±Ø¶ÛŒ (2 Ø¨Ø§ÛŒØª Ù‡Ø± Ø¯Ø³ØªÙˆØ±)
        }
        else {
            bool ok;
            imm = labelOrImm.toInt(&ok);
            if (!ok) {
                qDebug() << "Invalid label or prompt address:" << line;
                return "";
            }
        }

        return encodeJ(rd, imm, fmt);
    }

    qDebug() << "Unknown command type on line:" << line;
    return "";
}
QStringList assembleProgram(const QStringList& inputLines) {
    // Ù…Ø±Ø­Ù„Ù‡ Ø§ÙˆÙ„: Ø§Ø³ØªØ®Ø±Ø§Ø¬ Ø¨Ø±Ú†Ø³Ø¨â€ŒÙ‡Ø§ (LabelÙ‡Ø§)
    QMap<QString, int> labels = extractLabels(inputLines);

    // Ù…Ø±Ø­Ù„Ù‡ Ø¯ÙˆÙ…: Ø­Ø°Ù Ø®Ø·ÙˆØ· Ø¨Ø±Ú†Ø³Ø¨â€ŒÙ‡Ø§ Ùˆ Ú¯Ø±ÙØªÙ† ÙÙ‚Ø· Ø¯Ø³ØªÙˆØ±Ø§Øª
    QStringList lines = removeLabels(inputLines);

    QStringList binaryLines;

    // ØªØ¨Ø¯ÛŒÙ„ Ù‡Ø± Ø¯Ø³ØªÙˆØ± Ø¨Ù‡ Ø¨Ø§ÛŒÙ†Ø±ÛŒ
    for (int i = 0; i < lines.size(); ++i) {
        QString binary = assembleLine(lines[i], labels, i);
        if (!binary.isEmpty()) {
            binaryLines.append(binary);
        }
        else {
            qDebug() << "Error converting command on line number" << i + 1 << ":" << lines[i];
        }
    }
    return binaryLines;
}
// تابع جدید برای استفاده مستقیم در شبیه‌ساز
QVector<quint32> assembleBinary(const QString& codeText) {
    QStringList lines = codeText.split('\n', Qt::SkipEmptyParts);
    QMap<QString, int> labels = extractLabels(lines);
    QStringList noLabelLines = removeLabels(lines);
    QVector<quint32> binary;

    for (int i = 0; i < noLabelLines.size(); ++i) {
        QString bin = assembleLine(noLabelLines[i], labels, i);
        if (!bin.isEmpty()) {
            bool ok;
            quint32 instr = bin.toUInt(&ok, 2);
            if (ok) binary.append(instr);
        }
    }
    return binary;
}
QStringList assembleFromPlainText(const QString& code) {
    QStringList lines = code.split('\n', Qt::SkipEmptyParts);
    QMap<QString, int> labels = extractLabels(lines);
    QStringList pureLines = removeLabels(lines);

    QStringList binaryLines;
    for (int i = 0; i < pureLines.size(); ++i) {
        QString binary = assembleLine(pureLines[i], labels, i);
        if (!binary.isEmpty()) {
            binaryLines.append(binary);
        } else {
            qDebug() << "Error assembling line " << i << ":" << pureLines[i];
        }
    }
    return binaryLines;
}
