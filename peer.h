#ifndef PEER_H
#define PEER_H

#include <QObject>
#include <QString>
#include <QHostAddress>
#include <QTimer>

class Peer : public QObject
{
    Q_OBJECT

    static const int KEEP_ALIVE_FOR = 10 * 60 * 1000; // 10 minutes

public:
    explicit Peer(QString nickname,
                  QHostAddress address,
                  QObject *parent = 0);

    void resetKeepaliveTimer();

    QString getFormattedString();
    
    QString getNickname() const;
    void setNickname(QString value);

    QHostAddress getAddress() const;
    void setAddress(QHostAddress value);

signals:
    void peerDied(QHostAddress address);
    
private slots:
    void keepAliveTimerExpired();

private:
    QString      nickname;
    QHostAddress address;

    QTimer       *keepalive_timer;
};

//bool operator==(Peer const& a, Peer const& b);
//bool operator!=(Peer const& a, Peer const& b);

#endif // PEER_H
