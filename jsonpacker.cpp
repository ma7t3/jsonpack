#include "jsonpacker.h"

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
    return true;
}

bool JsonPacker::pack(const QString &sourceDestinationPath, const QString &destintationFilePath) {
    return true;
}
