/*********************************************************************\
	fieldStruct.h													  
 	Copyright (c) Thomas Weber. All rights reserved.				
	Licensed under the MIT License.
	https://github.com/madweasel/madweasels-cpp
\*********************************************************************/

#ifndef FIELD_STRUCT_H
#define FIELD_STRUCT_H

#include <iostream>
#include <cstdio>
#include <vector>
#include <cstdlib>
#include <array>

/*** Enums *********************************************************/
enum class 						playerId  : unsigned int				{squareIsFree = 0, playerOne = 1, playerTwo = 2,   playerOneWarning =  4, playerTwoWarning =  8, playerBothWarning =  12, invalid = 1000};
enum class						warningId : unsigned int 				{noWarning    = 0,                                 playerOneWarning =  4, playerTwoWarning =  8, playerBothWarning =  12, invalid = 1000};

/*** Classes *********************************************************/

// forward declarations
class fieldStruct_variables;

// class representing a move
class moveInfo
{
public:
    using                       possibilityId                   = unsigned int;	            // type representing the id of a possibility

    unsigned int                from                            = 24;		                // position of the stone which is moved
    unsigned int                to                              = 24;		                // position of the stone where it is moved to
    unsigned int                removeStone                     = 24;		                // position of the stone which is removed, otherwise the value is 'size'

                                moveInfo                        () = default;
                                moveInfo                        (unsigned int from, unsigned int to, unsigned int removeStone);

    bool                        operator==                      (const moveInfo& other) const;
    possibilityId               getId                           () const;
    void                        setId                           (possibilityId id);
    bool                        isSettingPhase                  () const;
    static const moveInfo&      getMoveInfo                     (possibilityId id);
};

// class representing a player
class playerStruct
{
public:
	playerId					id;								                            // constant over lifetime
	warningId 					warning;						                            // constant over lifetime
	unsigned int 				numStones                       = 0;						// number of stones of this player on the field
	unsigned int 				numStonesMissing                = 0;				        // number of stones, which where stolen by the opponent
    unsigned int                numStonesSet                    = 0;                        // number of stones which have been set on the field, during the setting phase
    unsigned int 				numPossibleMoves                = 24;				        // Number of possible moves (setting and moving phase); does NOT include possible stone removals.
    unsigned int                numberOfMills                   = 0;                        // number of mills belonging to this player
    bool                        hasOnlyMills                    = false;                    // true if the player has only mills and no non-mill stones on the field

    bool                        operator==                      (const playerStruct& other) const;

    // class containing the a reduced set of variables, to skip unneeded computations
    class core
    {
    public:
    	playerId				id                              = playerId::squareIsFree;	// see above
        unsigned int 			numStones                       = 0;				        // see above
        unsigned int            numStonesMissing                = 0;                        // see above

                                core                            ();
                                core                            (const playerStruct& player );
    };
};

// class containing the types and constants
class fieldStruct_types
{
public:
	// constants
    static const unsigned int   maxNumPosMoves                  =  3 * 18 * 9;				// 3 stones can be moved to 18 positions, 9 stones can be removed
    static const playerId 		playerBlack						=  playerId::playerOne;		// define player one as black (playerId::playerOne is used for black stones)
	static const playerId 		playerWhite						=  playerId::playerTwo;		// ''
	static const unsigned int	numStonesPerPlayer				=  9;						// number of stones per player
	static const int			size							= 24;						// number of squares
	static const int			gameDrawn						=  3;						// only a nonzero value

    // typedef 
    using fieldPos 				= unsigned int;                                             // type representing the position of a stone on the field
    using fieldArray            = std::array<playerId, size>;                               // type representing the field as an array of playerIds, indicating the stone on each field position
    using millArray             = std::array<unsigned int, size>;                           // type representing the mills as an array of unsigned ints, indicating the number of mills, of which this stone is part of

    // class containing the a reduced set of variables, to skip unneeded computations
    class core
    {
    public: 
        // core variables
        fieldArray              field;	                                                    // one of the values above for each field position, initialized with 'squareIsFree'
        bool		 			settingPhase                    = true;                     // true if stonesSet < 18
        playerStruct::core      curPlayer;                                                  // pointers to the current player
        playerStruct::core      oppPlayer;                              			        // pointers to the opponent player

                                core                            ();
                                core                            (const fieldStruct_variables& vars);

        const playerStruct::core&     getCurPlayer                    () const;
        const playerStruct::core&     getOppPlayer                    () const;
        playerId                getStone                        (fieldPos pos) const;
        bool                    inSettingPhase                  () const;
    };

protected:
    // An alias template for a two-dimensional std::array    
    template <typename T, std::size_t Row, std::size_t Col>
    using Array2d = std::array<std::array<T, Col>, Row>;

    // An alias template for a three-dimensional std::array
    template <typename T, std::size_t Row, std::size_t Col, std::size_t Depth>
    using Array3d = std::array<std::array<std::array<T, Depth>, Col>, Row>;
};

// additional variables describing the game state
class fieldStruct_variables : public fieldStruct_types
{
friend class fieldStruct;

public:
	// functions
    bool                        setSituation                    (const core& core);
    bool                        setSituation                    (const fieldArray& field, bool settingPhase, unsigned int totalNumStonesMissing);
	void						reset		    				(playerId firstPlayer = playerId::playerOne);
    void                        invert                          ();
	bool					    isIntegrityOk				    () const;
	void						print   						() const;

    // getter
    playerId                    getWinner                       () const;
    bool                        hasGameFinished                 () const;
    bool                        inSettingPhase                  () const;
    unsigned int                getNumStonesSet                 () const;
    const playerStruct&         getCurPlayer                    () const;
    const playerStruct&         getOppPlayer                    () const;
    const fieldArray&           getField                        () const;
    playerId                    getStone                        (fieldPos pos) const;
    unsigned int                isStonePartOfMill               (fieldPos pos) const;

protected:
   
    // more constants
	static const Array2d<fieldPos, size, 4>             connectedSquare;		        // array containg the index of the neighbour or "size"
	static const Array3d<fieldPos, size, 2, 2>	        neighbour;			            // array containing the two neighbours of each squares

    // core variables
	fieldArray                  field;	                                                // one of the values above for each field position, initialized with 'squareIsFree'
    bool		 				settingPhase                    = true;                 // true if stonesSet < 18

    // deduced variables
	millArray               	stonePartOfMill;			                            // the number of mills, of which this stone is part of
    playerStruct                curPlayer;                                              // pointers to the current player
    playerStruct                oppPlayer;                              			    // pointers to the opponent player
    bool				        gameHasFinished                 = false;				// someone has won or current field is full

    // helper functions
	char						getCharFromStone				(playerId stone) const;
	static void					setConnection					(Array2d<fieldPos, size, 4>& connectedSquare, fieldPos index, int firstDirection, int secondDirection, int thirdDirection, int fourthDirection);
	static void					setNeighbour					(Array3d<fieldPos, size, 2, 2>& neighbour, fieldPos index, fieldPos firstNeighbour0, fieldPos secondNeighbour0, fieldPos firstNeighbour1, fieldPos secondNeighbour1);
    void				        setStonePartOfMill				(fieldPos stone, fieldPos firstNeighbour, fieldPos secondNeighbour);
    void				        calcNumPossibleMoves			(playerStruct& player) const;
    void                        calcStonePartOfMill             ();
    void                        calcHasOnlyMills                ();
    void                        calcNumberOfMills               ();
    void                        calcNumStones                   ();
    void                        calcNumStonesSet                (unsigned int totalNumStonesMissing);
};

// functions related to move and undo of stones
class fieldStruct_forward : virtual public fieldStruct_variables
{
public:
    // structure to save the backup of the field
    struct backupStruct
    {
        fieldArray              field;
        millArray               stonePartOfMill;
        unsigned int		    stonesSet;
        bool				    settingPhase;
        bool				    gameHasFinished;
        unsigned int		    stoneMustBeRemoved;
        playerStruct		    curPlayer;
        playerStruct		    oppPlayer;
    };

    // move functions
    bool                        move                            (const moveInfo& move, backupStruct& oldState);
    bool                        undo                            (                const backupStruct& oldState);

    // getter
    void					    getPossibilities				(std::vector<moveInfo::possibilityId>& possibilityIds) const;

private:

    // move functions
    bool			            setStone						(const moveInfo& move, backupStruct& backup);
    bool			            normalMove						(const moveInfo& move, backupStruct& backup);
    bool			            removeStone						(const moveInfo& move, backupStruct& backup);
    
    // get possibilities
    void				        getPossSettingPhase				(std::vector<moveInfo::possibilityId>& possibilityIds) const;
    void				        getPossNormalMove				(std::vector<moveInfo::possibilityId>& possibilityIds) const;
    void				        getPossStoneRemove				(std::vector<fieldPos>& removableStones) const;
    unsigned int                wouldMillBeClosed               (fieldPos from, fieldPos to) const;
    bool                        canStoneBeRemoved               (fieldPos pos) const;

    // helper functions
    void			            updateWarning					(fieldPos firstStone, fieldPos secondStone, playerId actingPlayer);
    void			            updatePossibleMoves				(fieldPos stone, playerStruct& stoneOwner, bool stoneRemoved, fieldPos ignoreStone);
    void			            updateStonePartOfMill			(fieldPos stoneOne, fieldPos stoneTwo, fieldPos stoneThree, playerId actingPlayer);
};

// functions related to get the predecessor states
class fieldStruct_reverse : virtual public fieldStruct_variables
{
public:
    void                        getPredecessors                 (std::vector<fieldStruct_types::core>& predFields) const;

private:

    // get predecessors
    void                        getPredecessors_normalMove      (std::vector<fieldStruct_types::core>& predFields, fieldStruct_reverse& field, bool millWasClosed) const;
    void                        getPredecessors_jumpingPhase    (std::vector<fieldStruct_types::core>& predFields, fieldStruct_reverse& field, bool millWasClosed) const;
    void                        getPredecessors_settingPhase    (std::vector<fieldStruct_types::core>& predFields, fieldStruct_reverse& field, bool millWasClosed) const;
    void                        getPredecessors_stoneRemove     (std::vector<fieldStruct_types::core>& predFields, fieldStruct_reverse& field) const;

    bool                        storePredecessor                (std::vector<fieldStruct_types::core>& predFields, const fieldStruct_reverse &field) const;
    bool                        anyLonelyStone                  (const fieldStruct_reverse &field, fieldPos removedFrom) const;
};

// class representing the field. this master class is supposed to be used by the class consumers.
class fieldStruct : public fieldStruct_forward, public fieldStruct_reverse
{
friend class stateAddressing;
friend class fieldStruct_Test_test_easy_ones_Test;
friend class StateAddressingTest_totalNumMissingStones_Test;

public:

	// constructor
                                fieldStruct						();
                                fieldStruct                     (const fieldStruct& other);
                                ~fieldStruct					();

    void                        getPredecessors_2               (std::vector<fieldStruct_types::core>& predFields) const;

    // operators
    bool                        operator==                      (const fieldStruct& other) const;  
};

#endif
