//
//  LogBrowser.h
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


#ifndef LOGBROWSER_H
#define LOGBROWSER_H

#include <QObject>

class LogBrowserDialog;

class LogBrowser : public QObject
{
    Q_OBJECT
    
public:
    
    explicit LogBrowser(QObject *parent = 0);
    ~LogBrowser();
    
public slots:
    void outputMessage( QtMsgType type, const QString &msg );
    
signals:
    void sendMessage( QtMsgType type, const QString &msg );
    
private:
    LogBrowserDialog *browserDialog;
    
};

#endif // LOGBROWSER_H
