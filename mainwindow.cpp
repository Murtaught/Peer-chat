#include "mainwindow.h"

#include <QtDebug>

MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent)
{
    // Windgets
    messages_widget   = new QTextEdit(this);
    message_line_edit = new QLineEdit(this);
    user_list_widget  = new QListWidget(this);
    send_button       = new QPushButton(tr("Send"), this);

    // Objects
    socket = new QUdpSocket(this);

    hello_timer     = new QTimer(this);
    keepalive_timer = new QTimer(this);
    deliever_timer  = new QTimer(this);
    send_timer      = new QTimer(this);
    send_timer->setSingleShot(true);

    // Layout-related
    QGridLayout *layout = new QGridLayout(this);

    layout->addWidget(messages_widget,   0, 0);
    layout->addWidget(user_list_widget,  0, 1);
    layout->addWidget(message_line_edit, 1, 0);
    layout->addWidget(send_button,       1, 1);

    layout->setColumnStretch(0, 2); // 2 : 1
    layout->setColumnStretch(1, 1);

    // Tweaking widgets
    setWindowTitle(tr("Network peer-to-peer chat"));

    message_line_edit->setFocus();
    messages_widget->setReadOnly(true);

    // User information and network routines
    socket->bind(PORT, QUdpSocket::ShareAddress);
    my_nickname = QDir::home().dirName();

    QList<QHostAddress> addr = QNetworkInterface::allAddresses();

    if ( addr.size() < 3 )
    {
        addSystemMessageToWidget(tr("Network interface wasn\'t found!"));
        my_address = QHostAddress::LocalHost;
    }
    else
        my_address = addr[2];

    // Connecting everything
    connect(send_button, SIGNAL(clicked()), this, SLOT(sendButtonPressed()));
    connect(send_timer, SIGNAL(timeout()),  this, SLOT(sendTimerExpired()));
    connect(hello_timer, SIGNAL(timeout()), this, SLOT(sendHello()));
    connect(keepalive_timer, SIGNAL(timeout()), this, SLOT(sendKeepalive()));
    connect(deliever_timer, SIGNAL(timeout()), this, SLOT(delieverMessages()));

    connect(socket, SIGNAL(readyRead()), this, SLOT(handleSocketMessage()));

    // Starting up
    hello_timer->start(HELLO_INTERVAL);
    sendHello();

    keepalive_timer->start(KEEPALIVE_INTERVAL);
    deliever_timer->start(DELIEVER_INTERVAL);
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

void MainWindow::addSystemMessageToWidget(const QString &message)
{
    messages_widget->append("<b><font color=\"red\">" + message + "<\font><\b>\n");
}

void MainWindow::addMessageToWidget(const QString &author, const QString &message)
{
    messages_widget->append("<b>" + author + ": </b> <i><font color=\"blue\">" + message + "</font></i>\n");
}

void MainWindow::sendString(const QString &what, const QHostAddress &where)
{
    QByteArray datagram;
    datagram.append(what);
    datagram.append("\r\n");

    socket->writeDatagram(datagram.data(), datagram.size(), where, PORT);
}

void MainWindow::sendToEverybody(const QString &what)
{
    for (int i = 0; i < peer_list.size(); ++i)
        sendString(what, peer_list[i]->getAddress());
}

void MainWindow::addPeerToList(const QString &nickname, const QHostAddress &address)
{
    Peer *newcomer = new Peer(nickname, address, this);

    qDebug() << "Trying to add new peer to list: "
             << nickname
             << address.toString();

    for (int i = 0; i < peer_list.size(); ++i)
        if ( address == peer_list[i]->getAddress() )
        {
            qDebug() << "Already know that buddy";
            delete newcomer;
            return;
        }

    qDebug() << "Added successfully";

    peer_list.push_back(newcomer);
    connect(newcomer, SIGNAL(peerDied(QHostAddress)), this, SLOT(removePeerFromList(QHostAddress)));

    user_list_widget->addItem(newcomer->getFormattedString());
    addSystemMessageToWidget(nickname + " " + tr("connected"));
}

void MainWindow::keepPeerAlive(const QHostAddress &peer_address)
{
    for (int i = 0; i < peer_list.size(); ++i)
        if (peer_list[i]->getAddress() == peer_address)
        {
            qDebug() << peer_list[i]->getNickname()
                     << peer_list[i]->getAddress().toString() << "kept alive";

            peer_list[i]->resetKeepaliveTimer();
            return;
        }
}

bool MainWindow::addToChatHistory(qulonglong time, QString author, QString msg)
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

    for (int i = 0; i < peer_list.size(); ++i)
        user_list_widget->addItem( peer_list[i]->getFormattedString() );
}

void MainWindow::removePeerFromList(QHostAddress peer_address)
{
    for (int i = 0; i < peer_list.size(); ++i)
        if (peer_list[i]->getAddress() == peer_address)
        {
            qDebug() << peer_list[i]->getNickname()
                     << peer_list[i]->getAddress().toString() << "removed from list";

            peer_list.remove(i);
            updateUserListWidget();
            return;
        }
}

QString MainWindow::responseString()
{
    QString msg = "RESPONSE " + my_nickname;
    for (int i = 0; i < peer_list.size(); ++i)
        msg += " " + peer_list[i]->getAddress().toString() +
               " " + peer_list[i]->getNickname();

    return msg;
}

void MainWindow::sendButtonPressed()
{
    QString msg_text = message_line_edit->text();
    message_line_edit->clear();
    message_line_edit->setFocus();

    qulonglong t = time(NULL);

    addToChatHistory(t, my_nickname, msg_text);

    Message message(t, my_nickname, my_address, msg_text);
    undelivered_messages.push_back(message);
    delieverMessages();

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
    quint16 sender_port;

    QByteArray datagram;
    datagram.resize( socket->pendingDatagramSize() );
    socket->readDatagram(datagram.data(), datagram.size(), &sender_address, &sender_port);

    QString text = QString(datagram.data());
    text.remove(text.size() - 2, 2); // Remove \r\n

    qDebug() << "Datagram arrived! here it is:" << text;

    QStringList list = text.split(' ');
    QString cmd = list[0];
    list.removeAt(0);

    if ( cmd == "HELLO" )
    {
        QString sender_nickname = list[0];

        if ( sender_nickname != my_nickname )
        {
            sendResponse(sender_address);
            sendJoin(sender_nickname, sender_address);
        }
    }
    else if ( cmd == "RESPONSE" )
    {
        hello_timer->stop();
        addPeerToList(list[0], sender_address);
        for (int i = 1; i < list.size(); i += 2)
            addPeerToList(list[i + 1], QHostAddress(list[i]));
    }
    else if ( cmd == "MESSAGE" )
    {
        if ( sender_address != my_address )
        {
            //QHostAddress author_ip( list[0] );
            QString      author_nickname( list[1] );
            qulonglong   time( list[2].toULongLong() );

            QString msg;
            for (int i = 3; i < list.size(); ++i)
                msg += list[i];

            if ( addToChatHistory(time, author_nickname, msg) )
                sendToEverybody(text);

            // Sending accept back
            sendAccepted(sender_address, author_nickname, time);
        }
    }
    else if ( cmd == "ACCEPTED" )
    {
        QString nickname = list[0];
        qulonglong time  = list[1].toULongLong();


    }
    else if ( cmd == "JOIN" )
    {
        addPeerToList(list[1], QHostAddress(list[0]));
    }
    else if ( cmd == "QUIT" )
    {
        removePeerFromList(sender_address);
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

void MainWindow::sendHello()
{
    qDebug() << "Sending HELLO";
    sendString("HELLO " + my_nickname, QHostAddress::Broadcast);
}

void MainWindow::sendResponse(QHostAddress to_whom)
{
    sendString(responseString(), to_whom);
}

void MainWindow::sendJoin(const QString &newcomer_nickname, const QHostAddress &newcomer_address)
{
    sendToEverybody("JOIN " + newcomer_address.toString() + " " + newcomer_nickname);
}

void MainWindow::sendMessage(qulonglong time, QString msg)
{
    sendToEverybody("MESSAGE " + my_address.toString() + " " + my_nickname + " " +
                    QString::number(time) + " " + msg);
}

void MainWindow::sendMessage(const Message &msg)
{
    sendToEverybody("MESSAGE " + msg.getAuthorAddress().toString() + " " + msg.getAuthorNickname() +
                    " " + QString::number(msg.getTime()) + " " + msg.getMsg());
}

void MainWindow::sendAccepted(QHostAddress to_whom, QString nickname, qulonglong time)
{
    sendString("ACCEPTED " + nickname + " " + QString::number(time), to_whom);
}

void MainWindow::sendKeepalive()
{
    sendToEverybody("KEEPALIVE");
}

void MainWindow::sendQuit()
{
    sendToEverybody("QUIT " + my_address.toString());
}

void MainWindow::delieverMessages()
{
    for (int i = 0; i < undelivered_messages.size(); ++i)
        sendMessage(undelivered_messages[i]);
}

void MainWindow::delieverConfirmed(QString nickname, qulonglong time)
{
    for (int i = 0; i < undelivered_messages.size(); ++i)
        if ( undelivered_messages[i].getAuthorNickname() == nickname &&
             undelivered_messages[i].getTime() == time )
        {
            undelivered_messages.removeAt(i);
            return;
        }
}
