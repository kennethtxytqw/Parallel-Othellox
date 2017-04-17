#ifndef __BOARD_H__
#define __BOARD_H__
#include "util.h"

uint ** initBoard(int width, int height, int player, char * whiteStrPtr, char * blackStrPtr);
uint** createBoard(int width, int height);
int countFlips(int x, int y, uint ** boardPtr, int player, int width, int height);
void placePiece(uint ** boardPtr, uint player, int x, int y, int width, int height);
uint ** makeMove(uint ** boardPtr, uint nextMove, uint player, int width, int height);
uint ** duplicateBoard(uint ** boardPtr, int width, int height);
void freeBoard(uint ** boardPtr);
int availableMoves(uint ** boardPtr, int * movesPtr, int player, int width, int height);
#endif