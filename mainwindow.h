#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "peer.h"
#include "message.h"

#include <ctime>

#include <QWidget>
#include <QGridLayout>

#include <QString>
#include <QStringList>
#include <QVector>
#include <QList>

#include <QPushButton>
#include <QTextEdit>
#include <QLineEdit>
#include <QListWidget>

#include <QKeyEvent>
#include <QTimer>

#include <QUdpSocket>
#include <QNetworkInterface>
#include <QByteArray>
#include <QHostAddress>

#include <QPair>
#include <QDir>

class MainWindow : public QWidget
{
    Q_OBJECT

    typedef QVector< QPair<qulonglong, QString> > HistoryType;

    static const int PORT               = 3141;
    static const int SEND_INTERVAL      = 1000;           // 1000 ms = 1 s
    static const int HELLO_INTERVAL     = 30 * 1000;      // 30 s
    static const int KEEPALIVE_INTERVAL = 2 * 60 * 1000;  // 2 min
    static const int DELIEVER_INTERVAL  = 60 * 1000;      // 1 min

    
public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    // events:
    void keyPressEvent(QKeyEvent *event);

    // methods:
    void addSystemMessage(const QString &message);
    void addMessageToHistory(const QString &author, const QString &message);

    void sendString(QString const& what, QHostAddress const& where);
    void sendToEverybody(QString const& what);

    bool add_to_chat_history(qulonglong time, QString author, QString msg);

    void update_user_list_widget();

    QString responseString();

    // fields:
    QPushButton     *send_button;
    QTextEdit       *messages_widget;
    QLineEdit       *message_line_edit;
    QListWidget     *user_list_widget;

    QTimer          *send_timer;
    QTimer          *hello_timer;
    QTimer          *keepalive_timer;
    QTimer          *deliever_timer;

    QUdpSocket      *socket;

    QString         my_nickname;
    QHostAddress    my_address;

    QVector<Peer*>  peer_list;
    HistoryType     chat_history;
    QList<Message>  undelivered_messages;

private slots:
    void sendButtonPressed();
    void sendTimerExpired();

    void handleSocketMessage();

    void sendHello();
    void sendResponse(QHostAddress to_whom);
    void sendJoin(QString const& newcomer_nickname, QHostAddress const& newcomer_address);
    void sendMessage(qulonglong time, QString msg);
    void sendMessage(const Message &msg);
    void sendAccepted(QHostAddress to_whom, QString nickname, qulonglong time);
    void sendKeepalive();
    void sendQuit();

    void delieverMessages();

    void addPeerToList(QString const& nickname, QHostAddress const& address);
    void keepPeerAlive(QHostAddress const& peer_address);
    void removePeerFromList(QHostAddress peer_address);



};

#endif // MAINWINDOW_H
