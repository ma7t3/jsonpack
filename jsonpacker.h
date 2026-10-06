#ifndef JSONPACKER_H
#define JSONPACKER_H

#include <QObject>

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

    void setUidKeyName(const QString &newUidKeyName);
    void setAllowOverwrite(bool newAllowOverwrite);
    void setDisableCleanup(bool newDisableCleanup);
    void setDisableArrayInlining(bool newDisableArrayInlining);

public slots:
    bool unpack(const QString &sourceFilePath, const QString &destinationDirectoryPath);
    bool pack(const QString &sourceDestinationPath, const QString &destintationFilePath);

protected:
    QJsonDocument readJsonFromData(const QByteArray &data, bool *ok);
    static bool isFileInsideDir(const QString& filePath, const QString& dirPath);

signals:
    void message(JsonPacker::MessageType type, JsonPacker::MessageImportance importance, const QString &text);

private:
    QString _uidKeyName;
    bool _allowOverwrite, _disableCleanup, _disableArrayInlining;
};

#endif // JSONPACKER_H
