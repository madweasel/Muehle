/*********************************************************************
	ticTacToe.cpp													  
 	Copyright (c) Thomas Weber. All rights reserved.				
	Licensed under the MIT License.
	https://github.com/madweasel/weaselLibrary
\*********************************************************************/

#include "ticTacToe.h"
#include <cassert>

#pragma region ticTacToe

// constructor: here the symmetry variables are initialized
ticTacToe::ticTacToe(stateAddressing& sa) : 
	symVars{getNumberOfLayers(), sa},
	sa(sa)
{
	// tv[] must be initialized
	prepareCalculation();
}

// checks if the game is finished and if so, it returns true
bool ticTacToe::hasAnyBodyWon(unsigned int threadNo)
{
	if (threadNo >= tv.size() || threadNo>=mm.getNumThreads()) return false;
	return tv[threadNo].curState.hasPlayerWon(gameId::egoPlayerId)
		|| tv[threadNo].curState.hasPlayerWon(gameId::oppPlayerId);
}

// set the stone on the field for the current player and corresponding thread. each thread has its own field.
bool ticTacToe::setStone(unsigned int pos, unsigned int threadNo)
{
	if (threadNo >= tv.size() || threadNo>=mm.getNumThreads()) return false;		// invalid thread number
	if (pos >= gameState::size) return false;										// invalid position
	if (tv[threadNo].curState.field[pos] != gameState::squareIsFree) return false;	// position already set
	tv[threadNo].curState.field[pos]	= gameId::egoPlayerId;						// set stone onto field
	tv[threadNo].curState.invert();													// invert field, since it shall be the view from the opponent player, which is now on turn
	return true;
}

// let the make a choice and calls setStone() with the best choice
void ticTacToe::letComputerSetStone()
{
	unsigned int bestChoice;
	miniMax::stateInfo infoAboutChoices;
	mm.getBestChoice(bestChoice, infoAboutChoices);
	setStone(bestChoice);
}

// to be called before the database calculation starts
void ticTacToe::prepareCalculation()
{
	tv.resize(mm.getNumThreads());
	unsigned int threadIdCounter = 0;
	for (auto& curThread : tv) {
		curThread.moveHistory.resize(getMaxNumPlies());
		setSituation(threadIdCounter, 0, 0);
		threadIdCounter++;
	}
}

// returns true if retro analysis shall be used, otherwise alpha-beta algortihmn is used
bool ticTacToe::shallRetroAnalysisBeUsed(unsigned int layerNum)
{
	return useRetroAnalysis;
}

// returns the maximum number of possibilities to make a move
unsigned int ticTacToe::getMaxNumPossibilities()
{
	return gameState::size;
}

// return the number of layers
unsigned int ticTacToe::getNumberOfLayers()
{
	return 10;
}

// returns the maximum number of moves necessary to win/lose the game 
unsigned int ticTacToe::getMaxNumPlies()
{
	return gameState::size;
}

// returns the states in a certain layer
unsigned int ticTacToe::getNumberOfKnotsInLayer(unsigned int layerNum)
{
	if (symVars.considerSymmetry) {
		return symVars.numStatesWithSymmetry[layerNum];
	} else {
		return sa.getNumberOfKnotsInLayer(layerNum);
	}
}

// Returns the layer numbers of the successor layers
void ticTacToe::getSuccLayers(unsigned int layerNum, vector<unsigned int>& succLayers)
{
	sa.getSuccLayers(layerNum, succLayers);
}

// Returns the layer number of the partner layer
miniMax::gameInterface::uint_1d ticTacToe::getPartnerLayers(unsigned int layerNum)
{
	return sa.getPartnerLayers(layerNum);
}

// Returns the value of the current situation for a specific thread
void ticTacToe::getValueOfSituation(unsigned int threadNo, float& floatValue, miniMax::twoBit& shortValue)
{
	// checks
	if (threadNo >= tv.size() || threadNo>=mm.getNumThreads()) return;	// invalid thread number

	auto&			f					= tv[threadNo].curState.field;
	unsigned int	numCurPlayerStones	= gameState::countStonesOnField(f, gameId::egoPlayerId);
	unsigned int	numOppPlayerStones	= gameState::countStonesOnField(f, gameId::oppPlayerId);
	bool			curPlayerHasWon		= tv[threadNo].curState.hasPlayerWon(gameId::egoPlayerId);
	bool			oppPlayerHasWon		= tv[threadNo].curState.hasPlayerWon(gameId::oppPlayerId);

	// situation valid?
	if (numCurPlayerStones == numOppPlayerStones || numCurPlayerStones + 1 == numOppPlayerStones) {
		if (curPlayerHasWon && oppPlayerHasWon) {
			shortValue = miniMax::SKV_VALUE_INVALID;
			floatValue = miniMax::FPKV_INV_VALUE;
		} else if (curPlayerHasWon) {
			shortValue = miniMax::SKV_VALUE_GAME_WON;
			floatValue = miniMax::FPKV_MAX_VALUE;
		} else if (oppPlayerHasWon) {
			shortValue = miniMax::SKV_VALUE_GAME_LOST;
			floatValue = miniMax::FPKV_MIN_VALUE;
		} else {
			shortValue = miniMax::SKV_VALUE_GAME_DRAWN;
			floatValue = 0;
		}
	} else {
		shortValue = miniMax::SKV_VALUE_INVALID;
		floatValue = miniMax::FPKV_INV_VALUE;
	}
}

// Returns the layer number of the current state for a specific thread
void ticTacToe::getLayerAndStateNumber(unsigned int threadNo, unsigned int& layerNum, unsigned int& stateNumber, unsigned int& symOp)
{
	if (threadNo>=tv.size() || threadNo>=mm.getNumThreads()) return;
	getLayerAndStateNumber(tv[threadNo].curState, layerNum, stateNumber, symOp, symVars);
}

// Returns the layer number of the current state for a specific thread
unsigned int ticTacToe::getLayerNumber(unsigned int threadNo)
{
	if (threadNo>=tv.size() || threadNo>=mm.getNumThreads()) return getNumberOfLayers();
	return sa.getLayerNumber(tv[threadNo].curState.field);
}

// Returns the possible moves for a specific thread
void ticTacToe::getPossibilities(unsigned int threadNo, vector<unsigned int>& possibilityIds)
{
	possibilityIds.clear();

	if (threadNo>=tv.size() || threadNo>=mm.getNumThreads()) return;
	if (!isStateIntegrityOk(threadNo)) return;
	if (hasAnyBodyWon(threadNo)) return;

	auto&			field				= tv[threadNo].curState.field;
	unsigned int	numCurPlayerStones	= gameState::countStonesOnField(field, gameId::egoPlayerId);
	unsigned int	numOppPlayerStones	= gameState::countStonesOnField(field, gameId::oppPlayerId);
	unsigned int	freeSquareCounter	= 0;

	possibilityIds.resize(gameState::size - numCurPlayerStones - numOppPlayerStones);

	for (unsigned int pos = 0; pos < gameState::size; pos++) {
		if (field[pos] == gameState::squareIsFree) {
			possibilityIds[freeSquareCounter] = pos;
			freeSquareCounter++;
		}
	}
}

// Modify the current state for a specific thread
void ticTacToe::move(unsigned int threadNo, unsigned int idPossibility, bool& playerToMoveChanged, void* &pBackup)
{
	if (threadNo >= tv.size() || threadNo>=mm.getNumThreads()) return;
	if (idPossibility >= gameState::size) return;

	auto&			field				= tv[threadNo].curState.field;
	unsigned int	pos					= idPossibility;

	// set stone onto field
	if (field[pos] == gameState::squareIsFree) {
		field[pos] = tv[threadNo].curState.curPlayerId;

		// remember move so that it can be undone
		unsigned int stonesSet = tv[threadNo].curState.getNumStonesOnField();
		tv[threadNo].moveHistory[stonesSet - 1] = pos;
		pBackup = (void*) &tv[threadNo].moveHistory[stonesSet - 1];

		// invert field, since it shall be the view from the opponent player, which is now on turn
		playerToMoveChanged = true;
		tv[threadNo].curState.invert();
	}
}

// Undo the current state for a specific thread
void ticTacToe::undo(unsigned int threadNo, unsigned int idPossibility, bool& playerToMoveChanged, void* pBackup)
{
	if (threadNo >= tv.size() || threadNo>=mm.getNumThreads()) return;
	if (pBackup == nullptr) return;

	auto&			field	= tv[threadNo].curState.field;
	unsigned int	pos 	= *((unsigned int*) pBackup);

	field[pos] = gameState::squareIsFree;

	// invert field, since it shall be the view from the opponent player, which is now on turn
	playerToMoveChanged = true;
	tv[threadNo].curState.invert();
}

// Prints a string with information about the current state
void ticTacToe::printMoveInformation(unsigned int threadNo, unsigned int idPossibility)
{
	if (threadNo>=tv.size() || threadNo>=mm.getNumThreads()) return;
	if (idPossibility >= gameState::size) return;
	cout << "\nMove: " << idPossibility << " - " << gameState::squareChar[idPossibility] << endl;
}

// Return the predecessor states for a specific thread
void ticTacToe::getPredecessors(unsigned int threadNo, vector<miniMax::retroAnalysis::predVars>& predVars)
{
	if (threadNo>=tv.size() || threadNo>=mm.getNumThreads()) return;

	predVars.clear();

	miniMax::retroAnalysis::predVars pv;
	gameState myGameState = tv[threadNo].curState;

	// when current player has won, there are no predecessors
	if (myGameState.hasPlayerWon(gameId::egoPlayerId)) {
		return;
	}

	// check if meta state
	getLayerAndStateNumber(myGameState, pv.predLayerNumber, pv.predStateNumber, pv.predSymOperation, symVars);

	// when a stone is removed than it's the turn of the opponent player
	myGameState.invert();

	// try to remove each own stone of the inverted field
	for (unsigned int pos = 0; pos < gameState::size; pos++) {

		// only remove own stones
		if (myGameState.field[pos] != gameState::curPlayerId) continue;

		// if player has won, then stone must be removed from a winning line
		if (myGameState.hasPlayerWon(gameId::egoPlayerId)) {
			if (!myGameState.isStonePartOfWinningAllLines(pos, gameId::egoPlayerId)) continue;
		}

		myGameState.field[pos]	= gameState::squareIsFree;

		pv.playerToMoveChanged	= true;

		// get state number of inverted field with remove opponent stone
		getLayerAndStateNumber(myGameState, pv.predLayerNumber, pv.predStateNumber, pv.predSymOperation, symVars);
		predVars.push_back(pv);

		// put stone back
		myGameState.field[pos]	= gameState::curPlayerId;
	}
}

// Returns true if the state is valid
bool ticTacToe::isStateIntegrityOk(unsigned int threadNo)
{
	if (threadNo>=tv.size() || threadNo>=mm.getNumThreads()) return false;

	bool		curPlayerHasWon				= tv[threadNo].curState.hasPlayerWon(gameId::egoPlayerId);
	bool		oppPlayerHasWon				= tv[threadNo].curState.hasPlayerWon(gameId::oppPlayerId);
	unsigned int stonesCountedOnField		= 0;
	unsigned int stonesEgoPlayerOnField		= 0;
	unsigned int stonesOppPlayerOnField		= 0;
	unsigned int stonesFreeSquaresOnField	= 0;

	for (unsigned int pos = 0; pos < gameState::size; pos++) {
		auto curStone = tv[threadNo].curState.field[pos];
		if (curStone == gameId::egoPlayerId 	) stonesEgoPlayerOnField  ++;
		if (curStone == gameId::oppPlayerId 	) stonesOppPlayerOnField  ++;
		if (curStone == gameState::squareIsFree	) stonesFreeSquaresOnField++;
	}
	
	stonesCountedOnField = stonesEgoPlayerOnField + stonesOppPlayerOnField;

	if (stonesCountedOnField + stonesFreeSquaresOnField != gameState::size) return false;
	if (stonesCountedOnField != tv[threadNo].curState.getNumStonesOnField()) return false;
	if (stonesEgoPlayerOnField != stonesOppPlayerOnField && stonesEgoPlayerOnField + 1 != stonesOppPlayerOnField) return false;
	if (curPlayerHasWon && oppPlayerHasWon) return false;

	for (auto& curMove : tv[threadNo].moveHistory) {
		if (curMove >= gameState::size) return false;
	}

	return true;
}

// this is a game specific information
bool ticTacToe::lostIfUnableToMove(unsigned int threadNo)
{
	// the only reason not being able to move is that the game is already finished,
	// but it is drawn when all fields are set, and not lost
	return false;
}

// get the state numbers of the symmetric states for a specific thread
void ticTacToe::getSymStateNumWithDuplicates(unsigned int threadNo, vector<miniMax::stateAdressStruct>& symStates)
{
	if (threadNo >= tv.size() || threadNo>=mm.getNumThreads()) return;
	
	unsigned int layerNum;
	unsigned int stateNum;
	unsigned int symOpTmp;
	symStates.clear();

	for (unsigned int symOp = 0; symOp < symVars.numSymOperations; symOp++) {
		applySymOp(threadNo, symOp, false, false);
		getLayerAndStateNumber(threadNo, layerNum, stateNum, symOpTmp);
		applySymOp(threadNo, symOp, true, false);
		symStates.push_back(miniMax::stateAdressStruct{stateNum, (unsigned char) layerNum});
	}
}

// set the a specific situation based on the state number for a specific thread
bool ticTacToe::setSituation(unsigned int threadNo, unsigned int layerNum, unsigned int stateNumber)
{
	if (threadNo	>= tv.size() || threadNo>=mm.getNumThreads()) return false;
	if (layerNum	>= getNumberOfLayers()						) return false;
	if (stateNumber	>= getNumberOfKnotsInLayer(layerNum)		) return false;
	
	// get a shorter field pointer
	auto&			field					= tv[threadNo].curState.field;

	// map the state number if symmetry is used 
	if (symVars.considerSymmetry) {
		stateNumber = symVars.stateNumNoSymmetry[layerNum][stateNumber];
	}

	// consider preceeding stones
	sa.getField(layerNum, stateNumber, field);

	// set the current player id
	return isStateIntegrityOk(threadNo); 
}

// apply a symmetry operation to the current state of a specific thread
void ticTacToe::applySymOp(unsigned int threadNo, unsigned char symmetryOperationNumber, bool doInverseOperation, bool playerToMoveChanged)
{
	if (threadNo>=tv.size() || threadNo>=mm.getNumThreads()) return;
	if (symmetryOperationNumber >= symVars.numSymOperations) return;
	gameState& cs = tv[threadNo].curState;
	symVars.applySymOp(cs.field, doInverseOperation, symmetryOperationNumber);
}

// returns the layer number and the state number of the current state
void ticTacToe::getLayerAndStateNumber(gameState& gs, unsigned int& layerNum, unsigned int& stateNumber, unsigned int& symOp, const symmetryVars& symVars)
{
	layerNum	= sa.getLayerNumber(gs.field);
	stateNumber = sa.getStateNumber(layerNum, gs.field);
	symOp		= 0;

	// consider symmetry
	if (symVars.considerSymmetry) {
		symOp		= symVars.symOpWithSymmetry   [layerNum][stateNumber];
		stateNumber = symVars.stateNumWithSymmetry[layerNum][stateNumber];
	}
}

// print the field
void ticTacToe::printField(unsigned int threadNo, miniMax::twoBit value, unsigned int indentSpaces)
{
	if (threadNo>=tv.size() || threadNo>=mm.getNumThreads()) return;
	char  wonStr[]  = "WON";
    char  lostStr[] = "LOST";
    char  drawStr[] = "DRAW";
    char  invStr[]  = "INVALID";
    char* table[4]  = {invStr, lostStr, drawStr, wonStr};
	cout << "\n" << string(indentSpaces, ' ') <<  "state value             : " << table[value];
	cout << "\n" << string(indentSpaces, ' ') <<  "current player          : " << gameState::squareChar[gameId::egoPlayerId];
	cout << "\n" << string(indentSpaces, ' ') <<  "opponent player         : " << gameState::squareChar[gameId::oppPlayerId];
	tv[threadNo].curState.print(indentSpaces);
}

// returns a string with information about the current state
wstring ticTacToe::getOutputInformation(unsigned int layerNum)
{
	return wstring(L"");
}

// returns base ^ exp
int ticTacToe::ipow(int base, int exp)
{
    int result = 1;
    while (true) {
        if (exp & 1) result *= base;
        exp >>= 1;
        if (!exp) break;
        base *= base;
    }
    return result;
}

#pragma endregion

#pragma region gameState

const char ticTacToe::gameState::squareChar[3] = {' ', 'o', 'x'};

// inverts the field, switching the current player
void ticTacToe::gameState::invert()
{
	for (unsigned int pos = 0; pos < gameState::size; pos++) {
		if (field[pos] != gameState::squareIsFree) {
			field[pos] = (field[pos] == gameId::egoPlayerId ? gameId::oppPlayerId : gameId::egoPlayerId);
		}
	}
}

// returns the number of stones set already on the field
unsigned int ticTacToe::gameState::getNumStonesOnField()
{
	if (field.size() < gameState::size) return 0; // Ensure field has enough elements
	return gameState::countStonesOnField(field, gameId::egoPlayerId) 
		 + gameState::countStonesOnField(field, gameId::oppPlayerId);
}

// print the field
void ticTacToe::gameState::print(unsigned int indentSpaces)
{
	cout << endl << setw(indentSpaces+1) << squareChar[field[0]] << " | " << squareChar[field[1]] << " | " << squareChar[field[2]];
	cout << endl << setw(indentSpaces+1) << '-' << " | " << '-' << " | " << '-';
	cout << endl << setw(indentSpaces+1) << squareChar[field[3]] << " | " << squareChar[field[4]] << " | " << squareChar[field[5]];
	cout << endl << setw(indentSpaces+1) << '-' << " | " << '-' << " | " << '-';
	cout << endl << setw(indentSpaces+1) << squareChar[field[6]] << " | " << squareChar[field[7]] << " | " << squareChar[field[8]] << endl;
}

// Returns true if the player has won on the field 'f'
bool ticTacToe::gameState::hasPlayerWon(char playerid)
{
	if (areValuesEqual(field[0], field[1], field[2], playerid)) return true;
	if (areValuesEqual(field[3], field[4], field[5], playerid)) return true;
	if (areValuesEqual(field[6], field[7], field[8], playerid)) return true;
	if (areValuesEqual(field[0], field[3], field[6], playerid)) return true;
	if (areValuesEqual(field[1], field[4], field[7], playerid)) return true;
	if (areValuesEqual(field[2], field[5], field[8], playerid)) return true;
	if (areValuesEqual(field[0], field[4], field[8], playerid)) return true;
	if (areValuesEqual(field[2], field[4], field[6], playerid)) return true;
	return false;
}

// Returns true if the stone at position 'pos' is part of a winning line
bool ticTacToe::gameState::isStonePartOfWinningAllLines(unsigned int pos, char playerid)
{
	vector<char> winningStones(gameState::size, 0);
	vector<vector<char>> linesPositions = {
		{0, 1, 2}, {3, 4, 5}, {6, 7, 8},		// horizontal lines
		{0, 3, 6}, {1, 4, 7}, {2, 5, 8},		// vertical lines
		{0, 4, 8}, {2, 4, 6}					// diagonal lines
	};
	for (auto& line : linesPositions) {
		if (areValuesEqual(field[line[0]], field[line[1]], field[line[2]], playerid)) {
			 winningStones[line[0]]++; winningStones[line[1]]++; winningStones[line[2]]++; 
		}
	}
	// get maximum number of winning stones
	char maxWinningStones = *std::max_element(winningStones.begin(), winningStones.end());
	// return true if the stone at position 'pos' is part of a winning line
	return winningStones[pos] == maxWinningStones;
}

// Counts the number of stones on the field for a specific player
unsigned int ticTacToe::gameState::countStonesOnField(const uint_1d &field, char playerid)
{
    return (unsigned int) count(&field[0], &field[0] + size, playerid);
}

// Returns true if all parameters are the same
bool ticTacToe::gameState::areValuesEqual(char a, char b, char c, char d)
{
	return (a == b && b == c && c == d);
}

#pragma endregion

#pragma region symmetryVars

// constructor
ticTacToe::symmetryVars::symmetryVars(unsigned int numLayers, stateAddressing& sa)
{
	// a prerequisite is that all symmetric states are within the same layer,
	// which is not the case for type A
	assert(typeid(sa) != typeid(ticTacToe::stateAddressingTypeA));

	// use hardcoded values for the symmetry operations
	symOpMap.resize(numSymOperations);
	symOpMap[symOp::NONE] = {			0, 1, 2,
										3, 4, 5,
										6, 7, 8};
	symOpMap[symOp::VERTICAL] = {		2, 1, 0,
										5, 4, 3,
										8, 7, 6};
	symOpMap[symOp::HORIZONTAL] = {		6, 7, 8,
										3, 4, 5,
										0, 1, 2};
	symOpMap[symOp::LEFT_DIAG] = {		0, 3, 6,
										1, 4, 7,
										2, 5, 8};
	symOpMap[symOp::RIGHT_DIAG] = {		8, 5, 2,
										7, 4, 1,
										6, 3, 0};
	symOpMap[symOp::LEFT] = {			2, 5, 8,
										1, 4, 7,
										0, 3, 6};
	symOpMap[symOp::HALF_TURN] = {		8, 7, 6,
										5, 4, 3,
										2, 1, 0};
	symOpMap[symOp::RIGHT] = {			6, 3, 0,
										7, 4, 1,
										8, 5, 2};

	numStatesWithSymmetry	.resize(numLayers);
	stateNumNoSymmetry		.resize(numLayers);
	stateNumWithSymmetry	.resize(numLayers);
	symOpWithSymmetry		.resize(numLayers);

	// locals
	unsigned int symOpAfterSymOp;		// ???
	unsigned int stateNumAfterSymOp;	// ???
	unsigned int layerNumAfterSymOp;	// ???
	uint_1d  field(gameState::size, 0); // field of the current state

	// mapping between symmytric and non-symmetric statenumbering is necessary for each layer
	for (unsigned int layerNum = 0; layerNum < numLayers; layerNum++) {

		unsigned int numStatesNoSym		= sa.getNumberOfKnotsInLayer(layerNum);			// number of states without symmetry
		unsigned int symStateCounter	= 0;											// counter for states with symmetry
		unsigned int threadNo			= 0;											// only one thread is used

		// initialize vectors with invalid values
		stateNumWithSymmetry	[layerNum].resize (numStatesNoSym, numStatesNoSym);		// mapping from non-symmetric to symmetric state numbers
		symOpWithSymmetry		[layerNum].resize (numStatesNoSym, numSymOperations);	// mapping from symmetric state numbers to symmetry operations
		stateNumNoSymmetry		[layerNum].reserve(numStatesNoSym);						// mapping from symmetric to non-symmetric state numbers

		// process each non-symmetric state number
		for (unsigned int stateNumNoSym = 0; stateNumNoSym < numStatesNoSym; stateNumNoSym++) {

			// has this non-symmetric state number already been found by a symmetry operation?
			if (stateNumWithSymmetry[layerNum][stateNumNoSym] < numStatesNoSym) {
				continue;
			}

			// set the sitatuion for the current state number
			sa.getField(layerNum, stateNumNoSym, field);

			// store the non-symmetric state number
			stateNumNoSymmetry[layerNum].push_back(stateNumNoSym);

			// apply each single symmetry operation to get symmetric states to the current one
			for (unsigned int symOp = 0; symOp < numSymOperations; symOp++) {

				// apply symmetry operation
				applySymOp(field, false, symOp);

				// get layer and state number of the current state after symmetry operation
				layerNumAfterSymOp = sa.getLayerNumber(field);
				stateNumAfterSymOp = sa.getStateNumber(layerNumAfterSymOp, field);

				// layer number must be the same
				assert(layerNumAfterSymOp == layerNum);

				// store the symmetric state number
				if (stateNumWithSymmetry[layerNumAfterSymOp][stateNumAfterSymOp] == numStatesNoSym) {
					stateNumWithSymmetry[layerNumAfterSymOp][stateNumAfterSymOp] = symStateCounter;
					symOpWithSymmetry   [layerNumAfterSymOp][stateNumAfterSymOp] = symOp;
				}
				
				// undo symmetry operation
				applySymOp(field, true, symOp);
			}
			symStateCounter++;
		}

		// store number of symmetric states for this layer
		numStatesWithSymmetry[layerNum] = symStateCounter;
	}
	considerSymmetry = true;	
}

// applies a symmetry operation to the field
void ticTacToe::symmetryVars::applySymOp(uint_1d& field, bool doInverseOperation, unsigned int symmetryOperationNumber) 
{
	vector<char> tmpField;
	tmpField.resize(gameState::size);
	tmpField.assign(&field[0], &field[0] + gameState::size);

	for (size_t pos = 0; pos < gameState::size; pos++) {
		if (doInverseOperation) {
			field[pos] = tmpField[symOpMap[symmetryOperationNumber][pos]];
		} else {
			field[symOpMap[symmetryOperationNumber][pos]] = tmpField[pos];
		}
	}
}

#pragma endregion

#pragma region stateAddressingType
// --- type A --------------------------------------------------------------------------------
unsigned int ticTacToe::stateAddressingTypeA::getNumberOfKnotsInLayer(unsigned int layerNum)
{
	if (layerNum == gameState::size) {
		return ipow(2, 9);
	}
	return ipow(2, layerNum) * ipow(3, numLayers - layerNum - 2);
}

miniMax::gameInterface::uint_1d ticTacToe::stateAddressingTypeA::getPartnerLayers(unsigned int layerNum)
{
	// all layers shall be calculated at once
	uint_1d partnerLayers;
	for (unsigned int partnerLayer = 0; partnerLayer < numLayers; partnerLayer++) {
		if (partnerLayer != layerNum) partnerLayers.push_back(partnerLayer);
	}
	return partnerLayers;
}

void ticTacToe::stateAddressingTypeA::getSuccLayers(unsigned int layerNum, vector<unsigned int> &succLayers)
{
	succLayers.clear();

	// let's consider for example layer 2
	// #|#|x	
	// ?|?|?	
	// ?|?|?	
	// while the current player (x) can place a stone on any free field, 
	// layers with higher layer number will not follow anymore, only layers with lower layer number
	// the same layer number shall not be considered again
	for (unsigned int succLayer = 0; succLayer < layerNum; succLayer++) {
		succLayers.push_back(succLayer);
	}
}

void ticTacToe::stateAddressingTypeA::getField(unsigned int layerNum, unsigned int stateNumber, uint_1d &field)
{
	unsigned int	numPermutationsSoFar	= 1;

	// remove all stones
	for (size_t i = 0; i < gameState::size; i++) { field[i] = 0; }

	// set master stone
	if (layerNum < 9) {
		field[layerNum] = gameId::egoPlayerId;
	}

	// consider preceeding stones
	for (unsigned int pos = 0; pos < layerNum; pos++) {
		field[pos]				= (stateNumber % (numPermutationsSoFar*2)) / numPermutationsSoFar;
		stateNumber			   -= field[pos] * numPermutationsSoFar;
		numPermutationsSoFar   *= 2;
	}

	// consider subsequent stones
	for (unsigned int pos = layerNum + 1; pos < gameState::size; pos++) {
		field[pos]				= (stateNumber % (numPermutationsSoFar*3)) / numPermutationsSoFar;
		stateNumber			   -= field[pos] * numPermutationsSoFar;
		numPermutationsSoFar   *= 3;
	}
}

unsigned int ticTacToe::stateAddressingTypeA::getStateNumber(unsigned int layerNum, const uint_1d &field)
{
	unsigned int	numPermutationsSoFar	= 1;
	unsigned int	stateNumber				= 0;
	
	// consider preceeding stones
	for (unsigned int pos = 0; pos < layerNum; pos++) {
		stateNumber += numPermutationsSoFar * field[pos];
		numPermutationsSoFar *= 2;
	}

	// consider subsequent stones
	for (unsigned int pos = layerNum + 1; pos < gameState::size; pos++) {
		stateNumber += numPermutationsSoFar * field[pos];
		numPermutationsSoFar *= 3;
	}
	return stateNumber;
}

unsigned int ticTacToe::stateAddressingTypeA::getLayerNumber(const uint_1d &field)
{
	// search for the first stone of the current player
	unsigned int layerNum;
	for (layerNum = 0; layerNum < gameState::size; layerNum++) {
		if (field[layerNum] == gameId::egoPlayerId) break;
	}
	return layerNum;
}

// --- type B --------------------------------------------------------------------------------
// The layer number is equal to gameState::size minus the number of stones on the field.
// The state number is calculated by the following formula:
// number of states for layer 9 = 1
// number of states for layer 8 = 2*9
// number of states for layer 7 = 2*9*8
// number of states for layer 6 = 2*9*8*7
unsigned int ticTacToe::stateAddressingTypeB::getNumberOfKnotsInLayer(unsigned int layerNum)
{
	if (layerNum >= numLayers) return maxStateIndex;
	return numberOfKnotsInLayer[layerNum];
}

miniMax::gameInterface::uint_1d ticTacToe::stateAddressingTypeB::getPartnerLayers(unsigned int layerNum)
{
	return {layerNum};
}

void ticTacToe::stateAddressingTypeB::getSuccLayers(unsigned int layerNum, vector<unsigned int> &succLayers)
{
	succLayers.clear();
	if (layerNum >= numLayers) return;
	if (layerNum == 0) return;
	succLayers = {layerNum - 1};
}

void ticTacToe::stateAddressingTypeB::getField(unsigned int layerNum, unsigned int stateNumber, uint_1d &field)
{
	unsigned int numStateIndex = mappingStateNumberToIndex[layerNum][stateNumber];
	getFieldFromStateIndex(numStateIndex, field);
}

unsigned int ticTacToe::stateAddressingTypeB::getStateNumber(unsigned int layerNum, const uint_1d &field)
{
	unsigned int stateIndex = getStateIndex(field);
	return mappingStateIndexToNumber[layerNum][stateIndex];
}

unsigned int ticTacToe::stateAddressingTypeB::getLayerNumber(const uint_1d &field)
{
	if (field.size() < gameState::size) return 0; // Ensure field has enough elements
	unsigned int numStones = gameState::countStonesOnField(field, gameId::egoPlayerId) 
						   + gameState::countStonesOnField(field, gameId::oppPlayerId);
	return gameState::size - numStones;
}

ticTacToe::stateAddressingTypeB::stateAddressingTypeB()
{
	// locals
	unsigned int numStones;
	unsigned int stateNumber;
	uint_1d field(gameState::size, 0);

	// iterate over each possible state and calculate the mappings
	mappingStateIndexToNumber.resize(numLayers);
	mappingStateNumberToIndex.resize(numLayers);
	numberOfKnotsInLayer.resize(numLayers);

	for (unsigned int layerNum = 0; layerNum < numLayers; layerNum++) {
		numStones 	= gameState::size - layerNum;
		stateNumber = 0;
		mappingStateIndexToNumber[layerNum].resize(maxStateIndex, maxStateIndex);
		mappingStateNumberToIndex[layerNum].resize(maxStateIndex, maxStateIndex);

		for (unsigned int stateIndex = 0; stateIndex < maxStateIndex; stateIndex++) {
			getFieldFromStateIndex(stateIndex, field);
			if (getLayerNumber(field) != layerNum) {
				continue;	// skip invalid states
			}
			mappingStateNumberToIndex[layerNum][stateNumber] = stateIndex;
			mappingStateIndexToNumber[layerNum][stateIndex] = stateNumber;
			stateNumber++;
		}
		numberOfKnotsInLayer[layerNum] = stateNumber;								// store number of states for this layer
		mappingStateNumberToIndex[layerNum].resize(numberOfKnotsInLayer[layerNum]);	// resize to the number of states for this layer
	}
}

void ticTacToe::stateAddressingTypeB::getFieldFromStateIndex(unsigned int stateIdex, uint_1d &field)
{
	if (field.size() < gameState::size) return; // Ensure field has enough elements
	for (unsigned int pos = 0; pos < gameState::size; pos++) {
		field[pos] = (stateIdex / ipow(3, pos)) % 3;
	}
}

unsigned int ticTacToe::stateAddressingTypeB::getStateIndex(const uint_1d &field)
{
	unsigned int stateIndex = 0;
	for (int j = 0; j < gameState::size; ++j) {
		stateIndex += field[j] * ipow(3, j);
	}
	return stateIndex;
}
#pragma endregion
