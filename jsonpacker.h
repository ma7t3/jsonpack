#ifndef JSONPACKER_H
#define JSONPACKER_H

#include <QObject>
#include <QDir>
#include <QJsonDocument>

class QFile;

class JsonPacker : public QObject {
    Q_OBJECT

public:
    explicit JsonPacker(QObject *parent = nullptr);

    enum MessageType {
        InfoMessage,
        WarningMessage,
        ErrorMessage
    };

    enum MessageImportance {
        ImportantMessage,
        UnimportantMessage
    };

    enum ValueMetaType {
        ObjectType,
        ArrayType,
        PrimitiveType
    };

    void setUidKeyName(const QString &newUidKeyName);
    void setJsonFormat(QJsonDocument::JsonFormat newJsonFormat);
    void setAllowOverwrite(bool newAllowOverwrite);
    void setDisableCleanup(bool newDisableCleanup);
    void setDisableArrayInlining(bool newDisableArrayInlining);

public slots:
    bool unpack(const QString &sourceFilePath, const QString &destinationDirectoryPath);
    bool pack(const QString &sourceDestinationPath, const QString &destintationFilePath);

protected:
    bool writeDirectory(const QJsonValue &value, const QDir &directory);

    QJsonValue parseDirectory(const QDir &directory);

    QJsonDocument readJsonFromFile(const QString fileName, bool *ok);
    QJsonDocument readJsonFromData(const QByteArray &data, bool *ok);
    bool writeJsonToFile(const QString &fileName, const QJsonValue &value);
    static bool isFileInsideDir(const QString& filePath, const QString& dirPath);
    ValueMetaType metaType(const QJsonValue &value) const;
    QString metaTypeString(ValueMetaType type) const;
    QString metaTypeString(const QJsonValue &value) const;

signals:
    void message(JsonPacker::MessageType type, JsonPacker::MessageImportance importance, const QString &text);

private:
    QString _uidKeyName;
    bool _allowOverwrite, _disableCleanup, _disableArrayInlining;
    QJsonDocument::JsonFormat _jsonFormat;
};

#endif // JSONPACKER_H
