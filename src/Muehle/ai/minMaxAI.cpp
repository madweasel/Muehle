/*********************************************************************
	minMaxAI.cpp													  
 	Copyright (c) Thomas Weber. All rights reserved.				
	Licensed under the MIT License.
	https://github.com/madweasel/madweasels-cpp
\*********************************************************************/

#include "minMaxAI.h"

#pragma region minMaxAI

//-----------------------------------------------------------------------------
// Name: minMaxAI()
// Desc: minMaxAI class constructor
//-----------------------------------------------------------------------------
minMaxAI::minMaxAI()
{
	threadVars.resize(mm.getNumThreads(), threadVarsStruct());
}

//-----------------------------------------------------------------------------
// Name: ~minMaxAI()
// Desc: minMaxAI class destructor
//-----------------------------------------------------------------------------
minMaxAI::~minMaxAI()
{
}

//-----------------------------------------------------------------------------
// Name: play()
// Desc: 
//-----------------------------------------------------------------------------
void minMaxAI::play(const fieldStruct& theField, moveInfo& move)
{
	// globals
	for (auto& vars : threadVars) {
		vars.field			= theField;
		vars.curSearchDepth	= 0;
		vars.currentValue	= 0;
		vars.oldStates.clear();
	}
	unsigned int		bestChoice;
	unsigned int		searchDepth;
	miniMax::stateInfo 	infoAboutChoices;

	// automatic depth
	if (depthOfFullTree == 0) {
		if (theField.inSettingPhase())						searchDepth	=  5;  
		else if (theField.getCurPlayer().numStones <= 4)	searchDepth =  7;  
		else if (theField.getOppPlayer().numStones <= 4)	searchDepth =  7;  
		else												searchDepth =  7;  
	} else {
		searchDepth = depthOfFullTree;
	}

	// reserve memory
	for (auto& vars : threadVars) {
		vars.oldStates.resize(searchDepth + 3);
	}

	// start the miniMax-algorithmn
	mm.setSearchDepth(searchDepth);
	if (!mm.getBestChoice(bestChoice, infoAboutChoices)) {
		throw std::runtime_error("Error in minMaxAI::play() - getBestChoice() failed.");
	}

	// decode the best choice - convert possibility ID to moveInfo with integrated stone removal
	move.setId(bestChoice);
}

//-----------------------------------------------------------------------------
// Name: setSearchDepth()
// Desc: 
//-----------------------------------------------------------------------------
void minMaxAI::setSearchDepth(unsigned int depth)
{
	depthOfFullTree = depth;
}

//-----------------------------------------------------------------------------
// Name: prepareBestChoiceCalculation()
// Desc: 
//-----------------------------------------------------------------------------
void minMaxAI::prepareCalculation()
{
	// calculate current value
	for (auto& vars : threadVars) {
		vars.currentValue = 0;
	}
}

//-----------------------------------------------------------------------------
// Name: getPossibilities()
// Desc: 
//-----------------------------------------------------------------------------
void minMaxAI::getPossibilities(unsigned int threadNo, vector<unsigned int>& possibilityIds)
{
	threadVars[threadNo].field.getPossibilities(possibilityIds);
}

//-----------------------------------------------------------------------------
// Name: getValueOfSituation()
// Desc: 
//-----------------------------------------------------------------------------
void minMaxAI::getValueOfSituation(unsigned int threadNo, float &floatValue, miniMax::twoBit &shortValue)
{
	fieldClass 					&field 			= threadVars[threadNo].field;
	float						&currentValue 	= threadVars[threadNo].currentValue;

	if (field.hasGameFinished()) {
		currentValue = (field.getWinner() == field.getCurPlayer().id) ? miniMax::FPKV_MAX_VALUE 	: miniMax::FPKV_MIN_VALUE;
		shortValue 	 = (field.getWinner() == field.getCurPlayer().id) ? miniMax::SKV_VALUE_GAME_WON : miniMax::SKV_VALUE_GAME_LOST;
	} else {
		int oppMissing = field.getOppPlayer().numStonesMissing;
		int curMissing = field.getCurPlayer().numStonesMissing;
		float curMoves = field.getCurPlayer().numPossibleMoves * 0.1f;
		float oppMoves = field.getOppPlayer().numPossibleMoves * 0.1f;
		currentValue = static_cast<float>(oppMissing - curMissing) + curMoves - oppMoves;
		shortValue 		= miniMax::SKV_VALUE_GAME_DRAWN;
	}
	floatValue = currentValue;
}

//-----------------------------------------------------------------------------
// Name: getMaxNumPossibilities()
// Desc:
//-----------------------------------------------------------------------------
unsigned int minMaxAI::getMaxNumPossibilities()
{
	return fieldStruct::maxNumPosMoves;
}

//-----------------------------------------------------------------------------
// Name: move()
// Desc: 
//-----------------------------------------------------------------------------
void minMaxAI::move(unsigned int threadNo, unsigned int idPossibility, bool& playerToMoveChanged, void* &pBackup)
{
	// locals
	fieldClass 					&field 			= threadVars[threadNo].field;
	float						&currentValue 	= threadVars[threadNo].currentValue;
	unsigned int				&curSearchDepth = threadVars[threadNo].curSearchDepth;
	std::vector<backupStruct>	&oldStates 		= threadVars[threadNo].oldStates;
	minMaxAI::backupStruct& 	oldStateMm 		= oldStates[curSearchDepth];
	fieldStruct::backupStruct& 	oldStateFs 		= oldStateMm;
	moveInfo 					moveToMake;

	// convert possibility ID to moveInfo
	moveToMake.setId(idPossibility);

	// backup
	oldStateMm.value 	= currentValue;
	pBackup 			= (void*) &oldStates[curSearchDepth];
	curSearchDepth++;
	
	if (!field.move(moveToMake, oldStateFs)) {
		throw std::runtime_error("Invalid move detected during minMaxAI::move execution.");
	};

	// player changes after every move
	playerToMoveChanged = true;
}

//-----------------------------------------------------------------------------
// Name: undo()
// Desc: 
//-----------------------------------------------------------------------------
void minMaxAI::undo(unsigned int threadNo, unsigned int idPossibility, bool& playerToMoveChanged, void *pBackup)
{
	// locals
	fieldClass 					&field 			= threadVars[threadNo].field;
	float						&currentValue 	= threadVars[threadNo].currentValue;
	unsigned int				&curSearchDepth = threadVars[threadNo].curSearchDepth;
	std::vector<backupStruct>	&oldStates 		= threadVars[threadNo].oldStates;
	if (pBackup == nullptr) {
		throw std::runtime_error("Null backup pointer detected during minMaxAI::undo execution.");
	}
	backupStruct& oldStateMm = *static_cast<backupStruct*>(pBackup);

	// reset old value
	currentValue = oldStateMm.value;
	curSearchDepth--;

	fieldStruct::backupStruct& oldStateFs = oldStateMm;
	if (!field.undo(oldStateFs)) {
		throw std::runtime_error("Invalid undo detected during minMaxAI::undo execution.");
	}

	// player always changes back after undo
	playerToMoveChanged = true;
}

//-----------------------------------------------------------------------------
// Name: printMoveInformation()
// Desc: 
//-----------------------------------------------------------------------------
void minMaxAI::printMoveInformation(unsigned int threadNo, unsigned int idPossibility)
{
	// convert possibility ID to moveInfo to print move details
	moveInfo move;
	move.setId(idPossibility);

	// move
	if (threadVars[threadNo].field.inSettingPhase())	cout << "set stone to "      << (char) (move.to + 97) << endl;															
	else												cout << "move from "		 << (char) (move.from + 97) << " to " << (char) (move.to + 97) << endl;
	if (move.removeStone < fieldStruct::size)			cout << "remove stone from " << (char) (move.removeStone + 97) << endl;														
}
#pragma endregion

#pragma region fieldClass
//-----------------------------------------------------------------------------
// Name: fieldClass()
// Desc:
//-----------------------------------------------------------------------------
minMaxAI::fieldClass::fieldClass() : fieldStruct() 
{
	warnings.fill(warningId::noWarning);
}

//-----------------------------------------------------------------------------
// Name: fieldClass()
// Desc:
//-----------------------------------------------------------------------------
minMaxAI::fieldClass::fieldClass(const fieldStruct &theField) : fieldStruct(theField) 
{
	warnings.fill(warningId::noWarning);
	
	// go in every direction
	for (unsigned int i=0; i<size; i++) {
		setWarningAndMill(i, neighbour[i][0][0], neighbour[i][0][1]);
		setWarningAndMill(i, neighbour[i][1][0], neighbour[i][1][1]);
	}
}

//-----------------------------------------------------------------------------
// Name: addWarning()
// Desc:
//-----------------------------------------------------------------------------
void minMaxAI::fieldClass::setWarningAndMill(unsigned int stone, unsigned int firstNeighbour, unsigned int secondNeighbour)
{
	playerId	rowOwner		= field[stone];
	warningId	rowOwnerWarning	= (rowOwner == playerId::playerOne) ? warningId::playerOneWarning : warningId::playerTwoWarning;

	if (rowOwner != playerId::squareIsFree && field[firstNeighbour ] == playerId::squareIsFree && field[secondNeighbour] == rowOwner) warnings[firstNeighbour ] = addWarning(warnings[firstNeighbour ], rowOwnerWarning);
	if (rowOwner != playerId::squareIsFree && field[secondNeighbour] == playerId::squareIsFree && field[firstNeighbour ] == rowOwner) warnings[secondNeighbour] = addWarning(warnings[secondNeighbour], rowOwnerWarning);
}

//-----------------------------------------------------------------------------
// Name: addWarning()
// Desc:
//-----------------------------------------------------------------------------
warningId minMaxAI::fieldClass::addWarning(warningId existingWarning, warningId newWarning)
{
	if (existingWarning == warningId::noWarning) return newWarning;
	if (newWarning == warningId::noWarning) return existingWarning;
	return static_cast<warningId>(static_cast<unsigned int>(existingWarning) | static_cast<unsigned int>(newWarning));
}
#pragma endregion
