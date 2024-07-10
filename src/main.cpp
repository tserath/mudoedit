// This is the main file of the application, containing the entry point (main function)

#include <QApplication>           // Include for the main application class
#include <KLocalizedString>       // Include for internationalization support
#include <KAboutData>             // Include for application metadata
#include "MainWindow.h"           // Include the main window of our application
#include <QStandardPaths>         // Include for handling standard paths
#include <QDir>                   // Include for directory operations
#include <QLoggingCategory>       // Include for configuring logging categories
// added so mudoedit can be set as default text editor
#include <QCommandLineParser>
#include <QFileInfo>
#include <QLocalSocket>
#include <QLocalServer>
#include <QIcon>
#include <QGuiApplication>

Q_LOGGING_CATEGORY(mainLog, "mudoedit.main")

int main(int argc, char *argv[])  // The main function, entry point of the application
{
    QApplication app(argc, argv);  // Create the main application object

    // Set the desktop file name
    QGuiApplication::setDesktopFileName(QStringLiteral("com.tserath.mudoedit"));
    
    // Set the application icon using the resource path
    QIcon::setThemeName(QIcon::themeName());
    app.setWindowIcon(QIcon(QStringLiteral(":/icons/mudoedit.svg")));

    // Set up logging rules for debug output
    QLoggingCategory::setFilterRules(QStringLiteral("mudoedit.fileoperations.debug=true\n"
                                                    "mudoedit.mainwindow.debug=true\n"
                                                    "mudoedit.main.debug=true"));
                                                
    // Set up internationalization for the application
    KLocalizedString::setApplicationDomain("mudoedit");

    // Create and set up the "About" data for the application
    KAboutData aboutData(
        QStringLiteral("mudoedit"),                    // Internal name
        i18n("mudoedit"),                           // Display name
        QStringLiteral("1.0"),                         // Version
        i18n("A simple text editor for KDE"),          // Short description
        KAboutLicense::GPL,                            // License
        i18n("(c) 2024, zachary mccormick"),           // Copyright statement
        QString(),                                     // Other text (empty in this case)
        QStringLiteral("zach@tserath.com")             // Contact email
    );

    // Set the application metadata
    KAboutData::setApplicationData(aboutData);

    // Set up command line parser
    QCommandLineParser parser;
    aboutData.setupCommandLine(&parser);
    parser.addPositionalArgument(QStringLiteral("files"), i18n("Files to open"), QStringLiteral("[files...]"));

    // Process the command line arguments
    parser.process(app);
    aboutData.processCommandLine(&parser);

    // Check if another instance is already running
    QLocalSocket socket;
    socket.connectToServer(QLatin1String("mudoeditserver"));
        if (socket.waitForConnected(500)) {
        // Another instance is running, send files to open
        QStringList args = parser.positionalArguments();
        if (!args.isEmpty()) {
            QByteArray message = args.join(QLatin1Char('\n')).toUtf8();
                        socket.write(message);
            socket.waitForBytesWritten();
        }

        return 0; // Exit this instance

    // Set the application icon to our custom SVG
    app.setWindowIcon(QIcon(QStringLiteral(":/icons/mudoedit.svg")));

    }

    // No other instance running, start a new one
    QLocalServer server;
    server.listen(QLatin1String("mudoeditserver"));

    // Create the main window
    MainWindow *window = new MainWindow;
    window->show();

    QObject::connect(&server, &QLocalServer::newConnection, [&]() {
        QLocalSocket *socket = server.nextPendingConnection();
        QObject::connect(socket, &QLocalSocket::readyRead, [socket, window]() {
            QByteArray message = socket->readAll();
            QStringList files = QString::fromUtf8(message).split(QLatin1Char('\n'));
                        for (const QString &file : files) {
                window->openFile(file);
            }
        });
    });

    // Get the list of files to open from the command line arguments
    const QStringList args = parser.positionalArguments();
    qCDebug(mainLog) << "Command line arguments:" << args;

    // Open each file specified in the command line arguments
    for (const QString &file : args) {
        QFileInfo fileInfo(file);
        if (fileInfo.exists() && fileInfo.isReadable()) {
            qCDebug(mainLog) << "Opening file:" << file;
            window->openFile(file);
        } else {
            qCWarning(mainLog) << "Unable to open file:" << file;
        }
    }

    // Start the application's event loop and return the exit code when it's done
    return app.exec();
}
