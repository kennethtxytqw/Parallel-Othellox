#ifndef __UTIL_H__
#define __UTIL_H__

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stddef.h>
#include <time.h>
#include <limits.h>
#include <sys/time.h>

#define BLACK 1
#define WHITE 0
#define EMPTY 2
#define TRUE 1
#define FALSE 0
#define uchar unsigned char
#define uint unsigned int
#define delimiters " ,"

void saveRunResult(uint* bestMovesArr, int bestMovesCount, int width, int numBoards, long long comp_time, long long comm_time, long long total_time, int numP);
void readInputs(int argc, char**argv, int* widthPtr, int* heightPtr, int* maxDepthPtr, int* maxBoardsPtr, int* maxPlayerPtr, int* abPruningPtr, char* whiteStrPtr, char* blackStrPtr, int* timeoutPtr, char* TYPE);
void printMovesArr(uint * MovesArr, uint count, int width);
void fprintMovesArr(FILE * ofp, uint * MovesArr, uint count, int width);
void printBoard(uint ** boardPtr, int width, int height);
char* mystrsep(char** stringp);
uint c_to_n(char c);
uint * to2d(uint i, int width);
int to1d(int x, int y, int width);
long long wall_clock_time();
#endif