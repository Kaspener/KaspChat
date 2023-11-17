#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QDebug>
#include <QtGui>
#include <QtCore>
#include "server.h"
#include "client.h"

namespace Ui {
    class Dialog;
}

class Dialog : public QDialog
{
    friend class Client;
    Q_OBJECT

public:
    explicit Dialog(QWidget *parent = 0);
    ~Dialog();

private:
    Ui::Dialog *ui;
    Server *_serv;
    void addToLog(QString text, QColor color);
    void closeEvent(QCloseEvent*);
    bool findUser(const QString&);

signals:
    void messageFromGui(QString message, const QString &users);

public slots:
    void onAddUserToGui(QString name);
    void onRemoveUserFromGui(QString name);
    void onMessageToGui(QString message, QString from, const QString &users);
    void onAddLogToGui(QString string, QColor color);

private slots:
    void on_send_clicked();   // на Send кликнули
    void on_startStopButton_toggled(bool checked); // на включение/выключение кликнули
};

#endif // DIALOG_H
