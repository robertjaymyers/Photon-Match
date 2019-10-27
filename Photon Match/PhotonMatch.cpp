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

PhotonMatch::PhotonMatch(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);

	puzzleCompleteSplash->setPixmap(QPixmap(appExecutablePath + "/splash/puzzle-complete-splash.png"));

	ui.newPuzzleBtn->setStyleSheet(pushButtonUtilityEnabledStyleSheet);
	ui.chooseLangBtn->setStyleSheet(pushButtonUtilityEnabledStyleSheet);
	ui.chooseCategoryBtn->setStyleSheet(pushButtonUtilityEnabledStyleSheet);

	connect(ui.chooseLangBtn, &QPushButton::clicked, this, &PhotonMatch::chooseLanguage);
	connect(ui.chooseCategoryBtn, &QPushButton::clicked, this, &PhotonMatch::chooseCategory);

	connect(ui.newPuzzleBtn, &QPushButton::clicked, this, &PhotonMatch::populateFlipCardList);

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
					line.replace(" ", "\n");
					QStringList wordPair = line.split(",");
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

	//QFile fileRead("english-french-animals-dictionary.txt");
	//if (fileRead.open(QIODevice::ReadOnly))
	//{
	//	QTextStream contents(&fileRead);
	//	while (!contents.atEnd())
	//	{
	//		QString line = contents.readLine();
	//		line.replace(" ", "\n");
	//		QStringList wordPair = line.split(",");
	//		wordPairsList.push_back(wordPair);
	//		//qDebug() << wordPair;
	//	}
	//	fileRead.close();
	//}

	populateFlipCardList();

	////this->setStyleSheet(pushButtonStyleSheet);

	////std::vector<QPushButton*> flipCardList(20);
	////flipCardList[0] = ui.pushButton;

	//shuffleWordPairsList();

	//if (wordPairsList.size() >= (flipCardListSize / 2))
	//{
	//	for (int i = 0; i < (flipCardListSize / 2); i++)
	//	{
	//		flipCard newFlipCard;
	//		newFlipCard.visState = flipCard::VisState::HIDDEN;
	//		newFlipCard.wordKey = wordPairsList[i][0];
	//		newFlipCard.wordDisplay = wordPairsList[i][0];
	//		flipCardList.push_back(newFlipCard);

	//		flipCard newFlipCardMatch;
	//		newFlipCardMatch.visState = flipCard::VisState::HIDDEN;
	//		newFlipCardMatch.wordKey = wordPairsList[i][0];
	//		newFlipCardMatch.wordDisplay = wordPairsList[i][1];
	//		flipCardList.push_back(newFlipCardMatch);
	//	}
	//	shuffleFlipCardList();
	//	flipCardList[0].pushButtonPointer = ui.pushButton_1;
	//	flipCardList[1].pushButtonPointer = ui.pushButton_2;
	//	flipCardList[2].pushButtonPointer = ui.pushButton_3;
	//	flipCardList[3].pushButtonPointer = ui.pushButton_4;
	//	flipCardList[4].pushButtonPointer = ui.pushButton_5;
	//	flipCardList[5].pushButtonPointer = ui.pushButton_6;
	//	flipCardList[6].pushButtonPointer = ui.pushButton_7;
	//	flipCardList[7].pushButtonPointer = ui.pushButton_8;
	//	flipCardList[8].pushButtonPointer = ui.pushButton_9;
	//	flipCardList[9].pushButtonPointer = ui.pushButton_10;
	//	flipCardList[10].pushButtonPointer = ui.pushButton_11;
	//	flipCardList[11].pushButtonPointer = ui.pushButton_12;
	//	flipCardList[12].pushButtonPointer = ui.pushButton_13;
	//	flipCardList[13].pushButtonPointer = ui.pushButton_14;
	//	flipCardList[14].pushButtonPointer = ui.pushButton_15;
	//	flipCardList[15].pushButtonPointer = ui.pushButton_16;
	//	flipCardList[16].pushButtonPointer = ui.pushButton_17;
	//	flipCardList[17].pushButtonPointer = ui.pushButton_18;
	//	flipCardList[18].pushButtonPointer = ui.pushButton_19;
	//	flipCardList[19].pushButtonPointer = ui.pushButton_20;
	//}

	////std::map<QString, QString>::iterator wordPairsMapIterator = wordPairs.begin();
	//////std::vector<flipCard> flipCardList(flipCardListSize);
	////{
	////	for (int i = 0; i < flipCardListSize; i += 2)
	////	{
	////		flipCard newFlipCard;
	////		//newFlipCard.pushButtonPointer = ui.pushButton_1;
	////		newFlipCard.visState = flipCard::VisState::HIDDEN;
	////		newFlipCard.wordKey = wordPairsMapIterator->first;
	////		newFlipCard.wordDisplay = wordPairsMapIterator->first;
	////		flipCardList.push_back(newFlipCard);
	////		//flipCardList[i] = newFlipCard;

	////		flipCard newFlipCardMatch;
	////		//newFlipCardMatch.pushButtonPointer = ui.pushButton_2;
	////		newFlipCardMatch.visState = flipCard::VisState::HIDDEN;
	////		newFlipCardMatch.wordKey = wordPairsMapIterator->first;
	////		newFlipCardMatch.wordDisplay = wordPairsMapIterator->second;
	////		flipCardList.push_back(newFlipCardMatch);
	////		//flipCardList[i + 1] = newFlipCardMatch;

	////		//wordPairsMapIterator++;
	////		std::advance(wordPairsMapIterator, 1);
	////	}
	////	flipCardList[0].pushButtonPointer = ui.pushButton_1;
	////	flipCardList[1].pushButtonPointer = ui.pushButton_2;
	////	flipCardList[2].pushButtonPointer = ui.pushButton_3;
	////	flipCardList[3].pushButtonPointer = ui.pushButton_4;
	////	flipCardList[4].pushButtonPointer = ui.pushButton_5;
	////	flipCardList[5].pushButtonPointer = ui.pushButton_6;
	////	flipCardList[6].pushButtonPointer = ui.pushButton_7;
	////	flipCardList[7].pushButtonPointer = ui.pushButton_8;
	////	flipCardList[8].pushButtonPointer = ui.pushButton_9;
	////	flipCardList[9].pushButtonPointer = ui.pushButton_10;
	////	flipCardList[10].pushButtonPointer = ui.pushButton_11;
	////	flipCardList[11].pushButtonPointer = ui.pushButton_12;
	////	flipCardList[12].pushButtonPointer = ui.pushButton_13;
	////	flipCardList[13].pushButtonPointer = ui.pushButton_14;
	////	flipCardList[14].pushButtonPointer = ui.pushButton_15;
	////	flipCardList[15].pushButtonPointer = ui.pushButton_16;
	////	flipCardList[16].pushButtonPointer = ui.pushButton_17;
	////	flipCardList[17].pushButtonPointer = ui.pushButton_18;
	////	flipCardList[18].pushButtonPointer = ui.pushButton_19;
	////	flipCardList[19].pushButtonPointer = ui.pushButton_20;
	////}

	//for (const auto &card : flipCardList)
	//{
	//	card.pushButtonPointer->setStyleSheet(pushButtonStyleSheet);
	//}

	///*for (auto& i : flipCardList)
	//{
	//	qDebug() << i.wordDisplay;
	//	i.pushButtonPointer->setText(i.wordDisplay);
	//}*/

	//for (int i = 0; i < flipCardListSize; i++)
	//{
	//	connect(flipCardList[i].pushButtonPointer, &QPushButton::released, this, [=]() {
	//		flipClickedCard(i);
	//	});
	//}

	////for (auto &card : flipCardList)
	////{
	////	connect(card.pushButtonPointer, &QPushButton::clicked, this, [=]() {
	////		if (flippedCount < maxFlipped)
	////		{
	////			//card.visState = flipCard::VisState::FLIPPED;
	////			//card.wordDisplay == "test";
	////		}
	////	});
	////}
	////connect(flipCardList[0].pushButtonPointer, &QPushButton::clicked, this, &PhotonMatch::flipClickedCard);
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

void PhotonMatch::populateFlipCardList()
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
			return;

		std::vector<QStringList> listInWordPairsMap = wordPairsMapIterator->second;
		shuffleVecOfQStringList(listInWordPairsMap);

		for (int i = 0; i < (flipCardListSize / 2); i++)
		{
			flipCard newFlipCard;
			newFlipCard.visState = flipCard::VisState::HIDDEN;
			newFlipCard.wordKey = listInWordPairsMap[i][0];
			newFlipCard.wordDisplay = listInWordPairsMap[i][0];
			tempFlipCardList.push_back(std::move(newFlipCard));

			flipCard newFlipCardMatch;
			newFlipCardMatch.visState = flipCard::VisState::HIDDEN;
			newFlipCardMatch.wordKey = listInWordPairsMap[i][0];
			newFlipCardMatch.wordDisplay = listInWordPairsMap[i][1];
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
}

//void PhotonMatch::populateFlipCardList()
//{
//	flippedCount = 0;
//	flippedFirstIndex = -1;
//	solvedCount = 0;
//	std::vector<flipCard> tempFlipCardList;
//
//	shuffleWordPairsList();
//
//	for (auto& card : flipCardList)
//	{
//		card.pushButtonPointer.get()->disconnect();
//		card.pushButtonPointer.release();
//	}
//
//	if (wordPairsList.size() >= (flipCardListSize / 2))
//	{
//		for (int i = 0; i < (flipCardListSize / 2); i++)
//		{
//			flipCard newFlipCard;
//			newFlipCard.visState = flipCard::VisState::HIDDEN;
//			newFlipCard.wordKey = wordPairsList[i][0];
//			newFlipCard.wordDisplay = wordPairsList[i][0];
//			tempFlipCardList.push_back(std::move(newFlipCard));
//
//			flipCard newFlipCardMatch;
//			newFlipCardMatch.visState = flipCard::VisState::HIDDEN;
//			newFlipCardMatch.wordKey = wordPairsList[i][0];
//			newFlipCardMatch.wordDisplay = wordPairsList[i][1];
//			tempFlipCardList.push_back(std::move(newFlipCardMatch));
//		}
//
//		flipCardList.swap(tempFlipCardList);
//
//		shuffleFlipCardList();
//
//		flipCardList[0].pushButtonPointer.reset(ui.pushButton_1);
//		flipCardList[1].pushButtonPointer.reset(ui.pushButton_2);
//		flipCardList[2].pushButtonPointer.reset(ui.pushButton_3);
//		flipCardList[3].pushButtonPointer.reset(ui.pushButton_4);
//		flipCardList[4].pushButtonPointer.reset(ui.pushButton_5);
//		flipCardList[5].pushButtonPointer.reset(ui.pushButton_6);
//		flipCardList[6].pushButtonPointer.reset(ui.pushButton_7);
//		flipCardList[7].pushButtonPointer.reset(ui.pushButton_8);
//		flipCardList[8].pushButtonPointer.reset(ui.pushButton_9);
//		flipCardList[9].pushButtonPointer.reset(ui.pushButton_10);
//		flipCardList[10].pushButtonPointer.reset(ui.pushButton_11);
//		flipCardList[11].pushButtonPointer.reset(ui.pushButton_12);
//		flipCardList[12].pushButtonPointer.reset(ui.pushButton_13);
//		flipCardList[13].pushButtonPointer.reset(ui.pushButton_14);
//		flipCardList[14].pushButtonPointer.reset(ui.pushButton_15);
//		flipCardList[15].pushButtonPointer.reset(ui.pushButton_16);
//		flipCardList[16].pushButtonPointer.reset(ui.pushButton_17);
//		flipCardList[17].pushButtonPointer.reset(ui.pushButton_18);
//		flipCardList[18].pushButtonPointer.reset(ui.pushButton_19);
//		flipCardList[19].pushButtonPointer.reset(ui.pushButton_20);
//
//		for (const auto &card : flipCardList)
//		{
//			card.pushButtonPointer->setStyleSheet(pushButtonStyleSheet);
//			card.pushButtonPointer->setEnabled(true);
//			card.pushButtonPointer->setText("");
//		}
//
//		for (int i = 0; i < flipCardListSize; i++)
//		{
//			connect(flipCardList[i].pushButtonPointer.get(), &QPushButton::released, this, [=]() {
//				flipClickedCard(i);
//			});
//		}
//	}
//
//	for (auto& card : flipCardList)
//	{
//		qDebug() << card.wordDisplay;
//	}
//}

void PhotonMatch::flipClickedCard(const int btnI)
{
	if (flippedCount < maxFlipped)
	{
		flippedCount++;
		flipCardList[btnI].pushButtonPointer->setStyleSheet(pushButtonFlippedStyleSheet);
		flipCardList[btnI].visState = flipCard::VisState::FLIPPED;
		flipCardList[btnI].pushButtonPointer->setText(flipCardList[btnI].wordDisplay);
		if (flippedCount == 1)
		{
			flippedFirstIndex = btnI;
		}
		else if (flippedCount == 2)
		{
			this->repaint();
			std::this_thread::sleep_for(std::chrono::milliseconds(1000));
			if (flipCardList[flippedFirstIndex].wordKey == flipCardList[btnI].wordKey)
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