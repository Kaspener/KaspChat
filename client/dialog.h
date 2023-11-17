#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QTcpSocket>
#include <QHostAddress>
#include "../server/client.h"
#include <QMessageBox>
class Client;

namespace Ui {
    class Dialog;
}

class Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog(QWidget *parent = 0);
    ~Dialog();

private slots:
    void onSocketConnected();
    void onSocketDisconnected();
    void onSocketReadyRead();
    void onSocketDisplayError(QAbstractSocket::SocketError socketError);
    void on_send_clicked();
    void on_connectButton_toggled(bool checked);

private:
    Ui::Dialog *ui;
    QTcpSocket *_socket;
    quint16 _blockSize;
    QString _name;
    void addToLog(QString text, QColor color = Qt::black);
    bool findUser(const QString&);

};

#endif // DIALOG_H
