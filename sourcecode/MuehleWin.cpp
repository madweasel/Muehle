/*********************************************************************
	miniMax.cpp													  
 	Copyright (c) Thomas Weber. All rights reserved.				
	Licensed under the MIT License.
	https://github.com/madweasel/madweasels-cpp
\*********************************************************************/

#include "MuehleWin.h"

#pragma region mainAndInit
//-----------------------------------------------------------------------------
// Name: _tWinMain()
// Desc: entry function
//-----------------------------------------------------------------------------
int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// load default settings	
	loadDefaultSettings();

	// create window with wrapper class
	ww = new wildWeasel::masterMind(hInstance, WndProc, IDS_APP_TITLE, IDC_MUEHLEWIN, IDI_MUEHLEWIN, IDC_ARROW, IDC_MUEHLEWIN, IDI_SMALL, IDC_MUEHLEWIN);
	ww->createWindow(defaultWindowSize.x, defaultWindowSize.y, nCmdShow, WS_OVERLAPPEDWINDOW, NULL);

	// show disclaimer
	if (showDisclaimerOnStart) {
		if (IDOK != ww->showMessageBox(L"Disclaimer", L"Please read the legal notice in the file 'LICENSE.txt' distributed with this software. Press 'OK' to agree.", MB_OKCANCEL)) {
			ww->exitProgram();
			return 0;
		}
	}

	// current set database directory ok?
	ww->checkPathAndModify(strDatabaseDir, wstring(L""));
	
	// expert mode
	if (!expertMode) {
		
		// do not show menu item "Calculate Database"
		RemoveMenu(ww->getHMenu(), IDM_CALCULATE_DATABASE, MF_BYCOMMAND);

		// do not show menu item "Inspect database"
		RemoveMenu(ww->getHMenu(), IDM_INSPECT_DATABASE, MF_BYCOMMAND);

		// limit search depth for minimax algorithmn
		RemoveMenu(ww->getHMenu(), ID_MINIMAXALGORITHMN_2_SEARCHDEPTH7, MF_BYCOMMAND);
		RemoveMenu(ww->getHMenu(), ID_MINIMAXALGORITHMN_2_SEARCHDEPTH8, MF_BYCOMMAND);
		RemoveMenu(ww->getHMenu(), ID_MINIMAXALGORITHMN_2_SEARCHDEPTH9, MF_BYCOMMAND);
		RemoveMenu(ww->getHMenu(), ID_MINIMAXALGORITHMN_2_SEARCHDEPTH10, MF_BYCOMMAND);
		RemoveMenu(ww->getHMenu(), ID_MINIMAXALGORITHMN_2_SEARCHDEPTH11, MF_BYCOMMAND);
		RemoveMenu(ww->getHMenu(), ID_MINIMAXALGORITHMN_SEARCHDEPTH7, MF_BYCOMMAND);
		RemoveMenu(ww->getHMenu(), ID_MINIMAXALGORITHMN_SEARCHDEPTH8, MF_BYCOMMAND);
		RemoveMenu(ww->getHMenu(), ID_MINIMAXALGORITHMN_SEARCHDEPTH9, MF_BYCOMMAND);
		RemoveMenu(ww->getHMenu(), ID_MINIMAXALGORITHMN_SEARCHDEPTH10, MF_BYCOMMAND);
		RemoveMenu(ww->getHMenu(), ID_MINIMAXALGORITHMN_SEARCHDEPTH11, MF_BYCOMMAND);

		// if database files are missing than quit
		if (!(filesystem::exists(strDatabaseDir + L"/plyInfo.dat") && filesystem::exists(strDatabaseDir + L"/shortKnotValue.dat"))) {
			ww->showMessageBox(L"ERROR", L"The database files are missing. Please download them and execute the batch file 'install.bat'. Program will stop now.", MB_OK);
			ww->exitProgram();
			return 0;
		}
	}

	// the loading of ressources is done in a seperate thread
	ww->setTexturesPath(strTexturePath);
	d3dFont2D.loadFontFile(ww, strFontFilename);
	textureLoading.loadFile(ww, strLoadingFilename);
	textureBackground.loadFile(ww, strBackgroundFilename);
	ww->setMinimumWindowSize(minimumWindowSize.x, minimumWindowSize.y);
	ww->setDummyWhiteTexture(strWhiteDummyFilename);
	if (loadingScreenFractions.size()) ww->mainLoadingScreen.setMeasuredFractions(loadingScreenFractions);
	ww->mainLoadingScreen.setTextures(&textureBackground, &textureLoading, &d3dFont2D);
	ww->mainLoadingScreen.show(mainInitFunc, NULL);
	ww->screenInfo.showFramesPerSecond(expertMode, &d3dFont2D);

	// Main message loop:
	ww->goIntoMainLoop();

	// release
	saveDefaultSettings();
	delete miniMaxCalcDb;
	delete miniMaxInspectDb;
	delete playerPerfect;
	delete playerMinMax;
	delete playerRandom;
	delete myGame;
	delete ww;

	return 0;
}

//-----------------------------------------------------------------------------
// Name: loadDefaultSettings()
// Desc: 
//-----------------------------------------------------------------------------
void loadDefaultSettings(void)
{
	// locals
	xmlNode	*	rootNode	= nullptr;
	xmlClass	xml(string("settings.xml"), rootNode);

	// file loaded?
	if (rootNode != nullptr) {

		// <database directory="">
		strDatabaseDir		= rootNode->node("database")->attribute("directory")->valueW();

		// <expertMode enabled="true">
		expertMode			= (bool) (rootNode->node("expertMode")->attribute("enabled")->valueW().compare(L"true") == 0);

		// <firstRun showDisclaimer="true">
		showDisclaimerOnStart	= (bool) !(rootNode->node("firstRun")->attribute("showDisclaimer")->valueW().compare(L"false") == 0);

		// <windowSize x="800" y="600"></>
		defaultWindowSize.x = (int) atoi(rootNode->node("windowSize")->attribute("x")->valueA().c_str());
		defaultWindowSize.y = (int) atoi(rootNode->node("windowSize")->attribute("y")->valueA().c_str());

		// loading screen progress fractions
		if (rootNode->node("loadingScreenProgress")->attribute("n")->exists()) {
			loadingScreenFractions.resize((int) atoi(rootNode->node("loadingScreenProgress")->attribute("n")->valueA().c_str()));
			wstringstream wss		(rootNode->node("loadingScreenProgress")->valueW());
			for (auto& curFrac : loadingScreenFractions) {
				wss >> curFrac;
				if (wss.peek() == L';') wss.ignore();
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Name: saveDefaultSettings()
// Desc: 
//-----------------------------------------------------------------------------
void saveDefaultSettings(void)
{
	// locals
	xmlClass		xml;
	xmlNode	*		rootNode		= xml.getRootNode();
	wstring			strRelative;	

	// <database directory="">
	ww->makeRelativePathToExe(strDatabaseDir, strRelative);
	rootNode->addSubNodeWithAttribute(L"database", L"directory", strRelative.c_str());

	// <expertMode enabled="true">
	rootNode->addSubNodeWithAttribute(L"expertMode", L"enabled", expertMode ? L"true" : L"false");
	
	// <firstRun showDisclaimer="true">
	rootNode->addSubNodeWithAttribute(L"firstRun", L"showDisclaimer", L"false");
	
	// <windowSize x="800" y="600"></>
	rootNode->addSubNodeWithAttribute(L"windowSize", L"x", ww->getWindowSizeX());
	rootNode->addSubNodeWithAttribute(L"windowSize", L"y", ww->getWindowSizeY());

	// <loadingScreenProgress n="999">0.1;0.2;1.0;</> 			 (loading screen progress fractions)
	vector<float> fractions;
	wstringstream wss;
	ww->mainLoadingScreen.getMeasuredFractions(fractions);
	for (auto& curFrac : fractions) {
		wss << setprecision(2) << curFrac << L";";
	}
	rootNode->addSubNodeWithAttribute(L"loadingScreenProgress", L"n", (int) fractions.size());
	rootNode->node(L"loadingScreenProgress")->setValue(wss.str().c_str());

	// write to file
	xml.writeFile(string("settings.xml"));
}

//-----------------------------------------------------------------------------
// Name: mainInitFunc()
// Desc: 
//-----------------------------------------------------------------------------
void mainInitFunc(void* pUser)
{
	// locals
	unsigned int pushFrom, pushTo;
	
	// default ww stuff
	ww->mainLoadingScreen.setCompletionFraction(0);
	ww->mainLoadingScreen.progress();
	ww->setBackground(strBackgroundFilename);

	// Initialize global strings
	textureBackground		.loadFile(ww, wstring(L"background.jpg"			));
	textureField			.loadFile(ww, wstring(L"field.png"				));
	textureLine				.loadFile(ww, wstring(L"line.png"				), false);
	textureCorner			.loadFile(ww, wstring(L"corner.png"				), false);
	
	// initialize mühle-classes
	myGame			= new muehle();													ww->mainLoadingScreen.progress();
	playerPerfect	= new perfectKI(mystring(strDatabaseDir.c_str()).c_strA());		ww->mainLoadingScreen.progress();
	playerMinMax	= new minMaxKI();												ww->mainLoadingScreen.progress();
	playerRandom	= new randomKI();												ww->mainLoadingScreen.progress();
	playerHuman		= nullptr;

	// begin a game by default
	playerPerfect->setDatabasePath(mystring(strDatabaseDir.c_str()).c_strA());		ww->mainLoadingScreen.progress();
	myGame->beginNewGame(playerOne = playerHuman, playerTwo = playerHuman, 0);		ww->mainLoadingScreen.progress();
	myGame->getChoiceOfSpecialKI(playerPerfect, &pushFrom, &pushTo);				ww->mainLoadingScreen.progress();

	CheckMenuItem(ww->getHMenu(), ID_PLAYER1_HUMAN,		    MF_CHECKED);
	CheckMenuItem(ww->getHMenu(), ID_PLAYER2_HUMAN,			MF_CHECKED);
	CheckMenuItem(ww->getHMenu(), ID_MOVESPEED_500MS,	    MF_CHECKED);
	CheckMenuItem(ww->getHMenu(), ID_GAME_SHOWPERFECTMOVE,	MF_UNCHECKED);
	CheckMenuItem(ww->getHMenu(), ID_GAME_SHOWSTATENUMBER,	MF_UNCHECKED);

	modeInspectDb.amAreaInspectDb.create(ww->alignmentRootFrame);					ww->mainLoadingScreen.progress();
	modeCalcDb.amAreaCalculation .create(ww->alignmentRootFrame);					ww->mainLoadingScreen.progress();

	miniMaxInspectDb= new miniMaxWinInspectDb(ww, playerPerfect, modeInspectDb.amAreaInspectDb, &d3dFont2D, &textureLine, guiField);	ww->mainLoadingScreen.progress();
	miniMaxCalcDb	= new miniMaxWinCalcDb   (ww, playerPerfect, modeCalcDb.amAreaCalculation , &d3dFont2D, &textureLine);				ww->mainLoadingScreen.progress();

	modePlayGame	.init();														ww->mainLoadingScreen.progress();
	modeInspectDb	.init();														ww->mainLoadingScreen.progress();
	modeCalcDb		.init();														ww->mainLoadingScreen.progress();
	modeSetupField	.init();														ww->mainLoadingScreen.progress();
	ww->scenarioManager.setActiveScenario(modePlayGame);

	// upload resources
	ww->graphicManager.performResourceUpload();

	// show render rect
	ww->mainLoadingScreen.setCompletionFraction(3);
}

//-----------------------------------------------------------------------------
// Name: WndProc()
// Desc: Processes messages for the main window.
//-----------------------------------------------------------------------------
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int				wmId, wmEvent;

	switch (message)
	{
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(ww->getHinst(), MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_PRIVACY:		ww->showMessageBox(L"Privacy", L"This application does not store nor send personal data.", MB_OK);			break;		
		case IDM_LICENSE:		ww->showMessageBox(L"License", L"\
                               The MIT License (MIT)\n\
\n\
Copyright (c) 2019 Thomas Weber\n\
\n\
Permission is hereby granted, free of charge, to any person obtaining a copy of this\
software and associated documentation files (the 'Software'), to deal in the Software\
without restriction, including without limitation the rights to use, copy, modify,\
merge, publish, distribute, sublicense, and/or sell copies of the Software, and to\
permit persons to whom the Software is furnished to do so, subject to the following\
conditions:\n\
\n\
The above copyright notice and this permission notice shall be included in all copies\
or substantial portions of the Software.\n\
\n\
THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,\
INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A\
PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT\
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF\
CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE\
OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.", MB_OK); break;
		case IDM_EXIT:
			ww->exitProgram();
			break;
        case ID_GAME_SETFIELD:
			ww->scenarioManager.setActiveScenario(modeSetupField);
            break;
		case ID_GAME_UNDOMOVE:
			modePlayGame.undoMove();
			break;
		case ID_GAME_NEWGAME:
			ww->scenarioManager.setActiveScenario(modePlayGame);
			modePlayGame.gameState = modePlayGameClass::newGameYetBlank;
			break;
		case IDM_CALCULATE_DATABASE:
			ww->scenarioManager.setActiveScenario(modeCalcDb);
			break;
		case IDM_INSPECT_DATABASE:
			ww->scenarioManager.setActiveScenario(modeInspectDb);
			break;
		case ID_GAME_SHOWSTATENUMBER:
			guiField.showStateNumber(!guiField.isShowingStateNumber());
            CheckMenuItem(ww->getHMenu(), ID_GAME_SHOWSTATENUMBER, guiField.isShowingStateNumber() ? MF_CHECKED : MF_UNCHECKED);
            break;
        case ID_GAME_SHOWPERFECTMOVE:
            guiField.showPerfectMove(!guiField.isShowingPerfectMove());
            CheckMenuItem(ww->getHMenu(), ID_GAME_SHOWPERFECTMOVE, guiField.isShowingPerfectMove() ? MF_CHECKED : MF_UNCHECKED);
            break;
		case ID_PLAYER1_PERFECTDATABASE:			modePlayGame.changeBot(wmId, fieldStruct::playerOne, playerPerfect);									break;
		case ID_PLAYER1_RANDOMBOT:					modePlayGame.changeBot(wmId, fieldStruct::playerOne, playerRandom);										break;
		case ID_PLAYER1_HUMAN:						modePlayGame.changeBot(wmId, fieldStruct::playerOne, playerHuman );										break;
		case ID_MINIMAXALGORITHMN_AUTOMATIC:		modePlayGame.changeBot(wmId, fieldStruct::playerOne, playerMinMax);	playerMinMax->setSearchDepth(0);	break;
		case ID_MINIMAXALGORITHMN_SEARCHDEPTH1:		modePlayGame.changeBot(wmId, fieldStruct::playerOne, playerMinMax);	playerMinMax->setSearchDepth(1);	break;
		case ID_MINIMAXALGORITHMN_SEARCHDEPTH2:		modePlayGame.changeBot(wmId, fieldStruct::playerOne, playerMinMax);	playerMinMax->setSearchDepth(2);	break;
		case ID_MINIMAXALGORITHMN_SEARCHDEPTH3:		modePlayGame.changeBot(wmId, fieldStruct::playerOne, playerMinMax);	playerMinMax->setSearchDepth(3);	break;
		case ID_MINIMAXALGORITHMN_SEARCHDEPTH4:		modePlayGame.changeBot(wmId, fieldStruct::playerOne, playerMinMax);	playerMinMax->setSearchDepth(4);	break;
		case ID_MINIMAXALGORITHMN_SEARCHDEPTH5:		modePlayGame.changeBot(wmId, fieldStruct::playerOne, playerMinMax);	playerMinMax->setSearchDepth(5);	break;
		case ID_MINIMAXALGORITHMN_SEARCHDEPTH6:		modePlayGame.changeBot(wmId, fieldStruct::playerOne, playerMinMax);	playerMinMax->setSearchDepth(6);	break;
		case ID_MINIMAXALGORITHMN_SEARCHDEPTH7:		modePlayGame.changeBot(wmId, fieldStruct::playerOne, playerMinMax);	playerMinMax->setSearchDepth(7);	break;
		case ID_MINIMAXALGORITHMN_SEARCHDEPTH8:		modePlayGame.changeBot(wmId, fieldStruct::playerOne, playerMinMax);	playerMinMax->setSearchDepth(8);	break;
		case ID_MINIMAXALGORITHMN_SEARCHDEPTH9:		modePlayGame.changeBot(wmId, fieldStruct::playerOne, playerMinMax);	playerMinMax->setSearchDepth(9);	break;
		case ID_MINIMAXALGORITHMN_SEARCHDEPTH10:	modePlayGame.changeBot(wmId, fieldStruct::playerOne, playerMinMax);	playerMinMax->setSearchDepth(10);	break;
		case ID_MINIMAXALGORITHMN_SEARCHDEPTH11:	modePlayGame.changeBot(wmId, fieldStruct::playerOne, playerMinMax);	playerMinMax->setSearchDepth(11);	break;
		case ID_PLAYER2_PERFECTDATABASE:			modePlayGame.changeBot(wmId, fieldStruct::playerTwo, playerPerfect);									break;
		case ID_PLAYER2_RANDOMBOT:					modePlayGame.changeBot(wmId, fieldStruct::playerTwo, playerRandom);										break;
		case ID_PLAYER2_HUMAN: 						modePlayGame.changeBot(wmId, fieldStruct::playerTwo, playerHuman );										break;
		case ID_MINIMAXALGORITHMN_2_AUTOMATIC:		modePlayGame.changeBot(wmId, fieldStruct::playerTwo, playerMinMax);	playerMinMax->setSearchDepth(0);	break;
		case ID_MINIMAXALGORITHMN_2_SEARCHDEPTH1:	modePlayGame.changeBot(wmId, fieldStruct::playerTwo, playerMinMax);	playerMinMax->setSearchDepth(1);	break;
		case ID_MINIMAXALGORITHMN_2_SEARCHDEPTH2:	modePlayGame.changeBot(wmId, fieldStruct::playerTwo, playerMinMax);	playerMinMax->setSearchDepth(2);	break;
		case ID_MINIMAXALGORITHMN_2_SEARCHDEPTH3:	modePlayGame.changeBot(wmId, fieldStruct::playerTwo, playerMinMax);	playerMinMax->setSearchDepth(3);	break;
		case ID_MINIMAXALGORITHMN_2_SEARCHDEPTH4:	modePlayGame.changeBot(wmId, fieldStruct::playerTwo, playerMinMax);	playerMinMax->setSearchDepth(4);	break;
		case ID_MINIMAXALGORITHMN_2_SEARCHDEPTH5:	modePlayGame.changeBot(wmId, fieldStruct::playerTwo, playerMinMax);	playerMinMax->setSearchDepth(5);	break;
		case ID_MINIMAXALGORITHMN_2_SEARCHDEPTH6:	modePlayGame.changeBot(wmId, fieldStruct::playerTwo, playerMinMax);	playerMinMax->setSearchDepth(6);	break;
		case ID_MINIMAXALGORITHMN_2_SEARCHDEPTH7:	modePlayGame.changeBot(wmId, fieldStruct::playerTwo, playerMinMax);	playerMinMax->setSearchDepth(7);	break;
		case ID_MINIMAXALGORITHMN_2_SEARCHDEPTH8:	modePlayGame.changeBot(wmId, fieldStruct::playerTwo, playerMinMax);	playerMinMax->setSearchDepth(8);	break;
		case ID_MINIMAXALGORITHMN_2_SEARCHDEPTH9:	modePlayGame.changeBot(wmId, fieldStruct::playerTwo, playerMinMax);	playerMinMax->setSearchDepth(9);	break;
		case ID_MINIMAXALGORITHMN_2_SEARCHDEPTH10:	modePlayGame.changeBot(wmId, fieldStruct::playerTwo, playerMinMax);	playerMinMax->setSearchDepth(10);	break;
		case ID_MINIMAXALGORITHMN_2_SEARCHDEPTH11:	modePlayGame.changeBot(wmId, fieldStruct::playerTwo, playerMinMax);	playerMinMax->setSearchDepth(11);	break;
		case ID_MOVESPEED_100MS:					modePlayGame.changeMoveSpeed(wmId, 100);																break;
		case ID_MOVESPEED_500MS:					modePlayGame.changeMoveSpeed(wmId, 500);																break;
		case ID_MOVESPEED_1S:						modePlayGame.changeMoveSpeed(wmId, 1000);																break;
		case ID_MOVESPEED_3S:						modePlayGame.changeMoveSpeed(wmId, 3000);																break;
		default:									return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	}
	return 0;
}

//-----------------------------------------------------------------------------
// Name: WndProc()
// Desc: Message handler for about box.
//-----------------------------------------------------------------------------
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}
#pragma endregion


/*** modePlayGameClass **************************************************************************************************************************************************************************/

#pragma region modePlayGameClass

//-----------------------------------------------------------------------------
// Name: createControls()
// Desc: 
//-----------------------------------------------------------------------------
void modePlayGameClass::init()
{
	alignmentGameField.create(ww->alignmentRootFrame);
	guiField.init(ww, *myGame, *playerPerfect, d3dFont2D, alignmentGameField, textureField);
	botTimer.start(ww, botTimerFunc, this, botTimerTime);
}

//-----------------------------------------------------------------------------
// Name: activateMode()
// Desc: 
//-----------------------------------------------------------------------------
void modePlayGameClass::activate()
{
	// locals
    unsigned int myPushFrom, myPushTo;

	gameState = modePlayGameClass::newGameYetBlank;
	if (modeSetupField.settingColor == fieldStruct::playerBlack || modeSetupField.settingColor == fieldStruct::playerWhite) {
		modeSetupField.settingColor = 0;
	} else {
		myGame->beginNewGame(playerOne, playerTwo, 0);
	}
	myGame->getChoiceOfSpecialKI(playerPerfect, &myPushFrom, &myPushTo);

	EnableMenuItem(ww->getHMenu(), ID_GAME_SHOWPERFECTMOVE, MF_ENABLED);
	EnableMenuItem(ww->getHMenu(), ID_GAME_SHOWSTATENUMBER, MF_ENABLED);
	EnableMenuItem(ww->getHMenu(), ID_GAME_UNDOMOVE,		MF_ENABLED);

	guiField.setVisibility(true);
	guiField.setAnimateStoneOnMove(false);
	guiField.setField(modeSetupField.settingColor, true);
	guiField.setFieldPosClickedFunc(bind(&modePlayGameClass::fieldPosClicked, this, placeholders::_1));
	guiField.setAlignment(modePlayGame.alignmentGameField);
	guiField.setAnimateStoneOnMove(true);

	processMove();
}

//-----------------------------------------------------------------------------
// Name: deactivateMode()
// Desc: 
//-----------------------------------------------------------------------------
void modePlayGameClass::deactivate()
{
	// Abfrage ob aktuelles Spiel abgebrochen werden soll
	if (gameState == playingGame) {
		if (ww->showMessageBox(L"CANCEL", L"Do you really want to stop the current game?", MB_YESNO) == IDNO) return;
	}
	
	gameState	= 0;
	
	EnableMenuItem(ww->getHMenu(), ID_GAME_SHOWPERFECTMOVE, MF_GRAYED);
	EnableMenuItem(ww->getHMenu(), ID_GAME_SHOWSTATENUMBER, MF_GRAYED);
	EnableMenuItem(ww->getHMenu(), ID_GAME_UNDOMOVE,		MF_GRAYED);

	guiField.setFieldPosClickedFunc(nullptr);
	guiField.setVisibility(false);
}

//-----------------------------------------------------------------------------
// Name: changeMoveSpeed()
// Desc: 
//-----------------------------------------------------------------------------
void modePlayGameClass::changeMoveSpeed(UINT wmId, DWORD duration)
{
	// uncheck all menus
	CheckMenuItem(ww->getHMenu(), ID_MOVESPEED_100MS,	MF_UNCHECKED);
	CheckMenuItem(ww->getHMenu(), ID_MOVESPEED_500MS,	MF_UNCHECKED);
	CheckMenuItem(ww->getHMenu(), ID_MOVESPEED_1S,		MF_UNCHECKED);
	CheckMenuItem(ww->getHMenu(), ID_MOVESPEED_3S,		MF_UNCHECKED);
	
	// check the selected one
	CheckMenuItem(ww->getHMenu(), wmId, MF_CHECKED);	
	botTimerTime = duration;
}

//-----------------------------------------------------------------------------
// Name: changeBot()
// Desc: 
//-----------------------------------------------------------------------------
void modePlayGameClass::changeBot(UINT wmId, int player, muehleKI *bot)
{
	// kill timer
	botTimer.terminate();

	// set new bot
	myGame->setKI(player, bot);

	// uncheck all menus
	if (player == fieldStruct::playerOne) {
		playerOne = bot;
		unCheckAllPlayerOne();	
	} else {
		playerTwo = bot;
		unCheckAllPlayerTwo();	
	}

	// check new selected bot
	CheckMenuItem(ww->getHMenu(), wmId, MF_CHECKED);		

	// disable undo button when both players are computers
	EnableMenuItem(ww->getHMenu(), ID_GAME_UNDOMOVE, (myGame->isCurrentPlayerHuman() || myGame->isOpponentPlayerHuman()) ? MF_ENABLED : MF_GRAYED);

	// set timer if bot has to move
	if (!isCurrentPlayerHuman() && !modeSetupField.isActive()) botTimer.start(ww, botTimerFunc, this, botTimerTime);
}

//-----------------------------------------------------------------------------
// Name: unCheckAllPlayerOne()
// Desc: 
//-----------------------------------------------------------------------------
void modePlayGameClass::unCheckAllPlayerOne()
{
	CheckMenuItem(ww->getHMenu(), ID_MINIMAXALGORITHMN_SEARCHDEPTH1,	MF_UNCHECKED);
	CheckMenuItem(ww->getHMenu(), ID_MINIMAXALGORITHMN_SEARCHDEPTH2,	MF_UNCHECKED);
	CheckMenuItem(ww->getHMenu(), ID_MINIMAXALGORITHMN_SEARCHDEPTH3,	MF_UNCHECKED);
	CheckMenuItem(ww->getHMenu(), ID_MINIMAXALGORITHMN_SEARCHDEPTH4,	MF_UNCHECKED);
	CheckMenuItem(ww->getHMenu(), ID_MINIMAXALGORITHMN_SEARCHDEPTH5,	MF_UNCHECKED);
	CheckMenuItem(ww->getHMenu(), ID_MINIMAXALGORITHMN_SEARCHDEPTH6,	MF_UNCHECKED);
	CheckMenuItem(ww->getHMenu(), ID_MINIMAXALGORITHMN_SEARCHDEPTH7,	MF_UNCHECKED);
	CheckMenuItem(ww->getHMenu(), ID_MINIMAXALGORITHMN_SEARCHDEPTH8,	MF_UNCHECKED);
	CheckMenuItem(ww->getHMenu(), ID_MINIMAXALGORITHMN_SEARCHDEPTH9,	MF_UNCHECKED);
	CheckMenuItem(ww->getHMenu(), ID_MINIMAXALGORITHMN_SEARCHDEPTH10,	MF_UNCHECKED);
	CheckMenuItem(ww->getHMenu(), ID_MINIMAXALGORITHMN_SEARCHDEPTH11,	MF_UNCHECKED);
	CheckMenuItem(ww->getHMenu(), ID_MINIMAXALGORITHMN_AUTOMATIC,		MF_UNCHECKED);
	CheckMenuItem(ww->getHMenu(), ID_PLAYER1_HUMAN,						MF_UNCHECKED);
	CheckMenuItem(ww->getHMenu(), ID_PLAYER1_PERFECTDATABASE,			MF_UNCHECKED);
	CheckMenuItem(ww->getHMenu(), ID_PLAYER1_RANDOMBOT,					MF_UNCHECKED);
}

//-----------------------------------------------------------------------------
// Name: unCheckAllPlayerTwo()
// Desc: 
//-----------------------------------------------------------------------------
void modePlayGameClass::unCheckAllPlayerTwo()
{
	CheckMenuItem(ww->getHMenu(), ID_MINIMAXALGORITHMN_2_SEARCHDEPTH1,		MF_UNCHECKED);
	CheckMenuItem(ww->getHMenu(), ID_MINIMAXALGORITHMN_2_SEARCHDEPTH2,		MF_UNCHECKED);
	CheckMenuItem(ww->getHMenu(), ID_MINIMAXALGORITHMN_2_SEARCHDEPTH3,		MF_UNCHECKED);
	CheckMenuItem(ww->getHMenu(), ID_MINIMAXALGORITHMN_2_SEARCHDEPTH4,		MF_UNCHECKED);
	CheckMenuItem(ww->getHMenu(), ID_MINIMAXALGORITHMN_2_SEARCHDEPTH5,		MF_UNCHECKED);
	CheckMenuItem(ww->getHMenu(), ID_MINIMAXALGORITHMN_2_SEARCHDEPTH6,		MF_UNCHECKED);
	CheckMenuItem(ww->getHMenu(), ID_MINIMAXALGORITHMN_2_SEARCHDEPTH7,		MF_UNCHECKED);
	CheckMenuItem(ww->getHMenu(), ID_MINIMAXALGORITHMN_2_SEARCHDEPTH8,		MF_UNCHECKED);
	CheckMenuItem(ww->getHMenu(), ID_MINIMAXALGORITHMN_2_SEARCHDEPTH9,		MF_UNCHECKED);
	CheckMenuItem(ww->getHMenu(), ID_MINIMAXALGORITHMN_2_SEARCHDEPTH10,		MF_UNCHECKED);
	CheckMenuItem(ww->getHMenu(), ID_MINIMAXALGORITHMN_2_SEARCHDEPTH11,		MF_UNCHECKED);
	CheckMenuItem(ww->getHMenu(), ID_MINIMAXALGORITHMN_2_AUTOMATIC,			MF_UNCHECKED);
	CheckMenuItem(ww->getHMenu(), ID_PLAYER2_HUMAN,							MF_UNCHECKED);
	CheckMenuItem(ww->getHMenu(), ID_PLAYER2_PERFECTDATABASE,				MF_UNCHECKED);
	CheckMenuItem(ww->getHMenu(), ID_PLAYER2_RANDOMBOT,						MF_UNCHECKED);
}

//-----------------------------------------------------------------------------
// Name: botTimerFunc()
// Desc: 
//-----------------------------------------------------------------------------
void modePlayGameClass::botTimerFunc(void* pUser)
{
	modePlayGameClass* pgc = (modePlayGameClass*) pUser;
	pgc->letComputerPlay();
}

//-----------------------------------------------------------------------------
// Name: fieldPosClicked()
// Desc: 
//-----------------------------------------------------------------------------
void modePlayGameClass::fieldPosClicked(unsigned int clickOnFieldPos)
{
	// when current player is not human or game has finished than do nothing
	if (!modePlayGame.isCurrentPlayerHuman() || myGame->getWinner()) return;

	// select stone to move
	if (myGame->mustStoneBeRemoved() || myGame->inSettingPhase()) {
		pushFrom	= clickOnFieldPos;
		pushTo		= clickOnFieldPos;
	} else {
		// destination choosen
		if (pushFrom < fieldStruct::size) {
			pushTo   = clickOnFieldPos;
		// a stone has been selected
		} else {							
			pushFrom = clickOnFieldPos;
			guiField.setGameStatusText(L"Choose a destination!");
			guiField.activateStonesOfCurrentPlayer(pushFrom, modeSetupField.settingColor, modePlayGame.pushFrom);
		}
	}
    				
	// move
	if (pushFrom < fieldStruct::size && pushTo < fieldStruct::size) {
		processMove();
	}
}

//-----------------------------------------------------------------------------
// Name: letComputerPlay()
// Desc: 
//-----------------------------------------------------------------------------
void modePlayGameClass::letComputerPlay()
{
	botTimer.terminate();

	// pre conditions correct?
	if (modeInspectDb	.isActive())							return;
	if (modeCalcDb		.isActive())							return;
    if (modeSetupField	.isActive())							return;
	if (isCurrentPlayerHuman() || myGame->getWinner())			return;

	// get move from AI
	myGame->getComputersChoice(&pushFrom, &pushTo);

	// move
	processMove();
}

//-----------------------------------------------------------------------------
// Name: letComputerPlay()
// Desc: 
//-----------------------------------------------------------------------------
void modePlayGameClass::processMove()
{
	// remember game state so that gui can show it
	bool			stoneMustBeRemoved	= myGame->mustStoneBeRemoved();
	bool			inSettingPhase		= myGame->inSettingPhase();
	unsigned int	numberOfStoneSet	= myGame->getNumStonesSet();
	int				currentPlayer		= myGame->getCurrentPlayer();

	// try to do move
	if (myGame->moveStone(pushFrom, pushTo)) {

		// function getChoiceOfSpecialKI() must be called so that playerPerfect.getValueOfMoves() considers the current field state. 
		// ... should be solved better
		if (myGame->getWinner() == 0) {
			unsigned int a, b;
		 	myGame->getChoiceOfSpecialKI(playerPerfect, &a, &b);
		 }

		// if move is feasible then show it
		gameState = modePlayGameClass::playingGame;
		guiField.moveStone(pushFrom, pushTo, stoneMustBeRemoved, inSettingPhase, numberOfStoneSet, currentPlayer);
		
		// ... may be a call back can be used, when stone animation has finished
		// guiField.deactivateAllStones();
		// ... function should be resumed her
	}

	pushFrom	= fieldStruct::size;
	pushTo   	= fieldStruct::size;

	// game finished ?
	if (myGame->getWinner() != 0) {
		guiField.deactivateAllStones();
		guiField.setGameStatusText(L"Game has hinished!");
		// ... guiField.labelGameState.setState(wildWeasel::guiElemState::HIDDEN);
		EnableMenuItem(ww->getHMenu(), ID_GAME_UNDOMOVE, MF_GRAYED);
		showWinnerAnimation();
    		
	// current player human ?
	} else if (isCurrentPlayerHuman()) {

		if (myGame->mustStoneBeRemoved()) {
			guiField.setGameStatusText(L"Remove a stone!");
		} else if (myGame->inSettingPhase()) {
			guiField.setGameStatusText(L"Set a stone!");
		} else {
			guiField.setGameStatusText(L"Select a stone!");
		}
		guiField.activateStonesOfCurrentPlayer(fieldStruct::size, modeSetupField.settingColor, modePlayGame.pushFrom);

	// current player a bot ?
	} else {
		guiField.activateStonesOfCurrentPlayer(fieldStruct::size, modeSetupField.settingColor, modePlayGame.pushFrom);
		guiField.deactivateAllStones();
		guiField.setGameStatusText(L"Bot is thinking.");
		botTimer.start(ww, botTimerFunc, this, botTimerTime + (unsigned int) (guiField.getMoveAnimationDuration() * 1000));
	}
}

//-----------------------------------------------------------------------------
// Name: isCurrentPlayerHuman()
// Desc: 
//-----------------------------------------------------------------------------
bool modePlayGameClass::isCurrentPlayerHuman() 
{
	MENUITEMINFO mii;

	mii.cbSize	= sizeof(MENUITEMINFO);
	mii.fMask	= MIIM_STATE;

	// player with black ?
	if (myGame->getCurrentPlayer() == fieldStruct::playerOne) {
		GetMenuItemInfo(ww->getHMenu(), ID_PLAYER1_HUMAN, false, &mii);
		return (mii.fState == MFS_CHECKED);
	} else {
		GetMenuItemInfo(ww->getHMenu(), ID_PLAYER2_HUMAN, false, &mii);
		return (mii.fState == MFS_CHECKED);
	}
}

//-----------------------------------------------------------------------------
// Name: showWinnerAnimation()
// Desc: 
//-----------------------------------------------------------------------------
void modePlayGameClass::showWinnerAnimation()
{
	if (myGame->getWinner() == fieldStruct::playerOne) {
		ww->showMessageBox(L"Game Finished", L"Black player has WON!", MB_OK);
	} else if (myGame->getWinner() == fieldStruct::playerTwo) {
		ww->showMessageBox(L"Game Finished", L"White player has WON!", MB_OK);
	}
}

//-----------------------------------------------------------------------------
// Name: undoMove()
// Desc: 
//-----------------------------------------------------------------------------
void modePlayGameClass::undoMove()
{
	if (myGame->getWinner() == 0 && !modeSetupField.isActive() && isCurrentPlayerHuman()) {
		unsigned int	myPushFrom, myPushTo;
		myGame->undoLastMove();
		if (!isCurrentPlayerHuman()) myGame->undoLastMove();
		myGame->getChoiceOfSpecialKI(playerPerfect, &myPushFrom, &myPushTo);
		guiField.setField(modeSetupField.settingColor, true);
	}
}

#pragma endregion

/*** modeSetupFieldClass **************************************************************************************************************************************************************************/

#pragma region modeSetupFieldClass
//-----------------------------------------------------------------------------
// Name: createControls()
// Desc: 
//-----------------------------------------------------------------------------
void modeSetupFieldClass::init()
{
}

//-----------------------------------------------------------------------------
// Name: activateMode()
// Desc: 
//-----------------------------------------------------------------------------
void modeSetupFieldClass::activate()
{
	// Show message box asking if game is in setting phase or not
    int  ret = MessageBoxW(ww->getHwnd(), L"Black player will be on turn.\nFirst place black stones with left mouse button,\nthen click right and place the white stones. Terminate again with the right mouse button.\n\n Game in setting phase?", L"set up an arbitrary game state", MB_YESNOCANCEL);
    bool settingPhase;

    switch (ret)
    {
    case IDYES:     settingPhase = true;    break;
    case IDNO:      settingPhase = false;   break;
    case IDCANCEL:  return;
    default:        return;
    }

    settingColor = fieldStruct::playerBlack;
    myGame->startSettingPhase(playerOne, playerTwo, fieldStruct::playerBlack, settingPhase);
	guiField.showPerfectMove(false);
	guiField.showStateNumber(false);
	guiField.setAnimateStoneOnMove(false);
	guiField.setFieldPosClickedFunc(bind(&modeSetupFieldClass::fieldPosClicked, this, placeholders::_1));
	guiField.setVisibility(true);
	guiField.setField(modeSetupField.settingColor, true);
	guiField.setGameStatusText(L"Set a stone!");
	guiField.setAlignment(modePlayGame.alignmentGameField);
	followEvent(this, wildWeasel::eventFollower::eventType::RIGHT_MOUSEBUTTON_PRESSED);
}

//-----------------------------------------------------------------------------
// Name: deactivateMode()
// Desc: 
//-----------------------------------------------------------------------------
void modeSetupFieldClass::deactivate()
{
	guiField.setFieldPosClickedFunc(nullptr);
	guiField.setVisibility(false);
	guiField.showPerfectMove(GetMenuState(ww->getHMenu(), ID_GAME_SHOWPERFECTMOVE, MF_BYCOMMAND) & MF_CHECKED);
	guiField.showStateNumber(GetMenuState(ww->getHMenu(), ID_GAME_SHOWSTATENUMBER, MF_BYCOMMAND) & MF_CHECKED);
	guiField.setAnimateStoneOnMove(true);
	forgetEvent(this, wildWeasel::eventFollower::eventType::RIGHT_MOUSEBUTTON_PRESSED);
}

//-----------------------------------------------------------------------------
// Name: fieldPosClicked()
// Desc: 
//-----------------------------------------------------------------------------
void modeSetupFieldClass::fieldPosClicked(unsigned int clickOnFieldPos)
{
    if (settingColor == fieldStruct::playerWhite) {
        myGame->putStone(clickOnFieldPos, fieldStruct::playerWhite);
    } else if (settingColor == fieldStruct::playerBlack) {
        myGame->putStone(clickOnFieldPos, fieldStruct::playerBlack);
    }

	guiField.setField(modeSetupField.settingColor, true);
	guiField.setGameStatusText(L"Set a stone!");
	
	if (myGame->inSettingPhase()) {
		int numWhiteStonesResting;
		int numBlackStonesResting;
		myGame->calcNumberOfRestingStones(numWhiteStonesResting, numBlackStonesResting);
		if ((settingColor == fieldStruct::playerBlack && numBlackStonesResting == 0) || (settingColor == fieldStruct::playerWhite && numWhiteStonesResting == 0)) {
			rightMouseButtonPressed(0, 0);
		}
	} else {
		if (settingColor == fieldStruct::playerBlack && myGame->getCurrentPlayer() == fieldStruct::playerBlack && myGame->getNumStonOfCurPlayer() == fieldStruct::numStonesPerPlayer) {
			rightMouseButtonPressed(0, 0);
		} else if (settingColor == fieldStruct::playerWhite && myGame->getCurrentPlayer() == fieldStruct::playerBlack && myGame->getNumStonOfOppPlayer() == fieldStruct::numStonesPerPlayer) {
			rightMouseButtonPressed(0, 0);
		}
	}
}

//-----------------------------------------------------------------------------
// Name: rightClicked()
// Desc: 
//-----------------------------------------------------------------------------
void modeSetupFieldClass::rightMouseButtonPressed(int xPos, int yPos)
{
    // setting up field
    if (settingColor == fieldStruct::playerWhite) {
        myGame->settingPhaseHasFinished();
		ww->scenarioManager.setActiveScenario(modePlayGame);
	// change stone color
	} else if (settingColor == fieldStruct::playerBlack) {
        settingColor = fieldStruct::playerWhite;
		guiField.setField(modeSetupField.settingColor, true);
    }
}

#pragma endregion

/*** modeInspectDbClass **************************************************************************************************************************************************************************/

#pragma region modeInspectDbClass
//-----------------------------------------------------------------------------
// Name: createControls()
// Desc: 
//-----------------------------------------------------------------------------
void modeInspectDbClass::init()
{
	miniMaxInspectDb->createControls();
}

//-----------------------------------------------------------------------------
// Name: activateMode()
// Desc: 
//-----------------------------------------------------------------------------
void modeInspectDbClass::activate()
{
	miniMaxInspectDb->showControls(true);
}

//-----------------------------------------------------------------------------
// Name: deactivateMode()
// Desc: 
//-----------------------------------------------------------------------------
void modeInspectDbClass::deactivate()
{
	miniMaxInspectDb->showControls(false);
}

#pragma endregion

/*** modeCalcDbClass **************************************************************************************************************************************************************************/

#pragma region modeCalcDbClass
//-----------------------------------------------------------------------------
// Name: createControls()
// Desc: 
//-----------------------------------------------------------------------------
void modeCalcDbClass::init()
{
	miniMaxCalcDb->createControls();
}

//-----------------------------------------------------------------------------
// Name: activateMode()
// Desc: 
//-----------------------------------------------------------------------------
void modeCalcDbClass::activate()
{
	miniMaxCalcDb->showControls(true);
}

//-----------------------------------------------------------------------------
// Name: deactivateMode()
// Desc: 
//-----------------------------------------------------------------------------
void modeCalcDbClass::deactivate()
{
	// Abfrage ob berechnung abgebrochen werden  soll
	if (isCalculationOngoing()) {
		if (ww->showMessageBox(L"CANCEL", L"Do you really want to cancel the database calculation?", MB_YESNO) == IDNO) return;
	}
	miniMaxCalcDb->showControls(false);
}

//-----------------------------------------------------------------------------
// Name: isCalculationOngoing()
// Desc: 
//-----------------------------------------------------------------------------
bool modeCalcDbClass::isCalculationOngoing()
{
	return miniMaxCalcDb->isCalculationOngoing();
}
#pragma endregion