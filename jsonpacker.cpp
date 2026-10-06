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

void JsonPacker::setJsonFormat(QJsonDocument::JsonFormat newJsonFormat) {
    _jsonFormat = newJsonFormat;
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
    QDir destinationDir(destinationDirectoryPath);

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

    bool ok;
    const QJsonDocument doc = readJsonFromFile(sourceFilePath, &ok);
    if(!ok) {
        emit message(ErrorMessage, ImportantMessage, tr("Failed to read from source file!"));
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

bool JsonPacker::writeDirectory(const QJsonValue &value, const QDir &directory) {
    return true;
}

QJsonValue JsonPacker::parseDirectory(const QDir &directory) {
    return QJsonValue();
}

QJsonDocument JsonPacker::readJsonFromFile(const QString fileName, bool *ok){
    QFile f(fileName);
    if(!f.open(QFile::ReadOnly)) {
        emit message(ErrorMessage, ImportantMessage, tr("Failed to read file %1: %2").arg(fileName, f.errorString()));
        if(ok)
            *ok = false;

        return QJsonDocument();
    }

    emit message(InfoMessage, UnimportantMessage, tr("Reading file %1 ...").arg(fileName));
    return readJsonFromData(f.readAll(), ok);
}

QJsonDocument JsonPacker::readJsonFromData(const QByteArray &data, bool *ok) {
    QJsonParseError error;
    emit message(InfoMessage, UnimportantMessage, tr("Parsing data ..."));
    const QJsonDocument doc = QJsonDocument::fromJson(data, &error);
    if(error.error != QJsonParseError::NoError) {
        emit message(ErrorMessage, ImportantMessage, tr("Failed to read source file. Invalid JSON: %1 at %2").arg(error.errorString()).arg(error.offset));
    }
    if(ok)
        *ok = error.error == QJsonParseError::NoError;

    return doc;
}

bool JsonPacker::writeJsonToFile(const QString &fileName, const QJsonValue &value) {
    QFile f(fileName);
    if(!f.open(QFile::WriteOnly)) {
        emit message(ErrorMessage, ImportantMessage, tr("Failed to write file %1: %2").arg(fileName, f.errorString()));
        return false;
    }

    QJsonDocument doc;
    if(value.isObject())
        doc.setObject(value.toObject());
    else if(value.isArray())
        doc.setArray(value.toArray());
    else {
        emit message(ErrorMessage, ImportantMessage, tr("Can only write Object or Array to QJsonDocument, given value is neither (%1): %2").arg(value.type()).arg(fileName));
        return false;
    }

    emit message(InfoMessage, UnimportantMessage, tr("Writing file %1 ...").arg(fileName));
    f.write(doc.toJson(_jsonFormat));
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

QString JsonPacker::metaTypeString(ValueMetaType type) const {
    return type == ObjectType ? "object" : type == ArrayType ? "array" : "primitive";
}

QString JsonPacker::metaTypeString(const QJsonValue &value) const {
    return metaTypeString(metaType(value));
}

JsonPacker::ValueMetaType JsonPacker::metaType(const QJsonValue &value) const {
    if(value.isObject())
        return ObjectType;
    else if(value.isArray()) {
        if(_disableArrayInlining)
            return ArrayType;

        const QJsonArray arr = value.toArray();
        bool complexFound = false;
        for(const QJsonValue &subVal : arr) {
            if(metaType(subVal) != PrimitiveType) {
                complexFound = true;
                break;
            }
        }
        return complexFound ? ArrayType : PrimitiveType;
    } else
        return PrimitiveType;
}
