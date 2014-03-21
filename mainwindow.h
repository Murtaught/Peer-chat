#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "peer.h"
#include "message.h"

#include <ctime>

#include <QtWidgets>
#include <QtNetwork>

#include <QVector>
#include <QList>

#include <QKeyEvent>
#include <QTimer>

class MainWindow : public QWidget
{
    Q_OBJECT

    typedef QVector< QPair<time_t, QString> > HistoryType;

    static const int PORT               = 3141;
    static const int SEND_INTERVAL      = 1000;           // 1000 ms = 1 s
    static const int HELLO_INTERVAL     = 10 * 1000;      // 10 s
    static const int KEEPALIVE_INTERVAL = 2 * 60 * 1000;  // 2 min
    static const int DELIEVER_INTERVAL  = 60 * 1000;      // 1 min

    
public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    // events:
    void keyPressEvent(QKeyEvent *event);

    // methods:
    void addSystemMessageToWidget(QString message);
    void addMessageToWidget(QString author, QString message);

    void sendString(QString what, QHostAddress where);
    void sendToEverybody(QString what);

    bool addToChatHistory(time_t time, QString author, QString msg);

    void updateUserListWidget();

    QString responseString();

private slots:
    void sendButtonPressed();
    void sendTimerExpired();

    void handleSocketMessage();

    void sendHello();
    void sendResponse(QHostAddress to_whom);
    void sendJoin(QString newcomer_nickname, QHostAddress newcomer_address);
    void sendMessage(time_t time, QString msg);
    void sendMessage(Message msg);
    void sendAccepted(QHostAddress to_whom, QString nickname, time_t time);
    void sendKeepalive();
    void sendQuit();

    void delieverMessages();
    void delieverConfirmed(QString nickname, time_t time);

    void addPeerToList(QString nickname, QHostAddress address);
    void keepPeerAlive(QHostAddress peer_address);
    void removePeerFromList(QHostAddress peer_address);

private:
    QPushButton     *send_button;
    QTextEdit       *messages_widget;
    QLineEdit       *input_line;
    QListWidget     *user_list_widget;

    QTimer          *send_timer;
    QTimer          *hello_timer;
    QTimer          *keepalive_timer;
    QTimer          *deliver_timer;

    QUdpSocket      *socket;

    QString         my_nickname;
    QHostAddress    my_address;

    QVector<Peer*>  peer_list;
    HistoryType     chat_history;
    QList<Message>  undelivered_messages;
};

#endif // MAINWINDOW_H
