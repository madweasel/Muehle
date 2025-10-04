# The Game

## Phases

The game is split into two phases: the setting phase and the moving phase.

## Setting Phase

During the setting phase, players take turns placing their stones on empty squares of the board. Each player has 9 stones to place. If a player forms a mill (three of their stones in a row), they can remove one of the opponent's stones from the board, provided that stone is not part of a mill. If all opponent's stones are in mills, any stone can be removed. The setting phase continues until all 18 stones are placed on the board.

### Moving Phase

During the moving phase, players take turns moving their stones to **adjacent** empty squares. If a player forms a mill during this phase, they can remove one of the opponent's stones from the board, following the same rules as in the setting phase. The moving phase continues until one player has lost all their stones or can no longer make a legal move.
If a player has only 3 stones left, they can move to any empty square on the board, not just adjacent ones.

## Variables

Let's use $T$ for total, $W$ for white, and $B$ for black.
### Variables

| Symbol | Description                                                                                             |
|--------|---------------------------------------------------------------------------------------------------------|
| $f_T$, $f_W$, $f_B$ | Number of total/white/black stones currently present on the field                          |
| $m_T$, $m_W$, $m_B$ | Number of total/white/black stones missing (removed from the field due to a closed mill)   |
| $s_T$, $s_W$, $s_B$ | Number of total/white/black stones set during the setting phase (equals 9 in moving phase) |


## Equations

The maximum number of stones missing, depends on the number of stones present on the field:

$0 <= m_T < 18 - f_T$

The total number of stones is conserved and always the sum of the black and white stones:

$x_T = x_W + x_B$

For each color, the number of set stones is equal to the number of stones currently present on the field plus the number of stones missing:

$s_X = f_X + m_X$

Since the white player always moves first, we can use this information to our advantage and map this to a certain number of white and black stones set.

| $s_T$ | $s_W$ | $s_B$ | turn |
|-------|-------|-------|------|
| 0     | 0     | 0     | $W$  |
| 1     | 0     | 1     | $B$  |
| 2     | 1     | 1     | $W$  |
| 3     | 1     | 2     | $B$  |
| 4     | 2     | 2     | $W$  |
| ...   | ...   | ...   | ...  |
| 18    | 9     | 9     | $B$  |

## The game state and addressing

The class [`fieldStruct`](./fieldStruct.h) is used to represent the current state of the game board, including the positions of all stones.
The main variables are stored in `fieldStruct_types::core`, being the stones on the field, a boolean indicating if the game is in the setting phase or moving phase, and the number of missing stones for each player.

The class `stateAddressing` is used to map the current state of the game to a unique identifier.

The function [`stateAddressing::getStateNumber()`](./ai/stateAddressing.h) takes the current game state as input and returns a unique identifier for that state. The reverse function is `stateAddressing::getFieldByStateNumber()`, which takes a state identifier and returns the corresponding game state.
