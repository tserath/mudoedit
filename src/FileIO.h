#ifndef FILEIO_H
#define FILEIO_H

#include <QString>
#include <QObject>

class QTextDocument;

// This class handles file reading and writing operations
class FileIO : public QObject
{
    Q_OBJECT

public:
    explicit FileIO(QObject *parent = nullptr);

    // Read the contents of a file
    QString readFile(const QString &filePath);

    // Write content to a file
    bool writeFile(const QString &filePath, const QString &content);

    // Check if a file exists and is readable
    bool isFileReadable(const QString &filePath);

Q_SIGNALS:
    // Signal emitted when a file operation encounters an error
    void errorOccurred(const QString &errorMessage);
};

#endif // FILEIO_H