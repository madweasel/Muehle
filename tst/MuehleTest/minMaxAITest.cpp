/**************************************************************************************************************************
	minMaxAI_Test.cpp
 	Copyright (c) Thomas Weber. All rights reserved.				
	Licensed under the MIT License.
	https://github.com/madweasel/madweasels-cpp
***************************************************************************************************************************/
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "ai/minMaxAI.h"
#include "ai/randomAI.h"

using ::testing::ElementsAre;

class minMaxAI_Test : public testing::Test {

protected:
	// Constants to simplify the test
	const playerId x 	= playerId::playerOne;
	const playerId o 	= playerId::playerTwo;
	const playerId _ 	= playerId::squareIsFree;

	minMaxAI 			myGame{};
	
	void SetUp() override {
	}
};

TEST_F(minMaxAI_Test, simpleFunctions) 
{
	// locals
	fieldStruct 				theField;
	moveInfo 					move;
	fieldStruct::backupStruct 	oldState;

	myGame.setSearchDepth(4);

	theField.reset(x);
	theField.setSituation({
		_,    _,    _,
		  _,  _,  o,
			_,_,_,
		_,_,_,  _,o,_,
			x,x,x,
		  o,  x,  _,
		_,    o,    _}, false, 0);

	myGame.play(theField, move);
	EXPECT_THAT(move.from, ::testing::AnyOf(15,16,17,19));
	EXPECT_THAT(move.to, ::testing::AnyOf(11,12,20));
	EXPECT_EQ(move.removeStone, 24);

	theField.move(move, oldState);
	myGame.play(theField, move);
	EXPECT_THAT(move.from, ::testing::AnyOf(5, 13, 18, 22));
	EXPECT_THAT(move.to, ::testing::AnyOf(4, 10, 12, 14, 19, 20, 21, 23));
	EXPECT_EQ(move.removeStone, 24);
}

TEST_F(minMaxAI_Test, winAgainstRandomAI)
{
	// locals
	fieldStruct 				theField;
	moveInfo 					move;	
	fieldStruct::backupStruct 	oldState;
	randomAI					randomAI{};
	muehleAI*					currentAI;
	const unsigned int			maxNumMoves 	= 1000;
	const unsigned int			numGames 		= 10;
	unsigned int				numMoves;
	unsigned int				minMaxAIWins 	= 0;
	unsigned int				randomAIWins 	= 0;

	myGame.setSearchDepth(4);

	for (unsigned int game = 0; game <= numGames; ++game) {
		std::cout << "Game " << game << std::endl;

		// first player shall be minMaxAI
		theField.reset(x);
		currentAI = &myGame;
		numMoves = 0;

		while (numMoves < maxNumMoves) {

			// ensure that minMaxAI is x
			if (currentAI == &myGame) {
				ASSERT_EQ(theField.getCurPlayer().id, x);
			} else {
				ASSERT_EQ(theField.getCurPlayer().id, o);
			}

			// let currentAI play
			currentAI->play(theField, move);
			if (!theField.move(move, oldState)) {
				std::cout << "*** Current state ***" << std::endl;
				std::cout << "Current AI:               " << (currentAI == &myGame ? "minMaxAI" : "randomAI") << std::endl;
				theField.print();
				std::cout << "Move failed:              " << move.from << " -> " << move.to << std::endl;
				ASSERT_TRUE(false);
			}

			// check if the game has finished
			if (theField.hasGameFinished()) {
				if (theField.getWinner() == x) {
					minMaxAIWins++;
				} else if (theField.getWinner() == o) {
					randomAIWins++;
				}
				break;
			}

			// switch AI
			if (currentAI == &myGame) {
				currentAI = &randomAI;
			} else {
				currentAI = &myGame;
			}
			numMoves++;
		}
	}

	// Print results
	std::cout << "minMaxAI Wins: " << minMaxAIWins << std::endl;
	std::cout << "randomAI Wins: " << randomAIWins << std::endl;

	EXPECT_GE(minMaxAIWins, randomAIWins);
	EXPECT_EQ(randomAIWins, 0);
}
