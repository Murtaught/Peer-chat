#ifndef MESSAGE_H
#define MESSAGE_H

#include <ctime>
#include <QString>
#include <QHostAddress>

class Message
{
public:
    explicit Message(time_t time, QString author_nickname, QHostAddress, QString msg);

    time_t getTime() const;
    void setTime(const time_t &value);

    QString getAuthorNickname() const;
    void setAuthorNickname(const QString &value);

    QString getMsg() const;
    void setMsg(const QString &value);

    QHostAddress getAuthorAddress() const;
    void setAuthorAddress(const QHostAddress &value);

private:
    time_t   time;
    QString      author_nickname;
    QHostAddress author_address;
    QString      msg;
};

#endif // MESSAGE_H
