#ifndef TABCHECKDOC_H
#define TABCHECKDOC_H

#include <qvariant.h>
#include <qwidget.h>

#include "scribusstructs.h"
class QVBoxLayout;
class QHBoxLayout;
class QComboBox;
class QCheckBox;
class QGroupBox;
class QLabel;
class QSpinBox;
class QPushButton;

class TabCheckDoc : public QWidget
{
	Q_OBJECT

public:
	TabCheckDoc( QWidget* parent, QMap<QString, checkerPrefs> prefsData, QString prefProfile );
	~TabCheckDoc() {};
	void restoreDefaults();

	QComboBox* curCheckProfile;
	QCheckBox* ignoreErrors;
	QCheckBox* automaticCheck;
	QCheckBox* missingGlyphs;
	QCheckBox* checkOrphans;
	QCheckBox* textOverflow;
	QCheckBox* tranparentObjects;
	QCheckBox* missingPictures;
	QCheckBox* useAnnotations;
	QCheckBox* rasterPDF;
	QGroupBox* pictResolution;
	QLabel* textLabel1;
	QSpinBox* resolutionValue;
	QPushButton* addProfile;
	QPushButton* removeProfile;
	QMap<QString, checkerPrefs> checkerProfile;
	QString currentProfile;
	void updateProfile(const QString& name);

public slots:
	void putProfile();
	void setProfile(const QString& name);
	void addProf();
	void delProf();

protected:
	QVBoxLayout* TabCheckDocLayout;
	QHBoxLayout* pictResolutionLayout;
	QHBoxLayout* layout1;
};

#endif // TABCHECKDOC_H
