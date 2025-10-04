/**************************************************************************************************************************
	threadSpecificTest.cpp
 	Copyright (c) Thomas Weber. All rights reserved.				
	Licensed under the MIT License.
	https://github.com/madweasel/madweasels-cpp
***************************************************************************************************************************/
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "ai/threadSpecific.h"

using ::testing::ElementsAre;

class threadVarsStruct_Test : public testing::Test {

protected:
	static const std::wstring tmpFileDirectory;

	stateAddressing								sa{tmpFileDirectory};
	unsigned int 								layerNum, stateNumber, symOp;
	vector<unsigned int> 						possibilityIds;
	vector<miniMax::stateAdressStruct> 			symStates;
	vector<miniMax::retroAnalysis::predVars> 	predVars;
	fieldStruct 								field;
	void* 										pBackup;

	void SetUp() override {
	}
};

const std::wstring threadVarsStruct_Test::tmpFileDirectory = [] {
	std::wstring path = (std::filesystem::temp_directory_path() / "Muehle" / "threadVarsStruct").c_str();
	std::filesystem::create_directories(path);
	return path;
}();

TEST_F(threadVarsStruct_Test, getter)
{
	// locals
	threadVarsStruct tvs{sa};

	// By default the state number 0 and layer 199 should be set, so the field should be empty
	tvs.printField(tvs.getValueOfSituation(), 10);
	EXPECT_EQ(tvs.getValueOfSituation(), miniMax::SKV_VALUE_GAME_DRAWN);
	EXPECT_EQ(tvs.getLayerNumber(), 199);
	tvs.getLayerAndStateNumber(layerNum, stateNumber, symOp);
	EXPECT_EQ(layerNum, 199);
	EXPECT_EQ(stateNumber, 0);
	EXPECT_EQ(symOp, stateAddressing::SO_DO_NOTHING);
	
	// get field
	field = tvs.getField();
	EXPECT_EQ(field.getNumStonesSet(), 0);
	EXPECT_EQ(field.inSettingPhase(), true);

	// get all possibilities 
	tvs.getPossibilities(possibilityIds);
	cout << "First possibility would be to set on the upper left corner (a): " << endl;
	tvs.printMoveInformation(0);
	cout << "last possibility would be to set on the lower right corner (x): " << endl;
	tvs.printMoveInformation(possibilityIds.back());
	
	tvs.getSymStateNumWithDuplicates(symStates);
	EXPECT_EQ(symStates.size(), 16);
	EXPECT_EQ(symStates[0].layerNumber, 199);
	EXPECT_EQ(symStates[0].stateNumber, 0);
	EXPECT_EQ(symStates.back().layerNumber, 199);
	EXPECT_EQ(symStates.back().stateNumber, 0);

	// there is no predecessor state
	tvs.getPredecessors(predVars);
	EXPECT_EQ(predVars.size(), 0);
}

TEST_F(threadVarsStruct_Test, setSituation)
{
	// locals
	threadVarsStruct tvs{sa};

	// set any arbitrary situation and check consistency
	EXPECT_TRUE(tvs.setSituation(88, 6172));
	tvs.printField(tvs.getValueOfSituation(), 0);
	EXPECT_EQ(tvs.getLayerNumber(), 88);
	tvs.getLayerAndStateNumber(layerNum, stateNumber, symOp);
	EXPECT_EQ(layerNum, 88);
	EXPECT_EQ(stateNumber, 6172);
	EXPECT_EQ(symOp, stateAddressing::SO_DO_NOTHING);

	// get and set field again
	tvs.setField(tvs.getField());
	tvs.getLayerAndStateNumber(layerNum, stateNumber, symOp);
	EXPECT_EQ(layerNum, 88);
	EXPECT_EQ(stateNumber, 6172);
	EXPECT_EQ(symOp, stateAddressing::SO_DO_NOTHING);

	// set any symmetrical situation and check consistency
	tvs.getSymStateNumWithDuplicates(symStates);
	EXPECT_EQ(symStates.size(), 16);
	tvs.setSituation(symStates[0].layerNumber, symStates[0].stateNumber);
	tvs.printField(tvs.getValueOfSituation(), 0);
	tvs.getLayerAndStateNumber(layerNum, stateNumber, symOp);
	EXPECT_EQ(layerNum, 88);
	EXPECT_EQ(stateNumber, 6172);
	EXPECT_EQ(symOp, stateAddressing::SO_DO_NOTHING);

	// apply symmetry operation
	tvs.applySymOp(stateAddressing::SO_TURN_RIGHT, false, false);
	tvs.printField(tvs.getValueOfSituation(), 0);
	tvs.getLayerAndStateNumber(layerNum, stateNumber, symOp);
	EXPECT_EQ(layerNum, 88);
	EXPECT_EQ(stateNumber, 6172);
	EXPECT_EQ(symOp, stateAddressing::SO_TURN_LEFT);

	// get all possibilities and check consistency
	EXPECT_TRUE(tvs.setSituation(88, 6172));
	tvs.getPossibilities(possibilityIds);
	EXPECT_EQ(possibilityIds.size(), 6);

	// do a move
	field = tvs.getField();
	tvs.move(possibilityIds[0], pBackup);
	EXPECT_NE(pBackup, nullptr);
	tvs.printField(tvs.getValueOfSituation(), 0);
	tvs.undo(possibilityIds[0], pBackup);
	EXPECT_EQ(tvs.getField(), field);

	// get predecessor states
	tvs.printField(tvs.getValueOfSituation(), 0);
	tvs.getPredecessors(predVars);
	EXPECT_EQ(predVars.size(), 18);
	tvs.setSituation(predVars[0].predLayerNumber, predVars[0].predStateNumber);
	tvs.printField(tvs.getValueOfSituation(), 0);
	tvs.getLayerAndStateNumber(layerNum, stateNumber, symOp);
	EXPECT_EQ(layerNum, predVars[0].predLayerNumber);
	EXPECT_EQ(stateNumber, predVars[0].predStateNumber);
	EXPECT_EQ(symOp, stateAddressing::SO_DO_NOTHING);

	// reset
	tvs.reset();
	EXPECT_EQ(tvs.getLayerNumber(), 199);
	tvs.getLayerAndStateNumber(layerNum, stateNumber, symOp);
	EXPECT_EQ(layerNum, 199);
	EXPECT_EQ(stateNumber, 0);
	EXPECT_EQ(symOp, stateAddressing::SO_DO_NOTHING);
}
