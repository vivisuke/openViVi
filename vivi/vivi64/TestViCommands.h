#pragma once

//----------------------------------------------------------------------
//
//			File:			"TestViCommands.h"
//			Created:		24-Feb-2011
//			Author:			Nobuhide Tsuda
//			Description:	é©ìÆÉeÉXÉgä÷êîêÈåæ
//
//----------------------------------------------------------------------



#ifndef TESTVICOMMANDS_H
#define TESTVICOMMANDS_H

class QString;
class MainWindow;
class ViEngine;

void testViCommands(MainWindow *, ViEngine *, const QString &);

#if 0
#include <QObject>

class TestViCommands : public QObject
{
	Q_OBJECT

public:
	TestViCommands(QObject *parent);
	~TestViCommands();

private:
	
};
#endif

#endif // TESTVICOMMANDS_H
