#include "mainwindow.h"

#include <QApplication>
#include <QIcon>
#include <QLocale>
#include <QTranslator>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setWindowIcon(QIcon(":/icons/message_tracer_logo.ico"));

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages)
    {
        const QString baseName = "embw-RTOS_Analyzer_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName))
        {
            a.installTranslator(&translator);
            break;
        }
    }
    MainWindow w;
    w.show();
    return QCoreApplication::exec();
}
