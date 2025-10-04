/*********************************************************************
	millField2D.h													  
 	Copyright (c) Thomas Weber. All rights reserved.				
	Licensed under the MIT License.
	https://github.com/madweasel/madweasels-cpp
\*********************************************************************/
#ifndef MILLFIELD2D_H
#define MILLFIELD2D_H

#include "wildWeasel/src/wildWeasel.h"
#include "../muehle.h"
#include "../ai/perfectAI.h"
#include "miniMax/src/gui/winInspectDb.h"
#include "miniMax/src/gui/winCalcDb.h"

// class which shows a mill field
class millField2D : public miniMax::miniMaxGuiField, private wildWeasel::eventFollower
{
public:

	// initialization
	void									init								(wildWeasel::masterMind* ww, muehle& game, perfectAI& playerPerfect, wildWeasel::font2D& d3dFont2D, wildWeasel::alignment& newAlignment, wildWeasel::texture& textureField);

	// actions
	void									moveStone							(unsigned int from, unsigned int to, bool stoneMustBeRemovedBeforeMove, bool inSettingPhaseBeforeMove, unsigned int numberOfStoneSetBeforeMove, playerId currentPlayerBeforeMove);
	void 									startStoneMoveAnimation				(unsigned int from, unsigned int to, bool stoneMustBeRemovedBeforeMove, bool inSettingPhaseBeforeMove, unsigned int numberOfStoneSetBeforeMove, playerId currentPlayerBeforeMove);

	// override virtual miniMaxGuiField functions
	void									setAlignment						(wildWeasel::alignment& newAlignment) override;
	void									setVisibility						(bool visible) override;
	void									setState							(unsigned int curShowedLayer, miniMax::stateNumberVarType curShowedState, unsigned char symOp, unsigned int curPlayer) override;

	// setter
	void									setField							(playerId settingColor, bool enableUserInput);
	void									setFieldPosClickedFunc				(function<void(unsigned int)> fieldPosClickedFunc);
	void									setGameStatusText					(const wchar_t* newText);
	void									setGameStatusText					(const wstring& newText);
	void									updateGameStatusText				(bool currentPlayerIsHuman);
	void									showStateNumber						(bool newState);
	void									showPerfectMove						(bool newState);
	void									activateStonesOfCurrentPlayer		(unsigned int markedPosition, playerId settingColor, unsigned int pushFrom);
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
	const wildWeasel::color					colTextGameStatus					= wildWeasel::color::gray();						// text color
	const float								stoneSizeFraction					= 0.08f;											// stone on the field
	const float								stoneSizeFractionInStock			= 0.08f;											// stone in the stock
	const float								stoneOverlapFractionInStock			= 0.5f;												// as fraction of a stone size
	const float								perfectMoveIconSizeFractionOfStone	= 0.60f;											// the icons on the stone are smaller than the stones
	const float								stoneMoveDuration					= 0.3f;												// in seconds
	const float								stoneRemovalDuration				= 2.0f;												// in seconds
	const float								perfectMoveTextSize					= 0.0005f;											// 
	const float								gameStateTextSize					= 0.001f;											// 
	const unsigned int						stoneRemovalBlinkTimes				= 5;												// when a stone is removed it blinks
	wildWeasel::vector2						stonePosOnField[fieldStruct::size]	= {																																			// positioning of each stone on the field
																						{.030f, .020f},									{.500f, .020f},									{.979f, .020f},
																										{.166f, .166f},					{.500f, .166f},					{.833f, .166f},
																														{.333f, .333f},	{.500f, .333f},	{.666f, .333f},	
																						{.030f, .500f},	{.166f, .500f},	{.333f, .500f},					{.666f, .500f},	{.833f, .500f},	{.979f, .500f},
																														{.333f, .666f},	{.500f, .666f},	{.666f, .666f},	
																										{.166f, .833f},					{.500f, .833f},					{.833f, .833f},
																						{.030f, .979f},									{.500f, .979f},									{.979f, .979f}
																					};

	// helper class to translate from miniMax::stateInfo to perfectMoveInfo for showPerfectMoveInfo() function
	class perfectMoveInfo
	{
	public:
		enum class							infoType							{ CENTER, DIR_UP, DIR_DOWN, DIR_LEFT, DIR_RIGHT, JUMP_SRC_1, JUMP_SRC_2, JUMP_SRC_3, JUMP_DST_1, JUMP_DST_2, JUMP_DST_3, INVALID };															// function return values
		static const fieldStruct::Array2d<infoType, fieldStruct::size, fieldStruct::size> direction;

		fieldStruct::fieldPos 				position;
		infoType 							type;
		miniMax::twoBit 					moveValue;
		unsigned int 						freqValuesSubMovesWon;
		unsigned int 						freqValuesSubMovesDrawn;
		unsigned int 						freqValuesSubMovesLost;
		miniMax::plyInfoVarType				plyInfo;
		wildWeasel::color 					textColor;

											perfectMoveInfo						(fieldStruct::fieldPos position, infoType type, miniMax::twoBit moveValue, unsigned int freqValuesSubMovesWon, unsigned int freqValuesSubMovesDrawn, unsigned int freqValuesSubMovesLost, miniMax::plyInfoVarType plyInfo);
											perfectMoveInfo 					(const miniMax::possibilityInfo &choice, const moveInfo &move, bool mustStoneBeRemoved, bool inSettingPhase, bool inJumpPhase, const vector<infoType>& mapJumpFromToType);

		static void 						initJumpMapping						(const miniMax::stateInfo & infoAboutChoices, std::vector<infoType>& mapJumpFromToType, set<fieldStruct::fieldPos>& jumpFromPositions);
		static void 						removeDuplicates					(vector<moveInfo>& stoneRemovalList);
		static void 						addBestMovesForClosedMills			(const vector<moveInfo>& stoneRemovalList, vector<perfectMoveInfo> &pmis, const miniMax::stateInfo &infoAboutChoices, bool mustStoneBeRemoved, bool inSettingPhase, bool inJumpPhase, const vector<infoType>& mapJumpFromToType);
		static bool 						calcPerfectMoveInfo 				(vector<perfectMoveInfo>& pmis, const miniMax::stateInfo &infoAboutChoices, bool mustStoneBeRemoved, bool inSettingPhase, bool inJumpPhase);
	};

	// variables
	wildWeasel::masterMind *				ww									= nullptr;											// contains all the winapi GUII stuff
	muehle *								game								= nullptr;											// object containing the game logic
	perfectAI *								playerPerfect						= nullptr;											// object containing the perfect move information
	function<void(unsigned int)>			fieldPosClickedFunc					= nullptr;											// user defined functions called when a stone or field position was clicked
	bool									showingPerfectMove					= false;											// checkbox state 
	bool									showingStateNumber					= false;											// checkbox state 
	bool									animateStoneOnMove					= true;												// move stones instantly or with a smooth linear animation to its target location

	// gui elements	
	vector<wildWeasel::sprite2D>			spriteField;																			// back ground, being the field 
	vector<wildWeasel::plainButton2D>		buttonCurrentPlayer;																	// image of a white or black stone indicating the current player
	vector<wildWeasel::plainButton2D>		buttonFieldPosition;																	// button a each stone position on the field
	vector<wildWeasel::plainButton2D>		buttonWhiteStones;																		// all white stones
	vector<wildWeasel::plainButton2D>		buttonBlackStones;																		// all black stones
	vector<wildWeasel::sprite2D>			spritePerfectMove;																		// for each possible move an icon is shown
	vector<wildWeasel::sprite2D>			spriteJumpingStone;																		// marker for each jumping stone
	wildWeasel::plainButton2D*				stoneOnFieldPos[fieldStruct::size];														// table mapping the position by index to the buttons for each position
	wildWeasel::textLabel2D					labelStateNumber;																		// text showing the layer and state number of the perfect playing AI
	wildWeasel::textLabel2D					labelGameState;																			// indicates the game state in view of the perfect AI
	wildWeasel::textLabel2D					labelGameStatus;																		// text showing the current state of the field

	// alignment
	vector<wildWeasel::alignment>			alignmentStonesOnField;
	vector<wildWeasel::alignment>			alignmentNormalStoneIcon;
	vector<wildWeasel::alignment>			alignmentJumpingStoneIcon;
	wildWeasel::alignment					alignmentField						= { wildWeasel::alignmentTypeX::FRACTION, 0.02f, wildWeasel::alignmentTypeY::FRACTION, 0.05f, wildWeasel::alignmentTypeX::FRACTION_WIDTH, 0.7f,						wildWeasel::alignmentTypeY::FRACTION_HEIGHT, 0.90f,						wildWeasel::alignmentTypeX::USER,		0};	
	wildWeasel::alignment					alignmentStoneStock					= { wildWeasel::alignmentTypeX::FRACTION, 0.91f, wildWeasel::alignmentTypeY::FRACTION, 0.03f, wildWeasel::alignmentTypeX::FRACTION_WIDTH, stoneSizeFractionInStock, wildWeasel::alignmentTypeY::FRACTION_HEIGHT, stoneSizeFractionInStock,	wildWeasel::alignmentTypeX::FRACTION,   0, wildWeasel::alignmentTypeY::FRACTION,	-stoneOverlapFractionInStock * stoneSizeFractionInStock,	1};
	wildWeasel::alignment					alignmentGameStatusText				= { wildWeasel::alignmentTypeX::FRACTION, 0.75f, wildWeasel::alignmentTypeY::FRACTION, 0.40f, wildWeasel::alignmentTypeX::FRACTION_WIDTH, 0.1f,						wildWeasel::alignmentTypeY::FRACTION_HEIGHT, 0.05f,						wildWeasel::alignmentTypeX::USER,		0};	
	wildWeasel::alignment					alignmentCurrentPlayer				= { wildWeasel::alignmentTypeX::FRACTION, 0.91f, wildWeasel::alignmentTypeY::FRACTION, 0.46f, wildWeasel::alignmentTypeX::FRACTION_WIDTH, stoneSizeFractionInStock, wildWeasel::alignmentTypeY::FRACTION_HEIGHT, stoneSizeFractionInStock,	wildWeasel::alignmentTypeX::USER,		0};	
	wildWeasel::alignment					alignmentStateNumber				= { wildWeasel::alignmentTypeX::FRACTION, 0.01f, wildWeasel::alignmentTypeY::FRACTION, 0.01f, wildWeasel::alignmentTypeX::FRACTION_WIDTH, stoneSizeFractionInStock, wildWeasel::alignmentTypeY::FRACTION_HEIGHT, 0.05f,						wildWeasel::alignmentTypeX::USER,		0};	
	wildWeasel::alignment					alignmentGameState					= { wildWeasel::alignmentTypeX::FRACTION, 0.75f, wildWeasel::alignmentTypeY::FRACTION, 0.82f, wildWeasel::alignmentTypeX::FRACTION_WIDTH, stoneSizeFractionInStock, wildWeasel::alignmentTypeY::FRACTION_HEIGHT, 0.05f,						wildWeasel::alignmentTypeX::USER,		0};	

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
	void									setStoneOnStock						(wildWeasel::plainButton2D& theStone, unsigned int stockPos, playerId player);
	void									showPerfectMoveInfo					(const perfectMoveInfo& pmi);
	void									windowSizeChanged					(int xSize, int ySize) override;
};

#endif