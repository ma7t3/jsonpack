#include <QCoreApplication>

#include <QCommandLineParser>
#include <QDebug>

#include "jsonpacker.h"

enum ArgumentNames {
    ActionArgument = 0,
    SourceArgument = 1,
    DestinationArgument = 2
};

enum Action {
    InvalidAction,
    UnpackAction,
    PackAction
};

int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);
    a.setApplicationName("jsonpack");
    a.setApplicationVersion("1.0.0");
    a.setOrganizationName("ma7t3");

    QCommandLineParser parser;
    parser.addPositionalArgument(QObject::tr("action"), QObject::tr(R"(The action to be executed ("pack" to pack a given directory into a single JSON file or "unpack" to unpack a given single JSON file into a directory.))"));
    parser.addPositionalArgument(QObject::tr("source"), QObject::tr("The source file or directory to read data from. It must exist."));
    parser.addPositionalArgument(QObject::tr("destination"), QObject::tr("The destination file or directory to read data from. If it doesn't already exist, it'll be created automatically"));
    parser.addOption(QCommandLineOption("disable-cleanup", QObject::tr("Disable the automatic cleanup of orphanded directories and files.")));
    parser.addOption(QCommandLineOption("disable-array-inlining", QObject::tr("Disable the automatic inlining of array only containing primitive values.")));
    parser.addOption(QCommandLineOption(QStringList{"u", "uid-key-name"}, QObject::tr("Specifies the name of the key of objects which is considered a unique identifier. Default ist \"id\"."), "KeyName", "id"));
    parser.addOption(QCommandLineOption("verbose", QObject::tr("Log more events, can produce a lot of output")));

    parser.addVersionOption();
    parser.addHelpOption();

    parser.process(a.arguments());
    const QStringList arguments = parser.positionalArguments();

    if(arguments.isEmpty()) {
        parser.showHelp();
        return 0;
    }

    const QString actionString = arguments[0];
    Action action = actionString == "unpack" ? UnpackAction : actionString == "pack" ? PackAction : InvalidAction;
    if(action == InvalidAction) {
        qCritical().noquote() << QObject::tr("Error: Invalid action parameter. You must specify <action> must be \"pack\" or \"unpack\". See --help for more details.");
        return 1;
    }

    if(arguments.size() != 3) {
        qCritical().noquote() << QObject::tr("Error: Invalid parameter count. You must specify <action> <source> <destination>. See --help for more details.");
        return 1;
    }

    const bool verboseMode = parser.isSet("verbose");
    JsonPacker packer;
    packer.setUidKeyName(parser.value("uid-key-name"));
    packer.setDisableCleanup(parser.isSet("disable-cleanup"));
    packer.setDisableArrayInlining(parser.isSet("disable-array-inlining"));

    QObject::connect(&packer, &JsonPacker::message, [&verboseMode](JsonPacker::MessageType type, JsonPacker::MessageImportance importance, const QString &text){
        if(!verboseMode && importance == JsonPacker::UnimportantMessage)
            return;

        if(type == JsonPacker::InfoMessage)
            qInfo().noquote() << text;
        else if(type == JsonPacker::WarningMessage)
            qInfo().noquote() << "Warning: " << text;
        else if(type == JsonPacker::ErrorMessage)
            qCritical().noquote() << "Error: " << text;
    });

    const QString source = arguments[SourceArgument];
    const QString destination = arguments[DestinationArgument];

    if(action == UnpackAction)
        return packer.unpack(source, destination);
    else
        return packer.pack(source, destination);
}
