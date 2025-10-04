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

	mw = new MuehleWin();
	mw->run(hInstance, nCmdShow);
	delete mw;

	return 0;
}

//-----------------------------------------------------------------------------
// Name: MuehleWin()
// Desc:
//-----------------------------------------------------------------------------
MuehleWin::MuehleWin()
{
}

//-----------------------------------------------------------------------------
// Name: ~MuehleWin()
// Desc:
//-----------------------------------------------------------------------------
MuehleWin::~MuehleWin()
{
}

//-----------------------------------------------------------------------------
// Name: run()
// Desc:
//-----------------------------------------------------------------------------
void MuehleWin::run(HINSTANCE hInstance, int nCmdShow)
{
	// load default settings	
	loadDefaultSettings();

	// create window with wrapper class
	ww = new wildWeasel::masterMind(hInstance, WndProc_static, IDS_APP_TITLE, IDC_MUEHLEWIN, IDI_MUEHLEWIN, IDC_ARROW, IDC_MUEHLEWIN, IDI_SMALL, IDC_MUEHLEWIN);
	ww->createWindow(defaultWindowSize.x, defaultWindowSize.y, nCmdShow, WS_OVERLAPPEDWINDOW, NULL);

	// show disclaimer
	if (showDisclaimerOnStart) {
		if (IDOK != ww->showMessageBox(L"Disclaimer", L"Please read the legal notice in the file 'LICENSE.txt' distributed with this software. Press 'OK' to agree.", MB_OKCANCEL)) {
			ww->exitProgram();
			return;
		}
	}

	// current set database directory ok?
	ww->checkPathAndModify(strDatabaseDir, wstring(L"./database/"), false);
	
	// do not show menu item "Calculate Database", since it is in beta stadium
	RemoveMenu(ww->getHMenu(), IDM_CALCULATE_DATABASE, MF_BYCOMMAND);

	// expert mode
	if (!expertMode) {
		
		// do not show menu item "Inspect database"
		RemoveMenu(ww->getHMenu(), IDM_INSPECT_DATABASE, MF_BYCOMMAND);

		// limit search depth for minimax algorithmn
		RemoveMenu(ww->getHMenu(), ID_MINIMAXALGORITHMN_2_SEARCHDEPTH7, 	MF_BYCOMMAND);
		RemoveMenu(ww->getHMenu(), ID_MINIMAXALGORITHMN_2_SEARCHDEPTH8, 	MF_BYCOMMAND);
		RemoveMenu(ww->getHMenu(), ID_MINIMAXALGORITHMN_2_SEARCHDEPTH9, 	MF_BYCOMMAND);
		RemoveMenu(ww->getHMenu(), ID_MINIMAXALGORITHMN_2_SEARCHDEPTH10, 	MF_BYCOMMAND);
		RemoveMenu(ww->getHMenu(), ID_MINIMAXALGORITHMN_2_SEARCHDEPTH11, 	MF_BYCOMMAND);
		RemoveMenu(ww->getHMenu(), ID_MINIMAXALGORITHMN_SEARCHDEPTH7, 		MF_BYCOMMAND);
		RemoveMenu(ww->getHMenu(), ID_MINIMAXALGORITHMN_SEARCHDEPTH8, 		MF_BYCOMMAND);
		RemoveMenu(ww->getHMenu(), ID_MINIMAXALGORITHMN_SEARCHDEPTH9, 		MF_BYCOMMAND);
		RemoveMenu(ww->getHMenu(), ID_MINIMAXALGORITHMN_SEARCHDEPTH10,		MF_BYCOMMAND);
		RemoveMenu(ww->getHMenu(), ID_MINIMAXALGORITHMN_SEARCHDEPTH11, 		MF_BYCOMMAND);

		// if database files are missing than quit
		if (!(filesystem::exists(strDatabaseDir + L"/database.dat"))) {
			ww->showMessageBox(L"ERROR", L"The database file is missing. Please download it from www.mad-weasel.de. Program will stop now.", MB_OK);
			ww->exitProgram();
			return;
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
	ww->mainLoadingScreen.show(mainInitFunc_static, this);
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
}

//-----------------------------------------------------------------------------
// Name: loadDefaultSettings()
// Desc: 
//-----------------------------------------------------------------------------
void MuehleWin::loadDefaultSettings(void)
{
	// locals
	xmlNode	*	rootNode	= nullptr;
	xmlClass	xml(L"settings.xml", rootNode);

	// file loaded?
	if (rootNode != nullptr) {

		// <database directory="">
		strDatabaseDir			= rootNode->node("database")->attribute("directory")->valueW();

		// <expertMode enabled="true">
		expertMode				= (bool) (rootNode->node("expertMode")->attribute("enabled")->valueW().compare(L"true") == 0);

		// <historyList enabled="true">
		showHistoryList			= (bool) (rootNode->node("historyList")->attribute("enabled")->valueW().compare(L"true") == 0);

		// <firstRun showDisclaimer="true">
		showDisclaimerOnStart	= (bool) !(rootNode->node("firstRun")->attribute("showDisclaimer")->valueW().compare(L"false") == 0);

		// numMovesToRemis
		numMovesToRemis 		= (unsigned int) atoi(rootNode->node("gameSettings")->attribute("numMovesToRemis")->valueA().c_str());

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
void MuehleWin::saveDefaultSettings(void)
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
	
	// <historyList enabled="true">
	rootNode->addSubNodeWithAttribute(L"historyList", L"enabled", showHistoryList ? L"true" : L"false");

	// <firstRun showDisclaimer="true">
	rootNode->addSubNodeWithAttribute(L"firstRun", L"showDisclaimer", L"false");
	
	// <numMovesToRemis>000</>
	rootNode->addSubNodeWithAttribute(L"gameSettings", L"numMovesToRemis", (int) numMovesToRemis);

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
	xml.writeFile(L"settings.xml");
}

//-----------------------------------------------------------------------------
// Name: mainInitFunc_static()
// Desc:
//-----------------------------------------------------------------------------
void MuehleWin::mainInitFunc_static(void* pUser)
{
	((MuehleWin*) pUser)->mainInitFunc();
}

//-----------------------------------------------------------------------------
// Name: WndProc_static()
// Desc: 
//-----------------------------------------------------------------------------
LRESULT CALLBACK MuehleWin::WndProc_static(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	return mw->WndProc(hWnd, message, wParam, lParam);
}

//-----------------------------------------------------------------------------
// Name: About_static()
// Desc: 
//-----------------------------------------------------------------------------
INT_PTR CALLBACK MuehleWin::About_static(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	return mw->About(hDlg, message, wParam, lParam);
}

//-----------------------------------------------------------------------------
// Name: Settings_static()
// Desc:
//-----------------------------------------------------------------------------
INT_PTR CALLBACK MuehleWin::Settings_static(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	return mw->Settings(hDlg, message, wParam, lParam);
}

//-----------------------------------------------------------------------------
// Name: mainInitFunc()
// Desc: 
//-----------------------------------------------------------------------------
void MuehleWin::mainInitFunc()
{
	// default ww stuff
	ww->mainLoadingScreen.setCompletionFraction(0);
	ww->mainLoadingScreen.progress();
	ww->setBackground(strBackgroundFilename);

	// Initialize global strings
	textureBackground		.loadFile(ww, wstring(L"background.jpg"			));
	textureField			.loadFile(ww, wstring(L"field.png"				));
	textureLine				.loadFile(ww, wstring(L"line.png"				), false);
	textureCorner			.loadFile(ww, wstring(L"corner.png"				), false);
	
	// initialize muehle-classes
	myGame			= new muehle();													ww->mainLoadingScreen.progress();
	playerPerfect	= new perfectAI(strDatabaseDir);								ww->mainLoadingScreen.progress();
	playerMinMax	= new minMaxAI();												ww->mainLoadingScreen.progress();
	playerRandom	= new randomAI();												ww->mainLoadingScreen.progress();
	playerHuman		= nullptr;

	// begin a game by default
	myGame->beginNewGame(playerOne = playerHuman, playerTwo = playerHuman, fieldStruct::playerWhite, true, true);	ww->mainLoadingScreen.progress();
	myGame->setNumMovesToRemis(numMovesToRemis);

	// set the checkmarks in the menu for the default settings	
	CheckMenuItem(ww->getHMenu(), ID_PLAYER1_HUMAN,		    MF_CHECKED);
	CheckMenuItem(ww->getHMenu(), ID_PLAYER2_HUMAN,			MF_CHECKED);
	CheckMenuItem(ww->getHMenu(), ID_MOVESPEED_500MS,	    MF_CHECKED);
	CheckMenuItem(ww->getHMenu(), ID_GAME_SHOWPERFECTMOVE,	MF_UNCHECKED);
	CheckMenuItem(ww->getHMenu(), ID_GAME_SHOWSTATENUMBER,	MF_UNCHECKED);
	CheckMenuItem(ww->getHMenu(), ID_GAME_SHOWHISTORY, 		showHistoryList ? MF_CHECKED : MF_UNCHECKED);

	// calculate the alignment areas
	amAreaInspectDb	  .create(ww->alignmentRootFrame);								ww->mainLoadingScreen.progress();
	amAreaCalculation .create(ww->alignmentRootFrame);								ww->mainLoadingScreen.progress();
	alignmentGameField.create(ww->alignmentRootFrame);								ww->mainLoadingScreen.progress();
	alignmentHistory  .create(ww->alignmentRootFrame);								ww->mainLoadingScreen.progress();

	guiHistory.init(ww, *myGame, d3dFont2D, alignmentHistory, textureLine);																ww->mainLoadingScreen.progress();
	guiField.init(ww, *myGame, *playerPerfect, d3dFont2D, alignmentGameField, textureField);											ww->mainLoadingScreen.progress();
	miniMaxInspectDb= new miniMax::miniMaxWinInspectDb(ww, &playerPerfect->mm, amAreaInspectDb,    &d3dFont2D, &textureLine, guiField);	ww->mainLoadingScreen.progress();
	miniMaxCalcDb	= new miniMax::miniMaxWinCalcDb   (ww, &playerPerfect->mm, amAreaCalculation , &d3dFont2D, &textureLine);			ww->mainLoadingScreen.progress();

	modePlayGame 	= new modePlayGameClass		();									ww->mainLoadingScreen.progress();
	modeSetupField 	= new modeSetupFieldClass	();									ww->mainLoadingScreen.progress();
	modeInspectDb 	= new modeInspectDbClass	();									ww->mainLoadingScreen.progress();
	modeCalcDb 		= new modeCalcDbClass		();									ww->mainLoadingScreen.progress();

	modePlayGame	->init();														ww->mainLoadingScreen.progress();
	modeInspectDb	->init();														ww->mainLoadingScreen.progress();
	modeCalcDb		->init();														ww->mainLoadingScreen.progress();
	modeSetupField	->init();														ww->mainLoadingScreen.progress();
	
	guiHistory.setCallBacks(std::bind(&modePlayGameClass::undoMove, modePlayGame), std::bind(&modePlayGameClass::redoMove, modePlayGame));
	guiHistory.show(showHistoryList);												ww->mainLoadingScreen.progress();
	
	// position the field right beside the history list
	windowSizeChanged(ww->getWindowSizeX(), ww->getWindowSizeY());					ww->mainLoadingScreen.progress();
	followEvent(this, wildWeasel::eventFollower::eventType::WINDOWSIZE_CHANGED);

	// Set process and thread priority to below normal to not disturb other applications too much
	SetPriorityClass(GetCurrentProcess(), BELOW_NORMAL_PRIORITY_CLASS);

	// begin a game by default
	ww->scenarioManager.setActiveScenario(*modePlayGame);	
	windowSizeChanged(ww->getWindowSizeX(), ww->getWindowSizeY());					ww->mainLoadingScreen.progress();

	// upload resources
	ww->graphicManager.performResourceUpload();

	// show render rect
	ww->mainLoadingScreen.setCompletionFraction(3);
}

//-----------------------------------------------------------------------------
// Name: WndProc()
// Desc: Processes messages for the main window.
//-----------------------------------------------------------------------------
LRESULT CALLBACK MuehleWin::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;

	switch (message)
	{
    case WM_ENTERSIZEMOVE:
	case WM_INITMENU:
		// since the GUI is not updated while in a menu loop, pause the timers
		if (modePlayGame && modePlayGame->isActive()) {
			modePlayGame->botTimer.pause();
			modePlayGame->moveTimer.pause();
		}
		break;
    case WM_EXITSIZEMOVE:		
	case WM_EXITMENULOOP:
		// since the GUI was not updated while in a menu loop, resume the timers
		if (modePlayGame && modePlayGame->isActive()) {
			modePlayGame->botTimer.resume();
			modePlayGame->moveTimer.resume();
		}
		break;
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(ww->getHinst(), MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About_static);
			break;
		case IDM_SETTINGS:
			DialogBox(ww->getHinst(), MAKEINTRESOURCE(IDD_SETTINGS), hWnd, Settings_static);
			break;			
		case IDM_PRIVACY:		ww->showMessageBox(L"Privacy", TEXT_PRIVACY, MB_OK); break;		
		case IDM_LICENSE:		ww->showMessageBox(L"License", TEXT_LICENSE, MB_OK); break;
		case IDM_EXIT:
			ww->exitProgram();
			break;
        case ID_GAME_SETFIELD:
			ww->scenarioManager.setActiveScenario(*modeSetupField);
            break;
		case ID_GAME_UNDOMOVE:
			modePlayGame->undoMove();
			break;
		case ID_GAME_NEWGAME:
			ww->scenarioManager.setActiveScenario(*modePlayGame);
			break;
		case IDM_CALCULATE_DATABASE:
			ww->scenarioManager.setActiveScenario(*modeCalcDb);
			break;
		case IDM_INSPECT_DATABASE:
			ww->scenarioManager.setActiveScenario(*modeInspectDb);
			break;
		case ID_GAME_SHOWSTATENUMBER:
			guiField.showStateNumber(!guiField.isShowingStateNumber());
            CheckMenuItem(ww->getHMenu(), ID_GAME_SHOWSTATENUMBER, guiField.isShowingStateNumber() ? MF_CHECKED : MF_UNCHECKED);
            break;
		case ID_GAME_SHOWHISTORY:
			showHistoryList = !showHistoryList;
			guiHistory.show(showHistoryList);
			windowSizeChanged(ww->getWindowSizeX(), ww->getWindowSizeY());
			CheckMenuItem(ww->getHMenu(), ID_GAME_SHOWHISTORY, showHistoryList ? MF_CHECKED : MF_UNCHECKED);
			break;
        case ID_GAME_SHOWPERFECTMOVE:
            guiField.showPerfectMove(!guiField.isShowingPerfectMove());
            CheckMenuItem(ww->getHMenu(), ID_GAME_SHOWPERFECTMOVE, guiField.isShowingPerfectMove() ? MF_CHECKED : MF_UNCHECKED);
            break;
		case ID_PLAYER1_PERFECTDATABASE:			modePlayGame->changeBot(wmId, playerId::playerOne, playerPerfect);										break;
		case ID_PLAYER1_RANDOMBOT:					modePlayGame->changeBot(wmId, playerId::playerOne, playerRandom);										break;
		case ID_PLAYER1_HUMAN:						modePlayGame->changeBot(wmId, playerId::playerOne, playerHuman );										break;
		case ID_MINIMAXALGORITHMN_AUTOMATIC:		modePlayGame->changeBot(wmId, playerId::playerOne, playerMinMax);	playerMinMax->setSearchDepth(0);	break;
		case ID_MINIMAXALGORITHMN_SEARCHDEPTH1:		modePlayGame->changeBot(wmId, playerId::playerOne, playerMinMax);	playerMinMax->setSearchDepth(1);	break;
		case ID_MINIMAXALGORITHMN_SEARCHDEPTH2:		modePlayGame->changeBot(wmId, playerId::playerOne, playerMinMax);	playerMinMax->setSearchDepth(2);	break;
		case ID_MINIMAXALGORITHMN_SEARCHDEPTH3:		modePlayGame->changeBot(wmId, playerId::playerOne, playerMinMax);	playerMinMax->setSearchDepth(3);	break;
		case ID_MINIMAXALGORITHMN_SEARCHDEPTH4:		modePlayGame->changeBot(wmId, playerId::playerOne, playerMinMax);	playerMinMax->setSearchDepth(4);	break;
		case ID_MINIMAXALGORITHMN_SEARCHDEPTH5:		modePlayGame->changeBot(wmId, playerId::playerOne, playerMinMax);	playerMinMax->setSearchDepth(5);	break;
		case ID_MINIMAXALGORITHMN_SEARCHDEPTH6:		modePlayGame->changeBot(wmId, playerId::playerOne, playerMinMax);	playerMinMax->setSearchDepth(6);	break;
		case ID_MINIMAXALGORITHMN_SEARCHDEPTH7:		modePlayGame->changeBot(wmId, playerId::playerOne, playerMinMax);	playerMinMax->setSearchDepth(7);	break;
		case ID_MINIMAXALGORITHMN_SEARCHDEPTH8:		modePlayGame->changeBot(wmId, playerId::playerOne, playerMinMax);	playerMinMax->setSearchDepth(8);	break;
		case ID_MINIMAXALGORITHMN_SEARCHDEPTH9:		modePlayGame->changeBot(wmId, playerId::playerOne, playerMinMax);	playerMinMax->setSearchDepth(9);	break;
		case ID_MINIMAXALGORITHMN_SEARCHDEPTH10:	modePlayGame->changeBot(wmId, playerId::playerOne, playerMinMax);	playerMinMax->setSearchDepth(10);	break;
		case ID_MINIMAXALGORITHMN_SEARCHDEPTH11:	modePlayGame->changeBot(wmId, playerId::playerOne, playerMinMax);	playerMinMax->setSearchDepth(11);	break;
		case ID_PLAYER2_PERFECTDATABASE:			modePlayGame->changeBot(wmId, playerId::playerTwo, playerPerfect);										break;
		case ID_PLAYER2_RANDOMBOT:					modePlayGame->changeBot(wmId, playerId::playerTwo, playerRandom);										break;
		case ID_PLAYER2_HUMAN: 						modePlayGame->changeBot(wmId, playerId::playerTwo, playerHuman );										break;
		case ID_MINIMAXALGORITHMN_2_AUTOMATIC:		modePlayGame->changeBot(wmId, playerId::playerTwo, playerMinMax);	playerMinMax->setSearchDepth(0);	break;
		case ID_MINIMAXALGORITHMN_2_SEARCHDEPTH1:	modePlayGame->changeBot(wmId, playerId::playerTwo, playerMinMax);	playerMinMax->setSearchDepth(1);	break;
		case ID_MINIMAXALGORITHMN_2_SEARCHDEPTH2:	modePlayGame->changeBot(wmId, playerId::playerTwo, playerMinMax);	playerMinMax->setSearchDepth(2);	break;
		case ID_MINIMAXALGORITHMN_2_SEARCHDEPTH3:	modePlayGame->changeBot(wmId, playerId::playerTwo, playerMinMax);	playerMinMax->setSearchDepth(3);	break;
		case ID_MINIMAXALGORITHMN_2_SEARCHDEPTH4:	modePlayGame->changeBot(wmId, playerId::playerTwo, playerMinMax);	playerMinMax->setSearchDepth(4);	break;
		case ID_MINIMAXALGORITHMN_2_SEARCHDEPTH5:	modePlayGame->changeBot(wmId, playerId::playerTwo, playerMinMax);	playerMinMax->setSearchDepth(5);	break;
		case ID_MINIMAXALGORITHMN_2_SEARCHDEPTH6:	modePlayGame->changeBot(wmId, playerId::playerTwo, playerMinMax);	playerMinMax->setSearchDepth(6);	break;
		case ID_MINIMAXALGORITHMN_2_SEARCHDEPTH7:	modePlayGame->changeBot(wmId, playerId::playerTwo, playerMinMax);	playerMinMax->setSearchDepth(7);	break;
		case ID_MINIMAXALGORITHMN_2_SEARCHDEPTH8:	modePlayGame->changeBot(wmId, playerId::playerTwo, playerMinMax);	playerMinMax->setSearchDepth(8);	break;
		case ID_MINIMAXALGORITHMN_2_SEARCHDEPTH9:	modePlayGame->changeBot(wmId, playerId::playerTwo, playerMinMax);	playerMinMax->setSearchDepth(9);	break;
		case ID_MINIMAXALGORITHMN_2_SEARCHDEPTH10:	modePlayGame->changeBot(wmId, playerId::playerTwo, playerMinMax);	playerMinMax->setSearchDepth(10);	break;
		case ID_MINIMAXALGORITHMN_2_SEARCHDEPTH11:	modePlayGame->changeBot(wmId, playerId::playerTwo, playerMinMax);	playerMinMax->setSearchDepth(11);	break;
		case ID_MOVESPEED_100MS:					modePlayGame->changeMoveSpeed(wmId, 100);																break;
		case ID_MOVESPEED_500MS:					modePlayGame->changeMoveSpeed(wmId, 500);																break;
		case ID_MOVESPEED_1S:						modePlayGame->changeMoveSpeed(wmId, 1000);																break;
		case ID_MOVESPEED_3S:						modePlayGame->changeMoveSpeed(wmId, 3000);																break;
		default:									return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	}
	return 0;
}

//-----------------------------------------------------------------------------
// Name: About()
// Desc: Message handler for about box.
//-----------------------------------------------------------------------------
INT_PTR CALLBACK MuehleWin::About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
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

//-----------------------------------------------------------------------------
// Name: Settings()
// Desc: Message handler for settings dialog.
//-----------------------------------------------------------------------------
INT_PTR CALLBACK MuehleWin::Settings(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		SetDlgItemInt(hDlg, IDC_NUM_MOVES_TO_REMIS, numMovesToRemis, FALSE);
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDCANCEL) {
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		} else if (LOWORD(wParam) == IDOK) {
			numMovesToRemis = GetDlgItemInt(hDlg, IDC_NUM_MOVES_TO_REMIS, NULL, FALSE);
			myGame->setNumMovesToRemis(numMovesToRemis);
			modePlayGame->guiHistory.update();
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

//-----------------------------------------------------------------------------
// Name: windowSizeChanged()
// Desc: Inform the gui elements about the new window size 
//-----------------------------------------------------------------------------
void MuehleWin::windowSizeChanged(int xSize, int ySize)
{
	if (modePlayGame->isActive() || modeSetupField->isActive()) {
		alignmentGameField.left.setPosition((showHistoryList ? alignmentHistory.right.getPosition() : 0) + 0.02f * ww->getWindowSizeX());
		alignmentGameField.right.setPosition(0.98f * ww->getWindowSizeX());
		guiField.setAlignment(alignmentGameField);
	} else if (modeInspectDb->isActive()) {
		miniMaxInspectDb->resize(amAreaInspectDb);
	} else if (modeCalcDb->isActive()) {
		miniMaxCalcDb->resize(amAreaCalculation);
	}
}

#pragma endregion

/*** modePlayGameClass **************************************************************************************************************************************************************************/

#pragma region modePlayGameClass

//-----------------------------------------------------------------------------
// Name: modePlayGameClass()
// Desc: 
//-----------------------------------------------------------------------------
modePlayGameClass::modePlayGameClass() :
	guiField(mw->guiField), guiHistory(mw->guiHistory), ww(*mw->ww), myGame(*mw->myGame)
{
}

//-----------------------------------------------------------------------------
// Name: createControls()
// Desc: 
//-----------------------------------------------------------------------------
void modePlayGameClass::init()
{
	botTimer.start(&ww, botTimerFunc, this, botTimerTime);
}

//-----------------------------------------------------------------------------
// Name: activateMode()
// Desc: 
//-----------------------------------------------------------------------------
void modePlayGameClass::activate()
{
	// locals
    unsigned int myPushFrom, myPushTo;

	curMove 	= moveInfo{};
	gameState 	= modePlayGameClass::gameStateEnum::newGameYetBlank;
	if (mw->settingColor == fieldStruct::playerBlack || mw->settingColor == fieldStruct::playerWhite) {
		mw->settingColor = playerId::squareIsFree;
	} else {
		myGame.beginNewGame(mw->playerOne, mw->playerTwo, fieldStruct::playerWhite, true, mw->resetFieldOnStart);
	}

	EnableMenuItem(ww.getHMenu(), ID_GAME_SHOWPERFECTMOVE, 	MF_ENABLED);
	EnableMenuItem(ww.getHMenu(), ID_GAME_SHOWSTATENUMBER, 	MF_ENABLED);
	EnableMenuItem(ww.getHMenu(), ID_GAME_UNDOMOVE,			MF_ENABLED);
	EnableMenuItem(ww.getHMenu(), ID_GAME_SHOWHISTORY,		MF_ENABLED);

	guiHistory.show(mw->showHistoryList);
	guiHistory.setButtonsEnable(true);
	guiField.setVisibility(true);
	guiField.setAnimateStoneOnMove(false);
	guiField.setField(mw->settingColor, true);
	guiField.setFieldPosClickedFunc(bind(&modePlayGameClass::fieldPosClicked, this, placeholders::_1));
	guiField.setAlignment(mw->alignmentGameField);
	guiField.setAnimateStoneOnMove(true);

	stoneAnimationFinished();
}

//-----------------------------------------------------------------------------
// Name: deactivateMode()
// Desc: 
//-----------------------------------------------------------------------------
bool modePlayGameClass::deactivate()
{
	// ask the user if he really wants to stop the game
	if (myGame.getMovesDone() > 0) {
		if (ww.showMessageBox(L"CANCEL", L"Do you really want to stop the current game?", MB_YESNO) == IDNO) {
			return false;
		}
	}
	
	mw->resetFieldOnStart 	= true;
	gameState 				= gameStateEnum::undefined;
	
	EnableMenuItem(ww.getHMenu(), ID_GAME_SHOWPERFECTMOVE, 	MF_GRAYED);
	EnableMenuItem(ww.getHMenu(), ID_GAME_SHOWSTATENUMBER, 	MF_GRAYED);
	EnableMenuItem(ww.getHMenu(), ID_GAME_SHOWHISTORY,		MF_GRAYED);
	EnableMenuItem(ww.getHMenu(), ID_GAME_UNDOMOVE,			MF_GRAYED);

	guiField.setFieldPosClickedFunc(nullptr);
	guiField.setVisibility(false);
	guiHistory.show(false);
	guiHistory.setButtonsEnable(false);
	return true;
}

//-----------------------------------------------------------------------------
// Name: changeMoveSpeed()
// Desc: Change the speed of the bot. Technically this is done by changing the timer interval.
//-----------------------------------------------------------------------------
void modePlayGameClass::changeMoveSpeed(UINT wmId, DWORD duration)
{
	// uncheck all menus
	CheckMenuItem(ww.getHMenu(), ID_MOVESPEED_100MS,	MF_UNCHECKED);
	CheckMenuItem(ww.getHMenu(), ID_MOVESPEED_500MS,	MF_UNCHECKED);
	CheckMenuItem(ww.getHMenu(), ID_MOVESPEED_1S,		MF_UNCHECKED);
	CheckMenuItem(ww.getHMenu(), ID_MOVESPEED_3S,		MF_UNCHECKED);
	
	// check the selected one
	CheckMenuItem(ww.getHMenu(), wmId, MF_CHECKED);	
	botTimerTime = duration;
}

//-----------------------------------------------------------------------------
// Name: changeBot()
// Desc: Change the bot for a player . Can be nullptr for human player.
//-----------------------------------------------------------------------------
void modePlayGameClass::changeBot(UINT wmId, playerId player, muehleAI *bot)
{
	// check input parameters
	if (player != playerId::playerOne && player != playerId::playerTwo) return;

	// kill timer
	botTimer.terminate();

	// set new bot
	myGame.setAI(player, bot);

	// uncheck all menus
	if (player == playerId::playerOne) {
		mw->playerOne = bot;
		unCheckAllPlayerOne();	
	} else {
		mw->playerTwo = bot;
		unCheckAllPlayerTwo();	
	}

	// check new selected bot
	CheckMenuItem(ww.getHMenu(), wmId, MF_CHECKED);		

	// disable undo button when both players are computers
	EnableMenuItem(ww.getHMenu(), ID_GAME_UNDOMOVE, (myGame.isCurrentPlayerHuman() || myGame.isOpponentPlayerHuman()) ? MF_ENABLED : MF_GRAYED);
	guiHistory.setButtonsEnable(myGame.isCurrentPlayerHuman() || myGame.isOpponentPlayerHuman());

	// set timer if bot has to move
	if (!isCurrentPlayerHuman() && !mw->modeSetupField->isActive()) botTimer.start(&ww, botTimerFunc, this, botTimerTime);
}

//-----------------------------------------------------------------------------
// Name: unCheckAllPlayerOne()
// Desc: Uncheck all player one related menu items 
//-----------------------------------------------------------------------------
void modePlayGameClass::unCheckAllPlayerOne()
{
	CheckMenuItem(ww.getHMenu(), ID_MINIMAXALGORITHMN_SEARCHDEPTH1,	MF_UNCHECKED);
	CheckMenuItem(ww.getHMenu(), ID_MINIMAXALGORITHMN_SEARCHDEPTH2,	MF_UNCHECKED);
	CheckMenuItem(ww.getHMenu(), ID_MINIMAXALGORITHMN_SEARCHDEPTH3,	MF_UNCHECKED);
	CheckMenuItem(ww.getHMenu(), ID_MINIMAXALGORITHMN_SEARCHDEPTH4,	MF_UNCHECKED);
	CheckMenuItem(ww.getHMenu(), ID_MINIMAXALGORITHMN_SEARCHDEPTH5,	MF_UNCHECKED);
	CheckMenuItem(ww.getHMenu(), ID_MINIMAXALGORITHMN_SEARCHDEPTH6,	MF_UNCHECKED);
	CheckMenuItem(ww.getHMenu(), ID_MINIMAXALGORITHMN_SEARCHDEPTH7,	MF_UNCHECKED);
	CheckMenuItem(ww.getHMenu(), ID_MINIMAXALGORITHMN_SEARCHDEPTH8,	MF_UNCHECKED);
	CheckMenuItem(ww.getHMenu(), ID_MINIMAXALGORITHMN_SEARCHDEPTH9,	MF_UNCHECKED);
	CheckMenuItem(ww.getHMenu(), ID_MINIMAXALGORITHMN_SEARCHDEPTH10,	MF_UNCHECKED);
	CheckMenuItem(ww.getHMenu(), ID_MINIMAXALGORITHMN_SEARCHDEPTH11,	MF_UNCHECKED);
	CheckMenuItem(ww.getHMenu(), ID_MINIMAXALGORITHMN_AUTOMATIC,		MF_UNCHECKED);
	CheckMenuItem(ww.getHMenu(), ID_PLAYER1_HUMAN,						MF_UNCHECKED);
	CheckMenuItem(ww.getHMenu(), ID_PLAYER1_PERFECTDATABASE,			MF_UNCHECKED);
	CheckMenuItem(ww.getHMenu(), ID_PLAYER1_RANDOMBOT,					MF_UNCHECKED);
}

//-----------------------------------------------------------------------------
// Name: unCheckAllPlayerTwo()
// Desc: Uncheck all player two related menu items
//-----------------------------------------------------------------------------
void modePlayGameClass::unCheckAllPlayerTwo()
{
	CheckMenuItem(ww.getHMenu(), ID_MINIMAXALGORITHMN_2_SEARCHDEPTH1,		MF_UNCHECKED);
	CheckMenuItem(ww.getHMenu(), ID_MINIMAXALGORITHMN_2_SEARCHDEPTH2,		MF_UNCHECKED);
	CheckMenuItem(ww.getHMenu(), ID_MINIMAXALGORITHMN_2_SEARCHDEPTH3,		MF_UNCHECKED);
	CheckMenuItem(ww.getHMenu(), ID_MINIMAXALGORITHMN_2_SEARCHDEPTH4,		MF_UNCHECKED);
	CheckMenuItem(ww.getHMenu(), ID_MINIMAXALGORITHMN_2_SEARCHDEPTH5,		MF_UNCHECKED);
	CheckMenuItem(ww.getHMenu(), ID_MINIMAXALGORITHMN_2_SEARCHDEPTH6,		MF_UNCHECKED);
	CheckMenuItem(ww.getHMenu(), ID_MINIMAXALGORITHMN_2_SEARCHDEPTH7,		MF_UNCHECKED);
	CheckMenuItem(ww.getHMenu(), ID_MINIMAXALGORITHMN_2_SEARCHDEPTH8,		MF_UNCHECKED);
	CheckMenuItem(ww.getHMenu(), ID_MINIMAXALGORITHMN_2_SEARCHDEPTH9,		MF_UNCHECKED);
	CheckMenuItem(ww.getHMenu(), ID_MINIMAXALGORITHMN_2_SEARCHDEPTH10,		MF_UNCHECKED);
	CheckMenuItem(ww.getHMenu(), ID_MINIMAXALGORITHMN_2_SEARCHDEPTH11,		MF_UNCHECKED);
	CheckMenuItem(ww.getHMenu(), ID_MINIMAXALGORITHMN_2_AUTOMATIC,			MF_UNCHECKED);
	CheckMenuItem(ww.getHMenu(), ID_PLAYER2_HUMAN,							MF_UNCHECKED);
	CheckMenuItem(ww.getHMenu(), ID_PLAYER2_PERFECTDATABASE,				MF_UNCHECKED);
	CheckMenuItem(ww.getHMenu(), ID_PLAYER2_RANDOMBOT,						MF_UNCHECKED);
}

//-----------------------------------------------------------------------------
// Name: botTimerFunc()
// Desc: A timer function for the bot. It is timed, so that the move is not instantly done. 
//-----------------------------------------------------------------------------
void modePlayGameClass::botTimerFunc(void* pUser)
{
	modePlayGameClass* pgc = (modePlayGameClass*) pUser;
	pgc->letComputerPlay();
}

//-----------------------------------------------------------------------------
// Name: moveTimerFunc()
// Desc: A timer function for the bot. It is timed, so that the move is not instantly done. 
//-----------------------------------------------------------------------------
void modePlayGameClass::moveTimerFunc(void* pUser)
{
	modePlayGameClass* pgc = (modePlayGameClass*) pUser;
	pgc->stoneAnimationFinished();
}

//-----------------------------------------------------------------------------
// Name: fieldPosClicked()
// Desc: A field position has been clicked.
//-----------------------------------------------------------------------------
void modePlayGameClass::fieldPosClicked(unsigned int clickOnFieldPos)
{
	// when current player is not human or game has finished than do nothing
	if (!isCurrentPlayerHuman() || myGame.gameHasFinished()				) return;
	if (gameState == modePlayGameClass::gameStateEnum::gameFinished		) return;
	if (gameState == modePlayGameClass::gameStateEnum::newGameYetBlank	) return;
	if (gameState == modePlayGameClass::gameStateEnum::animatingStone	) return;

	// select stone to re-move
	if (myGame.mustStoneBeRemoved() && gameState == modePlayGameClass::gameStateEnum::removingStone) {
		curMove.removeStone	= clickOnFieldPos;
	// select a field to set a stone
	} else if (myGame.inSettingPhase() && gameState == modePlayGameClass::gameStateEnum::settingStone) {
		curMove.from		= fieldStruct::size;
		curMove.to			= clickOnFieldPos;
	// move a stone and destination choosen
	} else if (gameState == modePlayGameClass::gameStateEnum::stoneSelectedForMove && curMove.from < fieldStruct::size) {
		curMove.to   		= clickOnFieldPos;
	// a stone has been selected
	} else if (gameState == modePlayGameClass::gameStateEnum::movingStone) {
		curMove.from		= clickOnFieldPos;
	// invalid move
	} else {
		return;
	}

	// move
	initiateStoneAnimation();
}

//-----------------------------------------------------------------------------
// Name: letComputerPlay()
// Desc: The computer has to make a move.
//-----------------------------------------------------------------------------
void modePlayGameClass::letComputerPlay()
{
	botTimer.terminate();

	// pre conditions correct?
	if (mw->modeInspectDb	->isActive()								) return;
	if (mw->modeCalcDb		->isActive()								) return;
    if (mw->modeSetupField	->isActive()								) return;
	if (isCurrentPlayerHuman() || myGame.gameHasFinished()				) return;
	if (gameState == modePlayGameClass::gameStateEnum::gameFinished		) return;
	if (gameState == modePlayGameClass::gameStateEnum::newGameYetBlank	) return;
	if (gameState == modePlayGameClass::gameStateEnum::animatingStone	) return;	

	// get move from AI
	myGame.getComputersChoice(curMove);

	// move
	initiateStoneAnimation();
}

//-----------------------------------------------------------------------------
// Name: processMove()
// Desc: Performs a move, if it is feasible, based on the current pushFrom and pushTo values.
//-----------------------------------------------------------------------------
void modePlayGameClass::initiateStoneAnimation()
{
	// remember game state so that gui can show it
	bool			inSettingPhase		= myGame.inSettingPhase();
	unsigned int	numberOfStoneSet	= myGame.getNumStonesSet();
	playerId		currentPlayer		= myGame.getCurrentPlayer();

	// remove a stone from the field
	if (gameState == modePlayGameClass::gameStateEnum::removingStone && myGame.mustStoneBeRemoved()) {
		animateFrom 	= curMove.removeStone;
		animateTo   	= fieldStruct::size;
	// set a stone during the setting phase
	} else if (gameState == modePlayGameClass::gameStateEnum::settingStone && inSettingPhase) {
		animateFrom 	= fieldStruct::size;
		animateTo 		= curMove.to;
	// de-select stone if it is the same as before
	} else if (gameState == modePlayGameClass::gameStateEnum::stoneSelectedForMove && curMove.to < fieldStruct::size && curMove.from < fieldStruct::size && curMove.from == curMove.to) {
		curMove = moveInfo{};
		guiField.updateGameStatusText(isCurrentPlayerHuman());
		guiField.activateStonesOfCurrentPlayer(fieldStruct::size, mw->settingColor, curMove.from);
		gameState = modePlayGameClass::gameStateEnum::movingStone;
		return;
	// moving stone to a new position
	} else if (gameState == modePlayGameClass::gameStateEnum::stoneSelectedForMove && curMove.from < fieldStruct::size && curMove.to < fieldStruct::size) {
		animateFrom 	= curMove.from;
		animateTo 		= curMove.to;	
	// moving stone to a new position
	} else if (gameState == modePlayGameClass::gameStateEnum::movingStone && curMove.from < fieldStruct::size && curMove.to < fieldStruct::size) {
		animateFrom 	= curMove.from;
		animateTo 		= curMove.to;
	// just select a stone
	} else if (gameState == modePlayGameClass::gameStateEnum::movingStone && curMove.from < fieldStruct::size && curMove.to == fieldStruct::size) {
		guiField.setGameStatusText(L"Choose a \ndestination!");
		guiField.activateStonesOfCurrentPlayer(curMove.from, mw->settingColor, curMove.from);
		gameState = modePlayGameClass::gameStateEnum::stoneSelectedForMove;
		return;
	// invalid move
	} else {
		guiField.setGameStatusText(L"Invalid move!");
		return;
	}

	// check if the move is allowed
	if (!myGame.isMoveAllowed(curMove, gameState != modePlayGameClass::gameStateEnum::removingStone)) {
		animateFrom = fieldStruct::size;
		animateTo   = fieldStruct::size;
		return;
	}
	
	// start stone move animation
	gameState = modePlayGameClass::gameStateEnum::animatingStone;
	guiField.startStoneMoveAnimation(animateFrom, animateTo, myGame.mustStoneBeRemoved(), inSettingPhase, numberOfStoneSet, currentPlayer);
	moveTimer.start(&ww, moveTimerFunc, this, guiField.getMoveAnimationDuration() * 1000);
}

//-----------------------------------------------------------------------------
// Name: finalizeMove()
// Desc: Finalizes a move. This function is called after the stone animation has finished.
//-----------------------------------------------------------------------------
void modePlayGameClass::stoneAnimationFinished()
{
	moveTimer.terminate();

	// remember game state so that gui can show it
	unsigned int 			mustStoneBeRemoved		= myGame.mustStoneBeRemoved();
	bool					inSettingPhase			= myGame.inSettingPhase();
	unsigned int			numberOfStoneSet		= myGame.getNumStonesSet();
	playerId				currentPlayer			= myGame.getCurrentPlayer();
	fieldStruct::fieldPos	queuedStoneRemoval		= fieldStruct::size;

	// check if move is feasible
	if (gameState == modePlayGameClass::gameStateEnum::animatingStone) {
		if (animateFrom >= fieldStruct::size && animateTo >= fieldStruct::size) {
			guiField.setGameStatusText(L"Invalid move!");
			return;
		}

		// if stone removal is already known, but corresponding state not yet active, so queue it
		if (curMove.removeStone != fieldStruct::size && !mustStoneBeRemoved) {
			queuedStoneRemoval 	= curMove.removeStone;
			curMove.removeStone = fieldStruct::size;
		}

		// do move, if not removing a stone
		if (myGame.moveStone(curMove)) {
			curMove = moveInfo{};
		}

		// show final state of move
		guiField.moveStone(animateFrom, animateTo, mustStoneBeRemoved, inSettingPhase, numberOfStoneSet, currentPlayer);
	}
	
	// a new animation is needed in any case
	animateFrom = fieldStruct::size;
	animateTo   = fieldStruct::size;

	// reset current considered move if completed
	if (myGame.gameHasFinished()) {
		gameState = modePlayGameClass::gameStateEnum::gameFinished;
	} else if (myGame.mustStoneBeRemoved()) {
		gameState = modePlayGameClass::gameStateEnum::removingStone;
	} else if (myGame.inSettingPhase()) {
		gameState = modePlayGameClass::gameStateEnum::settingStone;
	} else {
		gameState = modePlayGameClass::gameStateEnum::movingStone;
	}

	// update game status text
	guiField.updateGameStatusText(isCurrentPlayerHuman());

	// show move in history
	guiHistory.update();

	// game finished ?
	if (myGame.gameHasFinished()) {
		guiField.deactivateAllStones();
		EnableMenuItem(ww.getHMenu(), ID_GAME_UNDOMOVE, MF_GRAYED);
		guiHistory.setButtonsEnable(false);
		showWinnerAnimation();
    
	// any queue stone removal?
	} else if (queuedStoneRemoval != fieldStruct::size) {
		curMove.removeStone = queuedStoneRemoval;
		initiateStoneAnimation();

	// current player human ?
	} else if (isCurrentPlayerHuman()) {
		guiField.activateStonesOfCurrentPlayer(fieldStruct::size, mw->settingColor, animateFrom);
		guiHistory.setButtonsEnable(true);

	// current player a bot ?
	} else {
		guiField.activateStonesOfCurrentPlayer(fieldStruct::size, mw->settingColor, animateFrom);
		guiField.deactivateAllStones();
		guiHistory.setButtonsEnable(false);
		botTimer.start(&ww, botTimerFunc, this, botTimerTime + (unsigned int) (guiField.getMoveAnimationDuration() * 1000));
	}
}

//-----------------------------------------------------------------------------
// Name: isCurrentPlayerHuman()
// Desc: Returns true if the current player is a human player
//-----------------------------------------------------------------------------
bool modePlayGameClass::isCurrentPlayerHuman() 
{
	MENUITEMINFO mii;

	mii.cbSize	= sizeof(MENUITEMINFO);
	mii.fMask	= MIIM_STATE;

	// player with black ?
	if (myGame.getCurrentPlayer() == playerId::playerOne) {
		GetMenuItemInfo(ww.getHMenu(), ID_PLAYER1_HUMAN, false, &mii);
		return (mii.fState == MFS_CHECKED);
	} else {
		GetMenuItemInfo(ww.getHMenu(), ID_PLAYER2_HUMAN, false, &mii);
		return (mii.fState == MFS_CHECKED);
	}
}

//-----------------------------------------------------------------------------
// Name: showWinnerAnimation()
// Desc: Shows a message box with the winner. 
//-----------------------------------------------------------------------------
void modePlayGameClass::showWinnerAnimation()
{
	auto winner = myGame.getWinner();
	if (winner == playerId::playerOne) {
		ww.showMessageBox(L"Game Finished", L"Black player has WON!", MB_OK);
	} else if (winner == playerId::playerTwo) {
		ww.showMessageBox(L"Game Finished", L"White player has WON!", MB_OK);
	} else if (winner == playerId::squareIsFree) {
		ww.showMessageBox(L"Game Finished", L"The game ended in a draw!", MB_OK);
	}
}

//-----------------------------------------------------------------------------
// Name: redoMove()
// Desc: 
//-----------------------------------------------------------------------------
void modePlayGameClass::redoMove()
{
	if (myGame.gameHasFinished()) 		return;
	if (mw->modeSetupField->isActive()) return;
	if (!isCurrentPlayerHuman()) 		return;

	myGame.redoLastMove();
	while (!isCurrentPlayerHuman()) {
		myGame.redoLastMove();
	}
	guiField.setField(mw->settingColor, true);
	stoneAnimationFinished();
}

//-----------------------------------------------------------------------------
// Name: undoMove()
// Desc: Reverts the last move. Prerequisite is that the current player is a human player, no winner is determined and the application is not in the setup phase. 
//-----------------------------------------------------------------------------
void modePlayGameClass::undoMove()
{
	if (myGame.gameHasFinished()) 		return;
	if (mw->modeSetupField->isActive()) return;
	if (!isCurrentPlayerHuman()) 		return;
	if (myGame.getMovesDone() == 0) 	return;
	
	myGame.undoLastMove();
	while (!isCurrentPlayerHuman()) {
		myGame.undoLastMove();
	}
	guiField.setField(mw->settingColor, true);
	stoneAnimationFinished();
}

#pragma endregion

/*** modeSetupFieldClass **************************************************************************************************************************************************************************/

#pragma region modeSetupFieldClass
//-----------------------------------------------------------------------------
// Name: modeSetupFieldClass()
// Desc:
//-----------------------------------------------------------------------------
modeSetupFieldClass::modeSetupFieldClass() :
	guiField(mw->guiField), ww(*mw->ww), myGame(*mw->myGame)
{
}

//-----------------------------------------------------------------------------
// Name: createControls()
// Desc: 
//-----------------------------------------------------------------------------
void modeSetupFieldClass::init()
{
	labelSetup.create(&ww, wstring(L"Setting up a customized game state..."), &(mw->d3dFont2D), 0.0f);
	labelSetup.setTextColor(colTextSetup);
	labelSetup.setAlignment(alignmentSetupText);
	labelSetup.setTextState(wildWeasel::guiElemState::HIDDEN);
}

//-----------------------------------------------------------------------------
// Name: activateMode()
// Desc: 
//-----------------------------------------------------------------------------
void modeSetupFieldClass::activate()
{
	// Show message box asking if game is in setting phase or not
    int  ret = MessageBoxW(ww.getHwnd(), L"White player will be on turn.\nFirst place black stones with left mouse button,\nthen click right and place the white stones. Terminate again with the right mouse button.\n\n Game in setting phase?", L"set up an arbitrary game state", MB_YESNO);
    bool settingPhase;
	mw->resetFieldOnStart = false;

    switch (ret)
    {
    case IDYES:     settingPhase = true;    break;
    case IDNO:      settingPhase = false;   break;
    default:        return;
    }

    mw->settingColor = fieldStruct::playerWhite;
    myGame.beginNewGame(mw->playerOne, mw->playerTwo, fieldStruct::playerWhite, settingPhase, true);
	guiField.showPerfectMove(false);
	guiField.showStateNumber(false);
	guiField.setAnimateStoneOnMove(false);
	guiField.setFieldPosClickedFunc(bind(&modeSetupFieldClass::fieldPosClicked, this, placeholders::_1));
	guiField.setVisibility(true);
	guiField.setField(mw->settingColor, true);
	guiField.setGameStatusText(L"Set a stone!");
	guiField.setAlignment(mw->alignmentGameField);
	labelSetup.setTextState(wildWeasel::guiElemState::VISIBLE);
	followEvent(this, wildWeasel::eventFollower::eventType::RIGHT_MOUSEBUTTON_PRESSED);
}

//-----------------------------------------------------------------------------
// Name: deactivateMode()
// Desc: 
//-----------------------------------------------------------------------------
bool modeSetupFieldClass::deactivate()
{
	guiField.setFieldPosClickedFunc(nullptr);
	guiField.setVisibility(false);
	guiField.showPerfectMove(GetMenuState(ww.getHMenu(), ID_GAME_SHOWPERFECTMOVE, 	MF_BYCOMMAND) & MF_CHECKED);
	guiField.showStateNumber(GetMenuState(ww.getHMenu(), ID_GAME_SHOWSTATENUMBER, 	MF_BYCOMMAND) & MF_CHECKED);
	guiField.setAnimateStoneOnMove(true);
	labelSetup.setTextState(wildWeasel::guiElemState::HIDDEN);
	forgetEvent(this, wildWeasel::eventFollower::eventType::RIGHT_MOUSEBUTTON_PRESSED);
	return true;
}

//-----------------------------------------------------------------------------
// Name: fieldPosClicked()
// Desc: 
//-----------------------------------------------------------------------------
void modeSetupFieldClass::fieldPosClicked(unsigned int clickOnFieldPos)
{
	auto settingColor = mw->settingColor;
	
    if (settingColor == fieldStruct::playerWhite) {
        myGame.putStone(clickOnFieldPos, fieldStruct::playerWhite);
    } else if (settingColor == fieldStruct::playerBlack) {
        myGame.putStone(clickOnFieldPos, fieldStruct::playerBlack);
    }

	guiField.setField(settingColor, true);
	guiField.setGameStatusText(L"Set a stone!");
	
	if (myGame.inSettingPhase()) {
		int numWhiteStonesResting;
		int numBlackStonesResting;
		myGame.calcNumberOfRestingStones(numWhiteStonesResting, numBlackStonesResting);
		if ((settingColor == fieldStruct::playerBlack && numBlackStonesResting == 0) || (settingColor == fieldStruct::playerWhite && numWhiteStonesResting == 0)) {
			rightMouseButtonPressed(0, 0);
		}
	} else {
		if (settingColor == fieldStruct::playerWhite && myGame.getCurrentPlayer() == fieldStruct::playerWhite && myGame.getNumStonesOfCurPlayer() == fieldStruct::numStonesPerPlayer) {
			rightMouseButtonPressed(0, 0);
		} else if (settingColor == fieldStruct::playerBlack && myGame.getCurrentPlayer() == fieldStruct::playerBlack && myGame.getNumStonesOfOppPlayer() == fieldStruct::numStonesPerPlayer) {
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
	auto settingColor = mw->settingColor;

    // setting up field
    if (settingColor == fieldStruct::playerBlack) {
        myGame.settingPhaseHasFinished();
		ww.scenarioManager.setActiveScenario(*mw->modePlayGame);
	// change stone color
	} else if (settingColor == fieldStruct::playerWhite) {
        settingColor = fieldStruct::playerBlack;
		guiField.setField(settingColor, true);
		mw->settingColor = settingColor;
    }
}

#pragma endregion

/*** modeInspectDbClass **************************************************************************************************************************************************************************/

#pragma region modeInspectDbClass
//-----------------------------------------------------------------------------
// Name: modeInspectDbClass()
// Desc:
//-----------------------------------------------------------------------------
modeInspectDbClass::modeInspectDbClass()	
{
}

//-----------------------------------------------------------------------------
// Name: createControls()
// Desc: 
//-----------------------------------------------------------------------------
void modeInspectDbClass::init()
{
	mw->miniMaxInspectDb->createControls();
}

//-----------------------------------------------------------------------------
// Name: activateMode()
// Desc: 
//-----------------------------------------------------------------------------
void modeInspectDbClass::activate()
{
	mw->miniMaxInspectDb->showControls(true);
}

//-----------------------------------------------------------------------------
// Name: deactivateMode()
// Desc: 
//-----------------------------------------------------------------------------
bool modeInspectDbClass::deactivate()
{
	mw->miniMaxInspectDb->showControls(false);
	return true;
}

#pragma endregion

/*** modeCalcDbClass **************************************************************************************************************************************************************************/

#pragma region modeCalcDbClass
//-----------------------------------------------------------------------------
// Name: modeCalcDbClass()
// Desc:
//-----------------------------------------------------------------------------
modeCalcDbClass::modeCalcDbClass()
{
}

//-----------------------------------------------------------------------------
// Name: createControls()
// Desc: 
//-----------------------------------------------------------------------------
void modeCalcDbClass::init()
{
	mw->miniMaxCalcDb->createControls();
}

//-----------------------------------------------------------------------------
// Name: activateMode()
// Desc: 
//-----------------------------------------------------------------------------
void modeCalcDbClass::activate()
{
	mw->miniMaxCalcDb->showControls(true);
}

//-----------------------------------------------------------------------------
// Name: deactivateMode()
// Desc: 
//-----------------------------------------------------------------------------
bool modeCalcDbClass::deactivate()
{
	// Abfrage ob berechnung abgebrochen werden  soll
	if (isCalculationOngoing()) {
		if (mw->ww->showMessageBox(L"CANCEL", L"Do you really want to cancel the database calculation?", MB_YESNO) == IDNO) {
			return false;
		}
	}
	mw->miniMaxCalcDb->showControls(false);
	return true;
}

//-----------------------------------------------------------------------------
// Name: isCalculationOngoing()
// Desc: 
//-----------------------------------------------------------------------------
bool modeCalcDbClass::isCalculationOngoing()
{
	return mw->miniMaxCalcDb->isCalculationOngoing();
}
#pragma endregion
