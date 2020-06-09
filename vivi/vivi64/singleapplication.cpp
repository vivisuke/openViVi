#include "SingleApplication.h"
#include <QtGui>
#include <Windows.h>

SingleApplication::SingleApplication(int &argc, char *argv[], const QString uniqKeyString)
	: m_connected(false), m_uniqKeyString(uniqKeyString), QApplication(argc, argv)
{
	connect(&m_localServer, SIGNAL(newConnection()), this, SLOT(onAccepted()));
	connect(&m_localServer, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
}

bool SingleApplication::isFirstApp()
{
	m_localSocket.connectToServer(m_uniqKeyString);
	if( m_localSocket.waitForConnected(200) ) {
		return false;
	} else {
		m_localServer.listen(m_uniqKeyString);
		return true;
	}
}
void SingleApplication::onAccepted()
{
#if		1
	//QLocalSocket *
	m_sock = m_localServer.nextPendingConnection();
	//sock->waitForReadyRead(30*1000);
	connect(m_sock, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
#endif
}
void SingleApplication::onReadyRead()
{
	//QLocalSocket *sock = m_localServer.nextPendingConnection();
	QByteArray ba = m_sock->readAll();
	QTextCodec *codec = QTextCodec::codecForName("UTF-8");
	QString buff = codec->toUnicode(ba);
	emit onRecieved(buff);
}
//	最初に起動されたインスタンスへメッセージ送信
void SingleApplication::sendMessage(const QString &text)
{
	m_localSocket.write(text.toUtf8());
	m_localSocket.flush();
	if( !m_localSocket.waitForBytesWritten(1000) ) {
		return;
	}
}
bool SingleApplication::winEventFilter( MSG * msg, long * result )
{
	if( msg->message == WM_IME_NOTIFY ) {
		qDebug() << "winEventFilter()";
		DWORD dwCommand = (DWORD) msg->wParam;
		if( dwCommand == IMN_SETOPENSTATUS )		//	IME ON
			emit imeOpenStatusChanged();
	}
	return false;
}
