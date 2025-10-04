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
// Desc: Initialization
//-----------------------------------------------------------------------------
void millField2D::init(wildWeasel::masterMind* ww, muehle& game, perfectAI& playerPerfect, wildWeasel::font2D& d3dFont2D, wildWeasel::alignment& newAlignment, wildWeasel::texture& textureField)
{
	// pointers
	this->ww			= ww;
	this->game			= &game;
	this->playerPerfect	= &playerPerfect;
	
	followEvent(this, wildWeasel::eventFollower::eventType::WINDOWSIZE_CHANGED);

	// field
	spriteField.resize(1);
	spriteField[0].create(ww, wstring(L""), &d3dFont2D, 0);
	spriteField[0].setTexture(&textureField);
	spriteField[0].setState(wildWeasel::guiElemState::VISIBLE);

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
	buttonCurrentPlayer.resize(1);
	buttonCurrentPlayer[0].create(ww, buttonImagesWhiteStone, nullptr, nullptr, 1);
	buttonCurrentPlayer[0].setState(wildWeasel::guiElemState::HIDDEN);

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
	setField(playerId::squareIsFree, false);
}

//-----------------------------------------------------------------------------
// Name: setFieldPosClickedFunc()
// Desc: Set the function pointer which is called when a field position is clicked
//-----------------------------------------------------------------------------
void millField2D::setFieldPosClickedFunc(function<void(unsigned int)> newFieldPosClickedFunc)
{
	fieldPosClickedFunc = newFieldPosClickedFunc;
}

//-----------------------------------------------------------------------------
// Name: setStoneOnPos()
// Desc: Places the button representing a stone on the field. If animateStoneOnMove is true, the stone is moved with an animation to its new position. 
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
// Desc: Places the button representing a stone on the stock. 
//-----------------------------------------------------------------------------
void millField2D::setStoneOnStock(wildWeasel::plainButton2D& theStone, unsigned int stockPos, playerId player)
{
	if (stockPos >= fieldStruct::numStonesPerPlayer) return;

	unsigned int index = 0;

	if (player == playerId::playerOne) {
		index = stockPos;
	} else {
		index = fieldStruct::numStonesPerPlayer + 5 + stockPos;
	}

	theStone.setAlignment(alignmentStoneStock, index);
}

//-----------------------------------------------------------------------------
// Name: setAlignment()
// Desc: Set the alignment of all gui elements
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

	buttonCurrentPlayer[0]	.setAlignment(alignmentCurrentPlayer	);
	spriteField[0]			.setAlignment(alignmentField			);
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
		curAlignment.setSize	(wildWeasel::alignmentTypeX::FRACTION, perfectMoveIconSizeFractionOfStone, 
								 wildWeasel::alignmentTypeY::FRACTION, perfectMoveIconSizeFractionOfStone);
		curAlignment.setPosition(wildWeasel::alignmentTypeX::FRACTION, 0.5f, wildWeasel::alignmentHorizontal::LEFT, 
								 wildWeasel::alignmentTypeY::FRACTION, 0.5f, wildWeasel::alignmentVertical::TOP);
		curAlignment.setGrid	(wildWeasel::alignmentTypeX::FRACTION, -0.3f*perfectMoveIconSizeFractionOfStone, wildWeasel::alignmentHorizontal::CENTER,
								 wildWeasel::alignmentTypeY::FRACTION, -0.3f*perfectMoveIconSizeFractionOfStone, wildWeasel::alignmentVertical  ::CENTER, 3);
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
// Desc: Set the visibility of all gui elements 
//-----------------------------------------------------------------------------
void millField2D::setVisibility(bool visible)
{
	wildWeasel::guiElemState newSpriteState = (visible ? wildWeasel::guiElemState::  VISIBLE : wildWeasel::guiElemState::HIDDEN);
	wildWeasel::guiElemState newButtonState = (visible ? wildWeasel::guiElemState::INVISIBLE : wildWeasel::guiElemState::HIDDEN);

	spriteField[0].setState(newSpriteState);
	buttonCurrentPlayer[0].setState(newSpriteState);
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
// Desc: Set the current game state for a given layer and state number of the perfect AI.  
//-----------------------------------------------------------------------------
void millField2D::setState(unsigned int curShowedLayer, miniMax::stateNumberVarType curShowedState, unsigned char curShowedSymOp, unsigned int curPlayer)
{
	// draw field if necessary
	if (curShowedLayer != 0) {
		
		// locals
		fieldStruct		myField;
		bool			gameHasFinished;

		// get current field based on layer and state number
		myField.reset();
		playerPerfect->getField(curShowedLayer, curShowedState, curShowedSymOp, myField, gameHasFinished);
		if (curPlayer != 1) {
			myField.invert();
		}

		// set current field state to a mill game
		game->setCurrentGameState(myField);

		// set gui elements on field
		setField(playerId::squareIsFree, false);
			
		// status text
		updateGameStatusText(false);
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
// Name: updateGameStatusText()
// Desc: 
//-----------------------------------------------------------------------------
void millField2D::updateGameStatusText(bool currentPlayerIsHuman)
{
	if (game->gameHasFinished()) {
		if (game->getWinner() == fieldStruct::playerWhite) {
			setGameStatusText(L"White player has won!");
		} else if (game->getWinner() == fieldStruct::playerBlack) {
			setGameStatusText(L"Black player has won!");
		} else {
			setGameStatusText(L"The game is drawn!");
		}
	} else {
		if (currentPlayerIsHuman) {
			if (game->mustStoneBeRemoved()) {
				setGameStatusText(L"Remove a \nstone!");
			} else if (game->inSettingPhase()) {
				setGameStatusText(L"Set a \nstone!");
			} else {
				setGameStatusText(L"Select a \nstone!");
			}
		} else {
			setGameStatusText(L"Bot is \nthinking.");
		}
	}	
}

//-----------------------------------------------------------------------------
// Name: setField()
// Desc: Set the mill field based on the current game state. 
//-----------------------------------------------------------------------------
void millField2D::setField(playerId settingColor, bool enableUserInput)
{
	// locals
	size_t		curWhiteStoneId = 0, curBlackStoneId = 0;
	
	// stones in the stock
	if (game->inSettingPhase() || settingColor != playerId::squareIsFree) {

		unsigned int curStockPos;
		int numWhiteStonesResting;
		int numBlackStonesResting;

		if (settingColor != playerId::squareIsFree && !game->inSettingPhase()) {
			numWhiteStonesResting = fieldStruct::numStonesPerPlayer - game->getNumStonesOfCurPlayer();
			numBlackStonesResting = fieldStruct::numStonesPerPlayer - game->getNumStonesOfOppPlayer();
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
	for (auto& curFieldValue : game->getField()) {
		if (curFieldValue == fieldStruct::playerWhite && curWhiteStoneId < buttonWhiteStones.size()) {
			buttonWhiteStones[curWhiteStoneId].setState(wildWeasel::guiElemState::VISIBLE);
			setStoneOnPos(buttonWhiteStones[curWhiteStoneId], curPos);
			stoneOnFieldPos[curPos] = &buttonWhiteStones[curWhiteStoneId];
			curWhiteStoneId++;
		} else if (curFieldValue == fieldStruct::playerBlack && curBlackStoneId < buttonBlackStones.size()) {
			buttonBlackStones[curBlackStoneId].setState(wildWeasel::guiElemState::VISIBLE);
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
	if (settingColor != playerId::squareIsFree) {
		buttonCurrentPlayer[0].setImageFiles(settingColor == fieldStruct::playerWhite ? buttonImagesWhiteStone : buttonImagesBlackStone);
	} else {
		buttonCurrentPlayer[0].setImageFiles(game->getCurrentPlayer() == fieldStruct::playerWhite ? buttonImagesWhiteStone : buttonImagesBlackStone);
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
// Name: updatePerfectMoveInfo()
// Desc: Shows information regarding the perfect move of the perfect AI 
//-----------------------------------------------------------------------------
void millField2D::updatePerfectMoveInfo()
{
	// update infoAboutChoices
	moveInfo dummyMove;
	game->getChoiceOfSpecialAI(playerPerfect, dummyMove);

	// locals
	const miniMax::stateInfo &	infoAboutChoices = playerPerfect->getInfoAboutChoices();
	wstringstream				wss;

	// hide everything
	for (auto& curFieldPos : spritePerfectMove) {
		curFieldPos.setState(wildWeasel::guiElemState::HIDDEN);
	}
	for (auto& curFieldPos : spriteJumpingStone) {
		curFieldPos.setState(wildWeasel::guiElemState::HIDDEN);
	}

	// quit if game has finished
	if (!showingPerfectMove) 	 return;
	if (game->gameHasFinished()) return;

	// print game state
	switch (infoAboutChoices.shortValue)
	{
	case  miniMax::SKV_VALUE_GAME_WON:
		wss.str(L""); wss << L"WON in\n" << infoAboutChoices.bestAmountOfPlies << L" plies.";
		labelGameState.setText(wss.str());
		labelGameState.setTextColor(wildWeasel::color::green());
		break;
	case miniMax::SKV_VALUE_GAME_LOST:
		wss.str(L""); wss << L"LOST in\n" << infoAboutChoices.bestAmountOfPlies << L" plies.";
		labelGameState.setText(wss.str());
		labelGameState.setTextColor(wildWeasel::color::red());
		break;
	case miniMax::SKV_VALUE_GAME_DRAWN:
		wss.str(L""); wss << L"State value\nis\ndrawn!";
		labelGameState.setText(wss.str());
		labelGameState.setTextColor(wildWeasel::color{255,128,0});
		break;
	case miniMax::SKV_VALUE_INVALID:
		wss.str(L""); wss << L"State value\nis\ninvalid!";
		labelGameState.setText(wss.str());
		labelGameState.setTextColor(wildWeasel::color{255,128,255});
		break;
	default:
		wss.str(L""); wss << L"No information\navailable!";
		labelGameState.setText(wss.str());
		labelGameState.setTextColor(wildWeasel::color{255,128,255});
		break;
	}

	// Calculate perfect move information
	vector<perfectMoveInfo> pmis;
	perfectMoveInfo::calcPerfectMoveInfo(pmis, infoAboutChoices, game->mustStoneBeRemoved(), game->inSettingPhase(), game->getNumStonesOfCurPlayer() == 3 && !game->inSettingPhase());
	
	// Show the perfect move information
	for (const auto& pmi : pmis) {
		showPerfectMoveInfo(pmi);
	}
}

//-----------------------------------------------------------------------------
// Name: windowSizeChanged()
// Desc: Inform the gui elements about the new window size 
//-----------------------------------------------------------------------------
void millField2D::windowSizeChanged(int xSize, int ySize)
{
	labelGameStatus			.setTextSize(xSize * gameStateTextSize, 	ySize * gameStateTextSize);
	labelStateNumber		.setTextSize(xSize * perfectMoveTextSize, 	ySize * perfectMoveTextSize);
	labelGameState			.setTextSize(xSize * gameStateTextSize, 	ySize * gameStateTextSize);
	for (auto& curMove : spritePerfectMove) {
		curMove.setTextSize(xSize * perfectMoveTextSize, ySize * perfectMoveTextSize);
	}
}

//-----------------------------------------------------------------------------
// Name: showPerfectMoveInfo()
// Desc: Set the information regarding the perfect move of the perfect AI  
//-----------------------------------------------------------------------------
void millField2D::showPerfectMoveInfo(const perfectMoveInfo& pmi)
{
	using infoType = perfectMoveInfo::infoType;

	// locals
	wstringstream					wss;
	unsigned int					index;
	unsigned int					gridPos;
	wildWeasel::alignment*			pAlignment;
	wildWeasel::alignmentVertical	txtAlignVert = wildWeasel::alignmentVertical::BELOW;
	wildWeasel::alignmentHorizontal	txtAlignHorz = wildWeasel::alignmentHorizontal::CENTER;

	switch (pmi.type)
	{
	case infoType::CENTER:			index = pmi.position; 							gridPos	= 4;	pAlignment = &alignmentNormalStoneIcon [pmi.position];	break;
	case infoType::JUMP_SRC_1:		index = 0;										gridPos	= 4;	pAlignment = &alignmentNormalStoneIcon [pmi.position];	break;
	case infoType::JUMP_SRC_2:		index = 1;										gridPos	= 4;	pAlignment = &alignmentNormalStoneIcon [pmi.position];	break;
	case infoType::JUMP_SRC_3:		index = 2;										gridPos	= 4;	pAlignment = &alignmentNormalStoneIcon [pmi.position];	break;
	case infoType::DIR_UP:			index = 0 * fieldStruct::size + pmi.position;	gridPos	= 1;	pAlignment = &alignmentNormalStoneIcon [pmi.position];	break;
	case infoType::DIR_DOWN:		index = 1 * fieldStruct::size + pmi.position;	gridPos	= 7;	pAlignment = &alignmentNormalStoneIcon [pmi.position];	break;
	case infoType::DIR_LEFT:		index = 2 * fieldStruct::size + pmi.position;	gridPos	= 3;	pAlignment = &alignmentNormalStoneIcon [pmi.position];	break;
	case infoType::DIR_RIGHT:		index = 3 * fieldStruct::size + pmi.position;	gridPos	= 5;	pAlignment = &alignmentNormalStoneIcon [pmi.position];	break;
	case infoType::JUMP_DST_1:		index = 0 * fieldStruct::size + pmi.position;	gridPos	= 0;	pAlignment = &alignmentJumpingStoneIcon[pmi.position];	break;
	case infoType::JUMP_DST_2:		index = 1 * fieldStruct::size + pmi.position;	gridPos	= 4;	pAlignment = &alignmentJumpingStoneIcon[pmi.position];	break;
	case infoType::JUMP_DST_3:		index = 2 * fieldStruct::size + pmi.position;	gridPos	= 8;	pAlignment = &alignmentJumpingStoneIcon[pmi.position];	break;
	case infoType::INVALID:			return;
	default:						return;
	}

	// show numbers on jumping stones
	if (pmi.type == infoType::JUMP_SRC_1 || pmi.type == infoType::JUMP_SRC_2 || pmi.type == infoType::JUMP_SRC_3) {
		spriteJumpingStone[index].setAlignment(*pAlignment, gridPos);
		spriteJumpingStone[index].setState(wildWeasel::guiElemState::VISIBLE);

	// when move is not possible
	} else if (pmi.moveValue == miniMax::SKV_VALUE_INVALID || pmi.moveValue > miniMax::SKV_NUM_VALUES - 1) {
		spritePerfectMove[index].setState(wildWeasel::guiElemState::HIDDEN);

	// show perfect move value
	} else if (pmi.type != infoType::INVALID) {
		// draw value on stone
		spritePerfectMove[index].setAlignment(*pAlignment, gridPos);
		spritePerfectMove[index].setTextColor(pmi.textColor);
		spritePerfectMove[index].setState(wildWeasel::guiElemState::VISIBLE);

		switch (pmi.moveValue)
		{
		case miniMax::SKV_VALUE_GAME_WON:	spritePerfectMove[index].setTexture(&textureValueWon);		break;
		case miniMax::SKV_VALUE_GAME_DRAWN:	spritePerfectMove[index].setTexture(&textureValueDrawn);	break;
		case miniMax::SKV_VALUE_GAME_LOST:	spritePerfectMove[index].setTexture(&textureValueLost);		break;
		case miniMax::SKV_VALUE_INVALID:	spritePerfectMove[index].setTexture(&textureValueDrawn);	break;
		}

		// draw number of state values for opponents moves
		wss.str(L"");

		// align the text on the right side of the symbol for jumping destinations
		if (pmi.type == infoType::JUMP_DST_1 || pmi.type == infoType::JUMP_DST_2 || pmi.type == infoType::JUMP_DST_3) {
			txtAlignVert = wildWeasel::alignmentVertical::CENTER; 
			txtAlignHorz = wildWeasel::alignmentHorizontal::LEFT; 
			wss << ".       ";
		}

		wss << pmi.freqValuesSubMovesWon << L"|" << pmi.freqValuesSubMovesDrawn << L"|" << pmi.freqValuesSubMovesLost << L"|";
		if (pmi.plyInfo < miniMax::PLYINFO_VALUE_DRAWN) {
			wss << pmi.plyInfo;
		} else {
			wss << L"-";
		}
		spritePerfectMove[index].setText(wss.str());
		spritePerfectMove[index].setTextAlignmentVertical	(txtAlignVert);
		spritePerfectMove[index].setTextAlignmentHorizontal	(txtAlignHorz);
	} else {
		spritePerfectMove[index].setState(wildWeasel::guiElemState::HIDDEN);
	}

	windowSizeChanged(ww->getWindowSizeX(), ww->getWindowSizeX());
}

//-----------------------------------------------------------------------------
// Name: updateStateNumber()
// Desc: Update the state number of the perfect AI 
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
// Desc: Enable or disable the display of the state number of the perfect AI 
//-----------------------------------------------------------------------------
void millField2D::showStateNumber(bool newState)
{
	showingStateNumber = newState;
	labelStateNumber.setState(showingStateNumber && spriteField[0].getState() == wildWeasel::guiElemState::VISIBLE ? wildWeasel::guiElemState::VISIBLE : wildWeasel::guiElemState::HIDDEN);
}

//-----------------------------------------------------------------------------
// Name: showPerfectMove()
// Desc: Enable or disable the display of the perfect move of the perfect AI 
//-----------------------------------------------------------------------------
void millField2D::showPerfectMove(bool newState)
{
	showingPerfectMove = newState;
	wildWeasel::guiElemState newGuiElemState = (showingPerfectMove && spriteField[0].getState() == wildWeasel::guiElemState::VISIBLE ? wildWeasel::guiElemState::VISIBLE : wildWeasel::guiElemState::HIDDEN);
	
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
// Desc: Returns the duration of the move animation 
//-----------------------------------------------------------------------------
float millField2D::getMoveAnimationDuration()
{
	return stoneMoveDuration;
}

//-----------------------------------------------------------------------------
// Name: setGameStatusText()
// Desc: Set the text of the game status 
//-----------------------------------------------------------------------------
void millField2D::setGameStatusText(const wchar_t* newText)
{
	labelGameStatus.setText(newText);
}

//-----------------------------------------------------------------------------
// Name: setGameStatusText()
// Desc: Set the text of the game status 
//-----------------------------------------------------------------------------
void millField2D::setGameStatusText(const wstring& newText)
{
	labelGameStatus.setText(newText);
}

//-----------------------------------------------------------------------------
// Name: activateStonesOfCurrentPlayer()
// Desc: Activates the buttons of the stones of the current player 
//-----------------------------------------------------------------------------
void millField2D::activateStonesOfCurrentPlayer(unsigned int markedPosition, playerId settingColor, unsigned int pushFrom)
{
	unsigned int	curPos	= 0;
	const fieldStruct::fieldArray& myField = game->getField();
	
	for (curPos = 0; curPos < fieldStruct::size; curPos++) {

		// stone of current player on field
		if (myField[curPos] == game->getCurrentPlayer()) {

			if (!stoneOnFieldPos[curPos]) continue;

			if (curPos == markedPosition) {
				stoneOnFieldPos[curPos]->setImageFiles(myField[curPos] == fieldStruct::playerWhite ? buttonImagesWhiteStoneMarked : buttonImagesBlackStoneMarked);
			} else {
				stoneOnFieldPos[curPos]->setImageFiles(myField[curPos] == fieldStruct::playerWhite ? buttonImagesWhiteStone: buttonImagesBlackStone);
			}

			buttonFieldPosition[curPos].setState(wildWeasel::guiElemState::HIDDEN); 

			if ((game->inSettingPhase() || pushFrom < fieldStruct::size || settingColor != playerId::squareIsFree || game->mustStoneBeRemoved()) && pushFrom != curPos) {
				stoneOnFieldPos[curPos]   ->setState(wildWeasel::guiElemState::VISIBLE); 
			} else {
				stoneOnFieldPos[curPos]   ->setState(wildWeasel::guiElemState::DRAWED); 
			}

		// a free field position without stone on it
		} else if (myField[curPos] == playerId::squareIsFree) {
			if ((game->inSettingPhase() || pushFrom < fieldStruct::size || settingColor != playerId::squareIsFree) && !game->mustStoneBeRemoved()) {
				buttonFieldPosition[curPos].setState(wildWeasel::guiElemState::DRAWED);
			} else {
				buttonFieldPosition[curPos].setState(wildWeasel::guiElemState::HIDDEN); 
			}

		// opponent stone on field
		} else {
			if (!stoneOnFieldPos[curPos]) continue;
			
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
// Desc: Deactivates all stones, so that no stone nor field position can be clicked by the user 
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
// Name: startStoneMoveAnimation()
// Desc: Triggers the start of the move animation of a stone on the field, without actually updating the game state
//-----------------------------------------------------------------------------
void millField2D::startStoneMoveAnimation(unsigned int fromPos, unsigned int toPos, bool stoneMustBeRemovedBeforeMove, bool inSettingPhaseBeforeMove, unsigned int numberOfStoneSetBeforeMove, playerId currentPlayerBeforeMove)
{
	unsigned int				stoneIndex		= 0;
	wildWeasel::plainButton2D*	theStone		= nullptr;

	if (stoneMustBeRemovedBeforeMove) {
		theStone	= stoneOnFieldPos[fromPos];
		theStone->blinkVisibility(stoneRemovalDuration, stoneRemovalBlinkTimes, wildWeasel::guiElemState::HIDDEN);
	} else if (inSettingPhaseBeforeMove) {
		stoneIndex	= fieldStruct::numStonesPerPlayer - numberOfStoneSetBeforeMove / 2 - 1;
		theStone	= (currentPlayerBeforeMove == fieldStruct::playerBlack ? &buttonBlackStones[stoneIndex] : &buttonWhiteStones[stoneIndex]);
		setStoneOnPos(*theStone, toPos);
	} else {
		theStone	= stoneOnFieldPos[fromPos];
		setStoneOnPos(*theStone, toPos);
	}
}

//-----------------------------------------------------------------------------
// Name: moveStone()
// Desc: Triggers the move of a stone on the field 
//-----------------------------------------------------------------------------
void millField2D::moveStone(unsigned int fromPos, unsigned int toPos, bool stoneMustBeRemovedBeforeMove, bool inSettingPhaseBeforeMove, unsigned int numberOfStoneSetBeforeMove, playerId currentPlayerBeforeMove)
{
	unsigned int				stoneIndex		= 0;
	wildWeasel::plainButton2D*	theStone		= nullptr;

	if (stoneMustBeRemovedBeforeMove) {
		stoneOnFieldPos[fromPos] = nullptr;
	} else if (inSettingPhaseBeforeMove) {
		stoneIndex	= fieldStruct::numStonesPerPlayer - numberOfStoneSetBeforeMove / 2 - 1;
		theStone	= (currentPlayerBeforeMove == fieldStruct::playerBlack ? &buttonBlackStones[stoneIndex] : &buttonWhiteStones[stoneIndex]);
		stoneOnFieldPos[toPos] = theStone;
	} else {
		theStone	= stoneOnFieldPos[fromPos];
		stoneOnFieldPos[toPos]		= theStone;
		stoneOnFieldPos[fromPos]	= nullptr;
	}
			
	buttonCurrentPlayer[0].setImageFiles(game->getCurrentPlayer() == fieldStruct::playerWhite ? buttonImagesWhiteStone : buttonImagesBlackStone);

	updateStateNumber();
	updatePerfectMoveInfo();
}

//-----------------------------------------------------------------------------
// Name: stoneClicked()
// Desc: Calls the functions assigned to the stone clicked event
//-----------------------------------------------------------------------------
void millField2D::stoneClicked(void* pUser)
{
	wildWeasel::plainButton2D*	myButton = (wildWeasel::plainButton2D*) pUser;
	unsigned int				fieldPos;
	for (fieldPos = 0; fieldPos < fieldStruct::size; fieldPos++) {
		if (stoneOnFieldPos[fieldPos] == myButton) break;
	}
	if (fieldPosClickedFunc) fieldPosClickedFunc(fieldPos);
}

//-----------------------------------------------------------------------------
// Name: fieldPosClicked()
// Desc: Calls the functions assigned to the field position clicked event 
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

#pragma region "perfectMoveInfo"

//-----------------------------------------------------------------------------
// static variables
//-----------------------------------------------------------------------------
const fieldStruct::Array2d<millField2D::perfectMoveInfo::infoType, fieldStruct::size, fieldStruct::size> millField2D::perfectMoveInfo::direction = []() {

	using infoType = millField2D::perfectMoveInfo::infoType;

	fieldStruct::Array2d<infoType, fieldStruct::size, fieldStruct::size> direction{};

	for (fieldStruct::fieldPos from = 0; from < fieldStruct::size; from++) {
		for (fieldStruct::fieldPos to = 0; to < fieldStruct::size; to++) {
			direction[to][from] = infoType::INVALID;
		}
	}

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
	
	// Return the initialized array
	return direction;
}();

//-----------------------------------------------------------------------------
// perfectMoveInfo constructor
//-----------------------------------------------------------------------------
millField2D::perfectMoveInfo::perfectMoveInfo(fieldStruct::fieldPos position, infoType type, miniMax::twoBit moveValue, unsigned int freqValuesSubMovesWon, unsigned int freqValuesSubMovesDrawn, unsigned int freqValuesSubMovesLost, miniMax::plyInfoVarType plyInfo)
	: position(position), type(type), moveValue(moveValue), freqValuesSubMovesWon(freqValuesSubMovesWon), freqValuesSubMovesDrawn(freqValuesSubMovesDrawn), freqValuesSubMovesLost(freqValuesSubMovesLost), plyInfo(plyInfo) 
{
	// Set the text color based on the move value
	switch (moveValue)
	{
	case miniMax::SKV_VALUE_GAME_WON:		textColor = wildWeasel::color::green();		break;
	case miniMax::SKV_VALUE_GAME_DRAWN:		textColor = wildWeasel::color{255,128,0};	break;
	case miniMax::SKV_VALUE_GAME_LOST:		textColor = wildWeasel::color::red();		break;
	default:								textColor = wildWeasel::color{255,128,255};	break;
	}
}

//-----------------------------------------------------------------------------
// perfectMoveInfo constructor
//-----------------------------------------------------------------------------
millField2D::perfectMoveInfo::perfectMoveInfo(const miniMax::possibilityInfo &choice, const moveInfo &move, bool mustStoneBeRemoved, bool inSettingPhase, bool inJumpPhase, const vector<infoType>& mapJumpFromToType) :
	moveValue				(choice.shortValue),
	freqValuesSubMovesWon	(choice.freqValuesSubMoves[miniMax::SKV_VALUE_GAME_WON]),
	freqValuesSubMovesDrawn	(choice.freqValuesSubMoves[miniMax::SKV_VALUE_GAME_DRAWN]),
	freqValuesSubMovesLost	(choice.freqValuesSubMoves[miniMax::SKV_VALUE_GAME_LOST]),
	plyInfo					(choice.plyInfo)
{
	if (mustStoneBeRemoved) {
		position 	= move.removeStone;
		type 		= infoType::CENTER;
	} else if (inSettingPhase) {
		position 	= move.to;
		type 		= infoType::CENTER;
	} else if (!inJumpPhase) {
		position 	= move.from;
		type 		= direction[move.from][move.to];
	} else {
		position 	= move.to;
		type 		= mapJumpFromToType[move.from];
	}
	switch (moveValue)
	{
	case miniMax::SKV_VALUE_GAME_WON:		textColor = wildWeasel::color::green();		break;
	case miniMax::SKV_VALUE_GAME_DRAWN:		textColor = wildWeasel::color{255,128,0};	break;
	case miniMax::SKV_VALUE_GAME_LOST:		textColor = wildWeasel::color::red();		break;
	default:								textColor = wildWeasel::color{255,128,255};	break;
	}	
}

//----------------------------------------------------------------------------- 
// Name: perfectMoveInfo::initJumpMapping()
// Desc: Initializes the mapping from stone position to infoType::JUMP_DST_1, ...
//-----------------------------------------------------------------------------
void millField2D::perfectMoveInfo::initJumpMapping(const miniMax::stateInfo& infoAboutChoices, std::vector<infoType>& mapJumpFromToType, set<fieldStruct::fieldPos>& jumpFromPositions)
{
	jumpFromPositions.clear();
	mapJumpFromToType.assign(fieldStruct::size, infoType::INVALID);

	for (const auto& choice : infoAboutChoices.choices) {
		moveInfo move;
		move.setId(choice.possibilityId);
		jumpFromPositions.insert(move.from);
	}

	// assert(jumpFromPositions.size() == 3);
	if (jumpFromPositions.size() < 3) {
		return;
	}

	mapJumpFromToType[*std::next(jumpFromPositions.begin(), 0)] = infoType::JUMP_DST_1;
	mapJumpFromToType[*std::next(jumpFromPositions.begin(), 1)] = infoType::JUMP_DST_2;
	mapJumpFromToType[*std::next(jumpFromPositions.begin(), 2)] = infoType::JUMP_DST_3;
}

//----------------------------------------------------------------------------- 
// Name: perfectMoveInfo::removeDuplicates()
// Desc: Removes duplicate moves from the stoneRemovalList. Thereby the member .removeStone is ignored.
//----------------------------------------------------------------------------- 
void millField2D::perfectMoveInfo::removeDuplicates(vector<moveInfo>& stoneRemovalList)
{
	// Remove duplicates from stoneRemovalList based on .from and .to
	stoneRemovalList.erase(
		std::unique(stoneRemovalList.begin(), stoneRemovalList.end(),
					[](const moveInfo& a, const moveInfo& b) {
						return a.from == b.from && a.to == b.to;
					}),
		stoneRemovalList.end());
}

//----------------------------------------------------------------------------- 
// Name: perfectMoveInfo::addBestMovesForClosedMills()
// Desc: Adds the best moves for closed mills to the pmis vector
//----------------------------------------------------------------------------- 
void millField2D::perfectMoveInfo::addBestMovesForClosedMills(const vector<moveInfo>& stoneRemovalList, vector<perfectMoveInfo> &pmis, const miniMax::stateInfo &infoAboutChoices, bool mustStoneBeRemoved, bool inSettingPhase, bool inJumpPhase, const vector<infoType>& mapJumpFromToType)
{
	// process each stone removal move
	for (auto& curMove : stoneRemovalList) {
		perfectMoveInfo bestPmi{curMove.from, infoType::CENTER, miniMax::SKV_VALUE_INVALID, 0, 0, 0, miniMax::PLYINFO_VALUE_INVALID};
		for (auto& choice : infoAboutChoices.choices) {
			moveInfo move;
			move.setId(choice.possibilityId);
			if (curMove.from != move.from || curMove.to != move.to) {
				continue; // skip if the move does not match the current stone removal move
			}
			if (choice.shortValue > bestPmi.moveValue || (choice.plyInfo < bestPmi.plyInfo && choice.shortValue == bestPmi.moveValue)) {
				bestPmi = perfectMoveInfo{choice, move, mustStoneBeRemoved, inSettingPhase, inJumpPhase, mapJumpFromToType};
			}
		}
		pmis.push_back(bestPmi);
	}
}

//-----------------------------------------------------------------------------
// Name: perfectMoveInfo::calcPerfectMoveInfo()
// Desc: Translates the miniMax::stateInfo into perfect move information fitting to GUI
//-----------------------------------------------------------------------------
bool millField2D::perfectMoveInfo::calcPerfectMoveInfo(vector<perfectMoveInfo> &pmis, const miniMax::stateInfo &infoAboutChoices, bool mustStoneBeRemoved, bool inSettingPhase, bool inJumpPhase)
{
	// locals
	vector<moveInfo> 			stoneRemovalList;		// list of moves implying stone removal
	vector<infoType> 			mapJumpFromToType;		// mapping jumping from position to infoType
	set<fieldStruct::fieldPos> 	jumpFromPositions;		// positions from which a jump can occur

	// initialize the mapping for jumping moves
	if (inJumpPhase) {
		perfectMoveInfo::initJumpMapping(infoAboutChoices, mapJumpFromToType, jumpFromPositions);

		// show numbers on each jumping stone
		pmis.push_back(perfectMoveInfo{*std::next(jumpFromPositions.begin(), 0), infoType::JUMP_SRC_1, miniMax::SKV_VALUE_INVALID, 0, 0, 0, miniMax::PLYINFO_VALUE_INVALID});
		pmis.push_back(perfectMoveInfo{*std::next(jumpFromPositions.begin(), 1), infoType::JUMP_SRC_2, miniMax::SKV_VALUE_INVALID, 0, 0, 0, miniMax::PLYINFO_VALUE_INVALID});
		pmis.push_back(perfectMoveInfo{*std::next(jumpFromPositions.begin(), 2), infoType::JUMP_SRC_3, miniMax::SKV_VALUE_INVALID, 0, 0, 0, miniMax::PLYINFO_VALUE_INVALID});
	}

	// process each choice
	for (auto& choice : infoAboutChoices.choices) {
		moveInfo move;
		move.setId(choice.possibilityId);

		// removing a stone
		if (mustStoneBeRemoved) {
			// skip all moves where no mill is closed
			if (move.removeStone >= fieldStruct::size) {
				continue;
			}

		// normal move
		} else {
			// skip all moves where a mill is closed
			if (move.removeStone < fieldStruct::size) {
				stoneRemovalList.push_back(move);
				continue;
			}
		}

		// push back the perfect move info
		pmis.push_back(perfectMoveInfo{choice, move, mustStoneBeRemoved, inSettingPhase, inJumpPhase, mapJumpFromToType});
	}

	if (stoneRemovalList.size()) {
		perfectMoveInfo::removeDuplicates(stoneRemovalList);
		perfectMoveInfo::addBestMovesForClosedMills(stoneRemovalList, pmis, infoAboutChoices, mustStoneBeRemoved, inSettingPhase, inJumpPhase, mapJumpFromToType);
	}

	return true;
}

#pragma endregion
