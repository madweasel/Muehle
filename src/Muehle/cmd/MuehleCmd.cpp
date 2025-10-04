/**************************************************************************************************************************
	MuehleCmd.cpp
 	Copyright (c) Thomas Weber. All rights reserved.				
	Licensed under the MIT License.
	https://github.com/madweasel/madweasels-cpp
***************************************************************************************************************************/
#include "MuehleCmd.h"

//-----------------------------------------------------------------------------
// Name: main()
// Desc: 
//-----------------------------------------------------------------------------
int main(int argc, char **argv) 
{
    // locals
    muehleCmd muehle{};

    // start database calculation
    muehle.startDatabaseCalculation();
	muehle.calcDatabaseStatistics();

	return 0;
}

//-----------------------------------------------------------------------------
// Name: startDatabaseCalculation()
// Desc: start the database calculation
//-----------------------------------------------------------------------------
void muehleCmd::startDatabaseCalculation() 
{
	myAI.mm.openDatabase(L".\\database", false);
	myAI.mm.calculateDatabase();
	myAI.mm.closeDatabase();
}

//-----------------------------------------------------------------------------
// Name: calcDatabaseStatistics()
// Desc: calculate statistics for the database
//-----------------------------------------------------------------------------
void muehleCmd::calcDatabaseStatistics() 
{
	myAI.mm.openDatabase(L".\\database", false);
	myAI.mm.calculateStatistics();
	myAI.mm.closeDatabase();
}

//-----------------------------------------------------------------------------
// Name: muehleCmd()
// Desc: constructor
//-----------------------------------------------------------------------------
muehleCmd::muehleCmd() 
{
}

//-----------------------------------------------------------------------------
// Name: ~muehleCmd()
// Desc: destructor
//-----------------------------------------------------------------------------
muehleCmd::~muehleCmd() 
{
}
