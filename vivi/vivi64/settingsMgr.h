//----------------------------------------------------------------------
//
//			File:			"settingsMgr.h"
//			Created:		24-8-2013
//			Author:			í√ìcêLèG
//			Description:
//
//----------------------------------------------------------------------

#pragma once

#ifndef		_HEADER_SETTINGSMGR_H
#define		_HEADER_SETTINGSMGR_H

#include <QtCore>

class TypeSettings;

class SettingsMgr
{
public:
	SettingsMgr();
	~SettingsMgr();

public:
	void	writeSettings() const;
	int	typeNameToType(const QString &) const;
	QString		defaultExt(int type);

public:
	TypeSettings	*typeSettings(int type);		//	type ÇÕ GlobalSettings ÇÃ É^ÉCÉvIDÇ∆àÍívÇµÇƒÇÈÇ∆âºíË
	TypeSettings	*typeSettings(const QString typeName = QString());
	TypeSettings	*typeSettingsForExt(const QString ext = QString());
	QString		typeNameForExt(const QString &ext);

protected:
	void	readSettings();

private:
	QMap<QString, QString>	m_extToTypeMap;			//	ext -> type map
	QMap<QString, TypeSettings *>	m_typeMap;		//	type -> typeSettings map
};

#endif		//_HEADER_SETTINGSMGR_H
