#include "mainwindow.h"

#include <QtDebug>

#include <QDir>
#include <QPair>

MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent)
{
    // initialisation lists look ugly

    // Windgets
    messages_widget   = new QTextEdit(this);
    input_line        = new QLineEdit(this);
    user_list_widget  = new QListWidget(this);
    send_button       = new QPushButton(tr("Send"), this);

    // Objects
    socket            = new QUdpSocket(this);

    hello_timer       = new QTimer(this);
    keepalive_timer   = new QTimer(this);
    deliver_timer     = new QTimer(this);
    send_timer        = new QTimer(this);
    send_timer->setSingleShot(true);

    // Layout-related
    QGridLayout *layout = new QGridLayout(this);

    layout->addWidget(messages_widget,   0, 0);
    layout->addWidget(user_list_widget,  0, 1);
    layout->addWidget(input_line, 1, 0);
    layout->addWidget(send_button,       1, 1);

    layout->setColumnStretch(0, 2); // 2 : 1
    layout->setColumnStretch(1, 1);

    // Tweaking widgets
    setWindowTitle(tr("Network peer-to-peer chat"));

    input_line->setFocus();
    messages_widget->setReadOnly(true);

    // User information and network routines
    socket->bind(PORT, QUdpSocket::ShareAddress);
    my_nickname = QDir::home().dirName();

    QList<QHostAddress> addr = QNetworkInterface::allAddresses();

    qDebug() << "Avaliable network interfaces:";
    foreach (QHostAddress a, addr)
    {
        qDebug() << a.toString();
    }

    if ( addr.size() < 3 )
    {
        addSystemMessageToWidget(tr("Network interface wasn\'t found!"));
        my_address = QHostAddress::LocalHost;
    }
    else
    {
        // KOSTYL
        my_address = addr[ 2 ];
        addSystemMessageToWidget(tr("Hello ") + my_nickname + ", your network "
                                 "address is " + my_address.toString());
    }

    // Connecting everything
    connect(send_button, SIGNAL(clicked()), this, SLOT(sendButtonPressed()));
    connect(send_timer, SIGNAL(timeout()),  this, SLOT(sendTimerExpired()));
    connect(hello_timer, SIGNAL(timeout()), this, SLOT(sendHello()));
    connect(keepalive_timer, SIGNAL(timeout()), this, SLOT(sendKeepalive()));
    connect(deliver_timer, SIGNAL(timeout()), this, SLOT(delieverMessages()));

    connect(socket, SIGNAL(readyRead()), this, SLOT(handleSocketMessage()));

    // Starting up
    hello_timer->start(HELLO_INTERVAL);
    sendHello();

    keepalive_timer->start(KEEPALIVE_INTERVAL);
    deliver_timer->start(DELIEVER_INTERVAL);

    updateUserListWidget();

    qDebug() << "Constructor execution finished";
}

MainWindow::~MainWindow()
{
    sendQuit();
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if ( event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return )
    {
        if ( send_button->isEnabled() )
            sendButtonPressed();
    }
    else
        QWidget::keyPressEvent(event);
}

void MainWindow::addSystemMessageToWidget(QString message)
{
    messages_widget->append("<b><font color=\"red\">" + message + "<\font><\b>\n");
}

void MainWindow::addMessageToWidget(QString author, QString message)
{
    messages_widget->append("<b>" + author + ": </b> <i><font color=\"blue\">" + message + "</font></i>\n");
}

void MainWindow::addPeerToList(QString nickname, QHostAddress address)
{
    qDebug() << "Trying to add new peer to list: "
             << nickname
             << address.toString();

    if ( address == my_address )
    {
        qDebug() << "  Its me!";
        return;
    }

    foreach (Peer *p, peer_list)
        if ( address == p->getAddress() )
        {
            qDebug() << "  Already know that buddy";
            return;
        }

    Peer *newcomer = new Peer(nickname, address, this);

    peer_list.push_back(newcomer);
    connect(newcomer, SIGNAL(peerDied(QHostAddress)), this, SLOT(removePeerFromList(QHostAddress)));

    qDebug() << "Added successfully";

    user_list_widget->addItem(newcomer->getFormattedString());
    addSystemMessageToWidget( tr("%1 connected").arg( newcomer->getFormattedString() ) );
}

void MainWindow::keepPeerAlive(QHostAddress peer_address)
{
    foreach (Peer *p, peer_list)
        if (p->getAddress() == peer_address)
        {
            qDebug() << p->getNickname()
                     << p->getAddress().toString() << "kept alive";

            p->resetKeepaliveTimer();
            return;
        }
}

bool MainWindow::addToChatHistory(time_t time, QString author, QString msg)
{
    for (int i = chat_history.size() - 1; i >= 0 && chat_history[i].first >= time; --i)
        if ( chat_history[i].first == time && chat_history[i].second == author )
            return false;

    qDebug() << "Added message to history: " << time << author;

    chat_history.push_back( qMakePair(time, author) );
    addMessageToWidget(author, msg);

    return true;
}

void MainWindow::updateUserListWidget()
{
    user_list_widget->clear();

    user_list_widget->addItem( my_nickname + " (" + my_address.toString() + ")" );

    foreach (Peer *p, peer_list)
    {
        user_list_widget->addItem( p->getFormattedString() );
    }

    qDebug() << "User list widget updated";
}

void MainWindow::removePeerFromList(QHostAddress peer_address)
{
    for (int i = 0; i < peer_list.size(); ++i)
        if (peer_list[i]->getAddress() == peer_address)
        {
            Peer *p = peer_list[i];

            qDebug() << p->getFormattedString() << "removed from list";

            peer_list.removeAt(i);

            addSystemMessageToWidget( tr("%1 left").arg( p->getFormattedString() ) );
            updateUserListWidget();

            p->deleteLater();
            return;
        }
}

void MainWindow::sendButtonPressed()
{
    QString msg_text = input_line->text();
    input_line->clear();
    input_line->setFocus();

    time_t t = time(NULL);

    addToChatHistory(t, my_nickname, msg_text);

    Message message(t, my_nickname, my_address, msg_text);
    undelivered_messages.push_back(message);
    delieverMessages();

    // Artifically block sendButton for SEND_INTERVAL milliseconds
    send_button->setEnabled(false);
    send_timer->start(SEND_INTERVAL);
}

void MainWindow::sendTimerExpired()
{
    send_button->setEnabled(true);
}

void MainWindow::handleSocketMessage()
{
    QHostAddress sender_address;
    QStringList list;

    {
        quint16 sender_port;

        QByteArray datagram;
        datagram.resize( socket->pendingDatagramSize() );
        socket->readDatagram(datagram.data(), datagram.size(), &sender_address, &sender_port);

        QString text = QString(datagram.data());
        text.remove(text.size() - 2, 2); // Remove \r\n

        list = text.split(' ');

        qDebug() << "Datagram arrived: " << text;
    }

    QString cmd = list[0];
    list.removeAt(0);

    if ( cmd == "HELLO" )
    {
        QString sender_nickname = list[0];

        if ( sender_address != my_address )
        {
            sendResponse(sender_address);

            addPeerToList(sender_nickname, sender_address);
            updateUserListWidget();

            sendJoin(sender_nickname, sender_address);
        }
    }
    else if ( cmd == "RESPONSE" )
    {
        hello_timer->stop();

        addPeerToList(list[0], sender_address);
        for (int i = 1; i < list.size(); i += 2)
            addPeerToList(list[i + 1], QHostAddress(list[i]));

        updateUserListWidget();
    }
    else if ( cmd == "MESSAGE" )
    {
        if ( sender_address != my_address )
        {
            QHostAddress author_address( list[0] );
            QString      author_nickname( list[1] );
            time_t       time( list[2].toULongLong() );

            QString msg = list[3];
            for (int i = 4; i < list.size(); ++i)
                msg += " " + list[i];

            if ( addToChatHistory(time, author_nickname, msg) )
            {
                Message message(time, author_nickname, author_address, msg);
                undelivered_messages.push_back(message);
                delieverMessages();
            }

            // Sending accept back
            sendAccepted(sender_address, author_nickname, time);
        }
    }
    else if ( cmd == "ACCEPTED" )
    {
        QString nickname = list[0];
        time_t time  = list[1].toULongLong();

        delieverConfirmed(nickname, time);
    }
    else if ( cmd == "JOIN" )
    {
        QHostAddress newcomer_addr = QHostAddress(list[0]);

        addPeerToList(list[1], newcomer_addr);
        updateUserListWidget();
    }
    else if ( cmd == "QUIT" )
    {
        QHostAddress leaver_addr = QHostAddress(list[0]);

        removePeerFromList(leaver_addr);
    }
    else if ( cmd == "GET" )
    {
        sendResponse(sender_address);
    }
    else if ( cmd == "KEEPALIVE" )
    {
        keepPeerAlive(sender_address);
    }
}

void MainWindow::sendString(QString what, QHostAddress where)
{
    QByteArray datagram;
    datagram.append(what);
    datagram.append("\r\n");

    socket->writeDatagram(datagram.data(), datagram.size(), where, PORT);

    qDebug() << "   Send string: " << what
             << "to " << ( (where != QHostAddress::Broadcast) ? where.toString() : "BROADCAST" );
}

void MainWindow::sendToEverybody(QString what)
{
    qDebug() << "Sending to " << peer_list.size()
             << " peers in peer list: " << what;

    foreach (Peer *p, peer_list)
    {
        sendString(what, p->getAddress());
    }
}

void MainWindow::sendHello()
{
    qDebug() << "Sending HELLO";
    sendString("HELLO " + my_nickname, QHostAddress::Broadcast);
}

QString MainWindow::responseString()
{
    QString msg = "RESPONSE " + my_nickname;

    foreach (Peer *p, peer_list)
    {
        msg += " " + p->getAddress().toString() +
               " " + p->getNickname();
    }

    return msg;
}

void MainWindow::sendResponse(QHostAddress to_whom)
{
    qDebug() << "Sending RESPONSE to " << to_whom.toString();
    sendString(responseString(), to_whom);
}

void MainWindow::sendJoin(QString newcomer_nickname, QHostAddress newcomer_address)
{
    sendToEverybody("JOIN " + newcomer_address.toString() + " " + newcomer_nickname);
}

void MainWindow::sendMessage(time_t time, QString msg)
{
    sendToEverybody("MESSAGE " + my_address.toString() + " " + my_nickname + " " +
                    QString::number(time) + " " + msg);
}

void MainWindow::sendMessage(Message msg)
{
    sendToEverybody("MESSAGE " + msg.getAuthorAddress().toString() + " " + msg.getAuthorNickname() +
                    " " + QString::number(msg.getTime()) + " " + msg.getMsg());
}

void MainWindow::sendAccepted(QHostAddress to_whom, QString nickname, time_t time)
{
    sendString("ACCEPTED " + nickname + " " + QString::number(time), to_whom);
}

void MainWindow::sendKeepalive()
{
    qDebug() << "Requesting peers to keep me alive";

    sendToEverybody("KEEPALIVE");
}

void MainWindow::sendQuit()
{
    qDebug() << "I am disconnecting, so I request peers to remove "
                "me from their peer lists";

    sendToEverybody("QUIT " + my_address.toString());
}

void MainWindow::delieverMessages()
{
    qDebug() << "Sending " << undelivered_messages.size() << " undelievered messages";

    foreach (Message msg, undelivered_messages)
    {
        sendMessage(msg);
    }
}

void MainWindow::delieverConfirmed(QString nickname, time_t time)
{
    qDebug() << "Trying to match confirmation for ("
             << nickname << ", " << time << ")...";

    for (int i = 0; i < undelivered_messages.size(); ++i)
        if ( undelivered_messages[i].getAuthorNickname() == nickname &&
             undelivered_messages[i].getTime() == time )
        {
            qDebug() << "Delivery of message (" << nickname << ", "
                     <<  time << ") is confirmed!";

            undelivered_messages.removeAt(i);
            return;
        }
}
