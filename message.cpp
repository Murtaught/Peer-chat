#include "message.h"

Message::Message(time_t time, QString author_nickname,
                 QHostAddress author_address, QString msg) :
    time(time),
    author_nickname(author_nickname),
    author_address(author_address),
    msg(msg)
{
}

QString Message::getMsg() const
{
    return msg;
}

void Message::setMsg(QString value)
{
    msg = value;
}

time_t Message::getTime() const
{
    return time;
}

void Message::setTime(time_t value)
{
    time = value;
}

QString Message::getAuthorNickname() const
{
    return author_nickname;
}

void Message::setAuthorNickname(QString value)
{
    author_nickname = value;
}

QHostAddress Message::getAuthorAddress() const
{
    return author_address;
}

void Message::setAuthorAddress(QHostAddress value)
{
    author_address = value;
}
