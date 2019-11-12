
#include "chatterboxserver.h"

#include <QTcpSocket>
#include <QRegExp>

ChatterBoxServer::ChatterBoxServer(QObject *parent) : QTcpServer(parent)
{
}

void ChatterBoxServer::incomingConnection(int socketfd)
{
  QTcpSocket *client = new QTcpSocket(this);
  client->setSocketDescriptor(socketfd);
  clients.insert(client);

  qDebug() << "New client from:" << client->peerAddress().toString();

  connect(client, SIGNAL(readyRead()), this, SLOT(readyRead()));
  connect(client, SIGNAL(disconnected()), this, SLOT(disconnected()));
}

void ChatterBoxServer::readyRead()
{
  QTcpSocket *client = (QTcpSocket*)sender();
  while(client->canReadLine())
  {
    QString line = QString::fromUtf8(client->readLine()).trimmed();
    qDebug() << "Read line:" << line;

    QRegExp meRegex("^/me:(.*)$");

    if(meRegex.indexIn(line) != -1)
    {
      QString user = meRegex.cap(1);
      users[client] = user;
      foreach(QTcpSocket *client, clients)
        client->write(QString("Server:" + user + " has joined.\n").toUtf8());
      sendUserList();
    }
    else if(users.contains(client))
    {
      QString message = line;
      QString user = users[client];
      qDebug() << "User:" << user;
      qDebug() << "Message:" << message;

      foreach(QTcpSocket *otherClient, clients)
        otherClient->write(QString(user + ":" + message + "\n").toUtf8());
    }
    else
    {
      qWarning() << "Got bad message from client:" << client->peerAddress().toString() << line;
    }
  }
}

void ChatterBoxServer::disconnected()
{
  QTcpSocket *client = (QTcpSocket*)sender();
  qDebug() << "Client disconnected:" << client->peerAddress().toString();

  clients.remove(client);

  QString user = users[client];
  users.remove(client);

  sendUserList();
  foreach(QTcpSocket *client, clients)
    client->write(QString("Server:" + user + " has left.\n").toUtf8());
}

void ChatterBoxServer::sendUserList()
{
  QStringList userList;
  foreach(QString user, users.values())
    userList << user;

  foreach(QTcpSocket *client, clients)
    client->write(QString("/users:" + userList.join(",") + "\n").toUtf8());
}

Lookup::Lookup(const QString &filename)
{
  db = new Db(NULL, 0);
  try
  {
    db->open(NULL, filename.toLocal8Bit().constData(), NULL, DB_BTREE, 0, 0);
  }
  catch(DbException &e)
  {
    if (db)
      db->close(0);
    db = 0;
  }
  catch(std::exception &e)
  {
    if (db)
      db->close(0);
    db = 0;
  }
}

Lookup::~Lookup()
{
  if (db)
    db->close(0);
}

QStringList Lookup::lookup(const QString &key) const
{
  if (!db)
    return QStringList();
  QByteArray normkey = key.toLower().replace(" ", "").toLatin1();
  Dbt k(normkey.data(), normkey.size() + 1);
  Dbt v;
  if (db->get(NULL, &k, &v, 0) != 0)
    return QStringList();

  QString value((char *)v.get_data());
  return value.split(":");
}

void ChatterBoxServer::sendMessages()
{
//  QStringList messList;
//  foreach(QString mess, messages.values())
//    messList << mess;

  foreach(QTcpSocket *client, clients)
    client->write(QString("/old:" + messages+"\n").toUtf8());
}

