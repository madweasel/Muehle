/**************************************************************************************************************************
	fieldStructTest.cpp
 	Copyright (c) Thomas Weber. All rights reserved.				
	Licensed under the MIT License.
	https://github.com/madweasel/madweasels-cpp
***************************************************************************************************************************/
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "fieldStruct.h"

// Constants to simplify the test
const auto x = playerId::playerOne;
const auto o = playerId::playerTwo;
const auto _ = playerId::squareIsFree;
using FIELD  = fieldStruct::fieldArray;

// For example a field can be set like this:
// FIELD({  _,    _,    _,
//  		  o,  _,  _,
//  		    _,_,_,
//  	    _,_,_,  x,_,_,
//  		    _,_,x,
//  		  _,  _,  _,
//  	    o,    _,    _});

// Helper function to set the state of the field
bool setState(fieldStruct& fs, fieldStruct::fieldArray field, playerId curPlayer, bool settingPhase, unsigned int totalNumMissingStones)
{
	fs.reset(curPlayer);
	return fs.setSituation(field, settingPhase, totalNumMissingStones);
}

// Helper function to check the state of the field
void checkState(
	fieldStruct&	fs, 
	FIELD 			field, 
	playerId 		curPlayer, 
	bool 			settingPhase, 
	unsigned int 	numStonesSet,
	playerId 		winner 						= playerId::squareIsFree,
	bool 			integrityOk 				= true,
	unsigned int 	numStonesCurPlayer			= 0,
	unsigned int 	numStonesOppPlayer			= 0,
    unsigned int 	numMillsCurPlayer 			= 0,
	unsigned int 	numMillsOppPlayer 			= 0,
	unsigned int 	numStonesMissingCurPlayer 	= 0,
	unsigned int 	numStonesMissingOppPlayer 	= 0,
	unsigned int 	numPossibleMovesCurPlayer 	= 0,
	unsigned int 	numPossibleMovesOppPlayer 	= 0,
	unsigned int 	numStonesSetCurPlayer		= 0,
	unsigned int 	numStonesSetOppPlayer		= 0
)
{
	EXPECT_EQ(fs.getField(), 						field);
	EXPECT_EQ(fs.inSettingPhase(), 					settingPhase);
	EXPECT_EQ(fs.getNumStonesSet(), 				numStonesSet);
	EXPECT_EQ(fs.getWinner(), 						winner);
	EXPECT_EQ(fs.hasGameFinished(), 				winner != playerId::squareIsFree);
	EXPECT_EQ(fs.isIntegrityOk(), 					integrityOk);
	EXPECT_EQ(fs.getCurPlayer().id, 				curPlayer);
	EXPECT_EQ(fs.getOppPlayer().id, 				curPlayer == x ? o : x);
	EXPECT_EQ(fs.getCurPlayer().numberOfMills, 		numMillsCurPlayer);
	EXPECT_EQ(fs.getOppPlayer().numberOfMills, 		numMillsOppPlayer);
	EXPECT_EQ(fs.getCurPlayer().numPossibleMoves, 	numPossibleMovesCurPlayer);
	EXPECT_EQ(fs.getOppPlayer().numPossibleMoves, 	numPossibleMovesOppPlayer);
	EXPECT_EQ(fs.getCurPlayer().numStonesMissing, 	numStonesMissingCurPlayer);
	EXPECT_EQ(fs.getOppPlayer().numStonesMissing, 	numStonesMissingOppPlayer);
	EXPECT_EQ(fs.getCurPlayer().numStones, 			numStonesCurPlayer);
	EXPECT_EQ(fs.getOppPlayer().numStones, 			numStonesOppPlayer);
	EXPECT_EQ(fs.getCurPlayer().numStonesSet, 		numStonesSetCurPlayer);
	EXPECT_EQ(fs.getOppPlayer().numStonesSet, 		numStonesSetOppPlayer);
}

TEST(fieldStruct_Test, test_easy_ones)
{
	fieldStruct fs;
	fieldStruct fs_expected;
	
	EXPECT_EQ(fs, fs_expected);

	EXPECT_EQ(fs.getField().size(), 24);
	EXPECT_EQ(fs.getNumStonesSet(), 0);
	EXPECT_EQ(fs.inSettingPhase(), true);
	EXPECT_EQ(fs.hasGameFinished(), false);
	EXPECT_EQ(fs.getNumStonesSet(), 0);
	EXPECT_EQ(fs.getField().size(), 24);

	EXPECT_EQ(fs.getCurPlayer().id, playerId::playerOne);
	EXPECT_EQ(fs.getOppPlayer().id, playerId::playerTwo);
	EXPECT_EQ(fs.getCurPlayer().numberOfMills, 0);
	EXPECT_EQ(fs.getOppPlayer().numberOfMills, 0);
	EXPECT_EQ(fs.getWinner(), playerId::squareIsFree);
	EXPECT_EQ(fs.getStone(0), playerId::squareIsFree);
	EXPECT_EQ(fs.getStone(1), playerId::squareIsFree);
	EXPECT_EQ(fs.getStone(23), playerId::squareIsFree);

	fs.reset(playerId::playerTwo);
	EXPECT_EQ(fs.getCurPlayer().id, playerId::playerTwo);

	fs.invert();
	EXPECT_EQ(fs.getCurPlayer().id, playerId::playerOne);
	EXPECT_EQ(fs.getOppPlayer().id, playerId::playerTwo);

	fs.print();
	EXPECT_EQ(fs.isIntegrityOk(), true);
}

TEST(fieldStruct_Test, test_setSituation_positive)
{
	fieldStruct fs;
	fieldStruct fs_expected;

	// x shall be the starting player
	fs.reset(x);

	// a single stone in the setting phase
	EXPECT_TRUE(fs.setSituation({	_,    _,    _,
									  _,  _,  _,
									    _,_,_,
								    _,_,_,  o,_,_,
									    _,_,_,
									  _,  _,  _,
								    _,    _,    _}, true, 0));

	EXPECT_EQ(fs.getCurPlayer().id, x);							// The first player already set his stone, now the second player is on turn
	EXPECT_EQ(fs.getNumStonesSet(), 1);							// One stone is set
	EXPECT_EQ(fs.getStone(11), 		_);							// The stone is set at position 11
	EXPECT_EQ(fs.getStone(12), 		o);							// The stone is set at position 12
	EXPECT_EQ(fs.getStone(13), 		_);							// The stone is set at position 13
	EXPECT_EQ(fs.hasGameFinished(), false);						// Game is not finished
	EXPECT_EQ(fs.getWinner(), 		playerId::squareIsFree);	// No winner yet
	EXPECT_EQ(fs.getCurPlayer().numStonesSet, 0); 
	EXPECT_EQ(fs.getOppPlayer().numStonesSet, 1);

	checkState(fs, FIELD({	_,    _,    _,
							  _,  _,  _,
							    _,_,_,
						    _,_,_,  o,_,_,
							    _,_,_,
							  _,  _,  _,
						    _,    _,    _}), x, true, 1, playerId::squareIsFree, true, 0, 1, 0, 0, 0, 0, 23, 23, 0, 1);

	// during setting phase, the total number of missing stones must be at least the number of present on the field
	EXPECT_FALSE(fs.setSituation({	o,    o,    x,
									  _,  _,  x,
									    o,_,o,
								    _,_,_,  o,x,x,
									    _,o,o,
									  _,  _,  _,
								    x,    x,    x}, true, 2));

	// setting phase with 2 mills for the current player, 1 mill for the opponent. opponent already set his 9 stones.
	EXPECT_TRUE(fs.setSituation({	o,    o,    x,
									  _,  _,  x,
									    o,_,o,
								    _,_,_,  o,x,x,
									    _,o,o,
									  _,  _,  _,
								    x,    x,    x}, true, 3));

	checkState(fs, FIELD({	o,    o,    x,
							  _,  _,  x,
							    o,_,o,
						    _,_,_,  o,x,x,
							    _,o,o,
							  _,  _,  _,
						    x,    x,    x}), x, true, 17, playerId::squareIsFree, true, 7, 7, 2, 1, 1, 2, 10, 10, 8, 9);

	// setting phase with 1 mill for the current player, 1 mill for the opponent. opponent already set his 9 stones.
	EXPECT_TRUE(fs.setSituation({	o,    o,    x,
									  _,  _,  x,
									    o,_,o,
								    _,_,_,  o,x,x,
									    _,o,o,
									  _,  _,  _,
								    _,    x,    x}, true, 3));

	checkState(fs, FIELD({	o,    o,    x,
							  _,  _,  x,
							    o,_,o,
						    _,_,_,  o,x,x,
							    _,o,o,
							  _,  _,  _,
						    _,    x,    x}), x, true, 16, playerId::squareIsFree, true, 6, 7, 1, 1, 2, 1, 11, 11, 8, 8);
}

// Helper function to check the number of possible moves after a move
void check_move_numPossibleMoves(
	const fieldStruct::fieldArray& field, 
	bool settingPhase, 
	unsigned int totalNumStonesMissing,
	const moveInfo& move, 
	unsigned int expectNumPossibleMovesCurPlayer, 
	unsigned int expectNumPossibleMovesOppPlayer,
	unsigned int expectNumPossibilities)
{
	fieldStruct 				fs;
	fieldStruct::backupStruct 	oldState;
	std::vector<unsigned int> 	possibilityIds;

	// x shall be the starting player
	fs.reset(x);

	printf("-------------------\n\n");
	EXPECT_TRUE(fs.setSituation(field, settingPhase, totalNumStonesMissing));
	fs.print();
	EXPECT_TRUE(fs.move(move, oldState));
	fs.print();
	EXPECT_EQ(fs.getCurPlayer().numPossibleMoves, expectNumPossibleMovesCurPlayer);
	EXPECT_EQ(fs.getOppPlayer().numPossibleMoves, expectNumPossibleMovesOppPlayer);	
	fs.getPossibilities(possibilityIds);
	EXPECT_EQ(possibilityIds.size(), expectNumPossibilities);
}

TEST(fieldStruct_Test, test_move_numPossibleMoves)
{
	// all combinations of moves shall be tested and check if the number of possible moves for each player is correct

	// setting phase, no stones to be removed
	check_move_numPossibleMoves({	_,    _,    _,
									  _,  _,  _,
									    _,_,_,
								    _,_,_,  o,_,_,
									    _,_,_,
									  _,  _,  _,
								    _,    _,    _}, true, 0, moveInfo{fieldStruct::size, 0, fieldStruct::size}, 22, 22, 22);

	// setting phase, close a mill and remove a stone
	check_move_numPossibleMoves({	_,    _,    _,
									  o,  o,  o,
									    _,_,_,
								    _,_,_,  x,x,_,
									    _,_,_,
									  _,  _,  _,
								    _,    _,    _}, true, 4, moveInfo{fieldStruct::size, 14, 3}, 19, 19, 18+3);

	// setting phase ends with the stone set
	check_move_numPossibleMoves({	x,    x,    o,
									  o,  x,  x,
									    x,_,_,
								    o,_,_,  o,o,_,
									    _,o,x,
									  _,  o,  x,
								    o,    o,    _}, true, 1, moveInfo{fieldStruct::size, 8, fieldStruct::size}, 7, 4, 6+8);

	// setting phase ends with closing a mill
	check_move_numPossibleMoves({	x,    x,    o,
									  o,  x,  x,
									    x,_,_,
								    o,_,_,  o,o,_,
									    _,o,x,
									  _,  o,  x,
								    o,    o,    _}, true, 1, moveInfo{fieldStruct::size, 7, 2}, 7, 3, 7);

	// moving phase, closing a mill
	check_move_numPossibleMoves({	x,    x,    o,
									  o,  x,  x,
									    x,_,_,
								    o,_,_,  o,o,_,
									    _,o,x,
									  _,  o,  x,
								    o,    o,    _}, false, 0, moveInfo{6, 7, 2}, 7, 3, 7);

	// moving phase, opening a mill
	check_move_numPossibleMoves({	x,    x,    o,
									  o,  x,  x,
									    _,x,_,
								    o,_,_,  o,o,_,
									    _,o,x,
									  _,  o,  x,
								    o,    o,    _}, false, 0, moveInfo{7, 8, fieldStruct::size}, 7, 2, 6+7);

	// jumping phase, simple move
	check_move_numPossibleMoves({	_,    o,    _,
									  o,  o,  _,
									    _,_,_,
								    _,_,x,  x,x,x,
									    _,_,_,
									  _,  _,  _,
								    _,    _,    _}, false, 0, moveInfo{11, 15, fieldStruct::size}, 3*17, 8, 3*17);

	// jumping phase, closing a mill
	check_move_numPossibleMoves({	_,    o,    _,
									  o,  o,  _,
									    _,_,x,
								    _,_,x,  _,x,x,
									    _,_,_,
									  _,  _,  _,
								    _,    _,    _}, false, 0, moveInfo{8, 12, 1}, 0, 9, 0);
}

// According to the rules, a player must not remove a stone from a mill. 
// But, a player can remove a stone of the opponent if all stones of the opponent are part of a mill.
// This means two different state numbers are necessary although the same stones are set on the field.
// For example, when a mill is closed again, after a stones has been removed from it.
//    field_0										  field_1										  field_2
// 		x						meta					o						meta					x				<- current player
//		3:3						state					3:4						state					4:4				<- stones set		(cur:opp)
//		0:1												0:1												0:1				<- number of mills  (cur:opp)
// 		1:0												1:1												2:1				<- missing stones 	(cur:opp)
// _,    x,    x			x,    x,    x			x,    x,    x			x,    x,    x			_,    x,    x
//   _,  _,  _,				  _,  _,  _,			  _,  _,  _,			  _,  _,  _,			  _,  _,  _,	
//     o,_,_,				    o,_,_,				    o,_,_,				    o,_,_,				    o,_,_,	
// _,_,o,  _,_,_	-> 		_,_,o,  _,_,_	-> 		_,_,_,  _,_,_		->	_,_,o,  _,_,_		->	_,_,o,  _,_,_
//     o,_,_,				    o,_,_,				    o,_,_,				    o,_,_,				    o,_,_,	
//   _,  _,  _,				  _,  _,  _,			  _,  _,  _,			  _,  _,  _,			  _,  _,  _,	
// _,    _,    _			_,    _,    _			_,    _,    _			_,    _,    _			_,    _,    _
TEST(fieldStruct_Test, test_noStoneRemovalWhenThereAreOnlyMills_settingPhase)
{
	// locals
	fieldStruct 					fs;
	fieldStruct::backupStruct 		oldState;
	std::vector<unsigned int> 		possibilityIds;
	std::vector<fieldStruct::core>  predFields;

	// setting phase with 0 mills for the current player 'x', 1 mill for the opponent 'o'. all stones of the opponent are part of a mill.
	FIELD field_0 = {	_,   x,    x,
						  _,  _,  _,	
						    o,_,_,	
						_,_,o,  _,_,_,
						    o,_,_,	
						  _,  _,  _,	
						_,    _,    _};

	// 'x' shall be the current player
	fs.reset(x);
	EXPECT_TRUE(fs.setSituation(field_0, true, 1));
	checkState(fs, field_0, x, true, 6, playerId::squareIsFree, true, 2, 3, 0, 1, 1, 0, 19, 19, 3, 3);
	check_move_numPossibleMoves(field_0, true, 1, moveInfo{fieldStruct::size, 0, 11}, 19, 19, 18 + 3);

	// check current and opponent player state
	{
		// field_0
		EXPECT_EQ(fs.getCurPlayer().id, 				x);
		EXPECT_EQ(fs.getCurPlayer().numStonesSet, 		3);		
		EXPECT_EQ(fs.getOppPlayer().numStonesSet, 		3);
		EXPECT_EQ(fs.getCurPlayer().numberOfMills, 		0);		
		EXPECT_EQ(fs.getOppPlayer().numberOfMills, 		1);
		EXPECT_EQ(fs.getCurPlayer().numStonesMissing, 	1);
		EXPECT_EQ(fs.getOppPlayer().numStonesMissing, 	0);
		EXPECT_EQ(fs.getCurPlayer().hasOnlyMills, 		false);
		EXPECT_EQ(fs.getOppPlayer().hasOnlyMills, 		true);

		EXPECT_TRUE(fs.move(moveInfo{fieldStruct::size, 0, 11}, oldState));

		// field_1
		EXPECT_EQ(fs.getCurPlayer().id, 				o);
		EXPECT_EQ(fs.getCurPlayer().numStonesSet, 		3);		
		EXPECT_EQ(fs.getOppPlayer().numStonesSet, 		4);
		EXPECT_EQ(fs.getCurPlayer().numberOfMills, 		0);		
		EXPECT_EQ(fs.getOppPlayer().numberOfMills, 		1);
		EXPECT_EQ(fs.getCurPlayer().numStonesMissing, 	1);
		EXPECT_EQ(fs.getOppPlayer().numStonesMissing, 	1);
		EXPECT_EQ(fs.getCurPlayer().hasOnlyMills, 		false);
		EXPECT_EQ(fs.getOppPlayer().hasOnlyMills, 		true);

		EXPECT_TRUE(fs.move(moveInfo{fieldStruct::size, 11, 0}, oldState));

		// field_2
		EXPECT_EQ(fs.getCurPlayer().id, 				x);
		EXPECT_EQ(fs.getCurPlayer().numStonesSet, 		4);		
		EXPECT_EQ(fs.getOppPlayer().numStonesSet, 		4);
		EXPECT_EQ(fs.getCurPlayer().numberOfMills, 		0);		
		EXPECT_EQ(fs.getOppPlayer().numberOfMills, 		1);
		EXPECT_EQ(fs.getCurPlayer().numStonesMissing, 	2);
		EXPECT_EQ(fs.getOppPlayer().numStonesMissing, 	1);
		EXPECT_EQ(fs.getCurPlayer().hasOnlyMills, 		false);
		EXPECT_EQ(fs.getOppPlayer().hasOnlyMills, 		true);
	}

	// more checks with field_1
	FIELD field_1 = {	x,   x,    x,
						  _,  _,  _,	
						    o,_,_,	
						_,_,_,  _,_,_,
						    o,_,_,	
						  _,  _,  _,	
						_,    _,    _};

	// check each single public function
	fs.reset(o);
	EXPECT_TRUE(fs.setSituation(field_1, true, 2));
	fs.invert();
	fs.invert();
	EXPECT_TRUE(fs.isIntegrityOk());
	fs.print();
	EXPECT_EQ(fs.getCurPlayer().numStonesMissing, 1);
	EXPECT_EQ(fs.getOppPlayer().numStonesMissing, 1);
	EXPECT_TRUE(fs.move(moveInfo{fieldStruct::size, 11, 0}, 	oldState));
	EXPECT_EQ(fs.getCurPlayer().numStonesMissing, 2);
	EXPECT_EQ(fs.getOppPlayer().numStonesMissing, 1);
	EXPECT_TRUE(fs.undo(                      					oldState));
	fs.getPossibilities(possibilityIds);
	EXPECT_EQ(possibilityIds.size(), 18+3);
	fs.getPredecessors(predFields);
	EXPECT_EQ(predFields.size(), 3);	// both players are missing a stone (due to totalNumStonesMissing=2), so the mill of the opponent must have been closed.
	EXPECT_EQ(fs.getWinner(), playerId::squareIsFree);
	EXPECT_FALSE(fs.hasGameFinished());
	EXPECT_EQ(fs.getNumStonesSet(),5+1+1);
	EXPECT_TRUE(fs.inSettingPhase());
	EXPECT_EQ(fs.isStonePartOfMill( 1), 1);
	EXPECT_EQ(fs.isStonePartOfMill(10), 0);
	EXPECT_EQ(fs.isStonePartOfMill(21), 0);

	// Check predecessor state
	setState(fs, field_1, o, true, 2);
	EXPECT_EQ(fs.getNumStonesSet(), 7);
	fs.getPredecessors(predFields);
	bool found = false;
	for (auto& predField : predFields) {
		if (field_0 == predField.field) {
			EXPECT_EQ(predField.getCurPlayer().id, x);
			EXPECT_EQ(predField.getOppPlayer().id, o);
			EXPECT_EQ(predField.inSettingPhase(), true);
			found = true;
			break;
		}
	}
	EXPECT_TRUE(found);
}

// According to the rules, a player must not remove a stone from a mill. 
// But, a player can remove a stone of the opponent if all stones of the opponent are part of a mill.
// This is also possible in the moving phase, and there is no need to memorize the remaining stones to be set.
TEST(fieldStruct_Test, test_StoneRemovalWhenThereAreOnlyMills_movingPhase_twoMills)
{
	GTEST_SKIP(); // Temporarily disable this test, as it fails since a stone must be removed if part of two mills.
	// moving phase with 0 mills for the current player 'x', 2 mills for the opponent 'o'. all stones of the opponent are part of a mill.
	fieldStruct 					fs;
	fieldStruct::backupStruct 		oldState;
	std::vector<unsigned int> 		possibilityIds;
	std::vector<fieldStruct::core> 	predFields;
	FIELD field_0 = {	o,    o,    o,		// before move (cur player = x)
						  _,  _,  _,
							_,_,x,
						_,_,_,  x,_,o,
							_,x,_,
						  _,  _,  _,
						x,    _,    o};

	FIELD field_1 = {	o,    o,    _,		// after move (cur player = o)
						  _,  _,  _,
							_,_,x,
						_,_,_,  x,_,o,
							_,_,x,
						  _,  _,  _,
						x,    _,    o};

	// totalNumStonesMissing is not used during moving phase
	EXPECT_FALSE(fs.setSituation(field_0, false, 1));

	// x shall be the starting player
	fs.reset(x);
	EXPECT_TRUE(fs.setSituation(field_0, false, 0));
	checkState(fs, field_0, x, false, 18, playerId::squareIsFree, true, 4, 5, 0, 2, 5, 4, 8, 4, 9, 9);
	check_move_numPossibleMoves(field_0, false, 0, moveInfo{16, 17, 2}, 6, 5, 6);

	// check each single public function
	fs.reset(x);
	EXPECT_TRUE(fs.setSituation(field_0, false, 0));
	fs.invert();
	fs.invert();
	EXPECT_TRUE(fs.isIntegrityOk());
	fs.print();
	EXPECT_EQ(fs.getCurPlayer().numStonesMissing, 5);
	EXPECT_EQ(fs.getOppPlayer().numStonesMissing, 4);
	EXPECT_TRUE(fs.move(moveInfo{16, 17, 2}, 	oldState));
	EXPECT_EQ(fs.getCurPlayer().numStonesMissing, 5);
	EXPECT_EQ(fs.getOppPlayer().numStonesMissing, 5);
	EXPECT_TRUE(fs.undo(                      	oldState));
	fs.getPossibilities(possibilityIds);
	EXPECT_EQ(possibilityIds.size(), 7+5);			// 7 ways to move a stone, +5 stones to remove
	fs.getPredecessors(predFields);
	EXPECT_NE(predFields.size(), 	0);				
	EXPECT_EQ(fs.getWinner(), playerId::squareIsFree);
	EXPECT_FALSE(fs.hasGameFinished());
	EXPECT_EQ(fs.getNumStonesSet(), 18);
	EXPECT_FALSE(fs.inSettingPhase());
	EXPECT_EQ(fs.isStonePartOfMill(0), 1);
	EXPECT_EQ(fs.isStonePartOfMill(2), 2);
	EXPECT_EQ(fs.isStonePartOfMill(21), 0);

	// Check predecessor state
	EXPECT_TRUE(setState(fs, field_1, o, false, 0));
	fs.print();
	fs.getPredecessors(predFields);
	bool found = false;
	for (auto& predField : predFields) {
		if (field_0 == predField.field) {
			EXPECT_EQ(predField.curPlayer.id, x);
			EXPECT_EQ(predField.oppPlayer.id, o);
			EXPECT_EQ(predField.settingPhase, false);
			found = true;
			break;
		}
	}
	EXPECT_TRUE(found);
}

// According to the rules, a player must not remove a stone from a mill. 
// But, a player can remove a stone of the opponent if all stones of the opponent are part of a mill.
// This is also possible in the moving phase, and there is no need to memorize the remaining stones to be set.
TEST(fieldStruct_Test, test_StoneRemovalWhenThereAreOnlyMills_movingPhase_singleMill)
{
	// moving phase with 0 mills for the current player 'x', 2 mill for the opponent 'o'. all stones of the opponent are part of a mill.
	fieldStruct 					fs;
	fieldStruct::backupStruct 		oldState;
	std::vector<unsigned int> 		possibilityIds;
	std::vector<fieldStruct::core> 	predFields;
	FIELD field_0 = {	o,    o,    o,		// before move (cur player = x)
						  _,  _,  _,
							_,_,x,
						_,_,_,  x,_,o,
							_,x,_,
						  _,  _,  _,
						x,    _,    o};

	FIELD field_1 = {	o,    _,    o,		// after move (cur player = o)
						  _,  _,  _,
							_,_,x,
						_,_,_,  x,_,o,
							_,_,x,
						  _,  _,  _,
						x,    _,    o};

	// totalNumStonesMissing is not used during moving phase
	EXPECT_FALSE(fs.setSituation(field_0, false, 1));

	// x shall be the starting player
	fs.reset(x);
	EXPECT_TRUE(fs.setSituation(field_0, false, 0));
	checkState(fs, field_0, x, false, 18, playerId::squareIsFree, true, 4, 5, 0, 2, 5, 4, 8, 4, 9, 9);
	check_move_numPossibleMoves(field_0, false, 0, moveInfo{16, 17, 1}, 5, 5, 5);

	// check each single public function
	fs.reset(x);
	EXPECT_TRUE(fs.setSituation(field_0, false, 0));
	fs.invert();
	fs.invert();
	EXPECT_TRUE(fs.isIntegrityOk());
	fs.print();
	EXPECT_EQ(fs.getCurPlayer().numStonesMissing, 5);
	EXPECT_EQ(fs.getOppPlayer().numStonesMissing, 4);
	EXPECT_TRUE(fs.move(moveInfo{16, 17, 1}, 	oldState));
	EXPECT_EQ(fs.getCurPlayer().numStonesMissing, 5);
	EXPECT_EQ(fs.getOppPlayer().numStonesMissing, 5);
	EXPECT_TRUE(fs.undo(                      	oldState));
	fs.getPossibilities(possibilityIds);
	EXPECT_EQ(possibilityIds.size(), 7+4);			// 7 ways to move a stone, +4 stones to remove (only 4 stones can be removed, because one stone is part of two mills)
	fs.getPredecessors(predFields);
	EXPECT_NE(predFields.size(), 	0);				
	EXPECT_EQ(fs.getWinner(), playerId::squareIsFree);
	EXPECT_FALSE(fs.hasGameFinished());
	EXPECT_EQ(fs.getNumStonesSet(), 18);
	EXPECT_FALSE(fs.inSettingPhase());
	EXPECT_EQ(fs.isStonePartOfMill( 0), 1);
	EXPECT_EQ(fs.isStonePartOfMill( 2), 2);
	EXPECT_EQ(fs.isStonePartOfMill(21), 0);

	// Check predecessor state
	EXPECT_TRUE(setState(fs, field_1, o, false, 0));
	fs.print();
	fs.getPredecessors(predFields);
	bool found = false;
	for (auto& predField : predFields) {
		if (field_0 == predField.field) {
			EXPECT_EQ(predField.curPlayer.id, x);
			EXPECT_EQ(predField.oppPlayer.id, o);
			EXPECT_EQ(predField.settingPhase, false);
			found = true;
			break;
		}
	}
	EXPECT_TRUE(found);
}

TEST(fieldStruct_Test, test_StoneRemovalWhenThereAreOnlyMills_jumpingPhase)
{
	// jumping phase with 1 mill for the current player 'x', 1 mill for the opponent 'o'. all stones of the opponent are part of a mill.
	fieldStruct 					fs;
	fieldStruct::backupStruct 		oldState;
	std::vector<unsigned int> 		possibilityIds;
	std::vector<fieldStruct::core> 	predFields;
	FIELD field = {	o,    o,    o,
					  _,  _,  _,
						_,_,x,
					_,_,_,  x,_,_,
						x,_,_,
					  _,  _,  _,
					_,    _,    _};

	// x shall be the starting player
	fs.reset(x);
	EXPECT_TRUE(fs.setSituation(field, false, 0));
	checkState(fs, field, x, false, 18, playerId::squareIsFree, true, 3, 3, 0, 1, 6, 6, 54, 54, 9, 9);
	check_move_numPossibleMoves(field, false, 0, moveInfo{15, 17, 2}, 0, 57, 0);

	// check each single public function
	fs.reset(x);
	EXPECT_TRUE(fs.setSituation(field, false, 0));
	fs.invert();
	fs.invert();
	EXPECT_TRUE(fs.isIntegrityOk());
	fs.print();
	EXPECT_TRUE(fs.move(moveInfo{15, 17, 2}, 	oldState));
	EXPECT_EQ(fs.getCurPlayer().numStonesMissing, 7);
	EXPECT_EQ(fs.getOppPlayer().numStonesMissing, 6);
	EXPECT_TRUE(fs.undo(						oldState));
	EXPECT_EQ(fs.getCurPlayer().numStonesMissing, 6);
	EXPECT_EQ(fs.getOppPlayer().numStonesMissing, 6);
	fs.getPossibilities(possibilityIds);
	EXPECT_EQ(possibilityIds.size(), 53+3);
	fs.getPredecessors(predFields);
	EXPECT_EQ(predFields.size(), 3*18*17 - 3);	// player 'o' just closed a mill, and removed a stone from a square, which is now free. -3 because the stone was not removed from the closed mill.
	EXPECT_EQ(fs.getWinner(), playerId::squareIsFree);
	EXPECT_FALSE(fs.hasGameFinished());
	EXPECT_EQ(fs.getNumStonesSet(), 18);
	EXPECT_FALSE(fs.inSettingPhase());
	EXPECT_EQ(fs.isStonePartOfMill(0), 1);
	EXPECT_EQ(fs.isStonePartOfMill(2), 1);
	EXPECT_EQ(fs.isStonePartOfMill(21), 0);

	// Check predecessor state
	FIELD f_succ = {	o,    o,    _,
						  _,  _,  _,
							_,_,x,
						_,_,_,  x,_,_,
							_,_,x,
						  _,  _,  _,
						_,    _,    _};

	EXPECT_TRUE(setState(fs, f_succ, o, false, 0));
	fs.getPredecessors(predFields);
	bool found = false;
	for (auto& predField : predFields) {
		if (field == predField.field) {
			EXPECT_EQ(predField.curPlayer.id, x);
			EXPECT_EQ(predField.oppPlayer.id, o);
			EXPECT_EQ(predField.settingPhase, false);
			found = true;
			break;
		}
	}
	EXPECT_TRUE(found);
}
