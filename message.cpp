#include "message.h"

Message::Message(qulonglong time, QString author_nickname,
                 QHostAddress author_address, QString msg) :
    time(time),
    author_nickname(author_nickname),
    author_address(author_address),
    msg(msg)
{
}

QString Message::getDatagram()
{
    return
}

QString Message::getMsg() const
{
    return msg;
}

void Message::setMsg(const QString &value)
{
    msg = value;
}

qulonglong Message::getTime() const
{
    return time;
}

void Message::setTime(const qulonglong &value)
{
    time = value;
}

QString Message::getAuthorNickname() const
{
    return author_nickname;
}

void Message::setAuthorNickname(const QString &value)
{
    author_nickname = value;
}

QHostAddress Message::getAuthorAddress() const
{
    return author_address;
}

void Message::setAuthorAddress(const QHostAddress &value)
{
    author_address = value;
}
