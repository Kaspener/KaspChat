#include "client.h"
#include <QWidget>
#include <QMessageBox>

const QString Client::constNameUnknown = QString(".Unknown");

Client::Client(int desc, Server *serv, QObject *parent) :QObject(parent)
{
    _serv = serv;
    _isAutched = false;
    _name = constNameUnknown;
    _blockSize = 0;
    _sok = new QTcpSocket(this);
    _sok->setSocketDescriptor(desc);
    connect(_sok, SIGNAL(connected()), this, SLOT(onConnect()));
    connect(_sok, SIGNAL(disconnected()), this, SLOT(onDisconnect()));
    connect(_sok, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
    connect(_sok, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onError(QAbstractSocket::SocketError)));

    qDebug() << "Client connected" << desc;
}

Client::~Client(){}

void Client::onConnect(){}

void Client::onDisconnect()
{
    qDebug() << "Client disconnected";
    if (_isAutched)
    {
        emit removeUserFromGui(_name);
        _serv->doSendToAllUserLeft(_name);
        emit removeUser(this);
    }
    deleteLater();
}

void Client::onError(QAbstractSocket::SocketError socketError) const
{
    QWidget w;
    switch (socketError) {
    case QAbstractSocket::RemoteHostClosedError:
        break;
    case QAbstractSocket::HostNotFoundError:
        QMessageBox::information(&w, "Error", "The host was not found");
        break;
    case QAbstractSocket::ConnectionRefusedError:
        QMessageBox::information(&w, "Error", "The connection was refused by the peer.");
        break;
    default:
        QMessageBox::information(&w, "Error", "The following error occurred: "+_sok->errorString());
    }
}

void Client::onReadyRead()
{
    QDataStream in(_sok);
    if (_blockSize == 0) {
        if (_sok->bytesAvailable() < (int)sizeof(quint16))
            return;
        in >> _blockSize;
        qDebug() << "_blockSize now " << _blockSize;
    }
    if (_sok->bytesAvailable() < _blockSize)
        return;
    else
        _blockSize = 0;
    quint8 command;
    in >> command;
    qDebug() << "Received command " << command;
    //для неавторизованный пользователей принимается только команда "запрос на авторизацию"
    if (!_isAutched && command != comAutchReq)
        return;
    switch(command)
    {
        //запрос на авторизацию
        case comAutchReq:
        {
            QString name;
            in >> name;
            if (!_serv->isNameValid(name))
            {
                doSendCommand(comErrNameInvalid);
                return;
            }
            if (_serv->isNameUsed(name))
            {
                doSendCommand(comErrNameUsed);
                return;
            }
            _name = name;
            _isAutched = true;
            doSendUsersOnline();
            emit addUserToGui(name);
            _serv->doSendToAllUserJoin(_name);
        }
        break;
        case comMessageToAll:
        {
            QString message;
            in >> message;
            _serv->doSendToAllMessage(message, _name);
            emit messageToGui(message, _name, " ");
        }
        break;
        case comMessageToUsers:
        {
            QString user;
            in >> user;
            QString message;
            in >> message;
            _serv->doSendMessageToUsers(message, user, _name);
            emit messageToGui(message, _name, user);
        }
        break;
    }
}

void Client::doSendCommand(quint8 comm) const
{
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << (quint16)0;
    out << comm;
    out.device()->seek(0);
    out << (quint16)(block.size() - sizeof(quint16));
    _sok->write(block);
    qDebug() << "Send to" << _name << "command:" << comm;
}

void Client::doSendUsersOnline() const
{
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << (quint16)0;
    out << comUsersOnline;
    QStringList l = _serv->getUsersOnline();
    QString s;
    for (int i = 0; i < l.length(); ++i)
        if (l.at(i) != _name)
            s += l.at(i)+(QString)",";
    s.remove(s.length()-1, 1);
    out << s;
    out.device()->seek(0);
    out << (quint16)(block.size() - sizeof(quint16));
    _sok->write(block);
    qDebug() << "Send user list to" << _name << ":" << s;
}
