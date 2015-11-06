//
//  LogBrowser.cpp
//
//
//  This is a Log Browser widget for QDebug output, taken from here:
//
// http://qt-project.org/wiki/Browser_for_QDebug_output
// It consists of the actual log browser (class LogBrowserDialog in logbrowserdialog.h and
// logbrowserdialog.h) and a small wrapper (class LogBrowser in logbrowser.h and logbrowser.h).
// The wrapper is instantiated in the main function of an application and creates the browser
// window. It also acts as an intermediary and converts the const char * based messages from the
// debug system into QString based messages. With this trick the debug messages can be sent to the
// actual browser by the means of signal/slot connections. This adds basic thread support to the browser.


#include "LogBrowser.h"

#include <QMetaType>

#include "LogBrowserDialog.h"

LogBrowser::LogBrowser(QObject *parent) :
QObject(parent)
{
    qRegisterMetaType<QtMsgType>("QtMsgType");
    browserDialog = new LogBrowserDialog;
    connect(this, SIGNAL(sendMessage(QtMsgType,QString)), browserDialog, SLOT(outputMessage(QtMsgType,QString)), Qt::QueuedConnection);
    browserDialog->show();
}


LogBrowser::~LogBrowser()
{
    delete browserDialog;
}


void LogBrowser::outputMessage(QtMsgType type, const QString &msg)
{
    emit sendMessage( type, msg );
}
