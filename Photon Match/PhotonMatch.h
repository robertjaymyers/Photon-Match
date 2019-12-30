/*
This file is part of Photon Match.
	Photon Match is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.
	Photon Match is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
	You should have received a copy of the GNU General Public License
	along with Photon Match.  If not, see <https://www.gnu.org/licenses/>.
*/

#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_PhotonMatch.h"
#include <QSplashScreen>
#include <QSoundEffect>
#include <memory>

class PhotonMatch : public QMainWindow
{
	Q_OBJECT

public:
	PhotonMatch(QWidget *parent = Q_NULLPTR);
	void closeEvent(QCloseEvent *event);

private:
	Ui::PhotonMatchClass ui;

	const QString appExecutablePath = QCoreApplication::applicationDirPath();

	const QString pushButtonStyleSheet =
		"QPushButton{ background-color: #CCCCCC; border-style: solid; border-width: 2px; border-color: #404040; }"
		"QPushButton:hover{ background-color: #FFD800; border-color: #282200; border-width: 10px; }";

	const QString pushButtonFlippedStyleSheet =
		"QPushButton{ background-color: #FFF6CC; border-style: solid; border-width: 2px; border-color: #26241E; font: bold 14px;}"
		"QPushButton:hover{ background-color: #FFF6CC; }";

	const QString pushButtonSolvedStyleSheet =
		"QPushButton{ background-color: #E5E5E5; border-style: solid; border-width: 2px; border-color: #999999; font: bold 14px;}"
		"QPushButton:hover{ background-color: #E5E5E5; }";

	const QString pushButtonUtilityEnabledStyleSheet =
		"QPushButton{ background-color: #CCCCCC; border-style: solid; border-width: 2px; border-color: #404040; padding: 4px; }"
		"QPushButton:hover{ background-color: #FFD800; border-color: #282200; color: #000000; border-width: 4px; }";

	const QString pushButtonUtilityDisabledStyleSheet =
		"QPushButton{ background-color: #E5E5E5; border-style: solid; border-width: 2px; border-color: #999999; padding: 4px; }"
		"QPushButton:hover{ background-color: #E5E5E5; }";

	QString textToSpeechSetting = "NONE";
	QString textToSpeechSettingDisplay = "SPEECH: %1";
	QStringList langChoiceDisplayList;
	QString currentLangKey;
	int currentLangIndex = 0;
	QStringList catChoiceDisplayList;
	QString currentCatKey;
	int currentCatIndex = 0;

	std::map<QString, std::vector<QStringList>> wordPairsMap;
	std::map<QString, std::vector<QStringList>>::iterator wordPairsMapIterator;

	struct flipCard
	{
		std::unique_ptr <QPushButton> pushButtonPointer = std::make_unique<QPushButton>();
		enum class VisState { HIDDEN, FLIPPED, SOLVED };
		VisState visState = VisState::HIDDEN;
		QString wordKey; // the first word in map of pairs
		QString wordDisplay; // the word to display on the button, can be first word in a pair or second
		QString soundPath;
		enum class SoundLang { LEFT, RIGHT, NONE };
		SoundLang soundLang = SoundLang::NONE;
	};

	const int maxFlipped = 2; // The maximum number of "pieces" that can be in the flipped up state at the same time.
	int flippedCount = 0;
	int flippedFirstIndex = -1;
	const int flipCardListSize = 20;
	int solvedCount = 0;
	std::vector<flipCard> flipCardList;

	std::unique_ptr<QSplashScreen> puzzleCompleteSplash = std::make_unique<QSplashScreen>();

	void prefLoad();
	void prefSave();
	void populateCatDisplayList();
	void shuffleVecOfQStringList(std::vector<QStringList> &listToShuffle);
	void shuffleFlipCardList();
	std::string extractSubstringInbetween(const std::string strBegin, const std::string strEnd, const std::string &strExtractFrom);
	QString extractSubstringInbetweenQt(const QString strBegin, const QString strEnd, const QString &strExtractFrom);

private slots:
	void chooseLanguage();
	void chooseCategory();
	void chooseAudio();
	bool populateFlipCardList();
	void flipClickedCard(const int btnI);
};
