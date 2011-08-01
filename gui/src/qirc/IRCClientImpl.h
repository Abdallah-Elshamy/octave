#ifndef IRCCLIENTIMPL_H
#define IRCCLIENTIMPL_H

#include <QTcpSocket>
#include "IRCClientInterface.h"

class IRCClientImpl : public IRCClientInterface
{
  Q_OBJECT
public:
  IRCClientImpl();

public slots:
  void connectToServer (const QHostAddress& host, int port);
  void disconnect ();
  void reconnect ();

  bool isConnected ();
  const QHostAddress& host();
  int port();

  void enterChannel (const QString& channel);
  void leaveChannel (const QString& channel, const QString& reason);

  void focusChannel (const QString& channel);
  void sendNicknameChangeRequest (const QString &nickname);
  void sendMessage (const QString& message);

  const QString& nickname ();

private slots:
  void handleConnected ();
  void handleDisconnected ();
  void handleReadyRead ();

private:
  void handleIncomingLine (const QString& line);

  QHostAddress  m_host;
  int           m_port;
  QString       m_nickname;
  bool          m_connected;

  QTcpSocket    m_tcpSocket;
};

#endif // IRCCLIENTIMPL_H
