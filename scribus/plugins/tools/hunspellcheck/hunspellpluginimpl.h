/*
For general Scribus (>=1.3.2) copyright and licensing information please refer
to the COPYING file provided with the program. Following this notice may exist
a copyright and/or license notice that predates the release of Scribus 1.3.2
for which a new license (GPL+exception) is in place.
*/
#ifndef HUNSPELLPLUGINIMPL_H
#define HUNSPELLPLUGINIMPL_H

#include <hunspell/hunspell.hxx>
#include "hunspellpluginstructs.h"

#include <QObject>
#include <QString>
#include <QStringList>

class QString;
class ScribusDoc;
class PageItem;



class HunspellPluginImpl : public QObject
{
	Q_OBJECT
	public:
		HunspellPluginImpl();
		~HunspellPluginImpl();
		bool run(const QString & target, ScribusDoc* doc=0);
		bool findDictionaries();
		bool initHunspell();
		bool checkWithHunspell();
		bool parseTextFrame(PageItem *frameToCheck);
		bool openGUIForTextFrame(PageItem *frameToCheck);
		QList<WordsFound> wordsToCorrect;

	protected:
		QStringList dictionaryPaths;
		QString dictPath, affPath;
		int numDicts, numAFFs;
		Hunspell **hspellers;
		QStringList dictEntries;
		QStringList affEntries;
		ScribusDoc* m_doc;
};

#endif

