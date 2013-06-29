#ifndef MESSAGE_H
#define MESSAGE_H

#include <QString>
#include <QHostAddress>

class Message
{
public:
    explicit Message(qulonglong time, QString author_nickname, QString msg);

    qulonglong getTime() const;
    void setTime(const qulonglong &value);

    QString getAuthorNickname() const;
    void setAuthorNickname(const QString &value);

    QString getMsg() const;
    void setMsg(const QString &value);

    QHostAddress getAuthorAddress() const;
    void setAuthorAddress(const QHostAddress &value);

private:
    qulonglong   time;
    QString      author_nickname;
    QHostAddress author_address;
    QString      msg;
};

#endif // MESSAGE_H
