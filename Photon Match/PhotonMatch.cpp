#include "PhotonMatch.h"
#include <vector>
#include <thread>
#include <random>
#include <chrono>
#include <algorithm>
#include <QPushButton>
#include <QDebug>
#include <QFile>
#include <QDir>
#include <QDirIterator>
#include <QInputDialog>
#include <QMessageBox>
#include <QCloseEvent>
#include <QSettings>
#include <QSound>

PhotonMatch::PhotonMatch(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);

	ui.menuFile->menuAction()->setVisible(false);
	ui.mainToolBar->setVisible(false);

	puzzleCompleteSplash->setPixmap(QPixmap(appExecutablePath + "/splash/puzzle-complete-splash.png"));

	ui.newPuzzleBtn->setStyleSheet(pushButtonUtilityEnabledStyleSheet);
	ui.chooseLangBtn->setStyleSheet(pushButtonUtilityEnabledStyleSheet);
	ui.chooseCategoryBtn->setStyleSheet(pushButtonUtilityEnabledStyleSheet);

	connect(ui.chooseLangBtn, &QPushButton::clicked, this, &PhotonMatch::chooseLanguage);
	connect(ui.chooseCategoryBtn, &QPushButton::clicked, this, &PhotonMatch::chooseCategory);

	connect(ui.newPuzzleBtn, &QPushButton::clicked, this, [=]() {
		if (QGuiApplication::queryKeyboardModifiers().testFlag(Qt::ShiftModifier))
		{
			std::random_device rd;
			std::mt19937 mt(rd());
			std::uniform_int_distribution<int> dist(0, catChoiceDisplayList.length()-1);
			int randomCatIndex = dist(mt);
			currentCatIndex = randomCatIndex;
			currentCatKey = catChoiceDisplayList[currentCatIndex];
			qDebug() << "Random index is: " + QString::number(randomCatIndex);
		}
		
		if (populateFlipCardList())
			QMessageBox::information(this, tr("New Puzzle Created"), tr("New puzzle created! Have fun!"));
		else
			QMessageBox::warning(this, tr("Puzzle Creation Error"), tr("There was an error when trying to create a new puzzle."));
	});

	QDirIterator dirIt(appExecutablePath + "/WordPairs", QDir::Files, QDirIterator::Subdirectories);
	while (dirIt.hasNext())
	{
		QString currentFile = dirIt.next();
		if (QFileInfo(currentFile).suffix() == "txt")
		{
			qDebug() << currentFile;

			// Get directory path of file that comes after the "WordPairs" part.
			// First directory path after is the language name.
			// Second directory path after is the category name.
			std::string langKey = extractSubstringInbetween("WordPairs/", "/", currentFile.toStdString());
			std::string catKey = extractSubstringInbetween(langKey + "/", "/", currentFile.toStdString());
			QString dictEntryKey = QString::fromStdString(langKey) + "_" + QString::fromStdString(catKey);

			qDebug() << dictEntryKey;

			// Read contents of file into vector/map entry, along with key based on combined Lang+Cat path name.
			std::vector<QStringList> newWordPairsList;
			QFile fileRead(currentFile);
			if (fileRead.open(QIODevice::ReadOnly))
			{
				QTextStream contents(&fileRead);
				while (!contents.atEnd())
				{
					QString line = contents.readLine();
					QStringList wordPair = line.split(",");

					QString soundPathFirst = QFileInfo(currentFile).path();
					soundPathFirst.replace("WordPairs", "TextToSpeech");
					soundPathFirst.append("/" + QFileInfo(currentFile).baseName());
					QString wordFirst = wordPair[0];
					wordFirst.replace(" ", "");
					wordFirst.replace("/", "");
					soundPathFirst.append("/" + wordFirst + ".wav");

					QString soundPathSecond = QFileInfo(currentFile).path();
					soundPathSecond.replace("WordPairs", "TextToSpeech");
					soundPathSecond.append("/" + QFileInfo(currentFile).baseName());
					QString wordSecond = wordPair[1];
					wordSecond.replace(" ", "");
					wordSecond.replace("/", "");
					soundPathSecond.append("/" + wordSecond + ".wav");

					wordPair.append(soundPathFirst);
					wordPair.append(soundPathSecond);

					wordPair[0].replace(" ", "\n");
					wordPair[1].replace(" ", "\n");
					newWordPairsList.push_back(wordPair);
					//qDebug() << wordPair;
				}
				fileRead.close();
			}

			// If there's a duplicate category entry, merge the two.
			wordPairsMapIterator = wordPairsMap.begin();
			wordPairsMapIterator = wordPairsMap.find(dictEntryKey);
			if (wordPairsMapIterator != wordPairsMap.end())
			{
				std::vector<QStringList> mergedWordPairsList = wordPairsMapIterator->second;
				mergedWordPairsList.insert(mergedWordPairsList.end(), newWordPairsList.begin(), newWordPairsList.end());
				wordPairsMap[dictEntryKey] = mergedWordPairsList;
			}
			else
				wordPairsMap.insert(std::pair<QString, std::vector<QStringList>>(dictEntryKey, newWordPairsList));
		}
	}

	{
		wordPairsMapIterator = wordPairsMap.begin();
		while (wordPairsMapIterator != wordPairsMap.end())
		{
			std::string displayLang = extractSubstringInbetween("", "_", wordPairsMapIterator->first.toStdString());
			langChoiceDisplayList.append(QString::fromStdString(displayLang));
			wordPairsMapIterator++;
		}
		langChoiceDisplayList.removeDuplicates();
		currentLangKey = langChoiceDisplayList[currentLangIndex];
		populateCatDisplayList();
		currentCatKey = catChoiceDisplayList[currentCatIndex];
	}

	prefLoad();

	populateFlipCardList();
}

void PhotonMatch::closeEvent(QCloseEvent *event)
{
	prefSave();
	event->accept();
}

void PhotonMatch::chooseLanguage()
{
	bool ok;
	QString langChoice = QInputDialog::getItem(this, tr("Choose Language"), tr("Language:"), langChoiceDisplayList, currentLangIndex, false, &ok, Qt::WindowTitleHint | Qt::WindowCloseButtonHint | Qt::MSWindowsFixedSizeDialogHint);
	if (ok && !langChoice.isEmpty())
	{
		currentLangKey = langChoice;
		currentLangIndex = langChoiceDisplayList.indexOf(langChoice);
		populateCatDisplayList();
		currentCatIndex = 0;
		currentCatKey = catChoiceDisplayList[currentCatIndex];
	}
}

void PhotonMatch::chooseCategory()
{
	bool ok;
	QString catChoice = QInputDialog::getItem(this, tr("Choose Category"), tr("Category:"), catChoiceDisplayList, currentCatIndex, false, &ok, Qt::WindowTitleHint | Qt::WindowCloseButtonHint | Qt::MSWindowsFixedSizeDialogHint);
	if (ok && !catChoice.isEmpty())
	{
		currentCatKey = catChoice;
		currentCatIndex = catChoiceDisplayList.indexOf(catChoice);
	}
}

bool PhotonMatch::populateFlipCardList()
{
	puzzleCompleteSplash->hide();

	flippedCount = 0;
	flippedFirstIndex = -1;
	solvedCount = 0;
	std::vector<flipCard> tempFlipCardList;

	//shuffleWordPairsList();

	for (auto& card : flipCardList)
	{
		card.pushButtonPointer.get()->disconnect();
		card.pushButtonPointer.release();
	}

	if (!wordPairsMap.empty())
	{
		wordPairsMapIterator = wordPairsMap.begin();
		QString currentKeyToFind = currentLangKey + "_" + currentCatKey;
		qDebug() << currentKeyToFind;
		wordPairsMapIterator = wordPairsMap.find(currentKeyToFind);
		if (wordPairsMapIterator == wordPairsMap.end())
			return false;

		std::vector<QStringList> listInWordPairsMap = wordPairsMapIterator->second;
		shuffleVecOfQStringList(listInWordPairsMap);

		for (int i = 0; i < (flipCardListSize / 2); i++)
		{
			flipCard newFlipCard;
			newFlipCard.visState = flipCard::VisState::HIDDEN;
			newFlipCard.wordKey = listInWordPairsMap[i][0];
			newFlipCard.wordDisplay = listInWordPairsMap[i][0];
			if (textToSpeechSetting == "ALL" || textToSpeechSetting == "FIRST")
				newFlipCard.soundPath = listInWordPairsMap[i][2];
			tempFlipCardList.push_back(std::move(newFlipCard));

			flipCard newFlipCardMatch;
			newFlipCardMatch.visState = flipCard::VisState::HIDDEN;
			newFlipCardMatch.wordKey = listInWordPairsMap[i][0];
			newFlipCardMatch.wordDisplay = listInWordPairsMap[i][1];
			if (textToSpeechSetting == "ALL" || textToSpeechSetting == "SECOND")
				newFlipCardMatch.soundPath = listInWordPairsMap[i][3];
			tempFlipCardList.push_back(std::move(newFlipCardMatch));
		}

		flipCardList.swap(tempFlipCardList);

		shuffleFlipCardList();

		flipCardList[0].pushButtonPointer.reset(ui.pushButton_1);
		flipCardList[1].pushButtonPointer.reset(ui.pushButton_2);
		flipCardList[2].pushButtonPointer.reset(ui.pushButton_3);
		flipCardList[3].pushButtonPointer.reset(ui.pushButton_4);
		flipCardList[4].pushButtonPointer.reset(ui.pushButton_5);
		flipCardList[5].pushButtonPointer.reset(ui.pushButton_6);
		flipCardList[6].pushButtonPointer.reset(ui.pushButton_7);
		flipCardList[7].pushButtonPointer.reset(ui.pushButton_8);
		flipCardList[8].pushButtonPointer.reset(ui.pushButton_9);
		flipCardList[9].pushButtonPointer.reset(ui.pushButton_10);
		flipCardList[10].pushButtonPointer.reset(ui.pushButton_11);
		flipCardList[11].pushButtonPointer.reset(ui.pushButton_12);
		flipCardList[12].pushButtonPointer.reset(ui.pushButton_13);
		flipCardList[13].pushButtonPointer.reset(ui.pushButton_14);
		flipCardList[14].pushButtonPointer.reset(ui.pushButton_15);
		flipCardList[15].pushButtonPointer.reset(ui.pushButton_16);
		flipCardList[16].pushButtonPointer.reset(ui.pushButton_17);
		flipCardList[17].pushButtonPointer.reset(ui.pushButton_18);
		flipCardList[18].pushButtonPointer.reset(ui.pushButton_19);
		flipCardList[19].pushButtonPointer.reset(ui.pushButton_20);

		for (const auto &card : flipCardList)
		{
			card.pushButtonPointer->setStyleSheet(pushButtonStyleSheet);
			card.pushButtonPointer->setEnabled(true);
			card.pushButtonPointer->setText("");
		}

		for (int i = 0; i < flipCardListSize; i++)
		{
			connect(flipCardList[i].pushButtonPointer.get(), &QPushButton::released, this, [=]() {
				flipClickedCard(i);
			});
		}
	}

	for (auto& card : flipCardList)
	{
		qDebug() << card.wordDisplay;
	}

	return true;
}

void PhotonMatch::flipClickedCard(const int btnI)
{
	if (flippedCount < maxFlipped)
	{
		flippedCount++;
		flipCardList[btnI].pushButtonPointer->setStyleSheet(pushButtonFlippedStyleSheet);
		flipCardList[btnI].visState = flipCard::VisState::FLIPPED;
		flipCardList[btnI].pushButtonPointer->setText(flipCardList[btnI].wordDisplay);
		if (textToSpeechSetting == "ALL" || textToSpeechSetting == "FIRST" || textToSpeechSetting == "SECOND")
		{
			if (!flipCardList[btnI].soundPath.isEmpty())
			{
				if (QFileInfo::exists(flipCardList[btnI].soundPath))
				{
					QSound::play(flipCardList[btnI].soundPath);
					//qDebug() << flipCardList[btnI].soundPath;
				}
			}
		}

		if (flippedCount == 1)
		{
			flippedFirstIndex = btnI;
		}
		else if (flippedCount == 2)
		{
			this->repaint();
			std::this_thread::sleep_for(std::chrono::milliseconds(1000));
			if (flipCardList[flippedFirstIndex].wordKey == flipCardList[btnI].wordKey &&
				flippedFirstIndex != btnI)
			{
				// match found, disable at both indices
				flipCardList[flippedFirstIndex].pushButtonPointer->setEnabled(false);
				flipCardList[btnI].pushButtonPointer->setEnabled(false);
				flipCardList[flippedFirstIndex].visState = flipCard::VisState::SOLVED;
				flipCardList[btnI].visState = flipCard::VisState::SOLVED;
				flipCardList[flippedFirstIndex].pushButtonPointer->setStyleSheet(pushButtonSolvedStyleSheet);
				flipCardList[btnI].pushButtonPointer->setStyleSheet(pushButtonSolvedStyleSheet);
				solvedCount++;
				flippedCount = 0;

				if (solvedCount == (flipCardListSize / 2))
				{
					// do puzzle is complete operations
					// probably call another function to do this
					qDebug("Puzzle complete!");
					puzzleCompleteSplash->show();
				}
			}
			else
			{
				// match not found, wait for a short period of time...
				// then change cards back to hidden state...
				// and reset flipped variables, like count and stored index
				flipCardList[flippedFirstIndex].visState = flipCard::VisState::HIDDEN;
				flipCardList[btnI].visState = flipCard::VisState::HIDDEN;
				flipCardList[flippedFirstIndex].pushButtonPointer->setText("");
				flipCardList[btnI].pushButtonPointer->setText("");
				flipCardList[flippedFirstIndex].pushButtonPointer->setStyleSheet(pushButtonStyleSheet);
				flipCardList[btnI].pushButtonPointer->setStyleSheet(pushButtonStyleSheet);
				flippedCount = 0;
			}
			this->repaint();
		}
	}
}

void PhotonMatch::prefLoad()
{
	QFile fileRead(appExecutablePath + "/preferences.txt");
	if (fileRead.open(QIODevice::ReadOnly))
	{
		QTextStream contents(&fileRead);
		while (!contents.atEnd())
		{
			QString line = contents.readLine();
			if (line.contains("preferredLanguage"))
			{
				QString preferredLanguage = QString::fromStdString(extractSubstringInbetween("=", "", line.toStdString()));
				if (!preferredLanguage.isEmpty() && langChoiceDisplayList.contains(preferredLanguage))
				{
					currentLangKey = preferredLanguage;
					currentLangIndex = langChoiceDisplayList.indexOf(currentLangKey);
					populateCatDisplayList();
					currentCatKey = catChoiceDisplayList[currentCatIndex];
				}
			}
			else if (line.contains("textToSpeech"))
			{
				QString textToSpeechState = QString::fromStdString(extractSubstringInbetween("=", "", line.toStdString()));
				if (
					textToSpeechState == "NONE" ||
					textToSpeechState == "ALL" ||
					textToSpeechState == "FIRST" ||
					textToSpeechState == "SECOND"
					)
				{
					textToSpeechSetting = textToSpeechState;
				}
			}
		}
		fileRead.close();
	}
}

void PhotonMatch::prefSave()
{
	QFile fileWrite(appExecutablePath + "/preferences.txt");
	if (fileWrite.open(QIODevice::WriteOnly))
	{
		QTextStream contents(&fileWrite);
		contents << "preferredLanguage=" + currentLangKey + "\r\n"; // \r is added for notepad linebreak compatibility
		contents << "textToSpeech=" + textToSpeechSetting;
		fileWrite.close();
	}
}

void PhotonMatch::populateCatDisplayList()
{
	QStringList newCategoriesList;
	wordPairsMapIterator = wordPairsMap.begin();
	while (wordPairsMapIterator != wordPairsMap.end())
	{
		QString mapKey = wordPairsMapIterator->first;
		if (mapKey.contains(currentLangKey))
		{
			std::string catStr = extractSubstringInbetween("_", "", wordPairsMapIterator->first.toStdString());
			newCategoriesList.append(QString::fromStdString(catStr));
		}
		wordPairsMapIterator++;
	}
	catChoiceDisplayList = newCategoriesList;
}

void PhotonMatch::shuffleVecOfQStringList(std::vector<QStringList> &listToShuffle)
{
	int seed = std::chrono::system_clock::now().time_since_epoch().count();
	shuffle(listToShuffle.begin(), listToShuffle.end(), std::default_random_engine(seed));
}

void PhotonMatch::shuffleFlipCardList()
{
	int seed = std::chrono::system_clock::now().time_since_epoch().count();
	shuffle(flipCardList.begin(), flipCardList.end(), std::default_random_engine(seed));
}

std::string PhotonMatch::extractSubstringInbetween(const std::string strBegin, const std::string strEnd, const std::string &strExtractFrom)
{
	std::string extracted = "";
	int posFound = 0;

	if (!strBegin.empty() && !strEnd.empty())
	{
		while (strExtractFrom.find(strBegin, posFound) != std::string::npos)
		{
			int posBegin = strExtractFrom.find(strBegin, posFound) + strBegin.length();
			int posEnd = strExtractFrom.find(strEnd, posBegin) + 1 - strEnd.length();
			extracted.append(strExtractFrom, posBegin, posEnd - posBegin);
			posFound = posEnd;
		}
	}
	else if (strBegin.empty() && !strEnd.empty())
	{
		int posBegin = 0;
		int posEnd = strExtractFrom.find(strEnd, posBegin) + 1 - strEnd.length();
		extracted.append(strExtractFrom, posBegin, posEnd - posBegin);
		posFound = posEnd;
	}
	else if (!strBegin.empty() && strEnd.empty())
	{
		int posBegin = strExtractFrom.find(strBegin, posFound) + strBegin.length();
		int posEnd = strExtractFrom.length();
		extracted.append(strExtractFrom, posBegin, posEnd - posBegin);
		posFound = posEnd;
	}
	return extracted;
}