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

    if(doc.isNull()) {
        emit message(ErrorMessage, ImportantMessage, tr("Source JSON is null."));
        return false;
    }

    if(doc.isEmpty()) {
        emit message(WarningMessage, ImportantMessage, tr("Source JSON is empty. Nothing to write."));
        // TODO: Delete all contents?
        return true;
    }

    if(doc.isObject())
        return writeDirectory(doc.object(), destinationDir);

    if(doc.isArray())
        return writeDirectory(doc.array(), destinationDir);

    return true;
}

bool JsonPacker::pack(const QString &sourceDirectoryPath, const QString &destinationFilePath) {
    // TODO: Ipmlement
    emit message(WarningMessage, ImportantMessage, tr("NOT IMPLEMENTED!"));
    return false;

    return true;
}

bool JsonPacker::writeDirectory(const QJsonValue &value, const QDir &directory) {
    QJsonObject indexObject;
    ValueMetaType type = metaType(value);
    indexObject.insert("$type", metaTypeString(value));

    QSet<QString> touchedChildren;

    if(type == ObjectType) {
        const QJsonObject sourceObj = value.toObject();
        const QStringList keys = sourceObj.keys();
        for(const QString &key : keys) {
            const QJsonValue &subVal = sourceObj.value(key);
            if(metaType(subVal) == PrimitiveType) {
                indexObject.insert(key, subVal);
            } else {
                if(!QDir(directory.path() + "/" + key).exists() && !directory.mkdir(key)) {
                    emit message(ErrorMessage, ImportantMessage, tr("Failed to create directory: %1").arg(directory.path() + "/" + key));
                    return false;
                } else {
                    touchedChildren << key;
                    if(!writeDirectory(subVal, directory.path() + "/" + key))
                        return false;
                }
            }
        }
    } else if(type == ArrayType) {
        QSet<QString> ids;
        QJsonArray valuesArray;
        const QJsonArray sourceArr = value.toArray();
        const int indexDigitCount = sourceArr.count() == 0 ? 1 : static_cast<int>(std::log10(std::abs(sourceArr.count()))) + 1;
        int i = -1;
        for(auto it = sourceArr.begin(); it != sourceArr.end(); ++it) {
            ++i;
            QString id;
            if(it->isObject() && it->toObject().contains(_uidKeyName) && it->toObject().value(_uidKeyName).isString()) {
                id = it->toObject().value(_uidKeyName).toString();
            } else {
                id = QString("idx_%1").arg(i, indexDigitCount, 10, '0');
            }
            if(ids.contains(id)) {
                emit message(ErrorMessage, ImportantMessage, tr("Duplicate id \"%1\" in %2").arg(id, directory.path()));
                return false;
            }
            ids << id;
            if(!directory.mkdir(id)) {
                emit message(ErrorMessage, ImportantMessage, tr("Failed to create directory: %1").arg(directory.path() + "/" + id));
                return false;
            } else {
                touchedChildren << id;
                if(!writeDirectory(*it, directory.path() + "/" + id))
                    return false;
            }
            valuesArray << id;
        }
        indexObject.insert("$values", valuesArray);
    } else if(type == PrimitiveType) {
        indexObject.insert("$value", value);
    }

    if(!_disableCleanup) {
        const QStringList subDirs = directory.entryList(QDir::Dirs|QDir::NoDotAndDotDot);
        for(const QString &subDir : subDirs) {
            if(subDir.startsWith(".")) // keep hidden files and directories (like .git etc.)
                continue;

            if(!touchedChildren.contains(subDir)) {
                if(!directory.rmpath(subDir)) {
                    emit message(ErrorMessage, ImportantMessage, tr("Failed to remove orphanded directory: %1").arg(directory.path() + "/" + subDir));
                    return false;
                }
            }
        }
    }

    writeJsonToFile(directory.path() + "/index.json", indexObject);

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

JsonPacker::ValueMetaType JsonPacker::metaTypeFromString(const QString &typeName) {
    return typeName == "object" ? ObjectType : typeName == "array" ? ArrayType : typeName == "primitive" ? PrimitiveType : UnknownType;
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
