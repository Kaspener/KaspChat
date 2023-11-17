#ifndef SERVER_H
#define SERVER_H

#include <QTcpServer>
#include <QDebug>
#include "dialog.h"
#include "client.h"
class Client;

class Server : public QTcpServer
{
    Q_OBJECT

public:
    explicit Server(QWidget *widget = 0, QObject *parent = 0);
    void doSendToAllUserJoin(QString name);	//уведомить о новом пользователе
    void doSendToAllUserLeft(QString name);
    void doSendToAllMessage(QString message, QString fromUsername);	//разослать сообщение
    void doSendToAllServerMessage(QString message);	//серверное сообщение
    void doSendServerMessageToUsers(QString message, const QString &users);	//приватное серверное сообщение
    void doSendMessageToUsers(QString message, const QString &user, QString fromUsername);
    void doDestroyClients();
    QStringList getUsersOnline() const;	//узнать список пользователей
    bool isNameValid(QString name) const;	//проверить имя
    bool isNameUsed(QString name) const;	//проверить используется ли имя

signals:
    void addLogToGui(QString string, QColor color = Qt::black);
    void addUserToGui(QString);
    void removeUserFromGui(QString);

public slots:
    void onMessageFromGui(QString message, const QString &users);
    void onRemoveUser(Client *client);

protected:
    void incomingConnection(qintptr handle);

private:
    QList<Client *> _clients;	//список пользователей
    QWidget *_widget;	//ссылка на виджет для подключения к нему сигналов от client

};

#endif // SERVER_H
