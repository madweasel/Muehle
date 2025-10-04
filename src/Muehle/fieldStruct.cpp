/*********************************************************************
	fieldStruct.cpp													  
 	Copyright (c) Thomas Weber. All rights reserved.				
	Licensed under the MIT License.
	https://github.com/madweasel/madweasels-cpp
\*********************************************************************/

#include "fieldStruct.h"
#include <cassert>
#include <algorithm>

// Field indices
//  0------- 1----- 2
// |   3---- 4--- 5 |
// |  |   6- 7- 8 | |
//  9-10-11    12-13-14
// |  |  15-16-17 | |
// |  18----19---20 |
// 21-------22------23

using namespace std;

#pragma region playerStruct

//-----------------------------------------------------------------------------
// Name: operator==()
// Desc: Compares two fieldStructs
//-----------------------------------------------------------------------------
bool playerStruct::operator==(const playerStruct &other) const
{
	return 
		id 					== other.id
	 && numStones 			== other.numStones
	 && numStonesMissing 	== other.numStonesMissing
	 && numPossibleMoves 	== other.numPossibleMoves
	 && warning 			== other.warning
	 && numStonesSet 		== other.numStonesSet;
}

#pragma endregion

#pragma region moveInfo

//-----------------------------------------------------------------------------
// Name: moveInfo
// Desc: Constructor
//-----------------------------------------------------------------------------
moveInfo::moveInfo(unsigned int from, unsigned int to, unsigned int removeStone)
        : from(from), to(to), removeStone(removeStone) 
{
}

//-----------------------------------------------------------------------------
// Name: moveInfo
// Desc: Compares two moveInfos
//-----------------------------------------------------------------------------
bool moveInfo::operator==(const moveInfo& other) const
{
	return from == other.from && to == other.to && removeStone == other.removeStone;
}

//-----------------------------------------------------------------------------
// Name: getId()
// Desc: Returns the id of the moveInfo
//-----------------------------------------------------------------------------
moveInfo::possibilityId moveInfo::getId() const
{
	return (from * (fieldStruct::size + 1) + to) * (fieldStruct::size + 1) + removeStone;
}

//-----------------------------------------------------------------------------
// Name: setId()
// Desc: Sets the id of the moveInfo
//-----------------------------------------------------------------------------
void moveInfo::setId(possibilityId id)
{
	// Perform consistency checks
	if (id >= ((fieldStruct::size + 1) * (fieldStruct::size + 1) * (fieldStruct::size + 1))) {
		throw std::out_of_range("Invalid possibilityId: id exceeds the maximum allowed value.");
	}
	from 		= id / ((fieldStruct::size + 1) * (fieldStruct::size + 1));
	to 			= (id / (fieldStruct::size + 1)) % (fieldStruct::size + 1);
	removeStone = id % (fieldStruct::size + 1);
}

//-----------------------------------------------------------------------------
// Name: isSettingPhase()
// Desc: Returns true if the moveInfo is in setting phase
//-----------------------------------------------------------------------------
bool moveInfo::isSettingPhase() const
{
	return from == fieldStruct::size && to < fieldStruct::size;
}

//-----------------------------------------------------------------------------
// Name: getMoveInfo()
// Desc: Returns the moveInfo for a given id
//-----------------------------------------------------------------------------
const moveInfo &moveInfo::getMoveInfo(possibilityId id)
{
	static moveInfo move;
	move.setId(id);
	return move;
}
#pragma endregion

#pragma region fieldStruct

//-----------------------------------------------------------------------------
// Name: fieldStruct()
// Desc: Constructor
//-----------------------------------------------------------------------------
fieldStruct::fieldStruct()
{
	reset();
}

//-----------------------------------------------------------------------------
// Name: fieldStruct()
// Desc: Copy Constructor
//-----------------------------------------------------------------------------
fieldStruct::fieldStruct(const fieldStruct& other)
    : fieldStruct_forward(other), fieldStruct_reverse(other)
{
	// Copy the core variables from the other fieldStruct
	this->field = other.field;
	this->settingPhase = other.settingPhase;
	this->curPlayer = other.curPlayer;
	this->oppPlayer = other.oppPlayer;
	this->stonePartOfMill = other.stonePartOfMill;
	this->gameHasFinished = other.gameHasFinished;
}

//-----------------------------------------------------------------------------
// Name: ~fieldStruct()
// Desc: Destructor
//-----------------------------------------------------------------------------
fieldStruct::~fieldStruct()
{
}

//-----------------------------------------------------------------------------
// Name: operator==()
// Desc: Compares two fieldStructs
//-----------------------------------------------------------------------------
bool fieldStruct::operator==(const fieldStruct &other) const
{
	return curPlayer         == other.curPlayer
		&& oppPlayer        == other.oppPlayer
		&& settingPhase     == other.settingPhase
		&& gameHasFinished  == other.gameHasFinished
		&& field            == other.field
		&& stonePartOfMill  == other.stonePartOfMill;
}

#pragma endregion

#pragma region core variants

//-----------------------------------------------------------------------------
// Name: playerStruct::core()
// Desc: Constructor
//-----------------------------------------------------------------------------
playerStruct::core::core()
{
}

//-----------------------------------------------------------------------------
// Name: playerStruct::core()
// Desc: Constructor
//-----------------------------------------------------------------------------
playerStruct::core::core(const playerStruct &player)
{
	id 					= player.id;
	numStonesMissing	= player.numStonesMissing;
	numStones 			= player.numStones;
}

//-----------------------------------------------------------------------------
// Name: fieldStruct::core()
// Desc: Constructor
//-----------------------------------------------------------------------------
fieldStruct::core::core()
{
}

//-----------------------------------------------------------------------------
// Name: fieldStruct::core()
// Desc: Constructor
//-----------------------------------------------------------------------------
fieldStruct::core::core(const fieldStruct_variables &vars)
{
	field               = vars.getField();
	settingPhase        = vars.inSettingPhase();
	curPlayer           = vars.getCurPlayer();
	oppPlayer           = vars.getOppPlayer();
}

//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
const playerStruct::core &fieldStruct_types::core::getCurPlayer() const
{
    return curPlayer;
}

//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
const playerStruct::core &fieldStruct_types::core::getOppPlayer() const
{
    return oppPlayer;
}

//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
playerId fieldStruct_types::core::getStone(fieldPos pos) const
{
    return field[pos];
}

//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
bool fieldStruct_types::core::inSettingPhase() const
{
    return settingPhase;
}

#pragma endregion

#pragma region fieldStruct_variables

//-----------------------------------------------------------------------------
// static variables
//-----------------------------------------------------------------------------
const fieldStruct::Array2d<fieldStruct::fieldPos, fieldStruct::size, 4> fieldStruct_variables::connectedSquare = []() {
	// locals
	Array2d<fieldPos, size, 4> connectedSquare;
	auto i = size;

	// set connections
	setConnection(connectedSquare,  0,  1,  9,  i,  i);
	setConnection(connectedSquare,  1,  2,  4,  0,  i);
	setConnection(connectedSquare,  2,  i, 14,  1,  i);
	setConnection(connectedSquare,  3,  4, 10,  i,  i);
	setConnection(connectedSquare,  4,  5,  7,  3,  1);
	setConnection(connectedSquare,  5,  i, 13,  4,  i); 
	setConnection(connectedSquare,  6,  7, 11,  i,  i);
	setConnection(connectedSquare,  7,  8,  i,  6,  4);
	setConnection(connectedSquare,  8,  i, 12,  7,  i);
	setConnection(connectedSquare,  9, 10, 21,  i,  0);
	setConnection(connectedSquare, 10, 11, 18,  9,  3);
	setConnection(connectedSquare, 11,  i, 15, 10,  6);
	setConnection(connectedSquare, 12, 13, 17,  i,  8);
	setConnection(connectedSquare, 13, 14, 20, 12,  5);
	setConnection(connectedSquare, 14,  i, 23, 13,  2);
	setConnection(connectedSquare, 15, 16,  i,  i, 11);
	setConnection(connectedSquare, 16, 17, 19, 15,  i);
	setConnection(connectedSquare, 17,  i,  i, 16, 12);
	setConnection(connectedSquare, 18, 19,  i,  i, 10);
	setConnection(connectedSquare, 19, 20, 22, 18, 16);
	setConnection(connectedSquare, 20,  i,  i, 19, 13);
	setConnection(connectedSquare, 21, 22,  i,  i,  9);
	setConnection(connectedSquare, 22, 23,  i, 21, 19);
	setConnection(connectedSquare, 23,  i,  i, 22, 14);

	return connectedSquare;
}();

const fieldStruct::Array3d<fieldStruct::fieldPos, fieldStruct::size, 2, 2> fieldStruct_variables::neighbour = []() {
	// locals
	Array3d<fieldPos, size, 2, 2> neighbour;

	// neighbours
	setNeighbour(neighbour,  0,  1,  2,  9, 21);
	setNeighbour(neighbour,  1,  0,  2,  4,  7);
	setNeighbour(neighbour,  2,  0,  1, 14, 23);
	setNeighbour(neighbour,  3,  4,  5, 10, 18);
	setNeighbour(neighbour,  4,  1,  7,  3,  5);
	setNeighbour(neighbour,  5,  3,  4, 13, 20);
	setNeighbour(neighbour,  6,  7,  8, 11, 15);
	setNeighbour(neighbour,  7,  1,  4,  6,  8);
	setNeighbour(neighbour,  8,  6,  7, 12, 17);
	setNeighbour(neighbour,  9, 10, 11,  0, 21);
	setNeighbour(neighbour, 10,  9, 11,  3, 18);
	setNeighbour(neighbour, 11,  9, 10,  6, 15);
	setNeighbour(neighbour, 12, 13, 14,  8, 17);
	setNeighbour(neighbour, 13, 12, 14,  5, 20);
	setNeighbour(neighbour, 14, 12, 13,  2, 23);
	setNeighbour(neighbour, 15,  6, 11, 16, 17);
	setNeighbour(neighbour, 16, 15, 17, 19, 22);
	setNeighbour(neighbour, 17, 15, 16,  8, 12);
	setNeighbour(neighbour, 18,  3, 10, 19, 20);
	setNeighbour(neighbour, 19, 18, 20, 16, 22);
	setNeighbour(neighbour, 20,  5, 13, 18, 19);
	setNeighbour(neighbour, 21,  0,  9, 22, 23);
	setNeighbour(neighbour, 22, 16, 19, 21, 23);
	setNeighbour(neighbour, 23,  2, 14, 21, 22);
	return neighbour;
}();

//-----------------------------------------------------------------------------
// Name: printField()
// Desc: Prints the field to the console
//-----------------------------------------------------------------------------
void fieldStruct_variables::print() const
{
	// locals
	std::array<char, size> c;
	const bool longVersion = false;

	for (fieldPos index=0; index<size; index++) {
		c[index] = getCharFromStone(field[index]);
	}

    cout << "current player          : " << getCharFromStone(curPlayer.id) << " has " << curPlayer.numStones << " stones (set " << curPlayer.numStonesSet << ")\n";
    cout << "opponent player         : " << getCharFromStone(oppPlayer.id) << " has " << oppPlayer.numStones << " stones (set " << oppPlayer.numStonesSet << ")\n";
	cout << "setting phase           : " << (settingPhase ? "true" : "false");
	if (longVersion) {
		cout << "\n";
		cout << "\n   a-----b-----c   " << c[0] << "-----" << c[1] << "-----" << c[2];
		cout << "\n   |     |     |   " << "|     |     |";
		cout << "\n   | d---e---f |   " << "| " << c[3] << "---" << c[4] << "---" << c[5] << " |";
		cout << "\n   | |   |   | |   " << "| |   |   | |";
		cout << "\n   | | g-h-i | |   " << "| | " << c[6] << "-" << c[7] << "-" << c[8] << " | |";
		cout << "\n   | | | | | | |   " << "| | |   | | |";
		cout << "\n   j-k-l   m-n-o   " << c[9] << "-" << c[10] << "-" << c[11] << "   " << c[12] << "-" << c[13] << "-" << c[14];
		cout << "\n   | | | | | | |   " << "| | |   | | |";
		cout << "\n   | | p-q-r | |   " << "| | " << c[15] << "-" << c[16] << "-" << c[17] << " | |";
		cout << "\n   | |   |   | |   " << "| |   |   | |";
		cout << "\n   | s---t---u |   " << "| " << c[18] << "---" << c[19] << "---" << c[20] << " |";
		cout << "\n   |     |     |   " << "|     |     |";
		cout << "\n   v-----w-----x   " << c[21] << "-----" << c[22] << "-----" << c[23];
		cout << "\n" << endl;
	} else {
		cout << "\n" << c[0] << "-----" << c[1] << "-----" << c[2];
		cout << "\n" << "| " << c[3] << "---" << c[4] << "---" << c[5] << " |";
		cout << "\n" << "| | " << c[6] << "-" << c[7] << "-" << c[8] << " | |";
		cout << "\n" << c[9] << "-" << c[10] << "-" << c[11] << "   " << c[12] << "-" << c[13] << "-" << c[14];
		cout << "\n" << "| | " << c[15] << "-" << c[16] << "-" << c[17] << " | |";
		cout << "\n" << "| " << c[18] << "---" << c[19] << "---" << c[20] << " |";
		cout << "\n" << c[21] << "-----" << c[22] << "-----" << c[23];
		cout << "\n" << endl;
	}
}

//-----------------------------------------------------------------------------
// Name: reset()
// Desc: Resets the field to the initial state, including both players' warnings and all relevant state.
//       This function ensures that both players' warnings, stone counts, mills, and the board state are fully reset.
//-----------------------------------------------------------------------------
void fieldStruct_variables::reset(playerId firstPlayer)
{
	gameHasFinished				= false;
	curPlayer.id				= firstPlayer;
	settingPhase				= true;
	curPlayer.warning			= (curPlayer.id == playerId::playerOne) ? warningId::playerOneWarning	: warningId::playerTwoWarning;
	oppPlayer.id				= (curPlayer.id == playerId::playerOne) ? playerId::playerTwo			: playerId::playerOne;
	oppPlayer.warning			= (curPlayer.id == playerId::playerOne) ? warningId::playerTwoWarning	: warningId::playerOneWarning;
	curPlayer.numStones			= 0;
	oppPlayer.numStones			= 0;
	curPlayer.numPossibleMoves	= 24;
	oppPlayer.numPossibleMoves	= 0;
	curPlayer.numStonesMissing	= 0;
	oppPlayer.numStonesMissing	= 0;
	curPlayer.numberOfMills		= 0;
	oppPlayer.numberOfMills		= 0;
	curPlayer.numStonesSet		= 0;
	oppPlayer.numStonesSet		= 0;
	curPlayer.hasOnlyMills		= false;
	oppPlayer.hasOnlyMills		= false;

	field.fill(playerId::squareIsFree);
	stonePartOfMill.fill(0);
}

//-----------------------------------------------------------------------------
// Name: invert()
// Desc: Switches the players and inverts the field
//-----------------------------------------------------------------------------
void fieldStruct_variables::invert()
{
	std::swap(curPlayer, oppPlayer);

    for (fieldPos k=0; k<size; k++) {
        switch(field[k]) {
		case playerId::playerOne:		field[k] = playerId::playerTwo;			break;
		case playerId::playerTwo:		field[k] = playerId::playerOne;			break;
		case playerId::playerOneWarning:field[k] = playerId::playerTwoWarning;	break;
		case playerId::playerTwoWarning:field[k] = playerId::playerOneWarning;	break;
		}
    }
}

//-----------------------------------------------------------------------------
// Name: getCharFromStone()
// Desc: Returns the char representation of a stone
//-----------------------------------------------------------------------------
char fieldStruct_variables::getCharFromStone(playerId stone) const
{
	switch (stone) 
	{
	case playerId::playerOne:			return 'x';
	case playerId::playerTwo:			return 'o';
	case playerId::playerOneWarning:	return '1';
	case playerId::playerTwoWarning:	return '2';
	case playerId::playerBothWarning:	return '3';
	case playerId::squareIsFree:		return ' ';
	}
	return 'f';
}

//-----------------------------------------------------------------------------
// Name: setConnection()
// Desc: Helper function to set 'connectedSquare'
//-----------------------------------------------------------------------------
void fieldStruct_variables::setConnection(Array2d<fieldPos, size, 4>& connectedSquare, fieldPos index, int firstDirection, int secondDirection, int thirdDirection, int fourthDirection)
{
	connectedSquare[index][0] = firstDirection;
	connectedSquare[index][1] = secondDirection;
	connectedSquare[index][2] = thirdDirection;
	connectedSquare[index][3] = fourthDirection;
}

//-----------------------------------------------------------------------------
// Name: setNeighbour()
// Desc: Helper function to set 'neighbour'
//-----------------------------------------------------------------------------
void fieldStruct_variables::setNeighbour(Array3d<fieldPos, size, 2, 2>& neighbour, fieldPos index, fieldPos firstNeighbour0, fieldPos secondNeighbour0, fieldPos firstNeighbour1, fieldPos secondNeighbour1)
{
	neighbour[index][0][0] = firstNeighbour0;
	neighbour[index][0][1] = secondNeighbour0;
	neighbour[index][1][0] = firstNeighbour1;
	neighbour[index][1][1] = secondNeighbour1;
}

//-----------------------------------------------------------------------------
// Name: getWinner()
// Desc: Returns the winner of the game, and playerId::squareIsFree if the game is not finished.
//-----------------------------------------------------------------------------
playerId fieldStruct_variables::getWinner() const
{
    playerId winner = playerId::squareIsFree;

    if ((!curPlayer.numPossibleMoves) && (!settingPhase) && (curPlayer.numStones > 3))	{ winner = oppPlayer.id; }
    if ((curPlayer.numStones < 3) && (!settingPhase))									{ winner = oppPlayer.id; }
    if ((oppPlayer.numStones < 3) && (!settingPhase))									{ winner = curPlayer.id; }

    return winner;
}

//-----------------------------------------------------------------------------
// Name: getCurPlayer()
// Desc: Returns a reference to the current player
//-----------------------------------------------------------------------------
const playerStruct &fieldStruct_variables::getCurPlayer() const
{
	return curPlayer;
}

//-----------------------------------------------------------------------------
// Name: getOppPlayer()
// Desc: Returns a reference to the opponent player
//-----------------------------------------------------------------------------
const playerStruct &fieldStruct_variables::getOppPlayer() const
{
	return oppPlayer;
}

//-----------------------------------------------------------------------------
// Name: getStone()
// Desc: Returns the player id of a stone
//-----------------------------------------------------------------------------
playerId fieldStruct_variables::getStone(fieldPos pos) const
{
    return field[pos];
}

//-----------------------------------------------------------------------------
// Name: isStonePartOfMill()
// Desc: Returns the number of mills, of which this stone is part of
//-----------------------------------------------------------------------------
unsigned int fieldStruct_variables::isStonePartOfMill(fieldPos pos) const
{
    return stonePartOfMill[pos];
}

//-----------------------------------------------------------------------------
// Name: getField()
// Desc: Returns the field
//-----------------------------------------------------------------------------
const fieldStruct_variables::fieldArray &fieldStruct_variables::getField() const
{
    return field;
}

//-----------------------------------------------------------------------------
// Name: hasGameFinished()
// Desc: Returns true if the game has finished
//-----------------------------------------------------------------------------
bool fieldStruct_variables::hasGameFinished() const
{
    return gameHasFinished;
}

//-----------------------------------------------------------------------------
// Name: getNumStonesSet()
// Desc: Returns the number of stones set in the setting phase
//-----------------------------------------------------------------------------
unsigned int fieldStruct_variables::getNumStonesSet() const
{
    return curPlayer.numStonesSet + oppPlayer.numStonesSet;
}

//-----------------------------------------------------------------------------
// Name: inSettingPhase()
// Desc: Returns true if the game is in the setting phase
//-----------------------------------------------------------------------------
bool fieldStruct_variables::inSettingPhase() const
{
    return settingPhase;
}

//-----------------------------------------------------------------------------
// Name: calcHasOnlyMills()
// Desc: Updates 'hasOnlyMills'.
//       IMPORTANT: stonePartOfMill and field must be in sync!
//-----------------------------------------------------------------------------
void fieldStruct_variables::calcHasOnlyMills()
{
	// update each player
	for (playerStruct* player : {&oppPlayer, &curPlayer}) {

		// if player has no mills then he cannot have only mills
		if (!player->numberOfMills) {
			player->hasOnlyMills = false;
			continue;
		}

		player->hasOnlyMills = true;
		for (fieldPos i = 0; i < size; i++) {
			if (field[i] == player->id && !stonePartOfMill[i]) {
				player->hasOnlyMills = false;
				break;
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Name: calcNumberOfMills()
// Desc: Updates the number of mills for each player 
//-----------------------------------------------------------------------------
void fieldStruct_variables::calcNumberOfMills()
{
	// count completed mills
	curPlayer.numberOfMills = 0;
	oppPlayer.numberOfMills = 0;
	for (fieldPos i=0; i<size; i++) {
		if (field[i] == curPlayer.id)	curPlayer.numberOfMills += stonePartOfMill[i];
		else							oppPlayer.numberOfMills += stonePartOfMill[i];
	}
	curPlayer.numberOfMills /= 3;
	oppPlayer.numberOfMills /= 3;
}

//-----------------------------------------------------------------------------
// Name: calcNumStones()
// Desc: Updates the number of stones for each player
//-----------------------------------------------------------------------------
void fieldStruct_variables::calcNumStones()
{
	// count stones
	curPlayer.numStones = 0;
	oppPlayer.numStones = 0;
	for (fieldPos i=0; i<size; i++) {
		if (field[i] == playerId::squareIsFree) continue;
		if (field[i] == curPlayer.id)	curPlayer.numStones++;
		else							oppPlayer.numStones++;
	}
}

//-----------------------------------------------------------------------------
// Name: calcNumStonesSet()
// Desc: Updates the number of stones set for each player
// 	     | Total Num Stones Set | curPlayer.numStonesSet | oppPlayer.numStonesSet |
//		 | -------------------- | ---------------------- | ---------------------- |
//		 | 0                    | 0                      | 0                      |
// 	     | 1                    | 0                      | 1                      |
//		 | 2 					| 1                      | 1                      |
//		 | 3 					| 1                      | 2                      |
//		 | 4 					| 2                      | 2                      |
//		 | 5 					| 2                      | 3                      |
//		 | ...
//-----------------------------------------------------------------------------
void fieldStruct_variables::calcNumStonesSet(unsigned int totalNumStonesMissing)
{
	unsigned int totalNumStonesSet = curPlayer.numStones + oppPlayer.numStones + totalNumStonesMissing;
	curPlayer.numStonesSet = totalNumStonesSet / 2;
	oppPlayer.numStonesSet = totalNumStonesSet / 2 + totalNumStonesSet % 2;
}

//-----------------------------------------------------------------------------
// Name: calcNumPossibleMoves()
// Desc: Updates the number of possible moves for a player. 
//       IMPORTANT: Thereby the possibilities containing a stone removal are not counted.
//-----------------------------------------------------------------------------
void fieldStruct_variables::calcNumPossibleMoves(playerStruct& player) const
{
	// locals
	fieldPos i, j , k;
	unsigned int movingDirection;	

	// setting phase
	if (settingPhase) {
		player.numPossibleMoves = size - curPlayer.numStones - oppPlayer.numStones;
		return;
	}

	// normal phase
	player.numPossibleMoves = 0;

	// Only adjacent moves allowed
	if (player.numStones > 3) {
		// Optimize by avoiding repeated bounds checks and unnecessary variable assignments
		for (fieldPos i = 0; i < size; ++i) {
			if (field[i] != player.id) continue;
			const auto& connections = connectedSquare[i];
			for (unsigned int dir = 0; dir < 4; ++dir) {
				fieldPos j = connections[dir];
				// Only increment if j is a valid position and free
				if (j < size && field[j] == playerId::squareIsFree) {
					++player.numPossibleMoves;
				}
			}
		}
	// Jumping allowed: any free position
	} else if (player.numStones == 3) {
		for (i = 0; i < size; ++i) {
			if (field[i] != player.id) continue;
			for (j = 0; j < size; ++j) {
				if (field[j] == playerId::squareIsFree) {
					player.numPossibleMoves++;
				}
			}
		}
	// Less than 3 stones: no moves possible
	} else {
		player.numPossibleMoves = 0;
	}
}

//-----------------------------------------------------------------------------
// Name: calcStonePartOfMill()
// Desc: Updates the stonePartOfMill array for each player
//-----------------------------------------------------------------------------
void fieldStruct_variables::calcStonePartOfMill()
{
	for (fieldPos i=0; i<size; i++) {
		stonePartOfMill[i] = 0;
	}
	for (fieldPos i=0; i<size; i++) {
		setStonePartOfMill(i, neighbour[i][0][0], neighbour[i][0][1]);
		setStonePartOfMill(i, neighbour[i][1][0], neighbour[i][1][1]);
	}
	// since every mill was detected 3 times
	for (fieldPos i=0; i<size; i++) {
		stonePartOfMill[i] /= 3;
	}
}

//-----------------------------------------------------------------------------
// Name: setStonePartOfMill()
// Desc: Sets the stonePartOfMill array 
//-----------------------------------------------------------------------------
void fieldStruct_variables::setStonePartOfMill(fieldPos stone, fieldPos firstNeighbour, fieldPos secondNeighbour)
{
	// locals
	playerId rowOwner = field[stone];

	// mill closed ?
	if (rowOwner != playerId::squareIsFree && field[firstNeighbour] == rowOwner && field[secondNeighbour] == rowOwner) {
		stonePartOfMill[stone]++;
		stonePartOfMill[firstNeighbour]++;
		stonePartOfMill[secondNeighbour]++;
	}
}

//-----------------------------------------------------------------------------
// Name: setSituation()
// Desc: Sets the field to a specific state.
//-----------------------------------------------------------------------------
bool fieldStruct_variables::setSituation(const fieldArray& field, bool settingPhase, unsigned int totalNumStonesMissing)
{
	// Check for too many stones missing
	if (totalNumStonesMissing > 2*numStonesPerPlayer) return false;

	// totalNumStonesMissing is not used during moving phase
	if (!settingPhase && totalNumStonesMissing) return false;

	// copy
	this->field					= field;
	this->settingPhase			= settingPhase;
	gameHasFinished				= false;

	// set .numStones
	calcNumStones();

	// if current player already set 9 stones, then it cannot be setting phase any more
	if (settingPhase && curPlayer.numStones >= 9) return false;

	// if there are too many stones missing, the situation is invalid
	if (totalNumStonesMissing > 2*numStonesPerPlayer - curPlayer.numStones - oppPlayer.numStones) return false;

	// set .stonePartOfMill
	calcStonePartOfMill();

	// set .numberOfMills
	calcNumberOfMills();

	// during setting phase, the total number of missing stones must be at least the number of present on the field
	if (settingPhase && totalNumStonesMissing < curPlayer.numberOfMills + oppPlayer.numberOfMills) return false;

	// stonesSet & numStonesMissing
	if (settingPhase) {
		// The number of of mills destroyed during setting phase is given from outside.
		calcNumStonesSet(totalNumStonesMissing);
		// there must not be more stones on the field then set
		if (curPlayer.numStones > curPlayer.numStonesSet) return false;
		if (oppPlayer.numStones > oppPlayer.numStonesSet) return false;
		curPlayer.numStonesMissing	= curPlayer.numStonesSet - curPlayer.numStones;
		oppPlayer.numStonesMissing	= oppPlayer.numStonesSet - oppPlayer.numStones;
	} else {
		curPlayer.numStonesMissing	= numStonesPerPlayer - curPlayer.numStones;
		oppPlayer.numStonesMissing	= numStonesPerPlayer - oppPlayer.numStones;
		curPlayer.numStonesSet 		= numStonesPerPlayer;
		oppPlayer.numStonesSet 		= numStonesPerPlayer;
	}

	// if current player set 9 stones, then it cannot be setting phase any more
	if (settingPhase && curPlayer.numStones + curPlayer.numStonesMissing >= 9) return false;

  	// calculate number of possible moves
	calcNumPossibleMoves(curPlayer);
	calcNumPossibleMoves(oppPlayer);

	// update .hasOnlyMills for each player
	calcHasOnlyMills();

	// when opponent is unable to move than current player has won
    if (getWinner() != playerId::squareIsFree) { gameHasFinished = true; }

	// test if field is ok
	return isIntegrityOk();
}

//-----------------------------------------------------------------------------
// Name: isIntegrityOk()
// Desc: Checks if the field is in a valid state.
//       CAUTION: The following member variables are NOT verified:
//				  .field, .stonePartOfMill, .gameHasFinished, .hasOnlyMills, .numPossibleMoves, .numStonesMissing, ...
//-----------------------------------------------------------------------------
bool fieldStruct_variables::isIntegrityOk() const
{
	if (settingPhase) {

		// if 18 stones have been set, then it cannot be the setting phase anymore
		if (getNumStonesSet() >= 18) {
			return false;
		}

		// if current player already set 9 stones, then it cannot be setting phase any more
		if (curPlayer.numStones >= 9) return false;

		// if there are too many stones missing, the situation is invalid
		if (curPlayer.numStonesMissing + oppPlayer.numStonesMissing > 2*numStonesPerPlayer - curPlayer.numStones - oppPlayer.numStones) return false;

		// during setting phase, the total number of missing stones must be at least the number of present on the field
		if (curPlayer.numStonesMissing + oppPlayer.numStonesMissing < curPlayer.numberOfMills + oppPlayer.numberOfMills) return false;

		// if current player set 9 stones, then it cannot be setting phase any more
		if (curPlayer.numStones + curPlayer.numStonesMissing >= 9) return false;
		if (oppPlayer.numStones + oppPlayer.numStonesMissing >  9) return false;

		// each missing stone of a player must correspond to a mill of the other player on the field or a former mill, which has already been destroyed.
		// each destroyed mill of a player, requires a missing stone of that player. 
		if (curPlayer.numStonesMissing > oppPlayer.numberOfMills + oppPlayer.numStonesMissing) return false;
		if (oppPlayer.numStonesMissing > curPlayer.numberOfMills + curPlayer.numStonesMissing) return false;

		// if there are stones missing, then at least one player must have mills
		if (curPlayer.numStonesMissing + oppPlayer.numStonesMissing > 0 && (curPlayer.numberOfMills + oppPlayer.numberOfMills) == 0) return false;

		// if next move would be in moving phase then game must not be lost
		if (curPlayer.numStonesSet >= 8 && curPlayer.numStones < 2) return false;
		if (oppPlayer.numStonesSet >= 9 && oppPlayer.numStones < 3) return false;

		// Check consistency of number of stones on field with the number of mills
		// number of stones set might be equal, or opponent might be one stone ahead
		int numStonesSetByCurPlayer = curPlayer.numStones + curPlayer.numStonesMissing;
		int numStonesSetByOppPlayer = oppPlayer.numStones + oppPlayer.numStonesMissing;
		if (!(numStonesSetByCurPlayer - numStonesSetByOppPlayer == 0 || numStonesSetByCurPlayer - numStonesSetByOppPlayer == -1)) {
			return false;
		}
		// check consistency between variables numStones, numStonesMissing and numStonesSet
		if (numStonesSetByCurPlayer != curPlayer.numStonesSet) return false;
		if (numStonesSetByOppPlayer != oppPlayer.numStonesSet) return false;

	// moving phase
	} else { 
		// each player must have at least 2 stones
		if (curPlayer.numStones < 2 || oppPlayer.numStones < 2) return false;
		
		if (gameHasFinished) {
			// if game is finished then the opponent must have a mill
			if (curPlayer.numStones < 3 && oppPlayer.numberOfMills == 0) return false;
			// or current player is immobilized
			if (curPlayer.numPossibleMoves && curPlayer.numStones >= 3) return false;
		}
	}

	return true;
}

#pragma endregion

#pragma region fieldStruct_forward

//-----------------------------------------------------------------------------
// Name: getPossibilities()
// Desc: Returns the possible moves for the current player 
//-----------------------------------------------------------------------------
void fieldStruct_forward::getPossibilities(std::vector<moveInfo::possibilityId>& possibilityIds) const
{
	// When game has ended of course nothing happens any more
	if (gameHasFinished || !isIntegrityOk()) {
		possibilityIds.clear();
	// look what is to do
	} else {
			 if (settingPhase)			getPossSettingPhase	(possibilityIds);
		else							getPossNormalMove	(possibilityIds);
	}
}

//-----------------------------------------------------------------------------
// Name: getPossSettingPhase()
// Desc: Helper function to get the possible moves in the setting phase 
//-----------------------------------------------------------------------------
void fieldStruct_forward::getPossSettingPhase(vector<moveInfo::possibilityId>& possibilityIds) const
{
	// locals
	fieldPos 			to;
	unsigned int 		numberOfMillsBeeingClosed;
	vector<fieldPos> 	removableStones;

	// get all removable stones
	getPossStoneRemove(removableStones);

	// clear possibilities
	possibilityIds.clear();

	// possibilities with cut off
	for (to=0; to<size; to++) {

		// move possible ?
		if (field[to] != playerId::squareIsFree) continue;

		// check if a mill is beeing closed
		numberOfMillsBeeingClosed = wouldMillBeClosed(fieldStruct::size, to);

		// if a mill is closed, generate moves with stone removal
		// don't allow to close two mills at once
		// don't allow to close a mill, although no stone can be removed from the opponent
		if (numberOfMillsBeeingClosed == 1 && removableStones.size()) {
			for (fieldPos removePos : removableStones) {
				possibilityIds.push_back(moveInfo{size, to, removePos}.getId());
			}
		// no mill closed, generate move without stone removal
		} else if (numberOfMillsBeeingClosed == 0) {
			possibilityIds.push_back(moveInfo{size, to, size}.getId());
		}
	}
}

//-----------------------------------------------------------------------------
// Name: getPossNormalMove()
// Desc: Helper function to get the possible moves in the normal phase
//-----------------------------------------------------------------------------
void fieldStruct_forward::getPossNormalMove(vector<unsigned int>& possibilityIds) const
{
	// locals
	fieldPos			from, to, dir, removePos;
	vector<fieldPos> 	removableStones;
	
	possibilityIds.clear();

	// get all removable stones
	getPossStoneRemove(removableStones);

	// if he is not allowed to jump
	if (curPlayer.numStones > 3) {

		for (from=0; from < size; from++) { for (dir=0; dir<4; dir++) {

			// destination 
			to = connectedSquare[from][dir];

			// move possible ?
			if (to < size && field[from] == curPlayer.id && field[to] == playerId::squareIsFree) {

				// if a mill is closed, generate moves with stone removal
				if (wouldMillBeClosed(from, to) && !removableStones.empty()) {
					for (fieldPos removePos : removableStones) {
						possibilityIds.push_back(moveInfo{from, to, removePos}.getId());
					}
				// no mill closed, generate move without stone removal
				} else {
					possibilityIds.push_back(moveInfo{from, to, size}.getId());
				}
	
	// current player is allowed to jump
	}}}} else if (curPlayer.numStones == 3) {

		for (from=0; from < size; from++) { for (to=0; to < size; to++) {

			// move possible ?
			if (field[from] == curPlayer.id &&  field[to] == playerId::squareIsFree) {

				// if a mill is closed, generate moves with stone removal
				if (wouldMillBeClosed(from, to) && !removableStones.empty()) {
					for (fieldPos removePos : removableStones) {
						possibilityIds.push_back(moveInfo{from, to, removePos}.getId());
					}
				// no mill closed, generate move without stone removal
				} else {
					possibilityIds.push_back(moveInfo{from, to, size}.getId());
				}
	}}}} else {
		// no possible moves
	}

	assert(possibilityIds.size() < fieldStruct::maxNumPosMoves);
}

//-----------------------------------------------------------------------------
// Name: getPossStoneRemove()
// Desc: Helper function to get the possible moves to remove a stone
//-----------------------------------------------------------------------------
void fieldStruct_forward::getPossStoneRemove(vector<fieldPos>& removableStones) const
{
	// locals
	fieldPos from;
	
	removableStones.clear();

	// possibilities with cut off
	for (from=0; from<size; from++) {

		// move possible ?
		if (canStoneBeRemoved(from)) {
			removableStones.push_back(from);
		}
	}
}

//-----------------------------------------------------------------------------
// Name: wouldMillBeClosed()
// Desc: Checks if a mill would be closed by moving from 'from' to 'to'
//       Does not return true if the stone is already part of a mill at 'from'.
//-----------------------------------------------------------------------------
unsigned int fieldStruct_forward::wouldMillBeClosed(fieldPos from, fieldPos to) const
{
	unsigned int numberOfMillsBeingClosed = 0;

	// check if a mill is being closed
	if (curPlayer.id == field[neighbour[to][0][0]] && curPlayer.id == field[neighbour[to][0][1]] && neighbour[to][0][0] != from && neighbour[to][0][1] != from) numberOfMillsBeingClosed++;
	if (curPlayer.id == field[neighbour[to][1][0]] && curPlayer.id == field[neighbour[to][1][1]] && neighbour[to][1][0] != from && neighbour[to][1][1] != from) numberOfMillsBeingClosed++;

	// return true if a mill would be closed
	return numberOfMillsBeingClosed;
}

//-----------------------------------------------------------------------------
// Name: canStoneBeRemoved()
// Desc: Checks if a stone can be removed
//-----------------------------------------------------------------------------
bool fieldStruct_forward::canStoneBeRemoved(fieldPos pos) const
{
	// Check if the position is valid
	if (pos >= size) return false;

	// Check if the stone belongs to the opponent
	if (field[pos] != oppPlayer.id) return false;

	// if stone is not part of a mill then it can be removed
	if (!stonePartOfMill[pos]) return true;

	// do not allow to remove a stone belonging to two mills
	if (stonePartOfMill[pos] > 1) return false;

	// if stone is part of a mill then it can only be removed if the opponent has no "free" stones
	if (stonePartOfMill[pos] > 0 && oppPlayer.hasOnlyMills) return true;

	// if this is the last to be set during the setting phase, then it can be removed
	if (oppPlayer.hasOnlyMills && getNumStonesSet() == 17) return true;

	// otherwise the stone cannot be removed
	return false;
}

//-----------------------------------------------------------------------------
// Name: setWarning()
// Desc: 
//-----------------------------------------------------------------------------
void fieldStruct_forward::updateStonePartOfMill(fieldPos stoneOne, fieldPos stoneTwo, fieldPos stoneThree, playerId actingPlayer)
{
	// if all 3 fields are occupied by current player than he closed a mill
	if (field[stoneOne] == actingPlayer && field[stoneTwo] == actingPlayer && field[stoneThree] == actingPlayer) {
		stonePartOfMill[stoneOne  ]++;
		stonePartOfMill[stoneTwo  ]++;
		stonePartOfMill[stoneThree]++;
	}

	// is a mill destroyed ?
	if (stonePartOfMill[stoneOne] && stonePartOfMill[stoneTwo] && stonePartOfMill[stoneThree]
	 && field[stoneOne] == playerId::squareIsFree && field[stoneTwo] == actingPlayer && field[stoneThree] == actingPlayer) {
		stonePartOfMill[stoneOne  ]--;
		stonePartOfMill[stoneTwo  ]--;
		stonePartOfMill[stoneThree]--;
	}
}

//-----------------------------------------------------------------------------
// Name: updateWarning()
// Desc: Updates 'stonePartOfMill'
//-----------------------------------------------------------------------------
void fieldStruct_forward::updateWarning(fieldPos firstStone, fieldPos secondStone, playerId actingPlayer)
{
	if (firstStone  < size) updateStonePartOfMill(firstStone,  neighbour[firstStone][0][0],  neighbour[firstStone][0][1], actingPlayer);
	if (firstStone  < size) updateStonePartOfMill(firstStone,  neighbour[firstStone][1][0],  neighbour[firstStone][1][1], actingPlayer);
	if (secondStone < size) updateStonePartOfMill(secondStone, neighbour[secondStone][0][0], neighbour[secondStone][0][1], actingPlayer);
	if (secondStone < size) updateStonePartOfMill(secondStone, neighbour[secondStone][1][0], neighbour[secondStone][1][1], actingPlayer);
}

//-----------------------------------------------------------------------------
// Name: updatePossibleMoves()
// Desc: Updates the number of possible moves for each player
//-----------------------------------------------------------------------------
void fieldStruct_forward::updatePossibleMoves(fieldPos stone, playerStruct& stoneOwner, bool stoneRemoved, fieldPos ignoreStone)
{
	// locals
	fieldPos	neighbor, direction;

	// look into every direction
	for (direction=0; direction<4; direction++) {

		neighbor = connectedSquare[stone][direction];

		// neighbor must exist
		if (neighbor < size) {

			// relevant when moving from one square to another connected square
			if (ignoreStone == neighbor) continue;

			// if there is no neighbour stone than it only affects the actual stone
			if (field[neighbor] == playerId::squareIsFree) {
			
				if (stoneRemoved)	stoneOwner.numPossibleMoves--;
				else				stoneOwner.numPossibleMoves++;
			
			// if there is a neighbour stone than it effects only this one
			} else if (field[neighbor] == curPlayer.id) {
				
				if (stoneRemoved)	curPlayer.numPossibleMoves++;
				else				curPlayer.numPossibleMoves--;

			} else {
				
				if (stoneRemoved)	oppPlayer.numPossibleMoves++;
				else				oppPlayer.numPossibleMoves--;
			}
	}}

	// only 3 stones resting
	if (curPlayer.numStones == 3 && !settingPhase) curPlayer.numPossibleMoves = curPlayer.numStones * (size - curPlayer.numStones - oppPlayer.numStones);
	if (oppPlayer.numStones == 3 && !settingPhase) oppPlayer.numPossibleMoves = oppPlayer.numStones * (size - curPlayer.numStones - oppPlayer.numStones);
	if (curPlayer.numStones  < 3) curPlayer.numPossibleMoves = 0;
	if (oppPlayer.numStones  < 3) oppPlayer.numPossibleMoves = 0;
}

//-----------------------------------------------------------------------------
// Name: setStone()
// Desc: Performs a move in the setting phase 
//-----------------------------------------------------------------------------
bool fieldStruct_forward::setStone(const moveInfo& move, backupStruct& backup)
{
	// parameter ok ?
	if (move.to >= size) return false;

	// is destination free ?
	if (field[move.to] != playerId::squareIsFree) return false;

	// check if removal of stone is correct
	if (move.removeStone < size) {
		if (!canStoneBeRemoved(move.removeStone)) return false;
	}

	// set stone into field
	field[move.to]		    = curPlayer.id;
	curPlayer.numStones++;
	curPlayer.numStonesSet++;

	// setting phase finished ?
	if (curPlayer.numStonesSet + oppPlayer.numStonesSet == 18) {
		settingPhase = false;
	}

	// update possible moves
	if (settingPhase) {
		curPlayer.numPossibleMoves--;
		oppPlayer.numPossibleMoves--;
	} else {
		calcNumPossibleMoves(curPlayer);
		calcNumPossibleMoves(oppPlayer);
	}
	
	// update warnings
	updateWarning(move.to, size, curPlayer.id);

	// handle stone removal if a mill was closed
	if (move.removeStone < size) {
		removeStone(move, backup);
	}

	// calculate number of mills
	calcNumberOfMills();

	// everything is ok
	return true;	
}

//-----------------------------------------------------------------------------
// Name: normalMove()
// Desc: Performs a normal move 
//-----------------------------------------------------------------------------
bool fieldStruct_forward::normalMove(const moveInfo& move, backupStruct& backup)
{
	// check if move is possible
	if (move.from 			>= size) 					return false;
	if (move.to   			>= size) 					return false;
	if (field[move.from] 	!= curPlayer.id) 			return false;
	if (field[move.to]   	!= playerId::squareIsFree) 	return false;

	// check if removal of stone is correct
	if (move.removeStone < size) {
		if (!canStoneBeRemoved(move.removeStone)) return false;
	}

	// set stone into field
	field[move.from]		= playerId::squareIsFree;
	field[move.to]		    = curPlayer.id;

	// update possible moves
	updatePossibleMoves(move.from, curPlayer, true,  move.to);
	updatePossibleMoves(move.to,   curPlayer, false, move.from);

	// update warnings
	updateWarning(move.from, move.to, curPlayer.id);
	calcNumberOfMills();

	// handle stone removal if a mill was closed
	if (move.removeStone < size) {
		removeStone(move, backup);
	}

	// everything is ok
	return true;
}

//-----------------------------------------------------------------------------
// Name: removeStone()
// Desc: Removes a stone from the field
//-----------------------------------------------------------------------------
bool fieldStruct_forward::removeStone(const moveInfo& move, backupStruct& backup) 
{
	// check if removal of stone is correct
	if (!canStoneBeRemoved(move.removeStone)) return false;

	// remove stone
	field[move.removeStone]		    = playerId::squareIsFree;
	oppPlayer.numStones--;
	oppPlayer.numStonesMissing++;
	
	// update possible moves
	if (settingPhase) {
		curPlayer.numPossibleMoves++;
		oppPlayer.numPossibleMoves++;
	} else {
		updatePossibleMoves(move.removeStone, oppPlayer, true, size);
	}

	// update warnings
	updateWarning(move.removeStone, size, oppPlayer.id);

	// end of game ?
	if ((oppPlayer.numStones < 3) && (!settingPhase)) 									gameHasFinished	= true;		// opponent has less than 3 stones
	if ((!oppPlayer.numPossibleMoves) && (!settingPhase) && (oppPlayer.numStones > 3)) 	gameHasFinished = true;		// opponent has no possible moves and more than 3 stones

	// everything is ok
	return true;
}

//-----------------------------------------------------------------------------
// Name: move()
// Desc: Performs a move
//-----------------------------------------------------------------------------
bool fieldStruct_forward::move(const moveInfo& move, backupStruct& oldState)
{
    // calculate place of stone
	oldState.gameHasFinished	= gameHasFinished;										
	oldState.curPlayer			= curPlayer;								
	oldState.oppPlayer			= oppPlayer;
	oldState.settingPhase		= settingPhase;									
	oldState.stonePartOfMill	= stonePartOfMill;
	oldState.field 				= field;

	// check if move is possible
	if (gameHasFinished)			return false;
	if (move.from > size)			return false;
	if (move.to   > size)			return false;
	if (move.removeStone > size)	return false;

	// move
	bool moveResult = false;
	if (settingPhase)	{ moveResult = setStone(move, oldState);    }
	else				{ moveResult = normalMove(move, oldState);	}	
	if (!moveResult) return false;

	// when opponent is unable to move than current player has won
	if ((!oppPlayer.numPossibleMoves) && (!settingPhase) && (oppPlayer.numStones > 3)) gameHasFinished = true;

	// set next player
	std::swap(curPlayer, oppPlayer);

	// update hasOnlyMills
	calcHasOnlyMills();

	return true;
}

//-----------------------------------------------------------------------------
// Name: undo()
// Desc: Reverts to an old state
//-----------------------------------------------------------------------------
bool fieldStruct_forward::undo(const backupStruct& oldState)
{
	gameHasFinished				= oldState.gameHasFinished;
	curPlayer					= oldState.curPlayer;							
	oppPlayer					= oldState.oppPlayer;							
	settingPhase				= oldState.settingPhase;						
	field 						= oldState.field;
    stonePartOfMill	            = oldState.stonePartOfMill;
	return true;
}

#pragma endregion

#pragma region fieldStruct_reverse

//-----------------------------------------------------------------------------
// Name: getPredecessors()
// Desc: Returns the predecessors fields of the current field
//-----------------------------------------------------------------------------
void fieldStruct_reverse::getPredecessors(vector<fieldStruct::core>& predFields) const
{
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // the important variables, which much be updated for the getLayerAndStateNumber function are the following ones:
    // - field.curPlayer.numStones and for the opponent player
    // - field.curPlayer.id        and for the opponent player
    // - field.field
    // - field.settingPhase
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	predFields.clear();

	// locals
	fieldStruct_reverse tmpField 		= *this;
    bool 				millWasClosed 	= false;

	// stone was removed
	getPredecessors_stoneRemove (predFields, tmpField);

    // in moving phase
	getPredecessors_normalMove  (predFields, tmpField, millWasClosed);

	// in jumping phase
	getPredecessors_jumpingPhase(predFields, tmpField, millWasClosed);

	// in setting phase
	getPredecessors_settingPhase(predFields, tmpField, millWasClosed);
}

//-----------------------------------------------------------------------------
// Name: getPredecessors_settingPhase()
// Desc: Helper function to get the predecessors in the stone setting phase
//-----------------------------------------------------------------------------
void fieldStruct_reverse::getPredecessors_settingPhase(vector<fieldStruct::core>& predFields, fieldStruct_reverse& field, bool millWasClosed) const
{
	// locals
	unsigned int			to;
	bool 					settingPhaseBackup;
	bool 		 			gameHasFinishedBackup;

	// number of stones set must be at least 1
	if (field.oppPlayer.numStonesSet < 1 || field.oppPlayer.numStones < 1) return;

	// if already in moving phase, but coming from setting phase
	if (!field.settingPhase) {

		// then both players must have at least 3 stones
	    if (field.curPlayer.numStones < 3 || field.oppPlayer.numStones < 3) return;

		// if a mill was closed then at least 3 stones must be present and a mill
		if (millWasClosed && field.curPlayer.numStones < 3 && field.curPlayer.numberOfMills > 0) return;

		// total number of stones must be correct
		if (field.getNumStonesSet() != field.curPlayer.numStones + field.oppPlayer.numStones + field.curPlayer.numStonesMissing + field.oppPlayer.numStonesMissing) return;

		// all stones must be set, to be in the moving phase
		if (field.curPlayer.numStonesSet != fieldStruct::numStonesPerPlayer || field.oppPlayer.numStonesSet != fieldStruct::numStonesPerPlayer) return;
	}

	// stone could have been placed anywhere
	for (to=0; to < field.size; to++) { 

		// do not allow to close two mills at once
		if (field.stonePartOfMill[to] >= 2) continue;

		// stone which was set must be owned by the current player, if a mill was closed
		// otherwise, it must be owned by the opponent player
		if (field.field[to] != (millWasClosed ? field.curPlayer.id : field.oppPlayer.id)) continue;

		// if a mill was closed so the stone must be part of a mill
		if ( millWasClosed && field.stonePartOfMill[to] == 0) continue;

		// if no mill was closed so the stone must not be part of a mill
		if (!millWasClosed && field.stonePartOfMill[to] != 0) continue;

		// remove stone set during this setting phase step
		{
			settingPhaseBackup 		= field.settingPhase;
			gameHasFinishedBackup 	= field.gameHasFinished;
			field.settingPhase 		= true;
			field.gameHasFinished 	= false;

			field.field[to] = playerId::squareIsFree;
			if (millWasClosed) {
				field.curPlayer.numStones--;
				field.curPlayer.numStonesSet--;
				field.curPlayer.numberOfMills--;
			} else {
				field.oppPlayer.numStones--;
				field.oppPlayer.numStonesSet--;
				std::swap(field.curPlayer, field.oppPlayer);
			}
		}

		storePredecessor(predFields, field);

		// put stone back
		{
			if (millWasClosed) {
				field.field[to] 			= field.curPlayer.id;
				field.curPlayer.numberOfMills++;
				field.curPlayer.numStonesSet++;
				field.curPlayer.numStones++;
			} else {
				std::swap(field.curPlayer, field.oppPlayer);
				field.oppPlayer.numStonesSet++;
				field.oppPlayer.numStones++;
				field.field[to] 			= field.oppPlayer.id;
			}

			field.settingPhase 		= settingPhaseBackup;
			field.gameHasFinished 	= gameHasFinishedBackup;
		}
	}
}

//-----------------------------------------------------------------------------
// Name: getPredecessors_normalMove()
// Desc: Helper function to get the predecessors in the normal phase
//-----------------------------------------------------------------------------
void fieldStruct_reverse::getPredecessors_normalMove(vector<fieldStruct::core>& predFields, fieldStruct_reverse& field, bool millWasClosed) const
{
	// locals
	unsigned int from, to, dir;
	bool 		 gameHasFinishedBackup;

	// game must not be in the setting phase any more
	if (field.settingPhase) return;

	// both players must have at least 3 stones and game must not be finished yet
    if (field.curPlayer.numStones < 3 || field.oppPlayer.numStones < 3 || 
		(field.gameHasFinished && field.curPlayer.numPossibleMoves != 0)) return;

	// test each destination
	for (to=0; to < field.size; to++) { 

		// stone which was moved must be owned by the current player, if a mill was closed
		// otherwise, it must be owned by the opponent player
		if (field.field[to] != (millWasClosed ? field.curPlayer.id : field.oppPlayer.id)) continue;

		// when stone is going to be removed than a mill must be closed
		if ( millWasClosed && field.stonePartOfMill[to] == 0) continue;

		// when stone is part of a mill then a stone must be removed
		if (!millWasClosed && field.stonePartOfMill[to] != 0) continue;

		// test each direction
		for (dir=0; dir<4; dir++) {

			// origin
			from = field.connectedSquare[to][dir];

			// move possible ?
			if (!(from < field.size && field.field[from] == playerId::squareIsFree)) continue;

			// make move
			{
				if (millWasClosed) {
					field.curPlayer.numberOfMills--;
				} else {
					std::swap(field.curPlayer, field.oppPlayer);
				}

				field.field[from]      		= field.field[to];
				field.field[to]        		= playerId::squareIsFree;
				gameHasFinishedBackup 		= field.gameHasFinished;
				field.gameHasFinished 		= false;
			}

			storePredecessor(predFields, field);

			// undo move
			{
				field.field[to]        		= field.field[from];
				field.field[from]      		= playerId::squareIsFree;
				field.gameHasFinished 		= gameHasFinishedBackup;

				if (millWasClosed) {
					field.curPlayer.numberOfMills++;
				} else {
					std::swap(field.curPlayer, field.oppPlayer);
				}
			}
	}}
}

//-----------------------------------------------------------------------------
// Name: getPredecessors_jumpingPhase()
// Desc: Helper function to get the predecessors in the jumping phase
//-----------------------------------------------------------------------------
void fieldStruct_reverse::getPredecessors_jumpingPhase(vector<fieldStruct::core>& predFields, fieldStruct_reverse& field, bool millWasClosed) const
{
	// locals
	unsigned int from, to;

	// game must not be in the setting phase any more
	if (field.settingPhase) return;

	// both players must have at least 3 stones and game must not be finished yet
    if (field.curPlayer.numStones < 3 || field.oppPlayer.numStones < 3 || field.gameHasFinished) return;

	// test each destination
	for (to=0; to < field.size; to++) { 

		// when stone must be removed than current player closed a mill, otherwise the opponent did a common spring move
		if (field.field[to] != (millWasClosed ? field.curPlayer.id : field.oppPlayer.id)) continue;

		// when stone is going to be removed than a mill must be closed
		if ( millWasClosed && field.stonePartOfMill[to] == 0) continue;

		// when stone is part of a mill then a stone must be removed
		if (!millWasClosed && field.stonePartOfMill[to] != 0) continue;

		// test each stone origin
		for (from=0; from<field.size; from++) {

			// move possible ?
			{
				if (field.field[from] != playerId::squareIsFree) continue;

				// is currently player allowed to jump?
				if (field.curPlayer.numStones > 3 &&  millWasClosed
				||  field.oppPlayer.numStones > 3 && !millWasClosed) {

					// determine moving direction
					unsigned int movingDirection = 4;
					for (unsigned int k=0; k<4; k++) if (field.connectedSquare[from][k] == to) movingDirection = k;

					// are both squares connected ?
					if (movingDirection == 4) continue;
				}
			}
			
			// make move
			{
				if (millWasClosed) {
					field.curPlayer.numberOfMills--;
				} else {
					std::swap(field.curPlayer, field.oppPlayer);
				}

				field.field[from]  = field.field[to];
				field.field[to]    = playerId::squareIsFree;
			}

			storePredecessor(predFields, field);

			// undo move
			{
				field.field[to]    = field.field[from];
				field.field[from]  = playerId::squareIsFree;

				if (millWasClosed) {
					field.curPlayer.numberOfMills++;
				} else {
					std::swap(field.curPlayer, field.oppPlayer);
				}
			}
	}}
}

//-----------------------------------------------------------------------------
// Name: getPredecessors_stoneRemove()
// Desc: Helper function to get the predecessors in the remove phase
//-----------------------------------------------------------------------------
void fieldStruct_reverse::getPredecessors_stoneRemove(vector<fieldStruct::core>& predFields, fieldStruct_reverse& field) const
{
	// locals
	unsigned int 	from;
	bool 			gameHasFinishedBackup;
	unsigned int	stoneFromMillWasRemoved;

    // a stone was only removed, when the current player has less than 9 stones and at least one stone missing
    if (field.curPlayer.numStones >= 9 || field.curPlayer.numStonesMissing == 0 || field.curPlayer.numStonesSet == 0) return;

	// at least 5 stones must be set (3 from the opponent for the mill and 2 from the current player (thereby 1 has been removed))
	if (field.getNumStonesSet() < 5 && field.oppPlayer.numStones < 3 && field.curPlayer.numStones < 1) return;

	// opponent must have a closed mill
	if (!field.oppPlayer.numberOfMills) return;

	// from each free position the opponent could have removed a stone from the current player
	for (from=0; from<field.size; from++) {

		// square free?
		if (field.field[from] != playerId::squareIsFree) continue;

		// stone mustn't be part of mill, except player has only mills
		stoneFromMillWasRemoved = 0;
		{
			vector<fieldPos> millStones = {
				  from, 
				  field.neighbour[from][0][0],
				  field.neighbour[from][0][1],
				  field.neighbour[from][1][0],
				  field.neighbour[from][1][1]
				};
			if (field.field[millStones[1]] == field.curPlayer.id && field.field[millStones[2]] == field.curPlayer.id) stoneFromMillWasRemoved++;
			if (field.field[millStones[3]] == field.curPlayer.id && field.field[millStones[4]] == field.curPlayer.id) stoneFromMillWasRemoved++;
			if (stoneFromMillWasRemoved && anyLonelyStone(field, from)) continue;
		}

		// if stone was removed from mill, then player must have at least one stone on the board
		if (stoneFromMillWasRemoved) {
			if (field.curPlayer.numStones == 0 || field.curPlayer.numStonesSet == 0) continue;
		}
		// do not allow to remove a stone being part of two mills
		if (stoneFromMillWasRemoved>1) continue;

		// put back stone
		{
			gameHasFinishedBackup		= field.gameHasFinished;
			field.gameHasFinished		= false;
			field.field[from]          	= field.curPlayer.id;
			field.curPlayer.numStones++;
			field.curPlayer.numStonesMissing--;
			if (stoneFromMillWasRemoved) {
				field.curPlayer.numberOfMills += stoneFromMillWasRemoved;
				field.curPlayer.hasOnlyMills   = true;
				field.calcStonePartOfMill();
			}
			std::swap(field.curPlayer, field.oppPlayer);
		}

		// get predecessor from closing the mill
		getPredecessors_normalMove  (predFields, field, true);
		getPredecessors_jumpingPhase(predFields, field, true);
		getPredecessors_settingPhase(predFields, field, true);

		// remove stone again
		{
			std::swap(field.curPlayer, field.oppPlayer);
			field.field[from]          	= playerId::squareIsFree;
			field.gameHasFinished		= gameHasFinishedBackup;
			field.curPlayer.numStones--;
			field.curPlayer.numStonesMissing++;
			if (stoneFromMillWasRemoved) {
				field.curPlayer.numberOfMills -= stoneFromMillWasRemoved;
				field.curPlayer.hasOnlyMills   = false;
				field.calcStonePartOfMill();
			}
		}
	}
}

//---------------------------------------------------
// Name: storePredecessor()
// Desc: store the current field state as a predecessor
//---------------------------------------------------
bool fieldStruct_reverse::storePredecessor(vector<fieldStruct_types::core>& predFields, const fieldStruct_reverse& field) const
{
	// store predecessor
	if (field.isIntegrityOk()) {
		predFields.push_back(field);
		return true;
	} else {
		// TODO: The conditions should be checked within the getPredecessor functions,
		// such that isIntegrity should not be necessary
		// assert(false);
		return false;
	}
}

//---------------------------------------------------
// Name: anyLonelyStone()
// Desc: check if there is any lonely stone, not being part of a mill.
//       thereby a stone is considered lonely if it is not part of any mill.
//		 'removedFrom' is the position of the stone that was removed from the field.
//---------------------------------------------------
bool fieldStruct_reverse::anyLonelyStone(const fieldStruct_reverse& field, fieldPos removedFrom) const
{
	// check every stone
	for (fieldPos k=0; k<field.size; k++) {

		// skip the current removed stone and the potential mills being closed by that stone
		if (k == removedFrom) continue;
		if (k == field.neighbour[removedFrom][0][1] && field.field[field.neighbour[removedFrom][0][0]] == field.curPlayer.id) continue;
		if (k == field.neighbour[removedFrom][0][0] && field.field[field.neighbour[removedFrom][0][1]] == field.curPlayer.id) continue;
		if (k == field.neighbour[removedFrom][1][1] && field.field[field.neighbour[removedFrom][1][0]] == field.curPlayer.id) continue;
		if (k == field.neighbour[removedFrom][1][0] && field.field[field.neighbour[removedFrom][1][1]] == field.curPlayer.id) continue;

		// check if the stone is lonely
		if (field.field[k] == field.curPlayer.id && !field.stonePartOfMill[k]) return true;
	}
	return false;
}
#pragma endregion
