#ifndef GREPENGINE_H
#define GREPENGINE_H

#include <QObject>
#include <QString>

class GlobalSettings;
class SSSearch;

class GrepEngine : public QObject
{
	Q_OBJECT

public:
	GrepEngine(GlobalSettings *, QObject *parent = 0);
	~GrepEngine();

public slots:
	void	doGrep(QString pat, QString ext, QString dirStr, QString exclude);
	void	terminate();
	
protected:
	int		doGrepDir(QString pat, QString ext, QString dirStr, const QRegExp &);
	int		doGrepFile(const QString &fullPath, const QString &pat, const QRegExp &);

signals:
	void	greppingDir(const QString &);
	void	doOutput(const QString &);
	void	finished(int);

private:
	GlobalSettings	*m_globSettings;
	SSSearch	*m_sssrc;
	bool	m_toTerminate;
	bool	m_terminated;
};

#endif // GREPENGINE_H
