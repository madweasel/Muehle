/***************************************************************************************************************************
	minDatabaseCompTrans.cpp
 	Copyright (c) Thomas Weber. All rights reserved.				
	Licensed under the MIT License.
	https://github.com/madweasel/weaselLibrary
***************************************************************************************************************************/
#include <iostream>
#include "miniMax/src/database/dbCompTrans.h"

int main()
{
    std::cout << "Hello World!\n"; 
	std::cout << "Welcome to the database compression tool.\n";

	miniMax::dbCompTrans* transformator = new miniMax::dbCompTrans();

	// TODO: Should be a parameter or a config file. Right now it is hard-coded for simplicity.
	// transformator->compressDataBase(L"C:\\Coding\\Muehle\\database", L"D:\\Muehle\\database_compressed");
	transformator->checkIfEqual<miniMax::database::uncompFile, miniMax::database::compFile>(L"C:\\Coding\\Muehle\\database", L"D:\\Muehle\\database_compressed");
}

