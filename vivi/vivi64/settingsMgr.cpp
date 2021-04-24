//----------------------------------------------------------------------
//
//			File:			"settingsMgr.cpp"
//			Created:		24-8-2013
//			Author:			í√ìcêLèG
//			Description:
//
//----------------------------------------------------------------------

#include "settingsMgr.h"
#include "typeSettings.h"
#include "globalSettings.h"

SettingsMgr::SettingsMgr()
{
	readSettings();
}
SettingsMgr::~SettingsMgr()
{
	for(auto itr = m_typeMap.begin();
		itr != m_typeMap.end();
		++itr)
	{
		TypeSettings *ptr = itr.value();
		delete ptr;
	}
}
TypeSettings *SettingsMgr::typeSettings(int type)
{
	QString typeName;
	switch( type ) {
		case GlobalSettings::DOC_TYPE_CPP:	typeName = "CPP";	break;
		case GlobalSettings::DOC_TYPE_CS:	typeName = "C#";	break;
		case GlobalSettings::DOC_TYPE_CSS:	typeName = "CSS";	break;
		case GlobalSettings::DOC_TYPE_FS:	typeName = "F#";	break;
		case GlobalSettings::DOC_TYPE_HLSL:	typeName = "HLSL";	break;
		case GlobalSettings::DOC_TYPE_HTML:	typeName = "HTML";	break;
		case GlobalSettings::DOC_TYPE_JAVA:	typeName = "JAVA";	break;
		case GlobalSettings::DOC_TYPE_JS:	typeName = "JS";	break;
		case GlobalSettings::DOC_TYPE_TS:	typeName = "TS";	break;
		case GlobalSettings::DOC_TYPE_LOG:	typeName = "LOG";	break;
		case GlobalSettings::DOC_TYPE_MARKDOWN:	typeName = "MARKDN";	break;
		case GlobalSettings::DOC_TYPE_PASCAL:	typeName = "PASCAL";	break;
		case GlobalSettings::DOC_TYPE_PERL:	typeName = "PERL";	break;
		case GlobalSettings::DOC_TYPE_PHP:	typeName = "PHP";	break;
		case GlobalSettings::DOC_TYPE_PYTHON:	typeName = "PYTHON";	break;
		case GlobalSettings::DOC_TYPE_GDSCRIPT:	typeName = "GDSCRIPT";	break;
		case GlobalSettings::DOC_TYPE_RUBY:	typeName = "RUBY";	break;
		case GlobalSettings::DOC_TYPE_SQL:	typeName = "SQL";	break;
		case GlobalSettings::DOC_TYPE_TXT:	typeName = "TXT";	break;
	}
	return typeSettings(typeName);
}
int SettingsMgr::typeNameToType(const QString &name) const
{
	if( name == "CPP" ) return GlobalSettings::DOC_TYPE_CPP;
	if( name == "C#") return GlobalSettings::DOC_TYPE_CS;
	if( name == "CSS") return GlobalSettings::DOC_TYPE_CSS;
	if( name == "F#") return GlobalSettings::DOC_TYPE_FS;
	if( name == "HLSL") return GlobalSettings::DOC_TYPE_HLSL;
	if( name == "HTML" ) return GlobalSettings::DOC_TYPE_HTML;
	if( name == "JAVA") return GlobalSettings::DOC_TYPE_JAVA;
	if( name == "JS" ) return GlobalSettings::DOC_TYPE_JS;
	if( name == "TS" ) return GlobalSettings::DOC_TYPE_TS;
	if( name == "LOG") return GlobalSettings::DOC_TYPE_LOG;
	if( name == "MARKDN") return GlobalSettings::DOC_TYPE_MARKDOWN;
	if( name == "PASCAL") return GlobalSettings::DOC_TYPE_PASCAL;
	if( name == "PERL") return GlobalSettings::DOC_TYPE_PERL;
	if( name == "PHP") return GlobalSettings::DOC_TYPE_PHP;
	if( name == "PYTHON") return GlobalSettings::DOC_TYPE_PYTHON;
	if( name == "GDSCRIPT") return GlobalSettings::DOC_TYPE_GDSCRIPT;
	if( name == "RUBY") return GlobalSettings::DOC_TYPE_RUBY;
	if( name == "SQL") return GlobalSettings::DOC_TYPE_SQL;
	//if( name == "TXT")
	return GlobalSettings::DOC_TYPE_TXT;
}
QString SettingsMgr::defaultExt(int type)
{
	switch( type ) {
		case GlobalSettings::DOC_TYPE_CPP:	return"cpp";
		case GlobalSettings::DOC_TYPE_CS:	return"cs";
		case GlobalSettings::DOC_TYPE_CSS:	return"css";
		case GlobalSettings::DOC_TYPE_FS:	return"fs";
		case GlobalSettings::DOC_TYPE_HLSL:	return"fx";
		case GlobalSettings::DOC_TYPE_HTML:	return"html";
		case GlobalSettings::DOC_TYPE_JAVA:	return"java";
		case GlobalSettings::DOC_TYPE_JS:	return"js";
		case GlobalSettings::DOC_TYPE_TS:	return"ts";
		case GlobalSettings::DOC_TYPE_LOG:	return"log";
		case GlobalSettings::DOC_TYPE_MARKDOWN:	return"md";
		case GlobalSettings::DOC_TYPE_PASCAL:	return"pas";
		case GlobalSettings::DOC_TYPE_PERL:	return"pl";
		case GlobalSettings::DOC_TYPE_PHP:	return"php";
		case GlobalSettings::DOC_TYPE_PYTHON:	return"py";
		case GlobalSettings::DOC_TYPE_GDSCRIPT:	return"gd";
		case GlobalSettings::DOC_TYPE_RUBY:	return"rb";
		case GlobalSettings::DOC_TYPE_SQL:	return"sql";
		default:
		case GlobalSettings::DOC_TYPE_TXT:	return"txt";
	}
}
TypeSettings *SettingsMgr::typeSettings(const QString typeName)		//	í èÌÅAtypeName ÇÕëÂï∂éö
{
	if( m_typeMap.contains(typeName) )
		return m_typeMap[typeName];
	QString ext = defaultExt(typeNameToType(typeName));
	TypeSettings *typeSettings = new TypeSettings(typeName, ext);
	return m_typeMap[typeName] = typeSettings;
}
TypeSettings *SettingsMgr::typeSettingsForExt(const QString ext)
{
	QString extLower = ext.toLower();
	QString typeName = "Default";
	if( m_extToTypeMap.contains(extLower) )
		typeName = m_extToTypeMap[extLower];
	return typeSettings(typeName);
}
QString SettingsMgr::typeNameForExt(const QString &ext)
{
	QString extLower = ext.toLower();
	QString typeName = "Default";
	if( m_extToTypeMap.contains(extLower) )
		typeName = m_extToTypeMap[extLower];
	return typeName;
}
void SettingsMgr::readSettings()
{
	m_extToTypeMap.clear();
	QSettings settings;
	QStringList lst = settings.value("typeMap").toStringList();
	if( lst.isEmpty() ) {
		lst << "TXT = txt"
			<< "CPP = h,hpp,hxx,c,cpp,cxx,cc,cp"
			<< "C# = cs"
			<< "HTML = html,htm,php,phtml,xml"
			<< "CSS = css"
			<< "MARKDN = md"
			<< "JS = js"
			<< "TS = ts"
			<< "JAVA = java"
			<< "PASCAL = pas,inc,int"
			<< "PERL = cgi,pl,pm,t"
			<< "RUBY = rb"
			//<< "PHP = php,phtml"
			<< "PYTHON = py"
			<< "GDSCRIPT = gd"
			<< "F# = fs,fsi,fsx,fsscript,ml,mli"
			<< "SQL = sql"
			<< "HLSL = fx"
			<< "SPR = spr"
			<< "LOG = log";
	}
	foreach(QString txt, lst) {
		int x = txt.indexOf(' ');
		if( x < 0 ) continue;
		QString type = txt.left(x);
		//QString exts = txt.mid(x + 3);
		QStringList extlst = txt.mid(x + 3).split(",");
		foreach(QString ext, extlst) {
			m_extToTypeMap[ext] = type;
		}
	}
	qDebug() << m_extToTypeMap;
}
void SettingsMgr::writeSettings() const
{
	for(auto itr = m_typeMap.begin();
		itr != m_typeMap.end();
		++itr)
	{
		TypeSettings *ptr = itr.value();
		ptr->writeSettings();
	}
}
