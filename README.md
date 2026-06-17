# sireButItsAnEngine
A C++ UCI chess engine

## Inspiration
An attempt to build a semi-strong chess engine, first inspired many years ago by Sebastian Lague's chess adventures

It's currently around ~2700 elo as of first release, so there are quite a lot more improvements to be added, but it should still beat virtually all human players.

## Architecture
#### Representing the Board
[Bitboards!](https://healeycodes.com/visualizing-chess-bitboards)

#### Move Generation
Move generation is initialized once at startup to build lookup tables. Moves are encoded as 32 bit integers the relevant color, squares, and flags for special moves (promotion, en passant, castling).

The engine utilizes PEXT instructions for `O(1)` move lookup. Note this means the engine will only run on x86-64

###### Sliding Pieces (Rook, Bishop, Queen)
Precomputation:
- Ray attacks for all 8 directions are calculated for every square
- All possible blocker configurations are generated using _pdep_u64
- Results are stored in flat arrays: rookLookup (102,400 entries) and bishopLookup (5,248 entries).

Runtime:
- The board's current occupancy is masked with the position of the moving piece and compressed into an index using _pext_u64.
- This index can be directly used to lookup the resulting possible moves

###### Knights and Kings
Knight and king attacks are precomputed into simple 64-entry lookup tables at init since they are not effected by "blockers"

###### Pawns
Pawn move generation was just edge case simulator...

#### Searching
The engine uses [alpha beta search](https://en.wikipedia.org/wiki/Alpha%E2%80%93beta_pruning) with the following optimizations

###### Iterative Deepening
Always searches from depth 1 up to the maximum depth, gaining useful information from shallower results to speed up later searches. This also means a search can be aborted at any time and still obtain a useful result, rather than being overly ambitious with an unreachable depth

###### Transposition Table
Uses [Zobrist hashing](https://www.chessprogramming.org/Zobrist_Hashing) to cache evaluations. Provides early cutoffs for positions already evaluated previously

###### Move Ordering
Ordering moves best to worst when exploring new positions allows for more alpha beta cutoffs. The folllowing heurestics are used to estimate the "goodness" of a move
- MVV-LVA
- Killer Heuristic
- History Heuristic

###### Pruning
Pruning is possibly lossy (i.e. we might discard the best move) However, the reduced amount of positions to search is worth it:
- Reverse Futility Pruning
- Null Move Pruning

###### Extensions/Reductions
Some lines are worth more attention than others. We dont't have to search every single move at the same depth:
- Singular Extensions
- Late Move Reductions

#### Evalutation
Uses an efficient neural network (NNUE) with architecture (768->32)x2->1 to evaluate any given position. Trained by @kevlu8 on self-play data generated from his engine [PZChessBot](https://github.com/kevlu8/PZChessBot).

To make evaluation faster, we use a system called "PZUE" (coined in [PZChessBot](https://github.com/kevlu8/PZChessBot)) where instead of incrementally tracking accumulators after each move, we cache the board and accumulator state at each evaluation call and perform updates accordingly. Although this method is slightly slower compared to SOTA UE (~10%), it is remarkably easier to implement.

## Usage
#### Installation
For the latest build, clone the repo:
```
git clone https://github.com/sireButItsUnique/sireButItsAnEngine
```
Enter the directory:
```
cd sireButItsAnEngine
```
Build the engine:
```
make -j
```
Run the engine:
```
./sireButItsAnEngine
```

#### UCI
In the terminal, most of the crucial [uci](https://publish.obsidian.md/modern-uci-doc/UCI+Docs/Intro) commands have been implemented. The most used commands are explained below

#### Printing
```
d
```
*Prints the current board state*

#### Position
```
position startpos
```
*Sets the board to the starting position*

```
position fen <fen>
```
*Sets the board to the given [fen](https://www.chess.com/terms/fen-chess) position*

```
position [...] moves <move1> <move2> <...>
```
*Sets the board to the position given, and plays the moves given on the board.*

#### Moves
Moves are represented with a notation that indicates the starting square and the ending square cosecutively 
- `e4` from the starting position would be represented as `e2e4` 
- Castling is represented by the movement of the king (i.e. `O-O` becomes `e1g1`)
- If a promotion is played, include an extra letter to indicate promotion peice (i.e. `e8=Q` becomes `e7e8q`)

#### Go
```
go
```
*Searches for the best move at iteratively increasing depths until the default thinking time is up. Higher depth usually means a more accurate evaluation of the position and thus better moves.*

```
go wtime <wtime> btime <btime>
```
*Searches for the best move, until its thinking time expires. Its thinking time is allocated appropriately according to the given time constraints*

```
go depth <depth>
```
*Searches for the best move at a fixed given depth. Note this might mean a slower search for that depth as we do not have information obtained about the position from searching at lower depths*

#### Example Run
```
./sireButItsAnEngine
info string Initiated move generator in 0.001489 secs
position fen 1q5r/p1k1pbnp/1p3p2/2p5/3NP3/5P2/PP3B1P/2Q2R1K w - - 0 1
d
+---+---+---+---+---+---+---+---+
|   | q |   |   |   |   |   | r | 8
+---+---+---+---+---+---+---+---+
| p |   | k |   | p | b | n | p | 7
+---+---+---+---+---+---+---+---+
|   | p |   |   |   | p |   |   | 6
+---+---+---+---+---+---+---+---+
|   |   | p |   |   |   |   |   | 5
+---+---+---+---+---+---+---+---+
|   |   |   | N | P |   |   |   | 4
+---+---+---+---+---+---+---+---+
|   |   |   |   |   | P |   |   | 3
+---+---+---+---+---+---+---+---+
| P | P |   |   |   | B |   | P | 2
+---+---+---+---+---+---+---+---+
|   |   | Q |   |   | R |   | K | 1
+---+---+---+---+---+---+---+---+
  a   b   c   d   e   f   g   h
Turn: White
Key: afed3375aa465587
White pieces: 404792228
Black pieces: 9436485994799955968
Moves: a2a3 a2a4 b2b3 b2b4 h2h3 h2h4 f3f4 e4e5 d4c2 d4e2 d4b3 d4b5 d4f5 d4c6 d4e6 f2e1 f2g1 f2e3 f2g3 f2h4 f1d1 f1e1 f1g1 c1a1 c1b1 c1d1 c1e1 c1c2 c1d2 c1c3 c1e3 c1c4 c1f4 c1c5 c1g5 c1h6 h1g1 h1g2
Static: -163
go
info depth 1 score cp -76 nodes 50 time 0 nps 827814 pv d4b5
info depth 2 score cp -95 nodes 637 time 0 nps 1219605 pv b2b4
info depth 3 score cp -17 nodes 1519 time 1 nps 1435863 pv b2b4
info depth 4 score cp -40 nodes 3236 time 1 nps 2119187 pv b2b4
info depth 5 score cp 57 nodes 9516 time 2 nps 3374109 pv b2b4
info depth 6 score cp 137 nodes 23738 time 6 nps 3964195 pv b2b4
info depth 7 score cp 206 nodes 40367 time 11 nps 3651172 pv b2b4
info depth 8 score cp 477 nodes 84638 time 23 nps 3622442 pv d4e6
info depth 9 score cp 477 nodes 88490 time 25 nps 3563532 pv d4e6
info depth 10 score cp 353 nodes 121589 time 32 nps 3739554 pv d4e6
info depth 11 score cp 339 nodes 181949 time 46 nps 3924672 pv d4e6
info depth 12 score cp 461 nodes 349529 time 81 nps 4321762 pv d4e6
info depth 13 score cp 460 nodes 584509 time 130 nps 4506529 pv d4e6
info depth 14 score cp 426 nodes 1001649 time 211 nps 4740546 pv d4e6
info depth 15 score cp 436 nodes 1752064 time 349 nps 5023293 pv d4e6
bestmove d4e6
position fen 1q5r/p1k1pbnp/1p3p2/2p5/3NP3/5P2/PP3B1P/2Q2R1K w - - 0 1 moves d4e6
go
info depth 1 score cp 1640 nodes 14 time 0 nps 864197 pv g7e6
info depth 2 score cp 1476 nodes 66 time 0 nps 183741 pv g7e6
info depth 3 score cp 192 nodes 152 time 1 nps 201832 pv g7e6
info depth 4 score cp 192 nodes 226 time 1 nps 188051 pv g7e6
info depth 5 score cp -327 nodes 456 time 1 nps 278899 pv g7e6
info depth 6 score cp -327 nodes 712 time 2 nps 347537 pv g7e6
info depth 7 score cp -477 nodes 1397 time 2 nps 531906 pv g7e6
info depth 8 score cp -374 nodes 3184 time 3 nps 939122 pv g7e6
info depth 9 score cp -435 nodes 9469 time 5 nps 1673411 pv g7e6
info depth 10 score cp -430 nodes 22601 time 10 nps 2190146 pv g7e6
info depth 11 score cp -436 nodes 24856 time 11 nps 2192003 pv g7e6
info depth 12 score cp -436 nodes 55322 time 19 nps 2919690 pv g7e6
info depth 13 score cp -430 nodes 122215 time 33 nps 3689320 pv g7e6
info depth 14 score cp -436 nodes 170428 time 44 nps 3815054 pv g7e6
info depth 15 score cp -443 nodes 756563 time 149 nps 5076244 pv g7e6
bestmove g7e6
```