/*********************************************************************
	millField2D.cpp												  
 	Copyright (c) Thomas Weber. All rights reserved.				
	Licensed under the MIT License.
	https://github.com/madweasel/madweasels-cpp
\*********************************************************************/

#include "millField2D.h"

#pragma region millField2D

//-----------------------------------------------------------------------------
// Name: init()
// Desc: 
//-----------------------------------------------------------------------------
void millField2D::init(wildWeasel::masterMind* ww, muehle& game, perfectKI& playerPerfect, wildWeasel::font2D& d3dFont2D, wildWeasel::alignment& newAlignment, wildWeasel::texture& textureField)
{
	// pointers
	this->ww			= ww;
	this->game			= &game;
	this->playerPerfect	= &playerPerfect;
	
	followEvent(this, wildWeasel::eventFollower::eventType::WINDOWSIZE_CHANGED);

	// field
	spriteField.create(ww, wstring(L""), &d3dFont2D, 0);
	spriteField.setTexture(&textureField);
	spriteField.setState(wildWeasel::guiElemState::HIDDEN);

	// buttons on field
	buttonFieldPosition.resize(fieldStruct::size);
	for (auto& curFieldPos : buttonFieldPosition) {
		curFieldPos.create(ww, buttonImagesFieldPos, bind(&millField2D::fieldPosClicked, this, placeholders::_1), &curFieldPos, 0.5f);
		curFieldPos.setState(wildWeasel::guiElemState::HIDDEN);
	}
	
	// stones
	buttonWhiteStones.resize(fieldStruct::numStonesPerPlayer);
	buttonBlackStones.resize(fieldStruct::numStonesPerPlayer);
	
	for (auto& curStone : buttonWhiteStones) {
		curStone.create(ww, buttonImagesWhiteStone, bind(&millField2D::stoneClicked, this, placeholders::_1), &curStone, 0.9f);
		curStone.setState(wildWeasel::guiElemState::HIDDEN);
	}

	for (auto& curStone : buttonBlackStones) {
		curStone.create(ww, buttonImagesBlackStone, bind(&millField2D::stoneClicked, this, placeholders::_1), &curStone, 0.9f);
		curStone.setState(wildWeasel::guiElemState::HIDDEN);
	}

	// current player
	buttonCurrentPlayer.create(ww, buttonImagesWhiteStone, nullptr, nullptr, 1);
	buttonCurrentPlayer.setState(wildWeasel::guiElemState::HIDDEN);

	// game status info
	labelGameStatus.create(ww, wstring(L"Set a stone!"), &d3dFont2D);
	labelGameStatus.setTextColor(colTextGameStatus);
	labelGameStatus.setTextState(wildWeasel::guiElemState::VISIBLE);

	// mapping table
	for (auto& curStone : stoneOnFieldPos) {
		curStone = nullptr;
	}

	// state number
	labelStateNumber.create(ww, wstring(L"state number"), &d3dFont2D, 1.0f);
	labelStateNumber.setTextColor(colTextGameStatus);
	labelStateNumber.setTextState(wildWeasel::guiElemState::VISIBLE);

	// textures
	textureValueLost		.loadFile(ww, wstring(L"valueLost.png"			));
	textureValueWon			.loadFile(ww, wstring(L"valueWon.png"			));
	textureValueDrawn		.loadFile(ww, wstring(L"valueDrawn.png"			));
	textureNumberOne		.loadFile(ww, wstring(L"numberOne.png"			));
	textureNumberTwo		.loadFile(ww, wstring(L"numberTwo.png"			));
	textureNumberThree		.loadFile(ww, wstring(L"numberThree.png"		));

	// perfect jumping moves
	spriteJumpingStone.resize(3);
	for (auto& curMove : spriteJumpingStone) {
		curMove.create(ww, wstring(L""), &d3dFont2D, 1);
		curMove.setState(wildWeasel::guiElemState::HIDDEN);
	}
	spriteJumpingStone[0].setTexture(&textureNumberOne);
	spriteJumpingStone[1].setTexture(&textureNumberTwo);
	spriteJumpingStone[2].setTexture(&textureNumberThree);

	// perfect normal moves
	spritePerfectMove.resize(fieldStruct::size * 4);
	for (auto& curMove : spritePerfectMove) {
		curMove.create(ww, wstring(L""), &d3dFont2D, 1);
		curMove.setTexture(&textureValueDrawn);
		curMove.setState(wildWeasel::guiElemState::HIDDEN);
		curMove.setTextAlignmentVertical(wildWeasel::alignmentVertical::BELOW);
	}
	labelGameState.create(ww, wstring(L"game state"), &d3dFont2D, 1.0f);
	labelGameState.setTextColor(colTextGameStatus);
	labelGameState.setTextState(wildWeasel::guiElemState::VISIBLE);
	
	// alignments
	alignmentJumpingStoneIcon	.resize(fieldStruct::size);
	alignmentNormalStoneIcon	.resize(fieldStruct::size);
	alignmentStonesOnField		.resize(fieldStruct::size);

	// set alignmend for all gui elements
	setAlignment(newAlignment);

	// stones on field and stones in stock
	setField(false, false);
}

//-----------------------------------------------------------------------------
// Name: setFieldPosClickedFunc()
// Desc: 
//-----------------------------------------------------------------------------
void millField2D::setFieldPosClickedFunc(function<void(unsigned int)> newFieldPosClickedFunc)
{
	fieldPosClickedFunc = newFieldPosClickedFunc;
}

//-----------------------------------------------------------------------------
// Name: setStoneOnPos()
// Desc: 
//-----------------------------------------------------------------------------
void millField2D::setStoneOnPos(wildWeasel::plainButton2D& theStone, unsigned int stonePos)
{
	if (animateStoneOnMove) {
		theStone.setAlignmentWithAnimation(alignmentStonesOnField[stonePos], stoneMoveDuration, true);
	} else {
		theStone.setAlignment(alignmentStonesOnField[stonePos]);
	}
}

//-----------------------------------------------------------------------------
// Name: setStoneOnStock()
// Desc: 
//-----------------------------------------------------------------------------
void millField2D::setStoneOnStock(wildWeasel::plainButton2D& theStone, unsigned int stockPos, int player)
{
	if (stockPos >= fieldStruct::numStonesPerPlayer) return;

	unsigned int index = 0;

	if (player == fieldStruct::playerOne) {
		index = stockPos;
	} else {
		index = fieldStruct::numStonesPerPlayer + 5 + stockPos;
	}

	theStone.setAlignment(alignmentStoneStock, index);
}

//-----------------------------------------------------------------------------
// Name: setAlignment()
// Desc: 
//-----------------------------------------------------------------------------
void millField2D::setAlignment(wildWeasel::alignment& newAlignment)
{
	unsigned int curPos = 0;

	alignmentCurrentPlayer	.create(ww->alignmentRootFrame);
	alignmentField			.create(ww->alignmentRootFrame);
	alignmentGameStatusText	.create(ww->alignmentRootFrame);
	alignmentStateNumber	.create(ww->alignmentRootFrame);
	alignmentGameState		.create(ww->alignmentRootFrame);
	alignmentStoneStock		.create(ww->alignmentRootFrame);

	alignmentCurrentPlayer	.setInsideAnotherRect(newAlignment);
	alignmentField			.setInsideAnotherRect(newAlignment);
	alignmentGameStatusText	.setInsideAnotherRect(newAlignment);
	alignmentStateNumber	.setInsideAnotherRect(newAlignment);
	alignmentGameState		.setInsideAnotherRect(newAlignment);
	alignmentStoneStock		.setInsideAnotherRect(newAlignment);

	buttonCurrentPlayer		.setAlignment(alignmentCurrentPlayer	);
	spriteField				.setAlignment(alignmentField			);
	labelGameStatus			.setAlignment(alignmentGameStatusText	);
	labelStateNumber		.setAlignment(alignmentStateNumber		);
	labelGameState			.setAlignment(alignmentGameState		);

	// alignments for the stones on the field
	curPos = 0;
	for (auto& curAlignment : alignmentStonesOnField) {
		curAlignment.create		(ww->alignmentRootFrame);
		curAlignment.setSize	(wildWeasel::alignmentTypeX::FRACTION, stoneSizeFraction, wildWeasel::alignmentTypeY::FRACTION, stoneSizeFraction);
		curAlignment.setPosition(wildWeasel::alignmentTypeX::FRACTION, stonePosOnField[curPos].x, wildWeasel::alignmentHorizontal::CENTER, wildWeasel::alignmentTypeY::FRACTION, stonePosOnField[curPos].y, wildWeasel::alignmentVertical::CENTER);
		curAlignment.setInsideAnotherRect(alignmentField);
		curPos++;
	}

	// alignments for the icons showing the perferct move for normal moves
	curPos = 0;
	for (auto& curAlignment : alignmentNormalStoneIcon) {
		curAlignment.create		(ww->alignmentRootFrame);
		curAlignment.setSize	(wildWeasel::alignmentTypeX::FRACTION, perfectMoveIconSizeFractionOfStone, wildWeasel::alignmentTypeY::FRACTION, perfectMoveIconSizeFractionOfStone);
		curAlignment.setPosition(wildWeasel::alignmentTypeX::FRACTION, 0.5f, wildWeasel::alignmentHorizontal::LEFT, wildWeasel::alignmentTypeY::FRACTION, 0.5f, wildWeasel::alignmentVertical::TOP);
		curAlignment.setGrid	(wildWeasel::alignmentTypeX::FRACTION, 0.5f*(1-perfectMoveIconSizeFractionOfStone), wildWeasel::alignmentHorizontal::CENTER, wildWeasel::alignmentTypeY::FRACTION, 0.5f*(1-perfectMoveIconSizeFractionOfStone), wildWeasel::alignmentVertical::CENTER, 3);
		curAlignment.setInsideAnotherRect(alignmentStonesOnField[curPos]);
		curPos++;
	}

	// alignments for the icons showing the perferct move for jumping moves
	curPos = 0;
	for (auto& curAlignment : alignmentJumpingStoneIcon) {
		curAlignment.create		(ww->alignmentRootFrame);
		curAlignment.setSize	(wildWeasel::alignmentTypeX::FRACTION, perfectMoveIconSizeFractionOfStone, wildWeasel::alignmentTypeY::FRACTION, perfectMoveIconSizeFractionOfStone);
		curAlignment.setPosition(wildWeasel::alignmentTypeX::FRACTION, 0.5f, wildWeasel::alignmentHorizontal::LEFT, wildWeasel::alignmentTypeY::FRACTION, 0.5f, wildWeasel::alignmentVertical::TOP);
		curAlignment.setGrid	(wildWeasel::alignmentTypeX::FRACTION, -0.3f*perfectMoveIconSizeFractionOfStone, wildWeasel::alignmentHorizontal::CENTER, wildWeasel::alignmentTypeY::FRACTION, -0.3f*perfectMoveIconSizeFractionOfStone, wildWeasel::alignmentVertical::CENTER, 3);
		curAlignment.setInsideAnotherRect(alignmentStonesOnField[curPos]);
		curPos++;
	}

	// buttons on field, when no stone is there
	curPos = 0;
	for (auto& curFieldPos : buttonFieldPosition) {
		curFieldPos.setAlignment(alignmentStonesOnField[curPos]);
		curPos++;
	}

	windowSizeChanged(ww->getWindowSizeX(), ww->getWindowSizeX());

	// stones on field and stones in stock
	// setField(false, false);
}

//-----------------------------------------------------------------------------
// Name: setVisibility()
// Desc: 
//-----------------------------------------------------------------------------
void millField2D::setVisibility(bool visible)
{
	wildWeasel::guiElemState newSpriteState = (visible ? wildWeasel::guiElemState::  VISIBLE : wildWeasel::guiElemState::HIDDEN);
	wildWeasel::guiElemState newButtonState = (visible ? wildWeasel::guiElemState::INVISIBLE : wildWeasel::guiElemState::HIDDEN);

	spriteField.setState(newSpriteState);
	buttonCurrentPlayer.setState(newSpriteState);
	labelGameStatus.setTextState(newSpriteState);

	for (auto& curStone : buttonWhiteStones) {
		curStone.setState(newSpriteState);
	}

	for (auto& curStone : buttonBlackStones) {
		curStone.setState(newSpriteState);
	}

	for (auto& curFieldPos : buttonFieldPosition) {
		curFieldPos.setState(newButtonState);
	}	

	for (auto& curFieldPos : spritePerfectMove) {
		curFieldPos.setState(showingPerfectMove ? newSpriteState : wildWeasel::guiElemState::HIDDEN);
	}
	spriteJumpingStone[0].setState(showingPerfectMove ? newSpriteState : wildWeasel::guiElemState::HIDDEN);
	spriteJumpingStone[1].setState(showingPerfectMove ? newSpriteState : wildWeasel::guiElemState::HIDDEN);
	spriteJumpingStone[2].setState(showingPerfectMove ? newSpriteState : wildWeasel::guiElemState::HIDDEN);

	labelGameState.setState(showingPerfectMove ? newSpriteState : wildWeasel::guiElemState::HIDDEN);
	
	labelStateNumber.setState(showingStateNumber ? newSpriteState : wildWeasel::guiElemState::HIDDEN);
}

//-----------------------------------------------------------------------------
// Name: setState()
// Desc: 
//-----------------------------------------------------------------------------
void millField2D::setState(unsigned int curShowedLayer, miniMax::stateNumberVarType curShowedState)
{
	// draw field if necessary
	if (curShowedLayer != 0) {
		
		// locals
		fieldStruct		myField;
		bool			gameHasFinished;

		// get current field based on layer and state number
		myField.createField();
		playerPerfect->getField(curShowedLayer, curShowedState, &myField, &gameHasFinished);

		// set current field state to a mill game
		game->setCurrentGameState(&myField);

		// set gui elements on field
		setField(false, false);
			
		// status text
		if (game->getWinner() != 0) {
			setGameStatusText(L"Game has hinished!");
		} else if (game->mustStoneBeRemoved()) {
			setGameStatusText(L"Remove a stone!");
		} else if (game->inSettingPhase()) {
			setGameStatusText(L"Set a stone!");
		} else {
			setGameStatusText(L"Move a stone!");
		}

		// release temporary allocated field structure
		myField.deleteField();
	} else {
		setGameStatusText(L"");
	}

	// user input
	deactivateAllStones();

	// hide infos
	showStateNumber(false);
	showPerfectMove(false);
}

//-----------------------------------------------------------------------------
// Name: setField()
// Desc: 
//-----------------------------------------------------------------------------
void millField2D::setField(int settingColor, bool enableUserInput)
{
	// locals
	size_t		curWhiteStoneId = 0, curBlackStoneId = 0;
	
	// stones in the stock
	if (game->inSettingPhase() || settingColor) {

		unsigned int curStockPos;
		int numWhiteStonesResting;
		int numBlackStonesResting;

		if (settingColor && !game->inSettingPhase()) {
			numWhiteStonesResting = fieldStruct::numStonesPerPlayer - game->getNumStonOfOppPlayer();
			numBlackStonesResting = fieldStruct::numStonesPerPlayer - game->getNumStonOfCurPlayer();
		} else {
			game->calcNumberOfRestingStones(numWhiteStonesResting, numBlackStonesResting);
		}

		// white
		if (numWhiteStonesResting > 0) {
			for (curStockPos = 0; (int) curStockPos < numWhiteStonesResting; ++curStockPos, ++curWhiteStoneId) {
				setStoneOnStock(buttonWhiteStones[curWhiteStoneId], fieldStruct::numStonesPerPlayer - curStockPos - 1, fieldStruct::playerWhite);
			}
		}

		// black
		if (numBlackStonesResting > 0) {
			for (curStockPos = 0; (int) curStockPos < numBlackStonesResting; ++curStockPos, ++curBlackStoneId) {
				setStoneOnStock(buttonBlackStones[curBlackStoneId], curStockPos, fieldStruct::playerBlack);
			}
		}
	} 

	// stones on the field
	unsigned int	curPos	= 0;
	int				myField[fieldStruct::size];

	game->getField(myField);

	for (auto& curFieldValue : myField) {
		if (curFieldValue == fieldStruct::playerWhite) {
			setStoneOnPos(buttonWhiteStones[curWhiteStoneId], curPos);
			stoneOnFieldPos[curPos] = &buttonWhiteStones[curWhiteStoneId];
			curWhiteStoneId++;
		} else if (curFieldValue == fieldStruct::playerBlack) {
			setStoneOnPos(buttonBlackStones[curBlackStoneId], curPos);
			stoneOnFieldPos[curPos] = &buttonBlackStones[curBlackStoneId];
			curBlackStoneId++;
		}
		curPos++;
	}

	// make resting stones invisble
	while (curWhiteStoneId < fieldStruct::numStonesPerPlayer) {
		buttonWhiteStones[curWhiteStoneId].setState(wildWeasel::guiElemState::HIDDEN);
		curWhiteStoneId++;
	}
	while (curBlackStoneId < fieldStruct::numStonesPerPlayer) {
		buttonBlackStones[curBlackStoneId].setState(wildWeasel::guiElemState::HIDDEN);
		curBlackStoneId++;
	}

	// show current player
	if (settingColor) {
		buttonCurrentPlayer.setImageFiles(settingColor == fieldStruct::playerWhite ? buttonImagesWhiteStone : buttonImagesBlackStone);
	} else {
		buttonCurrentPlayer.setImageFiles(game->getCurrentPlayer() == fieldStruct::playerWhite ? buttonImagesWhiteStone : buttonImagesBlackStone);
	}

	// user input
	if (enableUserInput) {
		activateStonesOfCurrentPlayer(fieldStruct::size, settingColor, fieldStruct::size);
	} else {
		deactivateAllStones();
	}

	// perfect AI stuff
	updatePerfectMoveInfo();
	updateStateNumber();
}

//-----------------------------------------------------------------------------
// Name: showStateNumber()
// Desc: 
//-----------------------------------------------------------------------------
void millField2D::updatePerfectMoveInfo()
{
	// locals
	unsigned char		knotValue;
	unsigned short		bestAmountOfPlies;
	infoType			threeStoneOrder		[fieldStruct::size];
	unsigned int		freqValuesSubMoves	[fieldStruct::size][fieldStruct::size][numDirections];
	unsigned int		moveQuality			[fieldStruct::size][fieldStruct::size];
	unsigned char		moveValue			[fieldStruct::size][fieldStruct::size];
	unsigned short		plyInfo				[fieldStruct::size][fieldStruct::size];
	infoType			direction			[fieldStruct::size][fieldStruct::size];
	unsigned int		j, i;
	unsigned int		curPos;
	wstringstream		wss;

	// hide everything
	for (auto& curFieldPos : spritePerfectMove) {
		curFieldPos.setState(wildWeasel::guiElemState::HIDDEN);
	}
	for (auto& curFieldPos : spriteJumpingStone) {
		curFieldPos.setState(wildWeasel::guiElemState::HIDDEN);
	}

	// quit if game has finished
	if (!showingPerfectMove) return;
	if (game->getWinner()) return;

	// direction
	for (i=0; i<fieldStruct::size; i++) { for (j=0; j<fieldStruct::size; j++) {
		direction[i][j] = infoType::INVALID;
		moveValue[i][j] = SKV_VALUE_INVALID;
	}}

	direction[ 0][ 9] = infoType::DIR_DOWN;		direction[ 0][ 1] = infoType::DIR_RIGHT;	
	direction[ 2][ 1] = infoType::DIR_LEFT;		direction[ 2][14] = infoType::DIR_DOWN;	
	direction[23][14] = infoType::DIR_UP;		direction[23][22] = infoType::DIR_LEFT;	
	direction[21][22] = infoType::DIR_RIGHT;	direction[21][ 9] = infoType::DIR_UP;		
	direction[ 6][ 7] = infoType::DIR_RIGHT;	direction[ 6][11] = infoType::DIR_DOWN;	
	direction[ 8][ 7] = infoType::DIR_LEFT;		direction[ 8][12] = infoType::DIR_DOWN;	
	direction[17][12] = infoType::DIR_UP;		direction[17][16] = infoType::DIR_LEFT;	
	direction[15][16] = infoType::DIR_RIGHT;	direction[15][11] = infoType::DIR_UP;		
	direction[ 1][ 0] = infoType::DIR_LEFT;		direction[ 1][ 4] = infoType::DIR_DOWN;		direction[ 1][ 2] = infoType::DIR_RIGHT;
	direction[14][ 2] = infoType::DIR_UP;		direction[14][13] = infoType::DIR_LEFT;		direction[14][23] = infoType::DIR_DOWN;
	direction[22][21] = infoType::DIR_LEFT;		direction[22][19] = infoType::DIR_UP;		direction[22][23] = infoType::DIR_RIGHT;
	direction[ 9][ 0] = infoType::DIR_UP;		direction[ 9][10] = infoType::DIR_RIGHT;	direction[ 9][21] = infoType::DIR_DOWN;
	direction[ 7][ 6] = infoType::DIR_LEFT;		direction[ 7][ 4] = infoType::DIR_UP;		direction[ 7][ 8] = infoType::DIR_RIGHT;
	direction[12][ 8] = infoType::DIR_UP;		direction[12][13] = infoType::DIR_RIGHT;	direction[12][17] = infoType::DIR_DOWN;
	direction[16][15] = infoType::DIR_LEFT;		direction[16][19] = infoType::DIR_DOWN;		direction[16][17] = infoType::DIR_RIGHT;
	direction[11][ 6] = infoType::DIR_UP;		direction[11][10] = infoType::DIR_LEFT;		direction[11][15] = infoType::DIR_DOWN;
	direction[ 3][ 4] = infoType::DIR_RIGHT;	direction[ 3][10] = infoType::DIR_DOWN;		
	direction[18][19] = infoType::DIR_RIGHT;	direction[18][10] = infoType::DIR_UP;		
	direction[ 5][ 4] = infoType::DIR_LEFT;		direction[ 5][13] = infoType::DIR_DOWN;
	direction[20][19] = infoType::DIR_LEFT;		direction[20][13] = infoType::DIR_UP;	
	direction[ 4][ 5] = infoType::DIR_RIGHT;	direction[ 4][ 7] = infoType::DIR_DOWN;		direction[ 4][ 3] = infoType::DIR_LEFT;	direction[ 4][ 1] = infoType::DIR_UP;	
	direction[13][14] = infoType::DIR_RIGHT;	direction[13][20] = infoType::DIR_DOWN;		direction[13][12] = infoType::DIR_LEFT;	direction[13][ 5] = infoType::DIR_UP;	
	direction[19][20] = infoType::DIR_RIGHT;	direction[19][22] = infoType::DIR_DOWN;		direction[19][18] = infoType::DIR_LEFT;	direction[19][16] = infoType::DIR_UP;	
	direction[10][11] = infoType::DIR_RIGHT;	direction[10][18] = infoType::DIR_DOWN;		direction[10][ 9] = infoType::DIR_LEFT;	direction[10][ 3] = infoType::DIR_UP;	

	// get values of move
	playerPerfect->getValueOfMoves(&moveValue[0][0], &freqValuesSubMoves[0][0][0], &plyInfo[0][0], &moveQuality[0][0], knotValue, bestAmountOfPlies);

	// print game state
	switch (knotValue)
	{
	case  SKV_VALUE_GAME_WON:
		wss.str(L""); wss << L"WON in\n" << bestAmountOfPlies << L" plies.";
		labelGameState.setText(wss.str());
		labelGameState.setTextColor(wildWeasel::color::green);
		break;
	case SKV_VALUE_GAME_LOST:
		wss.str(L""); wss << L"LOST in\n" << bestAmountOfPlies << L" plies.";
		labelGameState.setText(wss.str());
		labelGameState.setTextColor(wildWeasel::color::red);
		break;
	case SKV_VALUE_GAME_DRAWN:
		wss.str(L""); wss << L"State value is\ndrawn!";
		labelGameState.setText(wss.str());
		labelGameState.setTextColor(wildWeasel::color{255,128,0});
		break;
	case SKV_VALUE_INVALID:
		wss.str(L""); wss << L"State value is\ninvalid!";
		labelGameState.setText(wss.str());
		labelGameState.setTextColor(wildWeasel::color{255,128,255});
		break;
	default:
		wss.str(L""); wss << L"No information\navailable!";
		labelGameState.setText(wss.str());
		labelGameState.setTextColor(wildWeasel::color{255,128,255});
		break;
	}
	
	// set stone or remove stone
	if (game->mustStoneBeRemoved() || game->inSettingPhase()) {

		// test each stone
		for (curPos=0; curPos<fieldStruct::size; curPos++) {
			setPerfectMoveInfo(curPos, infoType::NORMAL, moveValue[0][curPos], freqValuesSubMoves[0][curPos][SKV_VALUE_GAME_WON], freqValuesSubMoves[0][curPos][SKV_VALUE_GAME_DRAWN], freqValuesSubMoves[0][curPos][SKV_VALUE_GAME_LOST], plyInfo[0][curPos], (moveQuality[0][curPos]==1)?wildWeasel::color{200,255,200}:wildWeasel::color{255,128,0});
		}

	// normal move
	} else {

		// consider stone order if player has only 3 stones
		if (3 == game->getNumStonOfCurPlayer()) {
			int			myField[fieldStruct::size];
			infoType	myStoneId;
			for (game->getField(myField), myStoneId=infoType::JUMP_1, j=0, curPos=0; curPos<fieldStruct::size; curPos++) {
				if (game->getCurrentPlayer() == myField[curPos]) {

					spriteJumpingStone[j].setAlignment(alignmentJumpingStoneIcon[curPos], 4);
					spriteJumpingStone[j].setState(wildWeasel::guiElemState::VISIBLE);

					// stone order
					threeStoneOrder[curPos] = myStoneId;
					if (myStoneId == infoType::JUMP_2) myStoneId = infoType::JUMP_3;
					if (myStoneId == infoType::JUMP_1) myStoneId = infoType::JUMP_2;
					j++;
				} else {
					threeStoneOrder[curPos] = infoType::INVALID;
				}
			}
		}

		// test each stone
		for (i=0; i<fieldStruct::size; i++) {

			// test each direction
			for (j=0; j<fieldStruct::size; j++) {

				// if player has only 3 stones
				if (3 == game->getNumStonOfCurPlayer()) {
					setPerfectMoveInfo(j, threeStoneOrder[i], moveValue[i][j], freqValuesSubMoves[i][j][SKV_VALUE_GAME_WON], freqValuesSubMoves[i][j][SKV_VALUE_GAME_DRAWN], freqValuesSubMoves[i][j][SKV_VALUE_GAME_LOST], plyInfo[i][j], (moveQuality[i][j]==1)?wildWeasel::color{200,255,200}:wildWeasel::color{255,128,0});
				} else {
					setPerfectMoveInfo(i, direction[i][j], moveValue[i][j], freqValuesSubMoves[i][j][SKV_VALUE_GAME_WON], freqValuesSubMoves[i][j][SKV_VALUE_GAME_DRAWN], freqValuesSubMoves[i][j][SKV_VALUE_GAME_LOST], plyInfo[i][j], (moveQuality[i][j]==1)?wildWeasel::color{200,255,200}:wildWeasel::color{255,128,0});
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Name: windowSizeChanged()
// Desc: 
//-----------------------------------------------------------------------------
void millField2D::windowSizeChanged(int xSize, int ySize)
{
	labelGameStatus			.setTextSize(xSize * perfectMoveTextSize, ySize * perfectMoveTextSize);
	labelStateNumber		.setTextSize(xSize * perfectMoveTextSize, ySize * perfectMoveTextSize);
	labelGameState			.setTextSize(xSize * perfectMoveTextSize, ySize * perfectMoveTextSize);
	for (auto& curMove : spritePerfectMove) {
		curMove.setTextSize(xSize * perfectMoveTextSize, ySize * perfectMoveTextSize);
	}
}

//-----------------------------------------------------------------------------
// Name: setPerfectMoveInfo()
// Desc: 
//-----------------------------------------------------------------------------
void millField2D::setPerfectMoveInfo(unsigned int position, infoType direction, unsigned char moveValue, int freqValuesSubMovesWon, int freqValuesSubMovesDrawn, int freqValuesSubMovesLost, unsigned short plyInfo, wildWeasel::color textColor)
{
	// locals
	wstringstream				wss;
	unsigned int				index;
	unsigned int				gridPos;
	wildWeasel::alignment*		pAlignment;	

	switch (direction) 
	{
	case infoType::NORMAL:			index = position;							gridPos	= 4;	pAlignment = &alignmentNormalStoneIcon [position];	break;
	case infoType::DIR_UP:			index = 0 * fieldStruct::size + position;	gridPos	= 1;	pAlignment = &alignmentNormalStoneIcon [position];	break;
	case infoType::DIR_DOWN:		index = 1 * fieldStruct::size + position;	gridPos	= 7;	pAlignment = &alignmentNormalStoneIcon [position];	break;
	case infoType::DIR_LEFT:		index = 2 * fieldStruct::size + position;	gridPos	= 3;	pAlignment = &alignmentNormalStoneIcon [position];	break;
	case infoType::DIR_RIGHT:		index = 3 * fieldStruct::size + position;	gridPos	= 5;	pAlignment = &alignmentNormalStoneIcon [position];	break;
	case infoType::JUMP_1:			index = 0 * fieldStruct::size + position;	gridPos	= 3;	pAlignment = &alignmentJumpingStoneIcon[position];	break;
	case infoType::JUMP_2:			index = 1 * fieldStruct::size + position;	gridPos	= 4;	pAlignment = &alignmentJumpingStoneIcon[position];	break;
	case infoType::JUMP_3:			index = 2 * fieldStruct::size + position;	gridPos	= 5;	pAlignment = &alignmentJumpingStoneIcon[position];	break;
	case infoType::INVALID:			return;
	default:						return;
	}

	// when move possible
	if (moveValue != SKV_VALUE_INVALID && moveValue <= SKV_MAX_VALUE) {
		
		// draw value on stone
		spritePerfectMove[index].setAlignment(*pAlignment, gridPos);
		spritePerfectMove[index].setTextColor(textColor);
		spritePerfectMove[index].setState(wildWeasel::guiElemState::VISIBLE);

		switch (moveValue)
		{
		case SKV_VALUE_GAME_WON:	spritePerfectMove[index].setTexture(&textureValueWon);		break;
		case SKV_VALUE_GAME_DRAWN:	spritePerfectMove[index].setTexture(&textureValueDrawn);	break;
		case SKV_VALUE_GAME_LOST:	spritePerfectMove[index].setTexture(&textureValueLost);		break;
		case SKV_VALUE_INVALID:		spritePerfectMove[index].setTexture(&textureValueDrawn);	break;
		}

		// draw number of state values for opponents moves
		wss.str(L""); 
		if (direction == infoType::JUMP_2) { 
			wss << "." << endl;			// ... the point is a work-around for the bug in the draw srpite function which vanishes endlines at the beginning. some sort of setTExtOffset or setTextBorder would be necessary
		} else if (direction == infoType::JUMP_3) { 
			wss << "." << endl << endl;
		}
		wss << freqValuesSubMovesWon << L"|" << freqValuesSubMovesDrawn << L"|" << freqValuesSubMovesLost << L"|";
		if (plyInfo < PLYINFO_VALUE_DRAWN) {
			wss << plyInfo;
		} else {
			wss << L"-";
		}
		spritePerfectMove[index].setText(wss.str());
	} else {
		spritePerfectMove[index].setState(wildWeasel::guiElemState::HIDDEN);
	}

	windowSizeChanged(ww->getWindowSizeX(), ww->getWindowSizeX());
}

//-----------------------------------------------------------------------------
// Name: updateStateNumber()
// Desc: 
//-----------------------------------------------------------------------------
void millField2D::updateStateNumber()
{
	wstringstream wss;
	unsigned int layerNumber;
	unsigned int stateNumber;

	playerPerfect->getLayerAndStateNumber(layerNumber, stateNumber);
	wss << L"layerNumber: " << layerNumber << L"\tstateNumber: " << stateNumber;

	labelStateNumber.setText(wss.str());
}

//-----------------------------------------------------------------------------
// Name: showStateNumber()
// Desc: 
//-----------------------------------------------------------------------------
void millField2D::showStateNumber(bool newState)
{
	showingStateNumber = newState;
	labelStateNumber.setState(showingStateNumber && spriteField.getState() == wildWeasel::guiElemState::VISIBLE ? wildWeasel::guiElemState::VISIBLE : wildWeasel::guiElemState::HIDDEN);
}

//-----------------------------------------------------------------------------
// Name: showPerfectMove()
// Desc: 
//-----------------------------------------------------------------------------
void millField2D::showPerfectMove(bool newState)
{
	showingPerfectMove = newState;
	wildWeasel::guiElemState newGuiElemState = (showingPerfectMove && spriteField.getState() == wildWeasel::guiElemState::VISIBLE ? wildWeasel::guiElemState::VISIBLE : wildWeasel::guiElemState::HIDDEN);
	
	for (auto& curFieldPos : spritePerfectMove) {
		curFieldPos.setState(newGuiElemState);
	}
	spriteJumpingStone[0].setState(newGuiElemState);
	spriteJumpingStone[1].setState(newGuiElemState);
	spriteJumpingStone[2].setState(newGuiElemState);
	labelGameState.setState(newGuiElemState);

	if (newState) updatePerfectMoveInfo();
}

//-----------------------------------------------------------------------------
// Name: getMoveAnimationDuration()
// Desc: 
//-----------------------------------------------------------------------------
float millField2D::getMoveAnimationDuration()
{
	return stoneMoveDuration;
}

//-----------------------------------------------------------------------------
// Name: setGameStatusText()
// Desc: 
//-----------------------------------------------------------------------------
void millField2D::setGameStatusText(const wchar_t* newText)
{
	labelGameStatus.setText(newText);
}

//-----------------------------------------------------------------------------
// Name: setGameStatusText()
// Desc: 
//-----------------------------------------------------------------------------
void millField2D::setGameStatusText(const wstring& newText)
{
	labelGameStatus.setText(newText);
}

//-----------------------------------------------------------------------------
// Name: activateStonesOfCurrentPlayer()
// Desc: 
//-----------------------------------------------------------------------------
void millField2D::activateStonesOfCurrentPlayer(unsigned int markedPosition, int settingColor, unsigned int pushFrom)
{
	unsigned int	curPos	= 0;
	int				myField[fieldStruct::size];

	game->getField(myField);
	
	for (curPos = 0; curPos < fieldStruct::size; curPos++) {

		// stone of current player on field
		if (myField[curPos] == game->getCurrentPlayer()) {

			if (curPos == markedPosition) {
				stoneOnFieldPos[curPos]->setImageFiles(myField[curPos] == fieldStruct::playerWhite ? buttonImagesWhiteStoneMarked : buttonImagesBlackStoneMarked);
			} else {
				stoneOnFieldPos[curPos]->setImageFiles(myField[curPos] == fieldStruct::playerWhite ? buttonImagesWhiteStone: buttonImagesBlackStone);
			}

			buttonFieldPosition[curPos].setState(wildWeasel::guiElemState::HIDDEN); 

			if ((game->inSettingPhase() || pushFrom < fieldStruct::size || settingColor != 0 || game->mustStoneBeRemoved()) && pushFrom != curPos) {
				stoneOnFieldPos[curPos]   ->setState(wildWeasel::guiElemState::VISIBLE); 
			} else {
				stoneOnFieldPos[curPos]   ->setState(wildWeasel::guiElemState::DRAWED); 
			}

		// a free field position without stone on it
		} else if (abs(myField[curPos]) != 1) {
			if ((game->inSettingPhase() || pushFrom < fieldStruct::size || settingColor != 0) && !game->mustStoneBeRemoved()) {
				buttonFieldPosition[curPos].setState(wildWeasel::guiElemState::DRAWED);
			} else {
				buttonFieldPosition[curPos].setState(wildWeasel::guiElemState::HIDDEN); 
			}

		// opponent stone on field
		} else if (abs(myField[curPos]) <= 1) {
			if (game->mustStoneBeRemoved()) {
				stoneOnFieldPos[curPos]   ->setState(wildWeasel::guiElemState::DRAWED); 
				stoneOnFieldPos[curPos]   ->setImageFiles(myField[curPos] == fieldStruct::playerWhite ? buttonImagesWhiteStone: buttonImagesBlackStone);
			} else {
				buttonFieldPosition[curPos].setState(wildWeasel::guiElemState::HIDDEN); 
				stoneOnFieldPos[curPos]   ->setState(wildWeasel::guiElemState::VISIBLE);
				stoneOnFieldPos[curPos]	  ->setImageFiles(myField[curPos] == fieldStruct::playerWhite ? buttonImagesWhiteStone: buttonImagesBlackStone);
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Name: deactivateAllStones()
// Desc: 
//-----------------------------------------------------------------------------
void millField2D::deactivateAllStones()
{
	for (auto& curStone : buttonWhiteStones) {
		if (curStone.getState() == wildWeasel::guiElemState::DRAWED) curStone.setState(wildWeasel::guiElemState::VISIBLE);
	}

	for (auto& curStone : buttonBlackStones) {
		if (curStone.getState() == wildWeasel::guiElemState::DRAWED) curStone.setState(wildWeasel::guiElemState::VISIBLE);
	}

	for (auto& curFieldPos : buttonFieldPosition) {
		if (curFieldPos.getState() == wildWeasel::guiElemState::DRAWED   ) curFieldPos.setState(wildWeasel::guiElemState::VISIBLE);
		if (curFieldPos.getState() == wildWeasel::guiElemState::INVISIBLE) curFieldPos.setState(wildWeasel::guiElemState::HIDDEN );
	}	
}

//-----------------------------------------------------------------------------
// Name: moveStone()
// Desc: 
//-----------------------------------------------------------------------------
void millField2D::moveStone(unsigned int fromPos, unsigned int toPos, bool stoneMustBeRemovedBeforeMove, bool inSettingPhaseBeforeMove, unsigned int numberOfStoneSetBeforeMove, int currentPlayerBeforeMove)
{
	unsigned int					stoneIndex		= 0;
	wildWeasel::plainButton2D*	theStone		= nullptr;

	if (stoneMustBeRemovedBeforeMove) {
		theStone	= stoneOnFieldPos[fromPos];
		stoneOnFieldPos[fromPos] = nullptr;
		theStone->blinkVisibility(stoneRemovalDuration, stoneRemovalBlinkTimes, wildWeasel::guiElemState::HIDDEN);
		// theStone->setState(wildWeasel::guiElemState::HIDDEN);
	} else if (inSettingPhaseBeforeMove) {
		stoneIndex	= fieldStruct::numStonesPerPlayer - numberOfStoneSetBeforeMove / 2 - 1;
		theStone	= (currentPlayerBeforeMove == fieldStruct::playerBlack ? &buttonBlackStones[stoneIndex] : &buttonWhiteStones[stoneIndex]);
		setStoneOnPos(*theStone, toPos);
		stoneOnFieldPos[toPos] = theStone;
	} else {
		theStone	= stoneOnFieldPos[fromPos];
		setStoneOnPos(*theStone, toPos);
		stoneOnFieldPos[toPos]		= theStone;
		stoneOnFieldPos[fromPos]	= nullptr;
	} 
		
	buttonCurrentPlayer.setImageFiles(game->getCurrentPlayer() == fieldStruct::playerWhite ? buttonImagesWhiteStone : buttonImagesBlackStone);

	updateStateNumber();
	updatePerfectMoveInfo();
}

//-----------------------------------------------------------------------------
// Name: stoneClicked()
// Desc: 
//-----------------------------------------------------------------------------
void millField2D::stoneClicked(void* pUser)
{
	wildWeasel::plainButton2D*	myButton = (wildWeasel::plainButton2D*) pUser;
	unsigned int					fieldPos;
	for (fieldPos = 0; fieldPos < fieldStruct::size; fieldPos++) {
		if (stoneOnFieldPos[fieldPos] == myButton) break;
	}
	if (fieldPosClickedFunc) fieldPosClickedFunc(fieldPos);
}

//-----------------------------------------------------------------------------
// Name: fieldPosClicked()
// Desc: 
//-----------------------------------------------------------------------------
void millField2D::fieldPosClicked(void* pUser)
{
	wildWeasel::plainButton2D*	myButton = (wildWeasel::plainButton2D*) pUser;
	unsigned int					fieldPos;
	for (fieldPos = 0; fieldPos < fieldStruct::size; fieldPos++) {
		if (&(buttonFieldPosition[fieldPos]) == pUser) break;
	}
	if (fieldPosClickedFunc) fieldPosClickedFunc(fieldPos);
}

#pragma endregion

