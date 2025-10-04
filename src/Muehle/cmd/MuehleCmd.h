/**************************************************************************************************************************
	MuehleCmd.h
 	Copyright (c) Thomas Weber. All rights reserved.				
	Licensed under the MIT License.
	https://github.com/madweasel/madweasels-cpp
***************************************************************************************************************************/
#include "../ai/perfectAI.h"
#include "../muehle.h"

class muehleCmd {
private:
    perfectAI 			myAI{L"./database/"};		// AI object

public:
    muehleCmd();							        // constructor
    ~muehleCmd();							        // destructor

    void startDatabaseCalculation();                // start database calculation 
    void calcDatabaseStatistics();                  // calculate statistics for a completed database
};
