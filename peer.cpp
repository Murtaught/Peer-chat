#include "peer.h"

Peer::Peer(QString nickname, QHostAddress address, QObject *parent)
    : QObject(parent),
      nickname(nickname),
      address(address)
{
    keepalive_timer = new QTimer(this);
    connect(keepalive_timer, SIGNAL(timeout()), this, SLOT(keepAliveTimerExpired()));
    resetKeepaliveTimer();
}

void Peer::resetKeepaliveTimer()
{
    keepalive_timer->start( KEEP_ALIVE_FOR );
}

QString Peer::getFormattedString()
{
    return nickname + " (" + address.toString() + ")";
}

void Peer::keepAliveTimerExpired()
{
    emit peerDied(address);
}

QHostAddress Peer::getAddress() const
{
    return address;
}

void Peer::setAddress(QHostAddress value)
{
    address = value;
}

QString Peer::getNickname() const
{
    return nickname;
}

void Peer::setNickname(QString value)
{
    nickname = value;
}
