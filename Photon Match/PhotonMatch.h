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
#include <QPushButton>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDebug>
#include <QFile>
#include <QDir>
#include <QDirIterator>
#include <QInputDialog>
#include <QMessageBox>
#include <QCloseEvent>
#include <QSettings>
#include <QSound>
#include <QTimer>
#include <memory>
#include <vector>
#include <random>
#include <chrono>
#include <algorithm>

class PhotonMatch : public QMainWindow
{
	Q_OBJECT

public:
	PhotonMatch(QWidget *parent = Q_NULLPTR);
	void closeEvent(QCloseEvent *event);

private:
	Ui::PhotonMatchClass ui;

	const QString appExecutablePath = QCoreApplication::applicationDirPath();

	std::unique_ptr<QVBoxLayout> baseLayout = std::make_unique<QVBoxLayout>();
	std::unique_ptr<QGridLayout> flipCardLayout = std::make_unique<QGridLayout>();
	std::unique_ptr<QHBoxLayout> uiLayout = std::make_unique<QHBoxLayout>();

	const QString flipCardBtnStyleSheet =
		"QPushButton{ background-color: #CCCCCC; border-style: solid; border-width: 2px; border-color: #404040; }"
		"QPushButton:hover{ background-color: #FFD800; border-color: #282200; border-width: 10px; }";

	const QString flipCardBtnFlippedStyleSheet =
		"QPushButton{ background-color: #FFF6CC; border-style: solid; border-width: 2px; border-color: #26241E; font: bold 14px;}"
		"QPushButton:hover{ background-color: #FFF6CC; }";

	const QString flipCardBtnSolvedStyleSheet =
		"QPushButton{ background-color: #E5E5E5; border-style: solid; border-width: 2px; border-color: #999999; font: bold 14px;}"
		"QPushButton:hover{ background-color: #E5E5E5; }";

	const QString uiBtnEnabledStyleSheet =
		"QPushButton{ font-weight: bold; background-color: #CCCCCC; border-style: solid; border-width: 2px; border-color: #404040; padding: 4px; }"
		"QPushButton:hover{ background-color: #FFD800; border-color: #282200; color: #000000; border-width: 4px; }";

	const QString uiBtnDisabledStyleSheet =
		"QPushButton{ font-weight: bold; background-color: #E5E5E5; border-style: solid; border-width: 2px; border-color: #999999; padding: 4px; }"
		"QPushButton:hover{ background-color: #E5E5E5; }";

	enum class UiBtnType { NEW_PUZZLE, CHOOSE_LANGUAGE, CHOOSE_CATEGORY, CHOOSE_AUDIO };
	struct uiBtn
	{
		const QString initText;
		const QSize minSize;
		std::unique_ptr<QPushButton> btn = std::make_unique<QPushButton>();
	};
	std::map<UiBtnType, uiBtn> uiBtnMap;

	QString textToSpeechSetting = "NONE";
	QString textToSpeechSettingDisplay = "SPEECH: %1";
	QStringList langChoiceDisplayList;
	QString currentLangKey;
	int currentLangIndex = 0;
	QStringList catChoiceDisplayList;
	QString currentCatKey;
	int currentCatIndex = 0;

	std::map<QString, std::vector<QStringList>> wordPairsMap;

	struct flipCard
	{
		std::unique_ptr<QPushButton> btn = std::make_unique<QPushButton>();
		enum class VisState { HIDDEN, FLIPPED, SOLVED };
		VisState visState = VisState::HIDDEN;
		QString wordKey; // the first word in map of pairs
		QString wordDisplay; // the word to display on the button, can be first word in a pair or second
		QString soundPath;
		enum class SoundLang { LEFT, RIGHT, NONE };
		SoundLang soundLang = SoundLang::NONE;
	};

	std::map<int, flipCard> flipCardMap;
	std::vector<int> flipCardKeyList;

	const QSize btnMinSize = QSize(100, 100);
	const int maxFlipped = 2; // The maximum number of "pieces" that can be in the flipped up state at the same time.
	int flippedCount = 0;
	int flippedFirstIndex = -1;
	const int flipCardListSize = 20;
	const int flipRowLength = 4;
	const int flipColLength = 5;
	int solvedCount = 0;

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
