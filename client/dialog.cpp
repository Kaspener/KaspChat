#include "dialog.h"
#include "ui_dialog.h"

//#include <QtGui>
#include <QDebug>

Dialog::Dialog(QWidget *parent) :QDialog(parent),ui(new Ui::Dialog)
{
    ui->setupUi(this);
    _name = "";
    _socket = new QTcpSocket(this);
    connect(_socket, SIGNAL(readyRead()), this, SLOT(onSocketReadyRead()));
    connect(_socket, SIGNAL(connected()), this, SLOT(onSocketConnected()));
    connect(_socket, SIGNAL(disconnected()), this, SLOT(onSocketDisconnected()));
    connect(_socket, SIGNAL(error(QAbstractSocket::SocketError)),this, SLOT(onSocketDisplayError(QAbstractSocket::SocketError)));
}

Dialog::~Dialog()
{
    delete ui;
}

void Dialog::onSocketDisplayError(QAbstractSocket::SocketError socketError)
{
    switch (socketError) {
    case QAbstractSocket::RemoteHostClosedError:
        break;
    case QAbstractSocket::HostNotFoundError:
        QMessageBox::information(this, "Error", "The host was not found");
        break;
    case QAbstractSocket::ConnectionRefusedError:
        QMessageBox::information(this, "Error", "The connection was refused by the peer.");
        break;
    default:
        QMessageBox::information(this, "Error", "The following error occurred: "+_socket->errorString());
        break;
    }
    ui->connectButton->setChecked(false);
    ui->connectButton->setText("Connect");
}

void Dialog::onSocketReadyRead()
{
    QDataStream in(_socket);
    if (_blockSize == 0) {
        if (_socket->bytesAvailable() < (int)sizeof(quint16))
            return;
        in >> _blockSize;
        qDebug() << "_blockSize now " << _blockSize;
    }
    if (_socket->bytesAvailable() < _blockSize)
        return;
    else
        _blockSize = 0;
    quint8 command;
    in >> command;
    qDebug() << "Received command " << command;

    switch (command)
    {
        case Client::comUsersOnline:
        {
            addToLog("Connected as "+_name,Qt::darkGreen);
            ui->send->setEnabled(true);
            QString users;
            in >> users;
            if (users == "")
                return;
            QStringList l =  users.split(",");
            ui->users->addItems(l);
        }
        break;
        case Client::comMessageToAll:
        {
            QString user;
            in >> user;
            QString message;
            in >> message;
            addToLog("<"+user+">: "+message);
        }
        break;
        case Client::comMessageToUsers:
        {
            QString user;
            in >> user;
            QString message;
            in >> message;
            addToLog("<"+user+">(private): "+message, Qt::blue);
        }
        break;
        case Client::comPublicServerMessage:
        {
            QString message;
            in >> message;
            addToLog("「Server」: "+message, Qt::darkRed);
        }
        break;
        case Client::comPrivateServerMessage:
        {
            QString message;
            in >> message;
            addToLog("「Server」(private) : "+message, Qt::darkMagenta);
        }
        break;
        case Client::comUserJoin:
        {
            QString name;
            in >> name;
            ui->users->addItem(name);
            addToLog(name+" joined", Qt::darkGreen);
        }
        break;
        case Client::comUserLeft:
        {
            QString name;
            in >> name;
            for (int i = 0; i < ui->users->count(); ++i)
                if (ui->users->item(i)->text() == name)
                {
                    ui->users->takeItem(i);
                    addToLog(name+" left", Qt::darkGreen);
                    break;
                }
        }
        break;
        case Client::comErrNameInvalid:
        {
            QMessageBox::information(this, "Error", "This name is invalid.");
            _socket->disconnectFromHost();
        }
        break;
        case Client::comErrNameUsed:
        {
            QMessageBox::information(this, "Error", "This name is already used.");
            _socket->disconnectFromHost();
        }
        break;

        case Client::comDestroy:
        {
            addToLog("Server closed", Qt::red);
            _socket->disconnectFromHost();
        }
        break;
    }
}

//когда подключились
void Dialog::onSocketConnected()
{
    ui->name->setEnabled(false);
    ui->ip->setEnabled(false);
    ui->port->setEnabled(false);
    _blockSize = 0;
    addToLog("Connected to "+_socket->peerAddress().toString()+":"+QString::number(_socket->peerPort()),Qt::darkGreen); //выводим сообщенеие о подключении

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << (quint16)0;
    out << (quint8)Client::comAutchReq;
    out << ui->name->text();
    _name = ui->name->text();
    out.device()->seek(0);
    out << (quint16)(block.size() - sizeof(quint16));
    _socket->write(block);
}

void Dialog::onSocketDisconnected()
{
    ui->send->setEnabled(false);
    ui->name->setEnabled(true);
    ui->ip->setEnabled(true);
    ui->port->setEnabled(true);
    ui->users->clear();
    ui->connectButton->setText("Connect");
    ui->connectButton->setChecked(false);
    addToLog("Disconnected from "+_socket->peerAddress().toString()+":" + ui->port->text(), Qt::darkGreen);
}

bool Dialog::findUser(const QString& name){
    for (int i = 0; i < ui->users->count(); ++i){
        if (ui->users->item(i)->text() == name) return true;
    }
    return false;
}

void Dialog::on_send_clicked()
{
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << (quint16)0;
    QString msg = ui->message->document()->toPlainText();
    msg = msg.trimmed();
    QStringList lst = msg.split(" ");
    bool find = false;
    if( lst[0]=="/msg"){
        if (!findUser(lst[1])){
            addToLog("No such user", Qt::darkRed);
            ui->message->clear();
            return;
        }
        find = true;
    }
    if (!find){
        out << (quint8)Client::comMessageToAll;
        out << ui->message->document()->toPlainText();
    }
    else
    {
        out << (quint8)Client::comMessageToUsers;
        QString s = lst[1];
        out << s;
        lst.removeAt(0);
        lst.removeAt(0);
        out << lst.join(" ");
    }
    out.device()->seek(0);
    out << (quint16)(block.size() - sizeof(quint16));
    _socket->write(block);
    ui->message->clear();
}

void Dialog::addToLog(QString text, QColor color)
{
    ui->chat->insertItem(0, "["+QTime::currentTime().toString()+"] "+text);
    ui->chat->item(0)->setTextColor(color);
}

void Dialog::on_connectButton_toggled(bool checked)
{
    if(checked){
        ui->connectButton->setText("Disconnect");
        _socket->connectToHost(ui->ip->text(), ui->port->text().toUShort());
    }
    else{
        ui->connectButton->setText("Connect");
        _socket->disconnectFromHost();
    }
}
