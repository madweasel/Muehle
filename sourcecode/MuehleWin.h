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

//#define DH_USE_DEBUG_HELPER
//#define CHECK_VARPARAM_DETAILS

#include "wildWeasel\\wildWeasel.h"
 #include <tchar.h>
#include <filesystem>
#include "resource.h"
#include "muehle.h"
#include "minMaxKI.h"
#include "randomKI.h"
#include "perfectKI.h"
#include "millField2D.h"
#include "miniMax\\miniMaxWin.h"

// user is playing a game (either by himself or by bots)
struct modePlayGameClass : wildWeasel::guiScenario
{
	static const unsigned int				newGameYetBlank						= 1;
	static const unsigned int				playingGame							= 2;

	wildWeasel::alignment					alignmentGameField					= { wildWeasel::alignmentTypeX::FRACTION,  0.02f, wildWeasel::alignmentTypeY::FRACTION,    0.02f, wildWeasel::alignmentTypeX::FRACTION, 0.98f, wildWeasel::alignmentTypeY::FRACTION, 0.98f};	
	unsigned int							gameState							= 0;
	unsigned int							pushFrom							= fieldStruct::size;								// variables for moving stone
	unsigned int							pushTo								= fieldStruct::size;								// variables for moving stone
	wildWeasel::timer						botTimer;
	DWORD									botTimerTime						= 500;												// pause in milliseconds, before computer makes a move

	void									init								();
	void									activate							();
	void									deactivate							();
	void									release								()	{};

	void									fieldPosClicked						(unsigned int clickOnFieldPos);
	void									letComputerPlay						();
	static void								botTimerFunc						(void* pUser);
	void									changeMoveSpeed						(UINT wmId, DWORD duration);
	void									unCheckAllPlayerOne					();
	void									unCheckAllPlayerTwo					();
	void									changeBot							(UINT wmId, int player, muehleKI *bot);
	bool									isCurrentPlayerHuman				();
	void									showWinnerAnimation					();
	void									undoMove							();
	void									processMove							();

} modePlayGame;

// user is setting up a custom game state
struct modeSetupFieldClass : wildWeasel::guiScenario, public wildWeasel::eventFollower
{
	int										settingColor						= 0;												// can be fieldStruct::playerWhite, 0, fieldStruct::playerBlack

	void									init								();
	void									activate							();
	void									deactivate							();
	void									release								()	{};

	void									fieldPosClicked						(unsigned int clickOnFieldPos);
	void									rightMouseButtonPressed				(int xPos, int yPos);
} modeSetupField;

// user is viewing infos about the database
struct modeInspectDbClass : wildWeasel::guiScenario
{
	wildWeasel::alignment					amAreaInspectDb						= { wildWeasel::alignmentTypeX::FRACTION,  0.02f, wildWeasel::alignmentTypeY::FRACTION,    0.02f, wildWeasel::alignmentTypeX::FRACTION, 0.98f, wildWeasel::alignmentTypeY::FRACTION, 0.98f};	

	void									init								();
	void									activate							();
	void									deactivate							();
	void									release								()	{};
} modeInspectDb;

// controls for the calculation of the database are shown
struct modeCalcDbClass : wildWeasel::guiScenario
{
	wildWeasel::alignment					amAreaCalculation					= { wildWeasel::alignmentTypeX::FRACTION,  0.00f, wildWeasel::alignmentTypeY::FRACTION,    0.00f, wildWeasel::alignmentTypeX::FRACTION, 1.00f, wildWeasel::alignmentTypeY::FRACTION, 1.00f};	

	bool									isCalculationOngoing				();

	void									init								();
	void									activate							();
	void									deactivate							();
	void									release								()	{};
} modeCalcDb;

muehle*										myGame								= nullptr;			
perfectKI*									playerPerfect						= nullptr;
minMaxKI*									playerMinMax						= nullptr;	
muehleKI*									playerRandom						= nullptr;	
muehleKI*									playerHuman							= nullptr;	
muehleKI*									playerOne							= nullptr;
muehleKI*									playerTwo							= nullptr;
miniMaxWinInspectDb*						miniMaxInspectDb					= nullptr;							// object with functions for the inspection and calculation of the database
miniMaxWinCalcDb*							miniMaxCalcDb						= nullptr;							// object with functions for the inspection and calculation of the database
wildWeasel::masterMind *					ww									= nullptr;							// contains all the winapi GUII stuff
millField2D									guiField;
vector<float>								loadingScreenFractions;													// fraction of the loading time between each 

POINT										defaultWindowSize					= {800, 600};
POINT										minimumWindowSize					= {800, 600};
bool										expertMode							= false;
bool										showDisclaimerOnStart				= true;
wstring										strDatabaseDir						= L"";
wstring										strFontFilename						= L"SegoeUI_18.spritefont";
wstring										strTexturePath						= L"textures";
wstring										strWhiteDummyFilename				= L"whiteDummy.bmp";
wstring										strBackgroundFilename				= L"background.jpg";
wstring										strLoadingFilename					= L"loading.jpg";
wildWeasel::buttonImageFiles				buttonImagesVoid					= { L"button_Void___normal.png",     1, 0, L"button_Void___mouseOver.png",    10, 100, L"button_Void___mouseLeave.png",    10, 100, L"button_Void___pressed.png",    10, 100, L"button_Void___grayedOut.png",    1, 0};
wildWeasel::buttonImageFiles				buttonImagesArrow					= { L"button_Arrow__normal.png",     1, 0, L"button_Arrow__mouseOver.png",    10, 100, L"button_Arrow__mouseLeave.png",    10, 100, L"button_Arrow__pressed.png",    10, 100, L"button_Arrow__grayedOut.png",    1, 0};
wildWeasel::texture							textureBackground;
wildWeasel::texture							textureLoading;
wildWeasel::texture							textureField;
wildWeasel::texture							textureLine;
wildWeasel::texture							textureCorner;
wildWeasel::font2D							d3dFont2D;
wildWeasel::font3D							d3dFont3D;	

// Global Functions:
LRESULT CALLBACK							WndProc								(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK							About								(HWND, UINT, WPARAM, LPARAM);

// init functions
void										loadDefaultSettings					(void);
void										saveDefaultSettings					(void);
void										mainInitFunc						(void* pUser);

#endif