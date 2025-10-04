/*********************************************************************
	muehle.cpp													  
 	Copyright (c) Thomas Weber. All rights reserved.				
	Licensed under the MIT License.
	https://github.com/madweasel/madweasels-cpp
\*********************************************************************/

#include "muehle.h"

using namespace std;

#pragma region muehle
//-----------------------------------------------------------------------------
// Name: muehle()
// Desc: muehle class constructor
//-----------------------------------------------------------------------------
muehle::muehle()
{
	srand( (unsigned)time( nullptr ) );
}

//-----------------------------------------------------------------------------
// Name: ~muehle()
// Desc: muehle class destructor
//-----------------------------------------------------------------------------
muehle::~muehle()
{
}

//-----------------------------------------------------------------------------
// Name: beginNewGame()
// Desc: Reinitializes the muehle object.
//-----------------------------------------------------------------------------
void muehle::beginNewGame(muehleAI *firstPlayerAI, muehleAI *secondPlayerAI, playerId currentPlayer, bool settingPhase, bool resetField)
{
	// delete history
	moveLogCurrentIndex = 0;
	moveLog.clear();

	// calc beginning player
	if (currentPlayer == playerId::playerOne || currentPlayer == playerId::playerTwo) {
		beginningPlayer					= currentPlayer;
	} else {
		beginningPlayer					= (rand() % 2) ? playerId::playerOne : playerId::playerTwo;
	}

	// create new field
	if (resetField) {
		field.reset(beginningPlayer);
	}
	field.setSituation(field.getField(), settingPhase, 0);
	if (field.getCurPlayer().id != beginningPlayer) {
		field.invert();
	}

	// remember initial field
	initialField 	= field;
	playerOneAI 	= firstPlayerAI;
	playerTwoAI 	= secondPlayerAI;
}

//-----------------------------------------------------------------------------
// Name: putStone()
// Desc: Put a stone onto the field during the setting phase.
//-----------------------------------------------------------------------------
bool muehle::putStone(fieldPos pos, playerId player)
{
    // check parameters
    if (player != playerId::playerOne && player != playerId::playerTwo) return false;
    if (pos    >= fieldStruct::size)                                    return false;
    if (field.getStone(pos) != playerId::squareIsFree)	                return false;

	// set stone
	fieldStruct::fieldArray theField = field.getField();
	theField[pos] = player;
	field.setSituation(theField, field.inSettingPhase(), 0);

	// return success
	return true;
}

//-----------------------------------------------------------------------------
// Name: settingPhaseHasFinished()
// Desc: This function has to be called when the setting phase has finished.
//-----------------------------------------------------------------------------
bool muehle::settingPhaseHasFinished()
{
	// remember initialField
	initialField = field;
    return true;
}

//-----------------------------------------------------------------------------
// Name: getField()
// Desc: Copy the current field state into the array 'pField'.
//-----------------------------------------------------------------------------
const fieldStruct::fieldArray& muehle::getField() const
{
	return field.getField();
}

//-----------------------------------------------------------------------------
// Name: wouldMillBeClosed()
// Desc: Checks if the given move would close a mill.
//-----------------------------------------------------------------------------
bool muehle::wouldMillBeClosed(const moveInfo &move) const
{
	// check if the move is valid
	if (!field.inSettingPhase()) {
		if (move.from > fieldStruct::size || move.to >= fieldStruct::size) return false;
		if (field.getStone(move.from) != field.getCurPlayer().id) return false;
	} else {
		if (move.from != fieldStruct::size || move.to >= fieldStruct::size) return false;
		if (field.getStone(move.to) != playerId::squareIsFree) return false;
	}

	// create a copy of the field
	fieldStruct tempField = field;
	fieldStruct::backupStruct dummyState;
	tempField.move(move, dummyState);

	// check if a mill is closed
	return tempField.isStonePartOfMill(move.to) > 0;
}

//-----------------------------------------------------------------------------
// Name: getLog()
// Desc: Copy the whole history of moves into the passed arrays, which must be of size [MaxNumMoves].
//-----------------------------------------------------------------------------
void muehle::getLog(vector<logItem>& log, unsigned int& currentIndex) const
{
	log 			= moveLog;
	currentIndex 	= moveLogCurrentIndex;
}

//-----------------------------------------------------------------------------
// Name: isCurrentPlayerHuman()
// Desc: Returns true if the current player is not assigned to an AI.
//-----------------------------------------------------------------------------
bool muehle::isCurrentPlayerHuman() const
{
	if (field.getCurPlayer().id == playerId::playerOne)	return (playerOneAI == nullptr) ? true : false;
	else												return (playerTwoAI == nullptr) ? true : false;
}

//-----------------------------------------------------------------------------
// Name: isOpponentPlayerHuman()
// Desc: Returns true if the opponent player is not assigned to an AI.
//-----------------------------------------------------------------------------
bool muehle::isOpponentPlayerHuman() const
{
	if (field.getOppPlayer().id == playerId::playerOne)	return (playerOneAI == nullptr) ? true : false;
	else												return (playerTwoAI == nullptr) ? true : false;
}

//-----------------------------------------------------------------------------
// Name: gameHasFinished()
// Desc: Checks if the game has finished.
//-----------------------------------------------------------------------------
bool muehle::gameHasFinished() const
{
	if (getNumTurnsToRemis() == 0) {
		return true;
	}
	if (getNumRepeatedMoves() >= NumRepeatedMovesToRemis) {
		return true;
	}
	return field.getWinner() != playerId::squareIsFree;
}

//-----------------------------------------------------------------------------
// Name: getWinner()
// Desc: Returns playerId::squareIsFree if no player has won yet or game drawn, otherwise the playerId of the winner.
//-----------------------------------------------------------------------------
playerId muehle::getWinner() const
{
	if (gameHasFinished()) {
		return field.getWinner();
	}
    return playerId::squareIsFree;
}

//-----------------------------------------------------------------------------
// Name: getNumTurnsToRemis()
// Desc:
//-----------------------------------------------------------------------------
unsigned int muehle::getNumTurnsToRemis() const
{
	// Only consider moves up to the current index
	std::vector<logItem> logSubset(moveLog.begin(), moveLog.begin() + moveLogCurrentIndex);
	unsigned int numNormalMovesDone = logItem::getNumNormalMovesWithoutRemoval(logSubset);
    return numMovesToRemis - numNormalMovesDone;
}

//-----------------------------------------------------------------------------
// Name: getNumRepeatedMoves()
// Desc:
//-----------------------------------------------------------------------------
float muehle::getNumRepeatedMoves() const
{
	// if we are in the setting phase, we cannot have repeated moves
	if (inSettingPhase()) {
		return 0.0f;
	}

	// Only consider moves up to the current index
	std::vector<logItem> logSubset(moveLog.begin(), moveLog.begin() + moveLogCurrentIndex);
	return logItem::getNumRepeatedMoves(logSubset);
}

//-----------------------------------------------------------------------------
// Name: isMoveAllowed()
// Desc: Checks if the move is allowed.
//------------------------------------------------------------------------------
bool muehle::isMoveAllowed(const moveInfo &move, bool ignoreStoneRemoval) const
{
	std::vector<unsigned int> possibilityIds;
	field.getPossibilities(possibilityIds);
	for (const auto& id : possibilityIds) {
		if (ignoreStoneRemoval) {
			moveInfo allowedMove;
			allowedMove.setId(id);
			if (move.from == allowedMove.from && move.to == allowedMove.to) {
				return true;
			}
		} else if (move.getId() == id) {
			return true;
		}
	}
	return false;
}

//-----------------------------------------------------------------------------
// Name: setAI()
// Desc: Assigns an AI to a player.
//-----------------------------------------------------------------------------
void muehle::setAI(playerId player, muehleAI *AI)
{
	if (player == playerId::playerOne) { playerOneAI = AI; }
	if (player == playerId::playerTwo) { playerTwoAI = AI; }
}

//-----------------------------------------------------------------------------
// Name: getChoiceOfSpecialAI()
// Desc: Returns the move the passed AI would do.
//-----------------------------------------------------------------------------
void muehle::getChoiceOfSpecialAI(muehleAI *AI, moveInfo& move) const
{
	fieldStruct theField;
	move 		= moveInfo{};
	theField 	= field;
	if (AI != nullptr && !gameHasFinished()) {
		AI->play(theField, move);
	}
}

//-----------------------------------------------------------------------------
// Name: getComputersChoice()
// Desc: Returns the move the AI of the current player would do.
//-----------------------------------------------------------------------------
void muehle::getComputersChoice(moveInfo& move) const
{
	fieldStruct theField;
	move 		= moveInfo{};
	theField 	= field;
	
    if (!gameHasFinished()) {
		if (field.getCurPlayer().id == playerId::playerOne)	{ if (playerOneAI != nullptr) playerOneAI->play(theField, move); }
		else												{ if (playerTwoAI != nullptr) playerTwoAI->play(theField, move); }
    }
}

//-----------------------------------------------------------------------------
// Name: moveStone()
// Desc: 
//-----------------------------------------------------------------------------
bool muehle::moveStone(const moveInfo& move)
{	
	// avoid index override
	if (getMovesDone() >= MaxNumMoves) return false;

	// is game still running ?
	if (gameHasFinished()) return false;

	// locals
	fieldStruct::backupStruct 	oldState;
	playerId 					movingPlayer 		= field.getCurPlayer().id;
	bool 						isMoveCompleted 	= false;

	// if a is mill closed and no information on which stone to remove is given, then the move is not completed yet
	if (move.removeStone == fieldStruct::size) {
		if (!stoneMustBeRemoved) {
			stoneMustBeRemoved = wouldMillBeClosed(move);
			isMoveCompleted = !stoneMustBeRemoved;
		} else {
			isMoveCompleted = true;
		}
		if (!isMoveCompleted) {
			return false;
		}
	}
	stoneMustBeRemoved = false;

	// perform move
	if (!field.move(move, oldState)) return false;

	// store history
	if (moveLogCurrentIndex < moveLog.size()) {
		moveLog[moveLogCurrentIndex] = logItem{move, movingPlayer};
		moveLog.resize(moveLogCurrentIndex + 1);
	} else {
		moveLog.push_back(logItem{move, movingPlayer});
	}
	moveLogCurrentIndex++;

	// everything is ok
	return true;
}

//-----------------------------------------------------------------------------
// Name: setNumMovesToRemis()
// Desc: Sets the number of moves after which the game is remis.
//-----------------------------------------------------------------------------
void muehle::setNumMovesToRemis(unsigned int numMoves)
{
	numMovesToRemis = numMoves ? numMoves : 10000;
}

//-----------------------------------------------------------------------------
// Name: setCurrentGameState()
// Desc: Set an arbitrary game state as the current one.
//-----------------------------------------------------------------------------
bool muehle::setCurrentGameState(fieldStruct& curState)
{
	field 				= curState;
	moveLogCurrentIndex = 0;
	moveLog.clear();
	return true;
}

//-----------------------------------------------------------------------------
// Name: printField()
// Desc: Calls the printField() function of the current field.
//       Prints the current game state on the screen.
//-----------------------------------------------------------------------------
void muehle::printField() const
{
	field.print();
}

//-----------------------------------------------------------------------------
// Name: redoLastMove()
// Desc: 
//-----------------------------------------------------------------------------
bool muehle::redoLastMove(void)
{
	if (moveLogCurrentIndex >= moveLog.size()) {
		return false; // no more moves to redo
	}
	vector<logItem> 	moveLog_bak = moveLog;

	// get last move
	logItem lastMove = moveLog[moveLogCurrentIndex];

	// perform move
	moveStone(lastMove.move);

	// restore the log
	moveLog = moveLog_bak; 
	return true;
}

//-----------------------------------------------------------------------------
// Name: undoLastMove()
// Desc: Sets the initial field as the current one and apply all (minus one) moves from the move history.
//-----------------------------------------------------------------------------
bool muehle::undoLastMove(void)
{
	// locals
	unsigned int 		movesDone_bak	  = getMovesDone();
	vector<logItem> 	moveLog_bak;
	
	// at least one move must be done
	if (!movesDone_bak) {
		return false;
	}

	// make backup of log
	moveLog_bak	= moveLog;

	// reset field
	field 				= initialField;
	moveLogCurrentIndex = 0;

	// and play again, except the last move
	for (unsigned int i=0; i<movesDone_bak-1; i++) {
		moveStone(moveLog_bak[i].move);
	}
	// restore the log
	moveLog = moveLog_bak; 
	return true;
}

//-----------------------------------------------------------------------------
// Name: calcNumberOfRestingStones()
// Desc: 
//-----------------------------------------------------------------------------
void muehle::calcNumberOfRestingStones(int &numWhiteStonesResting, int &numBlackStonesResting)
{
	if (getCurrentPlayer() == playerId::playerTwo) {
		numWhiteStonesResting  = fieldStruct::numStonesPerPlayer - field.getCurPlayer().numStonesMissing - field.getCurPlayer().numStones;
		numBlackStonesResting  = fieldStruct::numStonesPerPlayer - field.getOppPlayer().numStonesMissing - field.getOppPlayer().numStones;
	} else {
		numWhiteStonesResting  = fieldStruct::numStonesPerPlayer - field.getOppPlayer().numStonesMissing - field.getOppPlayer().numStones;
		numBlackStonesResting  = fieldStruct::numStonesPerPlayer - field.getCurPlayer().numStonesMissing - field.getCurPlayer().numStones;
	}
}
#pragma endregion

#pragma region logItem
//-----------------------------------------------------------------------------
// Name: logItem()
// Desc: Constructor of the logItem class.
//-----------------------------------------------------------------------------
muehle::logItem::logItem(const moveInfo& move, playerId player)
	: move(move), player(player)
{
}

//-----------------------------------------------------------------------------
// Name: getNumNormalMovesWithoutRemoval()
// Desc: Returns the number of last normal moves without any stone removal.
//-----------------------------------------------------------------------------
unsigned int muehle::logItem::getNumNormalMovesWithoutRemoval(const std::vector<logItem> &log)
{
	if (log.empty()) return 0; 							// leave if there are no moves
	if (log.size() < 2) return 0;						// leave if there cant be normal moves

	// locals
	unsigned int numNormalMovesWithoutRemoval = 0;
	
	// Iterate backwards through the log to count normal moves
	for (auto it = log.rbegin(); it != log.rend(); ++it) {
		// If we encounter a move that is not normal, we stop counting
		if (it->move.isSettingPhase()) {
			break;
		}
		// Count the normal move
		numNormalMovesWithoutRemoval++;
	}

	// count only the moves of one player
	numNormalMovesWithoutRemoval /= 2;

	return numNormalMovesWithoutRemoval;
}

//-----------------------------------------------------------------------------
// Name: getNumRepeatedMoves()
// Desc: Returns the number of repeated moves in the log.
//       A repeated move is a move where the player moves a stone back to the position it was before.
//       This function counts the number of such moves.
//       It does not count setting or removing stones.
// Example: From | To | Player  | repeated moves | Comment
//          ...
//          24   | 14  | White  | 0.0            | Setting phase (not counted)
//           0   |  1  | Black  | 0.0            | forwardMoveOpponentPlayer		
//           2   |  3  | White  | 0.0            | forwardMoveCurrentPlayer
//           1   |  0  | Black  | 0.5            | backwardMoveOpponentPlayer
//           3   |  2  | White  | 1.0            | backwardMoveCurrentPlayer
//------------------------------------------------------------------------------
float muehle::logItem::getNumRepeatedMoves(const std::vector<logItem> &log)
{
	// Check if the log is valid for counting repeated moves
	if (log.empty()) return 0.0f; 							// leave if there are no moves
	if (log.size() < 3) return 0.0f;						// leave if there cant be repeated moves
	
	// consider the last move as backwardMoveCurrentPlayer
	logItem backwardMoveCurrentPlayer 	= log[log.size() - 1];
	logItem backwardMoveOpponentPlayer 	= log[log.size() - 2];
	
	// consider the second last move as backwardMoveOpponentPlayer
	if (backwardMoveCurrentPlayer.move.isSettingPhase()) 	return 0.0f; 	// leave if last move is not a normal move
	if (backwardMoveOpponentPlayer.move.isSettingPhase()) 	return 0.0f; 	// leave if second last move is not a normal move

	// Create forward moves for the current and opponent player
	logItem forwardMoveCurrentPlayer 	= logItem{backwardMoveCurrentPlayer.move, backwardMoveCurrentPlayer.player};
	logItem forwardMoveOpponentPlayer 	= logItem{backwardMoveOpponentPlayer.move, backwardMoveOpponentPlayer.player};

	// Iterate backwards through the log to count repeated moves
	float repeatedMoves = 0.0f;
	std::vector<logItem>::const_iterator it;
	for (it = log.end() - 1; it != log.begin(); --it) {
		// Skip the last two moves, which are already considered
		if (it == log.end() - 1 || it == log.end() - 2) {
			continue;
		}

		// if not a normal move, break the loop
		// This is to ensure we only count normal moves for repeated moves
		if (it->move.isSettingPhase()) {
			break;
		}

		// Check the type of the move and count repeated moves accordingly
		if (*it == backwardMoveCurrentPlayer) {
			continue;
		} else if (*it == backwardMoveOpponentPlayer) {
			continue;
		} else if (*it == forwardMoveCurrentPlayer) {
			repeatedMoves += 0.5f; // count as half repeated move
		} else if (*it == forwardMoveOpponentPlayer) {
			repeatedMoves += 0.5f; // count as half repeated move
		// If the move is neither a forward or backward move of the current player nor the opponent player, break the loop
		} else {
			break;
		}
	}
	return repeatedMoves;
}

//-----------------------------------------------------------------------------
// Name: operator==()
// Desc: Compares two logItem objects for equality.
//------------------------------------------------------------------------------
bool muehle::logItem::operator==(const logItem &other) const
{
	return (move == other.move && player == other.player);
}
#pragma endregion
