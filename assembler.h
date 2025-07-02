#pragma once
#include <QString>
#include <QStringList>
#include <QMap>

struct InstrFormat {
    QString type;
    QString opcode;
    QString funct3;
    QString funct7;
};

extern QMap<QString, InstrFormat> instrMap;

QStringList assembleProgram(const QStringList& inputLines);
QString assembleLine(const QString& line, const QMap<QString, int>& labels, int currentLine);
QMap<QString, int> extractLabels(const QStringList& lines);
QStringList removeLabels(const QStringList& lines);
QStringList assembleFromPlainText(const QString&);
