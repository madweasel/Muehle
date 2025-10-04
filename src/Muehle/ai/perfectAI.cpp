/*********************************************************************
	perfectAI.cpp													  
 	Copyright (c) Thomas Weber. All rights reserved.				
	Licensed under the MIT License.
	https://github.com/madweasel/madweasels-cpp
\*********************************************************************/

#include "perfectAI.h"

//-----------------------------------------------------------------------------
// Name: perfectAI()
// Desc: perfectAI class constructor
//-----------------------------------------------------------------------------
perfectAI::perfectAI(wstring const& directory) :
	databaseDirectory(calcDatabaseDirectory(directory)),
	sa(databaseDirectory)
{
	// thread specific variables
	threadVars.resize(mm.getNumThreads(), threadVarsStruct(sa));
}

//-----------------------------------------------------------------------------
// Name: ~perfectAI()
// Desc: perfectAI class destructor
//-----------------------------------------------------------------------------
perfectAI::~perfectAI()
{
}

//-----------------------------------------------------------------------------
// Name: calcDatabaseDirectory()
// Desc: 
//-----------------------------------------------------------------------------
wstring perfectAI::calcDatabaseDirectory(wstring const& directory)
{
	wstring databaseDirectory = directory;
	if (!databaseDirectory.size()) { databaseDirectory = filesystem::current_path().wstring(); }
	wcout << "Path to database set to: " << filesystem::absolute(databaseDirectory) << endl;
	return databaseDirectory;
}

//-----------------------------------------------------------------------------
// Name: play()
// Desc: 
//-----------------------------------------------------------------------------
void perfectAI::play(const fieldStruct& theField, moveInfo& move)
{
	// locals
	threadVars[0].setField(theField);
	unsigned int				bestChoice;
	unsigned int 				depthOfFullTree = 3;
	
	// open database file
	if (!mm.openDatabase(databaseDirectory)) {
		cout << "ERROR: Could not open database file!\n";
		move = moveInfo{};
	}
	
	// current state already calculated?
	if (!mm.isCurrentStateInDatabase(0)) {
		cout << "ERROR: Current state is not in database!\n";
		move = moveInfo{};
	}
	
	// start the miniMax-algorithmn
	mm.setSearchDepth(depthOfFullTree);
	mm.getBestChoice(bestChoice, infoAboutChoices);

	// decode the best choice
	move.setId(bestChoice);
}

//-----------------------------------------------------------------------------
// Name: prepareDatabaseCalculation()
// Desc: 
//-----------------------------------------------------------------------------
void perfectAI::prepareCalculation()
{
	for (unsigned int curThread=0; curThread<mm.getNumThreads(); curThread++) {
		threadVars[curThread].reset();
	}

	// open database file
	mm.openDatabase(databaseDirectory);
}

//-----------------------------------------------------------------------------
// Name: getPossibilities()
// Desc: 
//-----------------------------------------------------------------------------
void perfectAI::getPossibilities(unsigned int threadNo, vector<unsigned int>& possibilityIds)
{
	if (threadNo >= mm.getNumThreads()) return;
	threadVars[threadNo].getPossibilities(possibilityIds);
}

//-----------------------------------------------------------------------------
// Name: getValueOfSituation()
// Desc: 
//-----------------------------------------------------------------------------
void perfectAI::getValueOfSituation(unsigned int threadNo, float &floatValue, miniMax::twoBit &shortValue)
{
	floatValue = 0;
	if (threadNo >= mm.getNumThreads()) { floatValue = 0; shortValue = miniMax::SKV_VALUE_INVALID;	return;	}
	shortValue = threadVars[threadNo].getValueOfSituation();
}

//-----------------------------------------------------------------------------
// Name: undo()
// Desc: 
//-----------------------------------------------------------------------------
void perfectAI::undo(unsigned int threadNo, unsigned int idPossibility, bool& playerToMoveChanged, void *pBackup)
{
	// locals
	if (threadNo >= mm.getNumThreads()) return;
	if (!pBackup) return;
	threadVars[threadNo].undo(idPossibility, pBackup);
	playerToMoveChanged = true;
}

//-----------------------------------------------------------------------------
// Name: move()
// Desc: 
//-----------------------------------------------------------------------------
void perfectAI::move(unsigned int threadNo, unsigned int idPossibility, bool& playerToMoveChanged, void* &pBackup)
{
	// locals
	if (threadNo >= mm.getNumThreads()) return;
	threadVars[threadNo].move(idPossibility, pBackup);
	playerToMoveChanged = true;
}

//-----------------------------------------------------------------------------
// Name: printMoveInformation()
// Desc: 
//-----------------------------------------------------------------------------
void perfectAI::printMoveInformation(unsigned int threadNo, unsigned int idPossibility)
{
	// locals
	if (threadNo >= mm.getNumThreads()) return;
	threadVars[threadNo].printMoveInformation(idPossibility);
}

//-----------------------------------------------------------------------------
// Name: getMaxNumPossibilities()
// Desc:
//-----------------------------------------------------------------------------
unsigned int perfectAI::getMaxNumPossibilities()
{
	return fieldStruct::maxNumPosMoves;
}

//-----------------------------------------------------------------------------
// Name: getNumberOfLayers()
// Desc: called one time
//-----------------------------------------------------------------------------
unsigned int perfectAI::getNumberOfLayers()
{
	return stateAddressing::NUM_LAYERS;
}

//-----------------------------------------------------------------------------
// Name: getMaxNumPlies()
// Desc: 
//-----------------------------------------------------------------------------
unsigned int perfectAI::getMaxNumPlies()
{
	return miniMax::PLYINFO_EXP_VALUE;
}

//-----------------------------------------------------------------------------
// Name: shallRetroAnalysisBeUsed()
// Desc: called one time for each layer time
//-----------------------------------------------------------------------------
bool perfectAI::shallRetroAnalysisBeUsed(unsigned int layerNum)
{
    if (layerNum < 100) 
	    return true;
    else 
        return false;
}

//-----------------------------------------------------------------------------
// Name: getNumberOfKnotsInLayer()
// Desc: called one time
//-----------------------------------------------------------------------------
unsigned int perfectAI::getNumberOfKnotsInLayer(unsigned int layerNum)
{
    return sa.getNumberOfKnotsInLayer(layerNum);
}

//-----------------------------------------------------------------------------
// Name: applySymOp()
// Desc: called very often
//-----------------------------------------------------------------------------
void perfectAI::applySymOp(unsigned int threadNo, unsigned char symmetryOperationNumber, bool doInverseOperation, bool playerToMoveChanged)
{
	if (threadNo >= mm.getNumThreads()) return;
	threadVars[threadNo].applySymOp(symmetryOperationNumber, doInverseOperation, playerToMoveChanged);
}

//-----------------------------------------------------------------------------
// Name: getLayerNumber()
// Desc: 
//-----------------------------------------------------------------------------
unsigned int perfectAI::getLayerNumber(unsigned int threadNo)
{
	if (threadNo >= mm.getNumThreads()) return getNumberOfLayers();
	return threadVars[threadNo].getLayerNumber();
}

//-----------------------------------------------------------------------------
// Name: getLayerAndStateNumber()
// Desc: 
//-----------------------------------------------------------------------------
void perfectAI::getLayerAndStateNumber(unsigned int threadNo, unsigned int& layerNum, unsigned int& stateNumber, unsigned int& symOp)
{
	if (threadNo >= mm.getNumThreads()) { layerNum = getNumberOfLayers(); stateNumber = 0; symOp = 0; return; }
	threadVars[threadNo].getLayerAndStateNumber(layerNum, stateNumber, symOp);
}

//-----------------------------------------------------------------------------
// Name: setSituation()
// Desc: Current player has white stones, the opponent the black ones.
//		 Sets up the game situation corresponding to the passed layer number and state.
//-----------------------------------------------------------------------------
bool perfectAI::setSituation(unsigned int threadNo, unsigned int layerNum, unsigned int stateNumber)
{
	// parameters ok ?
	if (threadNo				   >= mm.getNumThreads()) return false;
	if (getNumberOfLayers()				  <= layerNum   ) return false;
	if (getNumberOfKnotsInLayer(layerNum) <= stateNumber) return false;
	return threadVars[threadNo].setSituation(layerNum, stateNumber);
}

//-----------------------------------------------------------------------------
// Name: getOutputInformation()
// Desc: 
//-----------------------------------------------------------------------------
wstring perfectAI::getOutputInformation(unsigned int layerNum)
{
	wstringstream wss;
	wss << " white stones : " << sa.getLayer(layerNum).amountWhiteStones << "  \tblack stones  : " << sa.getLayer(layerNum).amountBlackStones;
	return wss.str();
}

//-----------------------------------------------------------------------------
// Name: printField()
// Desc: 
//-----------------------------------------------------------------------------
void perfectAI::printField(unsigned int threadNo, miniMax::twoBit value, unsigned int indentSpaces)
{
	if (threadNo >= mm.getNumThreads()) { cout << "\nERROR: invalid threadNo passed.\n"; return; }
	threadVars[threadNo].printField(value, indentSpaces);
}

//-----------------------------------------------------------------------------
// Name: getField()
// Desc: 
//-----------------------------------------------------------------------------
void perfectAI::getField(unsigned int layerNum, unsigned int stateNumber, unsigned char symOp, fieldStruct &field, bool &gameHasFinished)
{
	// set current desired state on thread zero
	setSituation(0, layerNum, stateNumber);
	applySymOp(0, symOp, true, false);

	// copy content of fieldStruct
	field = threadVars[0].getField();
	gameHasFinished = threadVars[0].getField().hasGameFinished();
}

//-----------------------------------------------------------------------------
// Name: getLayerAndStateNumber()
// Desc: 
//-----------------------------------------------------------------------------
void perfectAI::getLayerAndStateNumber(unsigned int& layerNum, unsigned int& stateNumber)
{
	unsigned int symOp;
	threadVars[0].getLayerAndStateNumber(layerNum, stateNumber, symOp);
}

//-----------------------------------------------------------------------------
// Name: getInfoAboutChoices()
// Desc: 
//-----------------------------------------------------------------------------
const miniMax::stateInfo &perfectAI::getInfoAboutChoices() const
{
    return infoAboutChoices;
}

//-----------------------------------------------------------------------------
// Name: getPartnerLayers()
// Desc: 
//-----------------------------------------------------------------------------
miniMax::gameInterface::uint_1d perfectAI::getPartnerLayers(unsigned int layerNum)
{
	if (layerNum < 100) 
		for (unsigned int i=0; i<100; i++) {
			if (sa.getLayer(layerNum).amountBlackStones == sa.getLayer(i).amountWhiteStones
			&&  sa.getLayer(layerNum).amountWhiteStones == sa.getLayer(i).amountBlackStones) {
				return {i};
			}
		}
	return {layerNum};
}

//-----------------------------------------------------------------------------
// Name: getSuccLayers()
// Desc: 
//-----------------------------------------------------------------------------
void perfectAI::getSuccLayers(unsigned int layerNum, vector<unsigned int>& succLayers)
{   
    // locals
    unsigned int i;
	unsigned int shift = (layerNum >= 100) ? 100 :  0;
			 int diff  = (layerNum >= 100) ?   1 : -1;
    
	succLayers.clear();

    // search layer with one white stone less
    for (i=0+shift; i<100+shift; i++) {
        if (sa.getLayer(i).amountWhiteStones == sa.getLayer(layerNum).amountBlackStones + diff
        &&  sa.getLayer(i).amountBlackStones == sa.getLayer(layerNum).amountWhiteStones    ) {
			succLayers.push_back(i);
            break;
        }
    }

    // search layer with one black stone less
    for (i=0+shift; i<100+shift; i++) {
        if (sa.getLayer(i).amountWhiteStones == sa.getLayer(layerNum).amountBlackStones
        &&  sa.getLayer(i).amountBlackStones == sa.getLayer(layerNum).amountWhiteStones + diff) {
			succLayers.push_back(i);
            break;
        }
    }
}

//-----------------------------------------------------------------------------
// Name: getSymStateNumWithDuplicates()
// Desc: 
//-----------------------------------------------------------------------------
void perfectAI::getSymStateNumWithDuplicates(unsigned int threadNo, vector<miniMax::stateAdressStruct>& symStates)
{
	// locals
	if (threadNo >= mm.getNumThreads()) return;
	threadVars[threadNo].getSymStateNumWithDuplicates(symStates);
}

//-----------------------------------------------------------------------------
// Name: getPredecessors()
// Desc: CAUTION: States musn't be returned twice.
//-----------------------------------------------------------------------------
void perfectAI::getPredecessors(unsigned int threadNo, vector<miniMax::retroAnalysis::predVars>& predVars)
{
	if (threadNo >= mm.getNumThreads()) return;
  	threadVars[threadNo].getPredecessors(predVars);
}

//-----------------------------------------------------------------------------
// Name: isStateIntegrityOk()
// Desc: Returns true if the field variables are consistent. 
//-----------------------------------------------------------------------------
bool perfectAI::isStateIntegrityOk(unsigned int threadNo)
{
	if (threadNo >= mm.getNumThreads()) return false;
	return threadVars[threadNo].getField().isIntegrityOk();
}

//-----------------------------------------------------------------------------
// Name: lostIfUnableToMove()
// Desc: If the current player is unable to move, the game is lost.
//-----------------------------------------------------------------------------
bool perfectAI::lostIfUnableToMove(unsigned int threadNo)
{
	return true;
}
