#include "jsonpacker.h"

#include <QFile>
#include <QDir>

#include <QJsonDocument>
#include <QJsonValue>
#include <QJsonObject>
#include <QJsonArray>

JsonPacker::JsonPacker(QObject *parent)
    : QObject{parent}, _uidKeyName{"id"}, _allowOverwrite{false}, _disableCleanup{false}, _disableArrayInlining{false}
{}

void JsonPacker::setUidKeyName(const QString &newUidKeyName) {
    _uidKeyName = newUidKeyName;
}

void JsonPacker::setAllowOverwrite(bool newAllowOverwrite) {
    _allowOverwrite = newAllowOverwrite;
}

void JsonPacker::setDisableCleanup(bool newDisableCleanup) {
    _disableCleanup = newDisableCleanup;
}

void JsonPacker::setDisableArrayInlining(bool newDisableArrayInlining) {
    _disableArrayInlining = newDisableArrayInlining;
}

bool JsonPacker::unpack(const QString &sourceFilePath, const QString &destinationDirectoryPath) {
    QFile sourceFile(sourceFilePath);
    QDir destinationDir(destinationDirectoryPath);

    if(!sourceFile.open(QFile::ReadOnly)) {
        emit message(ErrorMessage, ImportantMessage, tr("Failed to read from source file: %1").arg(sourceFile.errorString()));
        return false;
    }

    if(isFileInsideDir(sourceFilePath, destinationDirectoryPath)) {
        emit message(ErrorMessage, ImportantMessage, tr("Invalid destination directory. It cannot be a parent of the source file."));
        return 1;
    }

    if(destinationDir.exists() && !destinationDir.isEmpty() && !_allowOverwrite) {
        emit message(ErrorMessage, ImportantMessage, tr("Failed to write into destination dir. It must be empty. User another directory or try --allow-overwrite."));
        return false;
    }

    if(!destinationDir.exists() && !destinationDir.mkpath(".")) {
        emit message(ErrorMessage, ImportantMessage, tr("Failed to write into destination dir. Failed to create it."));
        return false;
    }

    const QByteArray jsonData = sourceFile.readAll();
    QJsonParseError error;
    const QJsonDocument doc = QJsonDocument::fromJson(jsonData, &error);
    if(error.error != QJsonParseError::NoError) {
        emit message(ErrorMessage, ImportantMessage, tr("Failed to read source file. Invalid JSON: %1 at %2").arg(error.errorString()).arg(error.offset));
        return false;
    }

    // TODO: Ipmlement
    emit message(WarningMessage, ImportantMessage, tr("NOT IMPLEMENTED!"));
    return false;

    return true;
}

bool JsonPacker::pack(const QString &sourceDestinationPath, const QString &destintationFilePath) {
    // TODO: Ipmlement
    emit message(WarningMessage, ImportantMessage, tr("NOT IMPLEMENTED!"));
    return false;

    return true;
}

bool JsonPacker::isFileInsideDir(const QString& filePath, const QString& dirPath) {
    QString cleanFile = QDir::cleanPath(QFileInfo(filePath).absoluteFilePath());
    QString cleanDir  = QDir::cleanPath(QDir(dirPath).absolutePath());

    QFileInfo fi(cleanFile);
    if(!fi.canonicalFilePath().isEmpty())
        cleanFile = fi.canonicalFilePath();

    QDir dir(cleanDir);
    if(!dir.canonicalPath().isEmpty())
        cleanDir = dir.canonicalPath();

    if (!cleanDir.endsWith('/'))
        cleanDir += '/';

#if defined(Q_OS_WIN)
    return cleanFile.startsWith(cleanDir, Qt::CaseInsensitive);
#else
    return cleanFile.startsWith(cleanDir, Qt::CaseSensitive);
#endif
}