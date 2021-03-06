#ifndef __APPCONFIGUPDATER_H__
#define __APPCONFIGUPDATER_H__

#include <QDebug>
#include <QSettings>
#include <QString>

#if QT_VERSION == 0x050600
#include <QObject>
#include <wobjectdefs.h>
#endif

#define MYTETRA_CONFIG_PARAM_NUM 100
#define MYTETRA_CONFIG_PARAM_FIELDS_AT_RECORD 3

class AppConfigUpdater : public QObject {
#if QT_VERSION == 0x050600
	W_OBJECT(AppConfigUpdater)
#else
	Q_OBJECT
#endif

	public:
	AppConfigUpdater(QObject *pobj = 0);
	virtual ~AppConfigUpdater();

	void set_config_file(QString fileName);

	void update_version(int versionFrom, int versionTo, QStringList baseTable, QStringList finalTable);

	private:
	QString update_version_allowcollision(int versionFrom, int versionTo, QString name, QString fromType, QString fromValue, QString toType, QString toValue);

	QSettings *conf;
};

#endif /* __APPCONFIGUPDATER_H__ */
