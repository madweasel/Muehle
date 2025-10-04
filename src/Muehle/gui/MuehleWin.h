/*********************************************************************
	MuehleWin.cpp													  
 	Copyright (c) Thomas Weber. All rights reserved.				
	Licensed under the MIT License.
	https://github.com/madweasel/madweasels-cpp
\*********************************************************************/
#ifndef MUEHLEWIN_H
#define MUEHLEWIN_H

/*** General information *****
- Player One has black stones
- Player Two has white stones
******************************/

#include <tchar.h>
#include <filesystem>

#include "wildWeasel/src/wildWeasel.h"
#include "resource.h"
#include "../muehle.h"
#include "../ai/minMaxAI.h"
#include "../ai/randomAI.h"
#include "../ai/perfectAI.h"
#include "millField2D.h"
#include "historyList.h"
#include "miniMax/src/gui/winInspectDb.h"
#include "miniMax/src/gui/winCalcDb.h"

// user is playing a game (either by himself or by bots)
struct modePlayGameClass : wildWeasel::guiScenario
{
	enum class gameStateEnum  { undefined = 0, newGameYetBlank, settingStone, removingStone, movingStone, stoneSelectedForMove, animatingStone, gameFinished };

	fieldStruct::fieldPos 					animateFrom							= fieldStruct::size;								// positions of the stone which is animated
	fieldStruct::fieldPos 					animateTo							= fieldStruct::size;								// positions of the stone which is animated
	gameStateEnum							gameState							= gameStateEnum::undefined;							// state of the game, indicating if the game is already running
	moveInfo								curMove;																				// current move, which is being processed
	wildWeasel::timer						moveTimer;																				// timer until the movement animation of a stone is finished
	wildWeasel::timer						botTimer;																				// timer until the next move of the computer
	DWORD									botTimerTime						= 500;												// pause in milliseconds, before computer makes a move
	wildWeasel::masterMind&					ww;
	millField2D&							guiField;
	historyList&							guiHistory;
	muehle&									myGame;

											modePlayGameClass					();

	void									init								();
	void									activate							() override;
	bool									deactivate							() override;
	void									release								()	{};

	void									fieldPosClicked						(unsigned int clickOnFieldPos);
	void									letComputerPlay						();
	static void								botTimerFunc						(void* pUser);
	static void								moveTimerFunc						(void* pUser);
	void									changeMoveSpeed						(UINT wmId, DWORD duration);
	void									unCheckAllPlayerOne					();
	void									unCheckAllPlayerTwo					();
	void									changeBot							(UINT wmId, playerId player, muehleAI *bot);
	bool									isCurrentPlayerHuman				();
	void									showWinnerAnimation					();
	void									redoMove							();
	void									undoMove							();
	void									initiateStoneAnimation							();
	void									stoneAnimationFinished						();
};

// user is setting up a custom game state
struct modeSetupFieldClass : wildWeasel::guiScenario, public wildWeasel::eventFollower
{
	millField2D&							guiField;
	wildWeasel::masterMind&					ww;
	muehle&									myGame;
	wildWeasel::textLabel2D					labelSetup;
	const wildWeasel::color					colTextSetup						= wildWeasel::color::gray();						// text color
	wildWeasel::alignment					alignmentSetupText					= { wildWeasel::alignmentTypeX::FRACTION, 0.3f, wildWeasel::alignmentTypeY::FRACTION, 0.01f, wildWeasel::alignmentTypeX::FRACTION_WIDTH, 0.2f, wildWeasel::alignmentTypeY::FRACTION_HEIGHT, 0.1f, wildWeasel::alignmentTypeX::USER, 0};	

											modeSetupFieldClass					();

	void									init								();
	void									activate							() override;
	bool									deactivate							() override;
	void									release								()	{};

	void									fieldPosClicked						(unsigned int clickOnFieldPos);
	void									rightMouseButtonPressed				(int xPos, int yPos);
};

// user is viewing infos about the database
struct modeInspectDbClass : wildWeasel::guiScenario
{
											modeInspectDbClass					();

	void									init								();
	void									activate							() override;
	bool									deactivate							() override;
	void									release								()	{};
};

// controls for the calculation of the database are shown
struct modeCalcDbClass : wildWeasel::guiScenario
{
											modeCalcDbClass						();
	bool									isCalculationOngoing				();

	void									init								();
	void									activate							() override;
	bool									deactivate							() override;
	void									release								()	{};
};

class MuehleWin : private wildWeasel::eventFollower
{
public:
	// constructor / destructor
											MuehleWin							();
											~MuehleWin							();

	// modes
	modePlayGameClass*						modePlayGame						= nullptr;							// object for playing a game
	modeSetupFieldClass*					modeSetupField						= nullptr;							// object for setting up a custom game state
	modeInspectDbClass*						modeInspectDb						= nullptr;							// object for inspecting the database
	modeCalcDbClass*						modeCalcDb							= nullptr;							// object for calculating the database

	// principal objects
	muehle*									myGame								= nullptr;							// object containing the game logic
	perfectAI*								playerPerfect						= nullptr;							// perfect player
	minMaxAI*								playerMinMax						= nullptr;							// minMax player
	muehleAI*								playerRandom						= nullptr;							// random player
	muehleAI*								playerHuman							= nullptr;							// human player
	muehleAI*								playerOne							= nullptr; 							// player one
	muehleAI*								playerTwo							= nullptr;							// player two
	miniMax::miniMaxWinInspectDb*			miniMaxInspectDb					= nullptr;							// object with functions for the inspection and calculation of the database
	miniMax::miniMaxWinCalcDb*				miniMaxCalcDb						= nullptr;							// object with functions for the inspection and calculation of the database
	wildWeasel::masterMind*					ww									= nullptr;							// contains all the winapi GUII stuff
	millField2D								guiField;																// object for the graphical representation of the field
	historyList 							guiHistory;

	// alignment areas
	wildWeasel::alignment					amAreaInspectDb						= { wildWeasel::alignmentTypeX::FRACTION,  0.02f, wildWeasel::alignmentTypeY::FRACTION,    0.02f, wildWeasel::alignmentTypeX::FRACTION, 0.98f, wildWeasel::alignmentTypeY::FRACTION, 0.98f};	
	wildWeasel::alignment					amAreaCalculation					= { wildWeasel::alignmentTypeX::FRACTION,  0.00f, wildWeasel::alignmentTypeY::FRACTION,    0.00f, wildWeasel::alignmentTypeX::FRACTION, 1.00f, wildWeasel::alignmentTypeY::FRACTION, 1.00f};	
	wildWeasel::alignment					alignmentGameField					= { wildWeasel::alignmentTypeX::FRACTION,  0.02f, wildWeasel::alignmentTypeY::FRACTION,    0.02f, wildWeasel::alignmentTypeX::FRACTION, 0.98f, wildWeasel::alignmentTypeY::FRACTION, 0.98f};
	wildWeasel::alignment 					alignmentHistory 					= { wildWeasel::alignmentTypeX::FRACTION,  0.02f, wildWeasel::alignmentTypeY::FRACTION,    0.02f, wildWeasel::alignmentTypeX::PIXEL_WIDTH, 250, wildWeasel::alignmentTypeY::FRACTION, 0.98f};

	// settings
	playerId								settingColor						= playerId::squareIsFree;			// can be fieldStruct::playerWhite, 0, fieldStruct::playerBlack
	vector<float>							loadingScreenFractions;													// fraction of the loading time between each 
	POINT									defaultWindowSize					= {800, 600};						// default window size, if not mentioned in the .xml file
	POINT									minimumWindowSize					= {800, 600};						// minimum window size in pixels
	bool									expertMode							= false;							// expert mode shows additional buttons, e.g. for the calculation of the database
	bool 									showHistoryList						= false;							// show the move history list
	bool									showDisclaimerOnStart				= true;								// show disclaimer on start
	bool 									resetFieldOnStart					= true;								// reset field when starting a new game
	unsigned int 							numMovesToRemis						= 205;								// number of moves after which the game is remis

	// strings
	wstring									strDatabaseDir						= L"./database/";					// directory containing the database files
	wstring									strFontFilename						= L"SegoeUI_18.spritefont";			// font file
	wstring									strTexturePath						= L"textures";						// path to the textures
	wstring									strWhiteDummyFilename				= L"whiteDummy.bmp";				// dummy texture for white stones
	wstring									strBackgroundFilename				= L"background.jpg";				// background texture filename
	wstring									strLoadingFilename					= L"loading.jpg";					// loading texture filename

	// textures
	wildWeasel::buttonImageFiles			buttonImagesVoid					= { L"button_Void___normal.png",     1, 0, L"button_Void___mouseOver.png",    10, 100, L"button_Void___mouseLeave.png",    10, 100, L"button_Void___pressed.png",    10, 100, L"button_Void___grayedOut.png",    1, 0};
	wildWeasel::buttonImageFiles			buttonImagesArrow					= { L"button_Arrow__normal.png",     1, 0, L"button_Arrow__mouseOver.png",    10, 100, L"button_Arrow__mouseLeave.png",    10, 100, L"button_Arrow__pressed.png",    10, 100, L"button_Arrow__grayedOut.png",    1, 0};
	wildWeasel::texture						textureBackground;
	wildWeasel::texture						textureLoading;
	wildWeasel::texture						textureField;
	wildWeasel::texture						textureLine;
	wildWeasel::texture						textureCorner;
	wildWeasel::font2D						d3dFont2D;
	wildWeasel::font3D						d3dFont3D;	

	// Window procedures
	static LRESULT CALLBACK					WndProc_static						(HWND, UINT, WPARAM, LPARAM);
	static INT_PTR CALLBACK					About_static						(HWND, UINT, WPARAM, LPARAM);
	static INT_PTR CALLBACK					Settings_static						(HWND, UINT, WPARAM, LPARAM);
	LRESULT CALLBACK						WndProc								(HWND, UINT, WPARAM, LPARAM);
	INT_PTR CALLBACK						About								(HWND, UINT, WPARAM, LPARAM);
    INT_PTR CALLBACK						Settings							(HWND, UINT, WPARAM, LPARAM);

    // functions
	void									loadDefaultSettings					(void);
	void									saveDefaultSettings					(void);
	void									mainInitFunc						();
	static void								mainInitFunc_static					(void* pUser);
	void  									run									(HINSTANCE hInstance, int nCmdShow);
	void									windowSizeChanged					(int xSize, int ySize) override;
};

// TODO: Check if https://stackoverflow.com/questions/77586105/cannot-compile-when-i-pass-a-member-function-to-winapis-callback helps to avoid the global variable
MuehleWin*									mw 									= nullptr;
int APIENTRY 								_tWinMain							(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow);

#endif