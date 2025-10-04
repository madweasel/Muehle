/*********************************************************************\
	threadSpecific.h													  
 	Copyright (c) Thomas Weber. All rights reserved.				
	Licensed under the MIT License.
	https://github.com/madweasel/muehle
\*********************************************************************/
#ifndef THREADSPECIFIC_H
#define THREADSPECIFIC_H

#include <array>

#include "../fieldStruct.h"
#include "miniMax/src/miniMax.h"
#include "stateAddressing.h"

// Variables used individually by each single thread
class threadVarsStruct
{
private:
    // backup struct, which is used to store the current state of the game field
    class stateBackup : public fieldStruct::backupStruct
    {
    public:
        miniMax::twoBit		    shortValue;
    };

    // constants
    static const unsigned int   MAX_DEPTH_OF_TREE		        = 100;

    // types
    using                       backupArray                     = std::array<stateBackup, MAX_DEPTH_OF_TREE + 1>;

    // variables
    char						padding[64];											            // Cache line padding to prevent false sharing in multithreaded contexts
    stateAddressing&	        sa;	                                                                // reference to the state addressing
    backupArray                 oldStates;						                                    // for undo()-function	
    vector<fieldStruct::core>    predFields;                                                         // buffer for storing predecessors states
    fieldStruct			        field;							                                    // current game field [changed by move()]
    unsigned int		        curSearchDepth                  = 0;	                            // current level
    miniMax::twoBit		        shortValue                      = miniMax::SKV_VALUE_INVALID;		// value of the current situation
    
    // database functions   
    bool				        storePredecessor				(const fieldStruct::core& predField, vector<miniMax::retroAnalysis::predVars>& predVars) const;

public: 
    // constructor
                                threadVarsStruct				(stateAddressing& sa);
                                ~threadVarsStruct				();
    void                        reset		        			();  
    
    // Assignment operator intentionally disabled to prevent copying of thread-specific resources.
    threadVarsStruct&           operator=                       (const threadVarsStruct& other) = delete;

    // getter (from miniMax::gameInterface)
    void					    getPossibilities				(vector<unsigned int>& possibilityIds) const;
	miniMax::twoBit 		    getValueOfSituation				() const;
	void					    getLayerAndStateNumber			(unsigned int &layerNum, unsigned int &stateNumber, stateAddressing::symOperationId& symOp) const;
	unsigned int			    getLayerNumber					() const;
	void					    getSymStateNumWithDuplicates	(vector<miniMax::stateAdressStruct>& symStates) const;
    void					    getPredecessors             	(vector<miniMax::retroAnalysis::predVars>& predVars);
    const fieldStruct&		    getField						() const;

	// setter (from miniMax::gameInterface)
	void					    applySymOp						(stateAddressing::symOperationId symmetryOperationNumber, bool doInverseOperation, bool playerToMoveChanged);
	bool					    setSituation					(unsigned int layerNum, unsigned int stateNumber);
    void                        setField                        (const fieldStruct& field);
	void					    move							(unsigned int idPossibility, void* &pBackup);
	void					    undo							(unsigned int idPossibility, void*  pBackup);
    
	// output (from miniMax::gameInterface)  
	void					    printField						(miniMax::twoBit value, unsigned int indentSpaces) const;
	void					    printMoveInformation			(unsigned int idPossibility) const;
};

#endif