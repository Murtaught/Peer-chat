#include "peer.h"

Peer::Peer(const QString &nickname, const QHostAddress &address, QObject *parent)
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

void Peer::setAddress(const QHostAddress &value)
{
    address = value;
}

QString Peer::getNickname() const
{
    return nickname;
}

void Peer::setNickname(const QString &value)
{
    nickname = value;
}

/*bool operator ==(const Peer &a, const Peer &b)
{
    return a.getAddress() == b.getAddress();
}

bool operator !=(const Peer &a, const Peer &b)
{
    return !(a == b);
}*/
