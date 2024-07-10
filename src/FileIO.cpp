#include "FileIO.h"
#include <QFile>
#include <QTextStream>
#include <QFileInfo>

FileIO::FileIO(QObject *parent) : QObject(parent)
{
}

QString FileIO::readFile(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        errorOccurred(QStringLiteral("Could not open file for reading: %1").arg(filePath));
        return QString();
    }

    QTextStream in(&file);
    QString content = in.readAll();
    file.close();
    return content;
}

bool FileIO::writeFile(const QString &filePath, const QString &content)
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        errorOccurred(QStringLiteral("Could not open file for writing: %1").arg(filePath));
        return false;
    }

    QTextStream out(&file);
    out << content;
    file.close();
    return true;
}

bool FileIO::isFileReadable(const QString &filePath)
{
    QFileInfo fileInfo(filePath);
    return fileInfo.exists() && fileInfo.isReadable();
}