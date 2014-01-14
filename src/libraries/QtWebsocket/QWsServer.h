#ifndef QWSSERVER_H
#define QWSSERVER_H

#include <QTcpServer>
#include <QTcpSocket>
#include <QNetworkProxy>
#include <QString>
#include <QStringList>
#include <QMap>
#include <QQueue>

#include "QWsSocket.h"

class QWsServer : public QObject
{
	Q_OBJECT

public:
	// ctor
	QWsServer(QObject * parent = 0);
	// dtor
	virtual ~QWsServer();

	// public functions
	void close();
	QString errorString();
	bool hasPendingConnections();
	bool isListening();
	bool listen(const QHostAddress & address = QHostAddress::Any, quint16 port = 0);
	int maxPendingConnections();
	virtual QWsSocket * nextPendingConnection();
	QNetworkProxy proxy();
	QHostAddress serverAddress();
	QAbstractSocket::SocketError serverError();
	quint16 serverPort();
	void setMaxPendingConnections( int numConnections );
	void setProxy( const QNetworkProxy & networkProxy );
	bool setSocketDescriptor( int socketDescriptor );
	int socketDescriptor();
	bool waitForNewConnection( int msec = 0, bool * timedOut = 0 );

signals:
	void newConnection();

protected:
	// protected functions
	void addPendingConnection( QWsSocket * socket );
	virtual void incomingConnection( int socketDescriptor );

private slots:
	// private slots
	void newTcpConnection();
	void closeTcpConnection();
	void dataReceived();

private:
	// private attributes
	QTcpServer * tcpServer;
	QQueue<QWsSocket*> pendingConnections;
	QMap<const QTcpSocket*, QStringList> headerBuffer;

public:
	// public static functions
	static QByteArray serializeInt( quint32 number, quint8 nbBytes = 4 );
	static QString computeAcceptV0( QString key1, QString key2, QString thirdPart );
	static QString computeAcceptV4( QString key );
	static QString generateNonce();
	static QString composeOpeningHandshakeResponseV0( QString accept, QString origin, QString hostAddress, QString hostPort, QString resourceName, QString protocol = "" );
	static QString composeOpeningHandshakeResponseV4( QString accept, QString nonce, QString protocol = "", QString extensions = "" );
	static QString composeOpeningHandshakeResponseV6( QString accept, QString protocol = "", QString extensions = "" );
	static QString composeBadRequestResponse( QList<EWebsocketVersion> versions = QList<EWebsocketVersion>() );

	// public static vars
	static const QString regExpResourceNameStr;
	static const QString regExpHostStr;
	static const QString regExpKeyStr;
	static const QString regExpKey1Str;
	static const QString regExpKey2Str;
	static const QString regExpKey3Str;
	static const QString regExpVersionStr;
	static const QString regExpOriginStr;
	static const QString regExpOrigin2Str;
	static const QString regExpProtocolStr;
	static const QString regExpExtensionsStr;
};

#endif // QWSSERVER_H
