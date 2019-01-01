/*********************************************************************
	millField2D.h													  
 	Copyright (c) Thomas Weber. All rights reserved.				
	Licensed under the MIT License.
	https://github.com/madweasel/madweasels-cpp
\*********************************************************************/
#ifndef MILLFIELD2D_H
#define MILLFIELD2D_H

#include "wildWeasel\\wildWeasel.h"
#include "miniMax\\miniMaxWin.h"
#include "muehle.h"
#include "perfectKI.h"

// class which shows a mill field
class millField2D : public miniMaxGuiField, private wildWeasel::eventFollower
{
public:

	// initialization
	void									init								(wildWeasel::masterMind* ww, muehle& game, perfectKI& playerPerfect, wildWeasel::font2D& d3dFont2D, wildWeasel::alignment& newAlignment, wildWeasel::texture& textureField);

	// actions
	void									moveStone							(unsigned int from, unsigned int to, bool stoneMustBeRemovedBeforeMove, bool inSettingPhaseBeforeMove, unsigned int numberOfStoneSetBeforeMove, int currentPlayerBeforeMove);

	// virtual miniMaxGuiField function
	void									setAlignment						(wildWeasel::alignment& newAlignment) override;
	void									setVisibility						(bool visible) override;
	void									setState							(unsigned int curShowedLayer, miniMax::stateNumberVarType curShowedState) override;

	// setter
	void									setField							(int settingColor, bool enableUserInput);
	void									setFieldPosClickedFunc				(function<void(unsigned int)> fieldPosClickedFunc);
	void									setGameStatusText					(const wchar_t* newText);
	void									setGameStatusText					(const wstring& newText);
	void									showStateNumber						(bool newState);
	void									showPerfectMove						(bool newState);
	void									activateStonesOfCurrentPlayer		(unsigned int markedPosition, int settingColor, unsigned int pushFrom);
	void									deactivateAllStones					();
	void									updateStateNumber					();
	void									updatePerfectMoveInfo				();
	void									setAnimateStoneOnMove				(bool enabled) { animateStoneOnMove = enabled; };
	
	// getter
	float									getMoveAnimationDuration			();
	bool									isShowingStateNumber				() { return showingStateNumber; };
	bool									isShowingPerfectMove				() { return showingPerfectMove; };

private:

	// constants
	enum class								infoType							{ NORMAL, DIR_UP, DIR_DOWN, DIR_LEFT, DIR_RIGHT, JUMP_1, JUMP_2, JUMP_3, INVALID };															// function return values
	static const unsigned int				numDirections						= 4;																																		// used by ???
	wildWeasel::vector2						stonePosOnField[fieldStruct::size]	= {																																			// positioning of each stone on the field
																						{.030f, .020f},									{.500f, .020f},									{.979f, .020f},
																										{.166f, .166f},					{.500f, .166f},					{.833f, .166f},
																														{.333f, .333f},	{.500f, .333f},	{.666f, .333f},	
																						{.030f, .500f},	{.166f, .500f},	{.333f, .500f},					{.666f, .500f},	{.833f, .500f},	{.979f, .500f},
																														{.333f, .666f},	{.500f, .666f},	{.666f, .666f},	
																										{.166f, .833f},					{.500f, .833f},					{.833f, .833f},
																						{.030f, .979f},									{.500f, .979f},									{.979f, .979f}
																					};
	
	// variables
	wildWeasel::masterMind *				ww									= nullptr;											// contains all the winapi GUII stuff
	muehle *								game								= nullptr;											//
	perfectKI *								playerPerfect						= nullptr;											//
	wildWeasel::sprite2D					spriteField;																			// back ground, being the field 
	vector<wildWeasel::plainButton2D>		buttonFieldPosition;																	// button a each stone position on the field
	vector<wildWeasel::plainButton2D>		buttonWhiteStones;																		// all white stones
	vector<wildWeasel::plainButton2D>		buttonBlackStones;																		// all black stones
	vector<wildWeasel::sprite2D>			spritePerfectMove;																		// for each possible move an icon is shown
	vector<wildWeasel::sprite2D>			spriteJumpingStone;																		// marker for each jumping stone
	wildWeasel::plainButton2D				buttonCurrentPlayer;																	// image of a white or black stone indicating the current player
	wildWeasel::plainButton2D*				stoneOnFieldPos[fieldStruct::size];														// table mapping the position by index to the buttons for each position
	wildWeasel::textLabel2D					labelStateNumber;																		// text showing the layer and state number of the perfect playing AI
	wildWeasel::textLabel2D					labelGameState;																			// indicates the game state in view of the perfect AI
	wildWeasel::textLabel2D					labelGameStatus;																		// text showing the current state of the field
	wildWeasel::color						colTextGameStatus					= wildWeasel::color::gray;							// text color
	function<void(unsigned int)>			fieldPosClickedFunc					= nullptr;											// user defined functions called when a stone or field position was clicked
	bool									showingPerfectMove					= false;											// checkbox state 
	bool									showingStateNumber					= false;											// checkbox state 
	bool									animateStoneOnMove					= true;												// move stones instantly or with a smooth linear animation to its target location
	float									stoneSizeFraction					= 0.08f;											// stone on the field
	float									stoneSizeFractionInStock			= 0.08f;											// stone in the stock
	float									stoneOverlapFractionInStock			= 0.5f;												// as fraction of a stone size
	float									perfectMoveIconSizeFractionOfStone	= 0.60f;											// the icons on the stone are smaller than the stones
	float									stoneMoveDuration					= 0.3f;												// in seconds
	float									stoneRemovalDuration				= 2.0f;												// in seconds
	float									perfectMoveTextSize					= 0.001f;											// 
	unsigned int							stoneRemovalBlinkTimes				= 5;												// when a stone is removed it blinks

	// alignment
	vector<wildWeasel::alignment>			alignmentStonesOnField;
	vector<wildWeasel::alignment>			alignmentNormalStoneIcon;
	vector<wildWeasel::alignment>			alignmentJumpingStoneIcon;
	wildWeasel::alignment					alignmentField						= { wildWeasel::alignmentTypeX::FRACTION, 0.02f, wildWeasel::alignmentTypeY::FRACTION, 0.05f, wildWeasel::alignmentTypeX::FRACTION_WIDTH, 0.7f,						wildWeasel::alignmentTypeY::FRACTION_HEIGHT, 0.90f,						wildWeasel::alignmentTypeX::USER,		0};	
	wildWeasel::alignment					alignmentStoneStock					= { wildWeasel::alignmentTypeX::FRACTION, 0.91f, wildWeasel::alignmentTypeY::FRACTION, 0.03f, wildWeasel::alignmentTypeX::FRACTION_WIDTH, stoneSizeFractionInStock, wildWeasel::alignmentTypeY::FRACTION_HEIGHT, stoneSizeFractionInStock,	wildWeasel::alignmentTypeX::FRACTION,   0, wildWeasel::alignmentTypeY::FRACTION,	-stoneOverlapFractionInStock * stoneSizeFractionInStock,	1};
	wildWeasel::alignment					alignmentGameStatusText				= { wildWeasel::alignmentTypeX::FRACTION, 0.73f, wildWeasel::alignmentTypeY::FRACTION, 0.40f, wildWeasel::alignmentTypeX::FRACTION_WIDTH, 0.1f,						wildWeasel::alignmentTypeY::FRACTION_HEIGHT, 0.05f,						wildWeasel::alignmentTypeX::USER,		0};	
	wildWeasel::alignment					alignmentCurrentPlayer				= { wildWeasel::alignmentTypeX::FRACTION, 0.91f, wildWeasel::alignmentTypeY::FRACTION, 0.46f, wildWeasel::alignmentTypeX::FRACTION_WIDTH, stoneSizeFractionInStock, wildWeasel::alignmentTypeY::FRACTION_HEIGHT, stoneSizeFractionInStock,	wildWeasel::alignmentTypeX::USER,		0};	
	wildWeasel::alignment					alignmentStateNumber				= { wildWeasel::alignmentTypeX::FRACTION, 0.01f, wildWeasel::alignmentTypeY::FRACTION, 0.01f, wildWeasel::alignmentTypeX::FRACTION_WIDTH, stoneSizeFractionInStock, wildWeasel::alignmentTypeY::FRACTION_HEIGHT, 0.05f,						wildWeasel::alignmentTypeX::USER,		0};	
	wildWeasel::alignment					alignmentGameState					= { wildWeasel::alignmentTypeX::FRACTION, 0.73f, wildWeasel::alignmentTypeY::FRACTION, 0.85f, wildWeasel::alignmentTypeX::FRACTION_WIDTH, stoneSizeFractionInStock, wildWeasel::alignmentTypeY::FRACTION_HEIGHT, 0.05f,						wildWeasel::alignmentTypeX::USER,		0};	

	// textures
	wildWeasel::buttonImageFiles			buttonImagesWhiteStone				= { L"WhiteStone_______normal.png",  1, 0, L"WhiteStone_______mouseOver.png", 5, 100, L"WhiteStone_______mouseLeave.png", 5, 100, L"WhiteStone_______pressed.png", 5, 100, L"WhiteStone_______grayedOut.png", 1, 0};
	wildWeasel::buttonImageFiles			buttonImagesBlackStone				= { L"BlackStone_______normal.png",  1, 0, L"BlackStone_______mouseOver.png", 5, 100, L"BlackStone_______mouseLeave.png", 5, 100, L"BlackStone_______pressed.png", 5, 100, L"BlackStone_______grayedOut.png", 1, 0};
	wildWeasel::buttonImageFiles			buttonImagesWhiteStoneMarked		= { L"WhiteStoneMarked_normal.png",  1, 0, L"WhiteStoneMarked_mouseOver.png", 5, 100, L"WhiteStoneMarked_mouseLeave.png", 5, 100, L"WhiteStoneMarked_pressed.png", 5, 100, L"WhiteStoneMarked_grayedOut.png", 1, 0};
	wildWeasel::buttonImageFiles			buttonImagesBlackStoneMarked		= { L"BlackStoneMarked_normal.png",  1, 0, L"BlackStoneMarked_mouseOver.png", 5, 100, L"BlackStoneMarked_mouseLeave.png", 5, 100, L"BlackStoneMarked_pressed.png", 5, 100, L"BlackStoneMarked_grayedOut.png", 1, 0};
	wildWeasel::buttonImageFiles			buttonImagesFieldPos				= { L"fieldPos_________normal.png",  1, 0, L"fieldPos_________mouseOver.png",10, 100, L"fieldPos_________mouseLeave.png",10, 100, L"fieldPos_________pressed.png",10, 100, L"fieldPos_________grayedOut.png", 1, 0};
	wildWeasel::texture						textureValueLost;
	wildWeasel::texture						textureValueDrawn;
	wildWeasel::texture						textureValueWon;
	wildWeasel::texture						textureNumberOne;
	wildWeasel::texture						textureNumberTwo;
	wildWeasel::texture						textureNumberThree;

	// functions
	void									stoneClicked						(void* pUser);										// function called when the user clicks on a stone button
	void									fieldPosClicked						(void* pUser);										// function called when the user clicks on a field position button
	void									setStoneOnPos						(wildWeasel::plainButton2D& theStone, unsigned int stonePos);
	void									setStoneOnStock						(wildWeasel::plainButton2D& theStone, unsigned int stockPos, int player);
	void									setPerfectMoveInfo					(unsigned int position, infoType direction, unsigned char moveValue, int freqValuesSubMovesWon, int freqValuesSubMovesDrawn, int freqValuesSubMovesLost, unsigned short plyInfo, wildWeasel::color textColor);
	void									windowSizeChanged					(int xSize, int ySize);
};

#endif