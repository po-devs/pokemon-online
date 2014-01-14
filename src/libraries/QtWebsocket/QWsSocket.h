#ifndef QWSSOCKET_H
#define QWSSOCKET_H

#include <QTcpSocket>
#include <QHostAddress>
#include <QTime>

enum EWebsocketVersion
{
	WS_VUnknow = -1,
	WS_V0 = 0,
	WS_V4 = 4,
	WS_V5 = 5,
	WS_V6 = 6,
	WS_V7 = 7,
	WS_V8 = 8,
	WS_V13 = 13
};

class QWsSocket : public QAbstractSocket
{
	Q_OBJECT

	friend class QWsServer;

public:
	enum EOpcode
	{
		OpContinue = 0x0,
		OpText = 0x1,
		OpBinary = 0x2,
		OpReserved3 = 0x3,
		OpReserved4 = 0x4,
		OpReserved5 = 0x5,
		OpReserved6 = 0x6,
		OpReserved7 = 0x7,
		OpClose = 0x8,
		OpPing = 0x9,
		OpPong = 0xA,
		OpReservedB = 0xB,
		OpReservedV = 0xC,
		OpReservedD = 0xD,
		OpReservedE = 0xE,
		OpReservedF = 0xF
	};
	enum ECloseStatusCode
	{
		NoCloseStatusCode = 0,
		CloseNormal = 1000,
		CloseGoingAway = 1001,
		CloseProtocolError = 1002,
		CloseDataTypeNotSupported = 1003,
		CloseReserved1004 = 1004,
		CloseMissingStatusCode = 1005,
		CloseAbnormalDisconnection = 1006,
		CloseWrongDataType = 1007,
		ClosePolicyViolated = 1008,
		CloseTooMuchData = 1009,
		CloseMissingExtension = 1010,
		CloseBadOperation = 1011,
		CloseTLSHandshakeFailed = 1015
	};

public:
	// ctor
    QWsSocket( QObject * parent = 0, QTcpSocket * socket = 0, EWebsocketVersion ws_v = WS_V13 );
	// dtor
	virtual ~QWsSocket();

	// Public methods
	EWebsocketVersion version();
	QString resourceName();
    QString host();
    QString ip();
	QString hostAddress();
	int hostPort();
	QString origin();
	QString protocol();
	QString extensions();

	void setResourceName( QString rn );
	void setHost( QString h );
	void setHostAddress( QString ha );
	void setHostPort( int hp );
	void setOrigin( QString o );
	void setProtocol( QString p );
	void setExtensions( QString e );

	qint64 write( const QString & string ); // write data as text
	qint64 write( const QByteArray & byteArray ); // write data as binary

public slots:
	void connectToHost( const QString & hostName, quint16 port, OpenMode mode = ReadWrite );
    void connectToHost( const QHostAddress & address, quint16 port, OpenMode mode = ReadWrite );
    void disconnectFromHost();
    void abort( QString reason = QString() );
	void ping();
    virtual void close( ECloseStatusCode closeStatusCode = NoCloseStatusCode, QString reason = QString() );

signals:
	void frameReceived(QString frame);
	void frameReceived(QByteArray frame);
	void pong(quint64 elapsedTime);

protected:
	qint64 writeFrames ( const QList<QByteArray> & framesList );
	qint64 writeFrame ( const QByteArray & byteArray );

protected slots:
	void processDataV0();
	void processDataV4();
    void processHandshake();
	void processTcpStateChanged( QAbstractSocket::SocketState socketState );

private:
	enum EReadingState
    {
		HeaderPending,
		PayloadLengthPending,
		BigPayloadLenghPending,
		MaskPending,
		PayloadBodyPending,
		CloseDataPending
	};

	// private vars
	QTcpSocket * tcpSocket;
	QByteArray currentFrame;
	QTime pingTimer;

	EWebsocketVersion _version;
	QString _resourceName;
	QString _host;
	QString _hostAddress;
	int _hostPort;
	QString _origin;
	QString _protocol;
	QString _extensions;
	bool serverSideSocket;

	bool closingHandshakeSent;
	bool closingHandshakeReceived;

	EReadingState readingState;
	EOpcode opcode;
	bool isFinalFragment;
	bool hasMask;
	quint64 payloadLength;
	QByteArray maskingKey;
	ECloseStatusCode closeStatusCode;

    static const QString regExpAcceptStr;
    static const QString regExpUpgradeStr;
    static const QString regExpConnectionStr;
    QString handshakeResponse;
    QString key;

public:
	// Static functions
	static QByteArray generateMaskingKey();
	static QByteArray generateMaskingKeyV4( QString key, QString nonce );
    static QByteArray mask(const QByteArray & data, const QByteArray & maskingKey );
	static QList<QByteArray> composeFrames( QByteArray byteArray, bool asBinary = false, int maxFrameBytes = 0 );
	static QByteArray composeHeader( bool end, EOpcode opcode, quint64 payloadLength, QByteArray maskingKey = QByteArray() );
	static QString composeOpeningHandShake( QString resourceName, QString host, QString origin, QString extensions, QString key );

	// static vars
	static int maxBytesPerFrame;
};

#endif // QWSSOCKET_H
