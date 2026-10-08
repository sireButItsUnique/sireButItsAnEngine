#include "Perft.hpp"

uint64_t perft(Board* board, int depth) {
    if (depth == 0) return 1;

    uint64_t nodes = 0;
    fast::vector<uint32_t> moveList;

    MoveGen::genMoves(*board, moveList, board->turn);
    for (const uint32_t move : moveList) {
        Board newBoard = *board;
        newBoard.movePiece(move);
        if (newBoard.kingIsAttacked(!newBoard.turn)) continue; // skip illegal moves that leave king in check
        nodes += perft(&newBoard, depth - 1);
    }
    return nodes;
}