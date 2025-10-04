/*********************************************************************\
	threadSpecific.cpp
 	Copyright (c) Thomas Weber. All rights reserved.				
	Licensed under the MIT License.
	https://github.com/madweasel/muehle
\*********************************************************************/

#include "threadSpecific.h"

#include <set>

using namespace std;

//-----------------------------------------------------------------------------
// Name: threadVarsStruct()
// Desc: constructor 
//-----------------------------------------------------------------------------
threadVarsStruct::threadVarsStruct(stateAddressing& sa) :
	sa(sa)
{
	reset();
}

//-----------------------------------------------------------------------------
// Name: threadVarsStruct()
// Desc: destructor
//-----------------------------------------------------------------------------
threadVarsStruct::~threadVarsStruct()
{
}

//-----------------------------------------------------------------------------
// Name: reset()
// Desc: set state 0 of layer 0 and reset all values
//-----------------------------------------------------------------------------
void threadVarsStruct::reset()
{
	curSearchDepth			= 0;	
	shortValue				= miniMax::SKV_VALUE_INVALID;
	
	field.reset(playerId::playerOne);
	setSituation(199, 0);	// 199 means no stones are set
}

#pragma region gameInterface
//-----------------------------------------------------------------------------
// Name: getPossibilities()
// Desc: Returns the possible moves for the current situation.
//-----------------------------------------------------------------------------
void threadVarsStruct::getPossibilities(vector<unsigned int> &possibilityIds) const
{
	field.getPossibilities(possibilityIds);
}

//-----------------------------------------------------------------------------
// Name: getValueOfSituation()
// Desc: Returns the value of the current situation. 
//-----------------------------------------------------------------------------
miniMax::twoBit threadVarsStruct::getValueOfSituation() const
{
	return shortValue;
}

//-----------------------------------------------------------------------------
// Name: getLayerAndStateNumber()
// Desc: Returns the symmetry operation, the layer number and the state number of the current situation.
//		 Current player has white stones, the opponent the black ones.
//-----------------------------------------------------------------------------
void threadVarsStruct::getLayerAndStateNumber(unsigned int &layerNum, unsigned int &stateNumber, stateAddressing::symOperationId &symOp) const
{
	layerNum 	= getLayerNumber();
	sa.getStateNumber(layerNum, stateNumber, symOp, field);
}

//-----------------------------------------------------------------------------
// Name: getLayerNumber()
// Desc: Returns the layer number of the current situation. 
//-----------------------------------------------------------------------------
unsigned int threadVarsStruct::getLayerNumber() const
{
    return sa.getLayerNumber(field);
}

//-----------------------------------------------------------------------------
// Name: getSymStateNumWithDuplicates()
// Desc: Returns the state number of the current situation for all symmetry operations. 
//-----------------------------------------------------------------------------
void threadVarsStruct::getSymStateNumWithDuplicates(vector<miniMax::stateAdressStruct> &symStates) const
{
	// locals
	unsigned int			symmetryOperation;
	unsigned int			layerNum = getLayerNumber();
	array<unsigned int, stateAddressing::NUM_SYM_OPERATIONS> stateNumbers;
				 
	symStates.clear();

	sa.getStateNumbersOfSymmetricStates(field, stateNumbers);

	for (symmetryOperation = 0; symmetryOperation < stateAddressing::NUM_SYM_OPERATIONS; ++symmetryOperation) {
		symStates.push_back(miniMax::stateAdressStruct{stateNumbers[symmetryOperation], (unsigned char) layerNum});
	}
}

//-----------------------------------------------------------------------------
// Name: getPredecessors()
// Desc: Returns the predecessors statenumbers of the current situation. 
//-----------------------------------------------------------------------------
void threadVarsStruct::getPredecessors(vector<miniMax::retroAnalysis::predVars> &predVars)
{
    predVars.clear();
	field.getPredecessors(predFields);
	for (auto &predField : predFields) {
		if (!storePredecessor(predField, predVars)) {
			predVars.clear();
			return;
		}
	}
}

//-----------------------------------------------------------------------------
// Name: getField()
// Desc: Get the current game field. 
//-----------------------------------------------------------------------------
const fieldStruct &threadVarsStruct::getField() const
{
	return field;
}

//-----------------------------------------------------------------------------
// Name: applySymOp()
// Desc: Apply a symmetry operation to the current situation. 
//-----------------------------------------------------------------------------
void threadVarsStruct::applySymOp(stateAddressing::symOperationId symmetryOperationNumber, bool doInverseOperation, bool playerToMoveChanged)
{
	// apply sym operation
	sa.applySymmetryTransfToField(symmetryOperationNumber, doInverseOperation, field);

	// invert field if necessary
	if (playerToMoveChanged) {
		field.invert();
	}
}

//-----------------------------------------------------------------------------
// Name: setSituation()
// Desc: Sets the field based on the layer number and the state number.
//		 Current player has white stones, the opponent the black ones.
//       Returns false if the field state is invalid.
//-----------------------------------------------------------------------------
bool threadVarsStruct::setSituation(unsigned int layerNum, unsigned int stateNumber)
{
	// locals
	bool 				fieldIntegrityOK;
	
	// reset values
    curSearchDepth     	= 0;
	shortValue		   	= miniMax::SKV_VALUE_GAME_DRAWN;

	// set the situation
	fieldIntegrityOK = sa.getFieldByStateNumber(layerNum, stateNumber, field, fieldStruct::playerWhite);

	// when opponent is unable to move than current player has won
	if (field.hasGameFinished()) {
		shortValue = (field.getWinner() == field.getCurPlayer().id ? miniMax::SKV_VALUE_GAME_WON : miniMax::SKV_VALUE_GAME_LOST);
	}
	
	// return
	return fieldIntegrityOK;
}

//-----------------------------------------------------------------------------
// Name: setField()
// Desc: Sets the field based on the passed field.
//-----------------------------------------------------------------------------
void threadVarsStruct::setField(const fieldStruct &field)
{
	// reset values
	curSearchDepth     	= 0;
	shortValue		   	= miniMax::SKV_VALUE_GAME_DRAWN;

	// set the situation
	this->field = field;

	// when opponent is unable to move than current player has won
	if (field.hasGameFinished()) {
		shortValue = (field.getWinner() == field.getCurPlayer().id ? miniMax::SKV_VALUE_GAME_WON : miniMax::SKV_VALUE_GAME_LOST);
	}
}

//-----------------------------------------------------------------------------
// Name: move()
// Desc: Perform a move based on the id of the possibility. 
//-----------------------------------------------------------------------------
void threadVarsStruct::move(unsigned int idPossibility, void *&pBackup)
{
    stateBackup &	oldState	= oldStates[curSearchDepth];
	moveInfo 		move;

	oldState.shortValue		    = shortValue;											
	pBackup						= (void*) &oldState;
	curSearchDepth++;

	move.setId(idPossibility);
	field.move(move, oldState);

	// when game has finished and it's the turn of the opponent, then the state is lost for him
	if (field.hasGameFinished()) {
		shortValue = miniMax::SKV_VALUE_GAME_LOST;
	}
}

//-----------------------------------------------------------------------------
// Name: undo()
// Desc: Undo a move based on the id of the possibility.
//-----------------------------------------------------------------------------
void threadVarsStruct::undo(unsigned int idPossibility, void *pBackup)
{
	stateBackup& oldState = *((stateBackup*) pBackup);

	// reset old value
	shortValue							= oldState.shortValue;
	curSearchDepth--;
	
	field.undo(oldState);
}

//-----------------------------------------------------------------------------
// Name: printField()
// Desc: Print the current situation. 
//-----------------------------------------------------------------------------
void threadVarsStruct::printField(miniMax::twoBit value, unsigned int indentSpaces) const
{
    char  wonStr[]  = "WON";
    char  lostStr[] = "LOST";
    char  drawStr[] = "DRAW";
    char  invStr[]  = "INVALID";
    char* table[4]  = {invStr, lostStr, drawStr, wonStr};

	cout << "\nstate value             : " << table[value];
	cout << "\nstones set              : " << field.getNumStonesSet() << "\n";
	field.print();	
}

//-----------------------------------------------------------------------------
// Name: printMoveInformation()
// Desc: Print the move information from where to where a stone is moved. 
//-----------------------------------------------------------------------------
void threadVarsStruct::printMoveInformation(unsigned int idPossibility) const
{
	moveInfo 		move;
	move.setId(idPossibility);

	// move
	if (field.inSettingPhase())		cout << "set stone to "      << (char) (move.to   + 97) << endl;															
	else							cout << "move from "		 << (char) (move.from + 97) << " to " << (char) (move.to + 97) << endl;

	if (move.removeStone != fieldStruct::size) {
		cout << "remove stone from " << (char) (move.removeStone + 97) << endl;
	}
}
#pragma endregion

#pragma region private functions
//-----------------------------------------------------------------------------
// Name: storePredecessor()
// Desc: Internal function for storing the predecessor state in the list of predecessors. 
//-----------------------------------------------------------------------------
bool threadVarsStruct::storePredecessor(const fieldStruct::core& predField, vector<miniMax::retroAnalysis::predVars>& predVars) const
{
	// locals
	unsigned int						symmetryOperation, symmetryOperation2;
	miniMax::retroAnalysis::predVars	newPredVar;
	fieldStruct::core					symField;
	stateAddressing::layerId			layerNumber = sa.getLayerNumber(predField);
	stateAddressing::stateId			stateNumber;

	// Use unordered_set to track unique stateAdressStructs
	std::set<miniMax::stateAdressStruct> seen;

	if (!sa.getStateNumber(layerNumber, stateNumber, symmetryOperation, predField)) {
		cout << "ERROR: getStateNumber() failed, when storing predecessor state!" << endl;
		return false;
	}

	// store current state, without any symmetry operation applied
	newPredVar.predLayerNumber 		= layerNumber;
	newPredVar.playerToMoveChanged	= true;
	newPredVar.predStateNumber 		= stateNumber;
	newPredVar.predSymOperation		= symmetryOperation;
	predVars.push_back(newPredVar);
	seen.insert(miniMax::stateAdressStruct{stateNumber, (unsigned char)layerNumber});

	for (symmetryOperation = 0; symmetryOperation < stateAddressing::NUM_SYM_OPERATIONS; ++symmetryOperation) {

		// TODO: Check if this is really correct. Shouldn't it be "if (symmetryOperation != SO_DO_NOTHING && sa.isSymOperationInvariant(...)) continue;"?
		// only add if sym operation actually does something
		if (!sa.isSymOperationInvariant(symmetryOperation, predField)) {
			continue;
		}

		// copy field
		symField = predField; 

		// appy symmetry operation
		sa.applySymmetryTransfToField(symmetryOperation, false, symField);

		// store state number
		if (!sa.getStateNumber(layerNumber, stateNumber, symmetryOperation2, symField)) {
			cout << "ERROR: getStateNumber() failed, when storing predecessor state!" << endl;
			return false;
		}

		// there are now two symmetry operations: the one applied to the field and the one applied due to mapping to the state number
		newPredVar.predStateNumber 	= stateNumber;
		newPredVar.predSymOperation = sa.concSymOperation[symmetryOperation][symmetryOperation2];

		// add only if not already in set
		miniMax::stateAdressStruct key{stateNumber, (unsigned char)layerNumber};
		if (seen.find(key) == seen.end()) {
			predVars.push_back(newPredVar);
			seen.insert(key);
		}
	}

	return true;
}
#pragma endregion
