/*********************************************************************\
	minMaxAI.h													  
 	Copyright (c) Thomas Weber. All rights reserved.				
	Licensed under the MIT License.
	https://github.com/madweasel/madweasels-cpp
\*********************************************************************/
#ifndef MINIMAXAI_H
#define MINIMAXAI_H

#include <cstdio>

#include <vector>
#include "../muehle.h"
#include "../fieldStruct.h"
#include "miniMax/src/miniMax.h"

/*** Klassen *********************************************************/
/**
 * @brief AI player using the Minimax algorithm for the Muehle game.
 * 
 * This class implements an AI player that utilizes the Minimax algorithm
 * to determine optimal moves in the Muehle game. It interfaces with the
 * miniMax library and provides methods for move calculation, evaluation,
 * and game state management.
 */
class minMaxAI : public muehleAI, miniMax::gameInterface
{
protected: 

	using warningArray            = std::array<warningId, fieldStruct::size>;

	// Classes
	class fieldClass : public fieldStruct
	{
	public:
		warningArray 			warnings;				// array containing the warnings for each field position

								fieldClass				();
								fieldClass				(const fieldStruct& theField);

	private:
		void 					setWarningAndMill		(unsigned int stone, unsigned int firstNeighbour, unsigned int secondNeighbour);
		static warningId 		addWarning				(warningId existingWarning, warningId newWarning);
	};

	struct backupStruct : public fieldStruct::backupStruct
	{
		float			value;
		warningArray	warnings;
	};

	struct threadVarsStruct
	{
		fieldClass 					field;										// pointer of the current field [changed by move()]
		float						currentValue		= 0;					// value of current situation for field->currentPlayer
		unsigned int				curSearchDepth		= 0;					// current level
		std::vector<backupStruct>	oldStates;									// for undo()-function	
	};

	// Variables
	// 'mm' is the minimax algorithm instance; 'this' passes the current AI as the game interface, and '100' sets the maximum search depth (chosen as a safe upper bound for practical search limits).
	miniMax::miniMax				mm					{this, 100};			// minimax algorithmn
	unsigned int					depthOfFullTree		= 0;					// search depth where the whole tree is explored
	std::vector<threadVarsStruct>	threadVars;									// information for each thread

	// init
	void				prepareCalculation				()																													override;	

	// getter
	void				getPossibilities				(unsigned int threadNo, std::vector<unsigned int>& possibilityIds)													override;	
	unsigned int		getMaxNumPossibilities			()																													override;
	void				getValueOfSituation				(unsigned int threadNo, float& floatValue, miniMax::twoBit& shortValue)												override;	

	// setter
	void				move							(unsigned int threadNo, unsigned int idPossibility, bool& playerToMoveChanged, void* &pBackup)						override;
	void				undo							(unsigned int threadNo, unsigned int idPossibility, bool& playerToMoveChanged, void*  pBackup)						override;
	
	// output
	void				printField						(unsigned int threadNo, miniMax::twoBit value, unsigned int indentSpaces)											override {};
	void				printMoveInformation			(unsigned int threadNo, unsigned int idPossibility)																	override;
	wstring				getOutputInformation			(unsigned int layerNum)																								override { return wstring(L""); };

public:
    // Constructor / destructor
						minMaxAI						();
						~minMaxAI						();

	// Functions
	void				play							(const fieldStruct& theField, moveInfo& move) override;
	void				setSearchDepth					(unsigned int depth);
};

#endif