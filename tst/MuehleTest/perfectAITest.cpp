/**************************************************************************************************************************
	MuehleTest.cpp
 	Copyright (c) Thomas Weber. All rights reserved.				
	Licensed under the MIT License.
	https://github.com/madweasel/madweasels-cpp
***************************************************************************************************************************/
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "ai/perfectAI.h"
#include "ai/stateAddressing.h"

const wstring PATH_TO_COMPLETE_DATABASE = L"C:\\Coding\\Muehle\\database";

using ::testing::ElementsAre;

class perfectAI_Test : public testing::Test {

protected:
	perfectAI 			myGame{L"./database"};
	unsigned int 		testQualityFactor = 1;				// set to 0 for full test, 1 for fast test, bigger numbers (e.g. 1000) for more aggressive testing

	void SetUp() override {
		myGame.mm.checker.setOutputFrequency(1e7);
		myGame.mm.checker.setMaxNumStatesToTest(1e3*testQualityFactor);
	}

	unsigned long long getTotalSystemMemory()
	{
		MEMORYSTATUSEX status;
		status.dwLength = sizeof(status);
		GlobalMemoryStatusEx(&status);
		return status.ullTotalPhys;
	}
};

TEST_F(perfectAI_Test, simpleFunctions) 
{
	// locals
	float 										floatValue;
	miniMax::twoBit 							shortValue;
 	vector<unsigned int> 						possibilityIds;
	vector<miniMax::retroAnalysis::predVars> 	predVars;
	vector<unsigned int> 						succLayers;
	vector<miniMax::stateAdressStruct>			symStates;
	unsigned int 								layerNum;
	unsigned int 								stateNumber;
	unsigned int								symOp;

	// empty field
	EXPECT_TRUE(myGame.setSituation(0, 199, 0));					// set the empty field
	EXPECT_EQ(myGame.getNumberOfKnotsInLayer(199), 18);				// only one field state, but potentially 18 stones are missing
	EXPECT_EQ(myGame.getPartnerLayers(199), vector<unsigned int>{199});	// no partner layer
	EXPECT_EQ(myGame.getOutputInformation(199), L" white stones : 0  \tblack stones  : 0");	// no stones set
	myGame.printField(0, miniMax::SKV_VALUE_GAME_DRAWN, 1);
	EXPECT_EQ(myGame.getNumberOfLayers(), 200);						// 200 layers
	EXPECT_EQ(myGame.getMaxNumPlies(), miniMax::PLYINFO_EXP_VALUE);	// 1000 plies
	EXPECT_EQ(myGame.getMaxNumPossibilities(), 3*18*9);				// the last 3 stones can jump to any free field. there are 18=24-6 free fields. in total 3*18*9=486 possibilities
	EXPECT_EQ(myGame.shallRetroAnalysisBeUsed(0), true);			// retro analysis for < 100
	EXPECT_EQ(myGame.shallRetroAnalysisBeUsed(100), false);			// no retro analysis
	myGame.getPossibilities(0, possibilityIds);		
	myGame.printMoveInformation(0, possibilityIds[0]);				// print the first move
	EXPECT_EQ(possibilityIds.size(), 3*8);							// still all possibilities are free
	myGame.getValueOfSituation(0, floatValue, shortValue);			// no value yet
	EXPECT_EQ(floatValue, 0.0f);									// somehow the float value is equal the short value
	EXPECT_EQ(shortValue, miniMax::SKV_VALUE_GAME_DRAWN);		
	EXPECT_TRUE(myGame.lostIfUnableToMove(0));						// if no move is possible, the game is lost
	EXPECT_TRUE(myGame.isStateIntegrityOk(0));						// the state is valid
	myGame.getPredecessors(0, predVars);							// no predecessors, since the field is empty
	EXPECT_EQ(predVars.size(), 0);		
	EXPECT_EQ(myGame.getLayerNumber(0), 199);						// empty field means no master stone
	myGame.getSuccLayers(199, succLayers);							// 
	EXPECT_EQ(succLayers.size(), 2);								// 198, 197
	myGame.getLayerAndStateNumber(0, layerNum, stateNumber, symOp);	// should be 199, 0, 0
	EXPECT_EQ(layerNum, 199);
	EXPECT_EQ(stateNumber, 0);
	// EXPECT_EQ(symOp, stateAddressing::SO_DO_NOTHING);
	myGame.getSymStateNumWithDuplicates(0, symStates);				// only one state, namely the empty field
	miniMax::stateAdressStruct emptyFieldState{0,199};
	EXPECT_THAT(symStates, ElementsAre(	emptyFieldState,emptyFieldState,emptyFieldState,emptyFieldState,
										emptyFieldState,emptyFieldState,emptyFieldState,emptyFieldState,
										emptyFieldState,emptyFieldState,emptyFieldState,emptyFieldState,
										emptyFieldState,emptyFieldState,emptyFieldState,emptyFieldState));			// 16 times the same state

	// try setting invalid situations
	EXPECT_FALSE(myGame.setSituation(0, 199, 19));					// invalid state number
	EXPECT_FALSE(myGame.setSituation(0, 200, 0));					// invalid layer number
	EXPECT_FALSE(myGame.setSituation(100, 0, 0));					// invalid thread number

	// set the stone on the first field
	EXPECT_TRUE(myGame.setSituation(0, 198, 0));					// white stone on the middle of the left side
	myGame.getValueOfSituation(0, floatValue, shortValue);
	myGame.printField(0, miniMax::SKV_VALUE_GAME_DRAWN, 1);
	myGame.applySymOp(0, stateAddressing::SO_DO_NOTHING, 	false, false);			// apply no symmetry
	myGame.applySymOp(0, stateAddressing::SO_MIRROR_VERT, 	false, false);			// mirror vertically
	myGame.printField(0, miniMax::SKV_VALUE_GAME_DRAWN, 1);
	EXPECT_EQ(shortValue, miniMax::SKV_VALUE_GAME_DRAWN);
}

TEST_F(perfectAI_Test, NullptrAndZeros) 
{	
	const unsigned int 							numThreads			= 4;
	unsigned int								symOp				= 0;
	unsigned int								stateNum			= 0;
	unsigned int								layerNum			= 0;
	float										floatValue			= 0;
	miniMax::twoBit								shortValue			= 0;
	miniMax::plyInfoVarType						plyInfo				= 0;
	bool										playerToMoveChanged	= false;
	void*										pBackup				= nullptr;
	vector<miniMax::retroAnalysis::predVars>	predVars;
	vector<unsigned int>						succLayers;
	vector<miniMax::stateAdressStruct>			symStates;
	vector<unsigned int>						possibiltyIds;

	myGame.mm.setNumThreads(numThreads);

	// robustness tests with nullptr and zeros and also invalid threadNo
	for (unsigned int threadNo = 0; threadNo <= numThreads + 1; threadNo++) {

		// reset
		layerNum		= 0;
		stateNum		= 0;

		// getter
		ASSERT_EQ(myGame.getLayerNumber(threadNo),									threadNo < numThreads ? 199 : myGame.getNumberOfLayers());
		ASSERT_EQ(myGame.getNumberOfKnotsInLayer(layerNum),							0);
		ASSERT_EQ(myGame.getNumberOfLayers(),										200);
		ASSERT_THAT(myGame.getPartnerLayers(layerNum), 								ElementsAre(layerNum));
		ASSERT_EQ(myGame.getOutputInformation(layerNum),							wstring(L" white stones : 0  \tblack stones  : 0"));
		ASSERT_EQ(myGame.shallRetroAnalysisBeUsed(layerNum),						true);
		myGame.getPossibilities(threadNo, possibiltyIds);
		myGame.getPredecessors(threadNo, predVars);									EXPECT_EQ(predVars.size(), 0);
		myGame.getSuccLayers(layerNum, succLayers);									EXPECT_EQ(succLayers.size(), 0);
		myGame.getSymStateNumWithDuplicates(threadNo, symStates);					EXPECT_EQ(symStates.size(), 16);
		myGame.getLayerAndStateNumber(threadNo, layerNum, stateNum, symOp);			EXPECT_EQ(layerNum, threadNo < numThreads ? 199 : 200);	EXPECT_EQ(stateNum, 0);	EXPECT_EQ(symOp, threadNo < numThreads ? stateAddressing::SO_DO_NOTHING : 0);
		myGame.getValueOfSituation(threadNo, floatValue, shortValue);				ASSERT_EQ(floatValue, 0.0f); ASSERT_EQ(shortValue, threadNo < numThreads ? miniMax::SKV_VALUE_GAME_DRAWN : miniMax::SKV_VALUE_INVALID);

		// setter
		ASSERT_EQ(myGame.setSituation(threadNo, layerNum, stateNum),				threadNo < numThreads ? true : false);
		myGame.move(threadNo, 0, playerToMoveChanged, pBackup);						
		myGame.undo(threadNo, 0, playerToMoveChanged, pBackup);						

		// output
		myGame.printField(threadNo, miniMax::SKV_VALUE_GAME_DRAWN, 0);
		myGame.printMoveInformation(threadNo, 0);
	}
}

TEST_F(perfectAI_Test, setSituationAndGetStateNumber) 
{
	for (unsigned int layerNumber = 0; layerNumber < myGame.getNumberOfLayers(); layerNumber++) {
		if (!myGame.getNumberOfKnotsInLayer(layerNumber)) continue;
		ASSERT_TRUE(myGame.mm.checker.testSetSituationAndGetStateNum(layerNumber));
	}
}

TEST_F(perfectAI_Test, moveAndUndo) 
{
	for (unsigned int layerNumber = 0; layerNumber < myGame.getNumberOfLayers(); layerNumber++) {
		if (!myGame.getNumberOfKnotsInLayer(layerNumber)) continue;
		ASSERT_TRUE(myGame.mm.checker.testMoveAndUndo(layerNumber));
	}
}

TEST_F(perfectAI_Test, getPredecessors) 
{
	myGame.mm.checker.setMaxNumStatesToTest(1e2*testQualityFactor);
	for (unsigned int layerNumber = 0; layerNumber < myGame.getNumberOfLayers(); layerNumber++) {
		if (!myGame.getNumberOfKnotsInLayer(layerNumber)) continue;
		ASSERT_TRUE(myGame.mm.checker.testGetPredecessors(layerNumber));
	}
	myGame.mm.checker.setMaxNumStatesToTest(1e3*testQualityFactor);
}

TEST_F(perfectAI_Test, getPossibilities) 
{
	for (unsigned int layerNumber = 0; layerNumber < myGame.getNumberOfLayers(); layerNumber++) {
		if (!myGame.getNumberOfKnotsInLayer(layerNumber)) continue;
		ASSERT_TRUE(myGame.mm.checker.testGetPossibilities(layerNumber));
	}
}

TEST_F(perfectAI_Test, testLayer) 
{
	// Skip test if no database is available
	if (!std::filesystem::exists(PATH_TO_COMPLETE_DATABASE + L"\\plyInfo.dat")) {
		GTEST_SKIP() << L"Skipping test: Database file 'plyInfo.dat' not found in " << PATH_TO_COMPLETE_DATABASE;
	}

	// Skip if less then 24 GB RAM
	if (getTotalSystemMemory() < 24LL * 1024 * 1024 * 1024) {
		GTEST_SKIP() << L"Skipping test: Less than 24 GB RAM available";
	}

	ASSERT_TRUE(myGame.mm.openDatabase(PATH_TO_COMPLETE_DATABASE));
	if (!myGame.mm.db.isComplete()) {
		GTEST_SKIP() << L"Skipping test: Incomplete database";
	}
	myGame.mm.checker.setMaxNumStatesToTest(1e2*testQualityFactor);
	for (unsigned int layerNumber = 0; layerNumber < myGame.getNumberOfLayers(); layerNumber++) {
		if (!myGame.getNumberOfKnotsInLayer(layerNumber)) continue;
		ASSERT_TRUE(myGame.mm.checker.testLayer(layerNumber));
	}
	myGame.mm.closeDatabase();
}

TEST_F(perfectAI_Test, testIfSymStatesHaveSameValue) 
{
	// Skip test if no database is available
	if (!std::filesystem::exists(PATH_TO_COMPLETE_DATABASE + L"\\plyInfo.dat")) {
		GTEST_SKIP() << L"Skipping test: Database file 'plyInfo.dat' not found in " << PATH_TO_COMPLETE_DATABASE;
	}

	// Skip if less then 24 GB RAM
	if (getTotalSystemMemory() < 24LL * 1024 * 1024 * 1024) {
		GTEST_SKIP() << L"Skipping test: Less than 24 GB RAM available";
	}

	ASSERT_TRUE(myGame.mm.openDatabase(PATH_TO_COMPLETE_DATABASE));
	if (!myGame.mm.db.isComplete()) {
		GTEST_SKIP() << L"Skipping test: Incomplete database";
	}	
	for (unsigned int layerNumber = 0; layerNumber < myGame.getNumberOfLayers(); layerNumber++) {
		if (!myGame.getNumberOfKnotsInLayer(layerNumber)) continue;
		ASSERT_TRUE(myGame.mm.checker.testIfSymStatesHaveSameValue(layerNumber));
	}
	myGame.mm.closeDatabase();
}
