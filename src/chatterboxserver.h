#ifndef __ChatterBoxServer_H__
#define __ChatterBoxServer_H__

#include <QStringList>
#include <QTcpServer>
#include <QTcpSocket>
#include <QMap>
#include <QSet>

#include <db_cxx.h>

class ChatterBoxServer : public QTcpServer
{
  Q_OBJECT

  public:
    ChatterBoxServer(QObject *parent=0);

  private slots:
    void readyRead();
    void disconnected();
    void sendUserList();
    void sendMessages();

  protected:
    void incomingConnection(int socketfd);

  private:
    QSet<QTcpSocket*> clients;
    QMap<QTcpSocket*,QString> users;
    QString messages;
};

class Lookup
{
  public:
    Lookup(const QString &filename);
    ~Lookup();
    
    QStringList lookup(const QString &key) const;
    
  private:
    Db *db;
};

#endif

