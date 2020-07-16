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

#include "PhotonMatch.h"

PhotonMatch::PhotonMatch(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);

	ui.centralWidget->setLayout(baseLayout.get());
	baseLayout.get()->setMargin(9);
	baseLayout.get()->addLayout(flipCardLayout.get(), 0);
	baseLayout.get()->insertSpacing(1, 20);
	baseLayout.get()->addLayout(uiLayout.get(), 2);

	puzzleCompleteSplash->setPixmap(QPixmap(appExecutablePath + "/splash/puzzle-complete-splash.png"));

	{
		int i = 0;
		for (int col = 0; col < flipColLength; col++)
		{
			for (int row = 0; row < flipRowLength; row++)
			{
				flipCardMap.try_emplace(i, flipCard{});
				flipCardMap.at(i).btn.get()->setParent(this);
				flipCardMap.at(i).btn.get()->setMinimumSize(btnMinSize);
				flipCardMap.at(i).btn.get()->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
				flipCardMap.at(i).btn.get()->setStyleSheet(flipCardBtnStyleSheet);
				flipCardMap.at(i).btn.get()->setText("");
				connect(flipCardMap.at(i).btn.get(), &QPushButton::released, this, [=]() {
					flipClickedCard(i);
				});
				flipCardLayout.get()->addWidget(flipCardMap.at(i).btn.get(), row, col);
				flipCardKeyList.emplace_back(i);
				i++;
			}
		}
	}

	uiBtnMap.try_emplace(UiBtnType::NEW_PUZZLE, uiBtn{ "NEW PUZZLE", QSize(200, 30)});
	uiBtnMap.try_emplace(UiBtnType::CHOOSE_LANGUAGE, uiBtn{ "PICK LANGUAGE", QSize(100, 30)});
	uiBtnMap.try_emplace(UiBtnType::CHOOSE_CATEGORY, uiBtn{ "PICK CATEGORY", QSize(100, 30)});
	uiBtnMap.try_emplace(UiBtnType::CHOOSE_AUDIO, uiBtn{ "SPEECH: NONE", QSize(100, 30)});

	for (auto& uiPair : uiBtnMap)
	{
		uiPair.second.btn.get()->setParent(this);
		uiPair.second.btn.get()->setMinimumSize(uiPair.second.minSize);
		uiPair.second.btn.get()->setText(uiPair.second.initText);
		uiPair.second.btn.get()->setStyleSheet(uiBtnEnabledStyleSheet);
		uiLayout.get()->addWidget(uiPair.second.btn.get(), Qt::AlignCenter);
	}

	connect(uiBtnMap.at(UiBtnType::CHOOSE_LANGUAGE).btn.get(), &QPushButton::clicked, this, &PhotonMatch::chooseLanguage);
	connect(uiBtnMap.at(UiBtnType::CHOOSE_CATEGORY).btn.get(), &QPushButton::clicked, this, &PhotonMatch::chooseCategory);
	connect(uiBtnMap.at(UiBtnType::CHOOSE_AUDIO).btn.get(), &QPushButton::clicked, this, &PhotonMatch::chooseAudio);

	connect(uiBtnMap.at(UiBtnType::NEW_PUZZLE).btn.get(), &QPushButton::clicked, this, [=]() {
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
					QString wordFirstId = extractSubstringInbetweenQt("[id]", "[/id]", wordPair[0]);
					soundPathFirst.append("/" + wordFirstId + ".wav");

					QString soundPathSecond = QFileInfo(currentFile).path();
					soundPathSecond.replace("WordPairs", "TextToSpeech");
					soundPathSecond.append("/" + QFileInfo(currentFile).baseName());
					QString wordSecondId = extractSubstringInbetweenQt("[id]", "[/id]", wordPair[1]);
					soundPathSecond.append("/" + wordSecondId + ".wav");

					if (QFileInfo::exists(soundPathFirst))
						wordPair.append(soundPathFirst);
					else
						wordPair.append("NO TTS");

					if (QFileInfo::exists(soundPathSecond))
						wordPair.append(soundPathSecond);
					else
						wordPair.append("NO TTS");

					wordPair[0] = extractSubstringInbetweenQt("[/id]", "", wordPair[0]);
					wordPair[1] = extractSubstringInbetweenQt("[/id]", "", wordPair[1]);
					wordPair[0].replace(" ", "\n");
					wordPair[1].replace(" ", "\n");
					wordPair[0].replace("[code]comma[/code]", ",", Qt::CaseSensitive);
					wordPair[1].replace("[code]comma[/code]", ",", Qt::CaseSensitive);
					newWordPairsList.push_back(wordPair);
				}
				fileRead.close();
			}

			// If there's a duplicate category entry, merge the two.
			//wordPairsMapIterator = wordPairsMap.begin();
			//wordPairsMapIterator = wordPairsMap.find(dictEntryKey);
			if (wordPairsMap.count(dictEntryKey) > 0)
			{
				std::vector<QStringList> mergedWordPairsList = wordPairsMap.at(dictEntryKey);
				mergedWordPairsList.insert(mergedWordPairsList.end(), newWordPairsList.begin(), newWordPairsList.end());
				wordPairsMap[dictEntryKey] = mergedWordPairsList;
			}
			else
				wordPairsMap.insert(std::pair<QString, std::vector<QStringList>>(dictEntryKey, newWordPairsList));
		}
	}

	{
		for (auto& wordPair : wordPairsMap)
		{
			std::string displayLang = extractSubstringInbetween("", "_", wordPair.first.toStdString());
			langChoiceDisplayList.append(QString::fromStdString(displayLang));
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

void PhotonMatch::chooseAudio()
{
	if (textToSpeechSetting == "NONE")
	{
		textToSpeechSetting = "ALL";
		uiBtnMap.at(UiBtnType::CHOOSE_AUDIO).btn.get()->setText(textToSpeechSettingDisplay.arg(textToSpeechSetting));
	}
	else if (textToSpeechSetting == "ALL")
	{
		textToSpeechSetting = "LEFT";
		uiBtnMap.at(UiBtnType::CHOOSE_AUDIO).btn.get()->setText(textToSpeechSettingDisplay.arg(textToSpeechSetting));
	}
	else if (textToSpeechSetting == "LEFT")
	{
		textToSpeechSetting = "RIGHT";
		uiBtnMap.at(UiBtnType::CHOOSE_AUDIO).btn.get()->setText(textToSpeechSettingDisplay.arg(textToSpeechSetting));
	}
	else if (textToSpeechSetting == "RIGHT")
	{
		textToSpeechSetting = "NONE";
		uiBtnMap.at(UiBtnType::CHOOSE_AUDIO).btn.get()->setText(textToSpeechSettingDisplay.arg(textToSpeechSetting));
	}
}

bool PhotonMatch::populateFlipCardList()
{
	puzzleCompleteSplash->hide();

	flippedCount = 0;
	flippedFirstIndex = -1;
	solvedCount = 0;

	if (!wordPairsMap.empty())
	{
		QString currentKeyToFind = currentLangKey + "_" + currentCatKey;
		qDebug() << currentKeyToFind;
		if (wordPairsMap.count(currentKeyToFind) == 0)
			return false;

		std::vector<QStringList> listInWordPairsMap = wordPairsMap.at(currentKeyToFind);
		shuffleVecOfQStringList(listInWordPairsMap);

		// We store a list of keys to the flip card map in a vector.
		// To shuffle cards, we shuffle the list of keys and then we 
		// apply from word pairs sequentially, using the list of keys sequentially.
		// Since the list of keys has been shuffled, the order gets applied 
		// shuffled, without needing to alter which key the flip card buttons are connected to.

		shuffleFlipCardList();

		for (int i = 0; i < flipCardListSize / 2; i++)
		{
			qDebug() << i;
			int flipKey = flipCardKeyList[i];
			int flipKeyMatch = flipCardKeyList[i + 10];

			flipCardMap.at(flipKey).visState = flipCard::VisState::HIDDEN;
			flipCardMap.at(flipKey).wordKey = listInWordPairsMap[i][0];
			flipCardMap.at(flipKey).wordDisplay = listInWordPairsMap[i][0];
			flipCardMap.at(flipKey).soundPath = listInWordPairsMap[i][2];
			flipCardMap.at(flipKey).soundLang = flipCard::SoundLang::LEFT;

			flipCardMap.at(flipKeyMatch).visState = flipCard::VisState::HIDDEN;
			flipCardMap.at(flipKeyMatch).wordKey = listInWordPairsMap[i][0];
			flipCardMap.at(flipKeyMatch).wordDisplay = listInWordPairsMap[i][1];
			flipCardMap.at(flipKeyMatch).soundPath = listInWordPairsMap[i][3];
			flipCardMap.at(flipKeyMatch).soundLang = flipCard::SoundLang::RIGHT;
		}

		for (const auto &card : flipCardMap)
		{
			card.second.btn.get()->setStyleSheet(flipCardBtnStyleSheet);
			card.second.btn.get()->setEnabled(true);
			card.second.btn.get()->setText("");
		}
	}

	for (auto& card : flipCardMap)
	{
		qDebug() << card.second.wordDisplay;
	}

	return true;
}

void PhotonMatch::flipClickedCard(const int btnI)
{
	if (flippedCount < maxFlipped)
	{
		flippedCount++;
		flipCardMap.at(btnI).btn.get()->setStyleSheet(flipCardBtnFlippedStyleSheet);
		flipCardMap.at(btnI).visState = flipCard::VisState::FLIPPED;
		flipCardMap.at(btnI).btn.get()->setText(flipCardMap.at(btnI).wordDisplay);
		if (textToSpeechSetting == "ALL" ||
			(textToSpeechSetting == "LEFT" && flipCardMap.at(btnI).soundLang == flipCard::SoundLang::LEFT) ||
			(textToSpeechSetting == "RIGHT" && flipCardMap.at(btnI).soundLang == flipCard::SoundLang::RIGHT))
		{
			if (!flipCardMap.at(btnI).soundPath.isEmpty() && flipCardMap.at(btnI).soundPath != "NO TTS")
				QSound::play(flipCardMap.at(btnI).soundPath);
			/*else
				qDebug() << flipCardList[btnI].soundPath;*/
		}

		if (flippedCount == 1)
		{
			flippedFirstIndex = btnI;
		}
		else if (flippedCount == 2)
		{
			QTimer::singleShot(1000, this, [=](){
				if (flipCardMap.at(flippedFirstIndex).wordKey == flipCardMap.at(btnI).wordKey &&
					flippedFirstIndex != btnI)
				{
					// match found, disable at both indices
					flipCardMap.at(flippedFirstIndex).btn.get()->setEnabled(false);
					flipCardMap.at(btnI).btn.get()->setEnabled(false);
					flipCardMap.at(flippedFirstIndex).visState = flipCard::VisState::SOLVED;
					flipCardMap.at(btnI).visState = flipCard::VisState::SOLVED;
					flipCardMap.at(flippedFirstIndex).btn.get()->setStyleSheet(flipCardBtnSolvedStyleSheet);
					flipCardMap.at(btnI).btn.get()->setStyleSheet(flipCardBtnSolvedStyleSheet);
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
					flipCardMap.at(flippedFirstIndex).visState = flipCard::VisState::HIDDEN;
					flipCardMap.at(btnI).visState = flipCard::VisState::HIDDEN;
					flipCardMap.at(flippedFirstIndex).btn.get()->setText("");
					flipCardMap.at(btnI).btn.get()->setText("");
					flipCardMap.at(flippedFirstIndex).btn.get()->setStyleSheet(flipCardBtnStyleSheet);
					flipCardMap.at(btnI).btn.get()->setStyleSheet(flipCardBtnStyleSheet);
					flippedCount = 0;
				}
				//qDebug("Timer went off.");
			});
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
					textToSpeechState == "LEFT" ||
					textToSpeechState == "RIGHT"
					)
				{
					textToSpeechSetting = textToSpeechState;
					uiBtnMap.at(UiBtnType::CHOOSE_AUDIO).btn.get()->setText(textToSpeechSettingDisplay.arg(textToSpeechSetting));
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
	for (auto& wordPair : wordPairsMap)
	{
		QString mapKey = wordPair.first;
		if (currentLangKey == extractSubstringInbetweenQt("", "_", mapKey))
		{
			std::string catStr = extractSubstringInbetween("_", "", wordPair.first.toStdString());
			newCategoriesList.append(QString::fromStdString(catStr));
		}
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
	shuffle(flipCardKeyList.begin(), flipCardKeyList.end(), std::default_random_engine(seed));
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
			int posEnd = strExtractFrom.find(strEnd, posBegin);
			extracted.append(strExtractFrom, posBegin, posEnd - posBegin);
			posFound = posEnd;
		}
	}
	else if (strBegin.empty() && !strEnd.empty())
	{
		int posBegin = 0;
		int posEnd = strExtractFrom.find(strEnd, posBegin);
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

QString PhotonMatch::extractSubstringInbetweenQt(const QString strBegin, const QString strEnd, const QString &strExtractFrom)
{
	QString extracted;
	int posFound = 0;

	if (!strBegin.isEmpty() && !strEnd.isEmpty())
	{
		while (strExtractFrom.indexOf(strBegin, posFound, Qt::CaseSensitive) != -1)
		{
			int posBegin = strExtractFrom.indexOf(strBegin, posFound, Qt::CaseSensitive) + strBegin.length();
			int posEnd = strExtractFrom.indexOf(strEnd, posBegin, Qt::CaseSensitive);
			extracted.append(strExtractFrom.mid(posBegin, posEnd - posBegin));
			posFound = posEnd;
		}
	}
	else if (strBegin.isEmpty() && !strEnd.isEmpty())
	{
		int posBegin = 0;
		int posEnd = strExtractFrom.indexOf(strEnd, posBegin, Qt::CaseSensitive);
		extracted.append(strExtractFrom.mid(posBegin, posEnd - posBegin));
		posFound = posEnd;
	}
	else if (!strBegin.isEmpty() && strEnd.isEmpty())
	{
		int posBegin = strExtractFrom.indexOf(strBegin, posFound, Qt::CaseSensitive) + strBegin.length();
		int posEnd = strExtractFrom.length();
		extracted.append(strExtractFrom.mid(posBegin, posEnd - posBegin));
		posFound = posEnd;
	}
	return extracted;
}