//
//  LogBrowserDialog.cpp
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

#include "LogBrowserDialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTextBrowser>
#include <QPushButton>
#include <QFileDialog>
#include <QDir>
#include <QFile>
#include <QMessageBox>
#include <QTextStream>
#include <QCloseEvent>
#include <QKeyEvent>

LogBrowserDialog::LogBrowserDialog(QWidget *parent)
: QWidget(parent)
{
    qRegisterMetaType<QtMsgType>("QtMsgType");

    QVBoxLayout *layout = new QVBoxLayout;
    setLayout(layout);
    
    browser = new QTextBrowser(this);
    layout->addWidget(browser);
    
    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->setContentsMargins(0, 0, 0, 0);
    layout->addLayout(buttonLayout);
    
    buttonLayout->addStretch(10);
    
    clearButton = new QPushButton(this);
    clearButton->setText("Clear");
    buttonLayout->addWidget(clearButton);
    connect(clearButton, SIGNAL(clicked()), browser, SLOT(clear()));
    
    saveButton = new QPushButton(this);
    saveButton->setText("Save Output");
    buttonLayout->addWidget(saveButton);
    connect(saveButton, SIGNAL(clicked()), this, SLOT(save()));
    
}


LogBrowserDialog::~LogBrowserDialog()
{
    
}


void LogBrowserDialog::outputMessage(QtMsgType type, const QString &msg)
{
    switch (type) {
        case QtDebugMsg:
            browser->append(msg);
            break;
            
        case QtWarningMsg:
            browser->append(tr("-- WARNING: %1").arg(msg));
            break;
            
        case QtCriticalMsg:
            browser->append(tr("-- CRITICAL: %1").arg(msg));
            break;
            
        case QtFatalMsg:
            browser->append(tr("-- FATAL: %1").arg(msg));
            break;
    }
}


void LogBrowserDialog::save()
{
    
    QString saveFileName("full-log.txt");
    
    QFile file(saveFileName);
    if(!file.open(QIODevice::WriteOnly)) {
        QMessageBox::warning(
                             this,
                             tr("Error"),
                             QString(tr("<nobr>File '%1'<br/>cannot be opened for writing.<br/><br/>"
                                        "The log output could <b>not</b> be saved!</nobr>"))
                             .arg(saveFileName));
        return;
    }
    
    QTextStream stream(&file);
    stream << browser->toPlainText();
    file.close();
}


void LogBrowserDialog::closeEvent(QCloseEvent *e)
{
    QMessageBox::StandardButton answer = QMessageBox::question(
                                                               this,
                                                               tr("Close Log Browser?"),
                                                               tr("Do you really want to close the log browser?"),
                                                               QMessageBox::Yes | QMessageBox::No
                                                               );
    
    if (answer == QMessageBox::Yes)
        e->accept();
    else
        e->ignore();
}


void LogBrowserDialog::keyPressEvent(QKeyEvent *e)
{
    // ignore all keyboard events
    // protects against accidentally closing of the dialog
    // without asking the user
    e->ignore();
}