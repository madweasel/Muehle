/*********************************************************************\
	perfectAI.h													  
 	Copyright (c) Thomas Weber. All rights reserved.				
	Licensed under the MIT License.
	https://github.com/madweasel/muehle
\*********************************************************************/
#ifndef PERFECT_AI_H
#define PERFECT_AI_H

#include <cstdio>

#include "../muehle.h"
#include "../fieldStruct.h"
#include "miniMax/src/miniMax.h"
#include "stateAddressing.h"
#include "threadSpecific.h"

/*** Klassen *********************************************************/
class perfectAI : public muehleAI, public miniMax::gameInterface
{
private:
	// members
	std::wstring				databaseDirectory;																						// directory containing the database files
	miniMax::stateInfo			infoAboutChoices;																						// contains the value of the situation, which will be achieved by that move
	stateAddressing				sa;																										// addressing each game situation is not trivial, thus it is done by this class
	vector<threadVarsStruct>	threadVars;																								// Variables used individually by each single thread

	// functions
	wstring 					calcDatabaseDirectory			(wstring const &directory);

public:	
	miniMax::miniMax			mm								{this, 100};

	// init
	void						prepareCalculation				()																													override;	

	// getter 
	unsigned int				getMaxNumPossibilities			()																													override;
	unsigned int				getNumberOfLayers				()																													override;	
	unsigned int 				getMaxNumPlies					()																													override;
    bool						shallRetroAnalysisBeUsed    	(unsigned int layerNum)																								override;
	unsigned int				getNumberOfKnotsInLayer			(unsigned int layerNum)																								override;
    void						getSuccLayers               	(unsigned int layerNum, vector<unsigned int>& succLayers)															override;
	uint_1d						getPartnerLayers				(unsigned int layerNum)																								override;

	// getter (thread specific)
	void						getPossibilities				(unsigned int threadNo, vector<unsigned int>& possibilityIds)														override;	
	void						getValueOfSituation				(unsigned int threadNo, float& floatValue, miniMax::twoBit& shortValue)												override;	
	void						getLayerAndStateNumber			(unsigned int threadNo, unsigned int &layerNum, unsigned int &stateNumber, unsigned int&symOp)						override;	
	unsigned int				getLayerNumber					(unsigned int threadNo)																								override;
	void						getSymStateNumWithDuplicates	(unsigned int threadNo, vector<miniMax::stateAdressStruct>& symStates)												override;
    void						getPredecessors             	(unsigned int threadNo, vector<miniMax::retroAnalysis::predVars>& predVars)											override;
	bool						isStateIntegrityOk				(unsigned int threadNo)																								override;
	bool						lostIfUnableToMove				(unsigned int threadNo)																								override;

	// setter
	void						applySymOp						(unsigned int threadNo, unsigned char symmetryOperationNumber, bool doInverseOperation, bool playerToMoveChanged)	override;
	bool						setSituation					(unsigned int threadNo, unsigned int layerNum, unsigned int stateNumber)											override;
	void						move							(unsigned int threadNo, unsigned int idPossibility, bool& playerToMoveChanged, void* &pBackup)						override;
	void						undo							(unsigned int threadNo, unsigned int idPossibility, bool& playerToMoveChanged, void*  pBackup)						override;
	
	// output
	void						printField						(unsigned int threadNo, miniMax::twoBit value, unsigned int indentSpaces)											override;
	void						printMoveInformation			(unsigned int threadNo, unsigned int idPossibility)																	override;
	wstring						getOutputInformation			(unsigned int layerNum)																								override;

    // Constructor / destructor
								perfectAI						(wstring const& directory);
								~perfectAI						();

	// Functions for using the AI with calculated database
	void						play							(const fieldStruct& theField, moveInfo& move) 																		override;
	void						getField						(unsigned int  layerNum, unsigned int  stateNumber, unsigned char symOp, fieldStruct &field, bool &gameHasFinished);
	void						getLayerAndStateNumber			(unsigned int& layerNum, unsigned int& stateNumber);
	const miniMax::stateInfo&	getInfoAboutChoices				() const;
};

#endif