#include "dialog.h"
#include "ui_dialog.h"
#include <QMessageBox>
#include <QCloseEvent>

Dialog::Dialog(QWidget *parent) :QDialog(parent), ui(new Ui::Dialog)
{
    ui->setupUi(this);
    _serv = new Server(this, this);
    connect(this, SIGNAL(messageFromGui(QString,QString)), _serv, SLOT(onMessageFromGui(QString,QString)));
    connect(_serv, SIGNAL(addLogToGui(QString,QColor)), this, SLOT(onAddLogToGui(QString,QColor)));
}

void Dialog::closeEvent(QCloseEvent* event){
    Q_UNUSED(event);
    _serv->doDestroyClients();
}

Dialog::~Dialog()
{
    delete ui;
}

void Dialog::onAddUserToGui(QString name)
{
    ui->users->addItem(name);
    ui->logs->insertItem(0, "["+QTime::currentTime().toString()+"] "+name+" joined");
    ui->logs->item(0)->setTextColor(Qt::darkGreen);
}

void Dialog::onRemoveUserFromGui(QString name)
{
    for (int i = 0; i < ui->users->count(); ++i)
        if (ui->users->item(i)->text() == name)
        {
            ui->users->takeItem(i);
            ui->logs->insertItem(0, "["+QTime::currentTime().toString()+"] "+name+" left");
            ui->logs->item(0)->setTextColor(Qt::darkGreen);
            break;
        }
}

void Dialog::onMessageToGui(QString message, QString from, const QString &users)
{
    if (users==" ")
        ui->logs->insertItem(0, "["+QTime::currentTime().toString()+"] message from "+from+": "+message+" to all");
    else
    {
        ui->logs->insertItem(0, "["+QTime::currentTime().toString()+"] message from "+from+": "+message+" to "+users);
        ui->logs->item(0)->setTextColor(Qt::blue);
    }
}

void Dialog::onAddLogToGui(QString string, QColor color)
{
    addToLog(string, color);
}

bool Dialog::findUser(const QString& name){
    for (int i = 0; i < ui->users->count(); ++i){
        if (ui->users->item(i)->text() == name) return true;
    }
    return false;
}

void Dialog::on_send_clicked()
{
    QString to = " ";
    QString text = ui->message->document()->toPlainText();
    QString msg = ui->message->document()->toPlainText();
    msg = msg.trimmed();
    QStringList lst = msg.split(" ");
    if( lst[0]=="/msg"){
        if (!findUser(lst[1])){
            addToLog("No such user", Qt::darkRed);
            ui->message->clear();
            return;
        }
        to = lst[1];
        lst.removeAt(0);
        lst.removeAt(0);
        text = lst.join(" ");
    }
    emit messageFromGui(text, to);
    ui->message->clear();
    if (to == " ")
        addToLog("Sended public server message:" + text, Qt::black);
    else
        addToLog("Sended private server message to "+to+ ": " + text, Qt::black);
}


void Dialog::on_startStopButton_toggled(bool checked)
{
    if (checked)
    {
        QHostAddress addr;
        if (!addr.setAddress(ui->ip->text()))
        {
            addToLog("Invalid address "+ui->ip->text(), Qt::darkRed);
            ui->startStopButton->setChecked(false);
            return;
        }
        if (_serv->listen(addr,ui->port->text().toUShort()))
        {
            addToLog("Server started at "+ui->ip->text()+":" + ui->port->text(), Qt::darkGreen);
            ui->send->setEnabled(true);
            ui->ip->setEnabled(false);
            ui->port->setEnabled(false);
            ui->startStopButton->setText("Stop server");
        }
        else
        {
            addToLog("Server not started at "+ui->ip->text()+":" + ui->port->text(), Qt::darkRed);
            ui->startStopButton->setText("Reload");
            ui->ip->setEnabled(false);
            ui->port->setEnabled(false);
        }
    }
    else
    {
        addToLog("Server stopped at "+_serv->serverAddress().toString()+":" + ui->port->text(), Qt::darkGreen);
        if (_serv->isListening()){_serv->doDestroyClients();_serv->close();}
        ui->send->setEnabled(false);
        ui->startStopButton->setText("Start server");
        ui->ip->setEnabled(true);
        ui->port->setEnabled(true);
    }
}

void Dialog::addToLog(QString text, QColor color)
{
    ui->logs->insertItem(0, "["+QTime::currentTime().toString()+"] "+text);
    ui->logs->item(0)->setTextColor(color);
}
