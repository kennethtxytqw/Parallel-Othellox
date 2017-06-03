#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stddef.h>
#include <time.h>
#include <limits.h>


#define BLACK 1
#define WHITE 0
#define EMPTY 2
#define TRUE 1
#define FALSE 0
#define uchar unsigned char
#define uint unsigned int
#define delimiters " ,"

const uint UP[2] = { 0,1 };
const uint DOWN[2] = { 0,-1 };
const uint LEFT[2] = { -1,0 };
const uint RIGHT[2] = { 1,0 };
const uint UPRIGHT[2] = { 1,1 };
const uint UPLEFT[2] = { -1,1 };
const uint DOWNLEFT[2] = { -1,-1 };
const uint DOWNRIGHT[2] = { 1,-1 };

clock_t before, after;
const uchar alphabets[26] = { 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z' };

const uint DIRECTIONS[8][2] = {
	{ 0,1 },
	{ 0,-1 },
	{ -1,0 },
	{ 1,0 },
	{ 1,1 },
	{ -1,1 },
	{ -1,-1 },
	{ 1,-1 }
};

const char DELIM[] = delimiters;
long int comm_time = 0;
long int comp_time = 0;
long int numBoards = 0;
long int maxBoards;
int ALPHA_BETA_PRUNING;


FILE *brdtxt, *evaltxt, *ofp;
char* whiteInitialPos;
char* blackInitialPos;
char whiteStr[26 * 26 * 3];
char blackStr[26 * 26 * 3];
char maxPlayerStr[6];
char* maxPlayerStrPtr = maxPlayerStr;
char* whiteStrPtr = whiteStr;
char* blackStrPtr = blackStr;

int width;
int height;
int maxDepth;
int maxPlayer;

void saveRunResult(uint * bestMovesArr, int bestMovesCount);
uint c_to_n(char c);
uint * to2d(uint i, int width);
int to1d(int x, int y, int width);
char * mystrsep(char ** stringp, const char * delim);
int availableMoves(uint ** boardPtr, int * movesPtr, int player);
int countFlips(int x, int y, uint ** boardPtr, int player);
uint ** createStandardBoard();
uint ** createBoard(int width, int height, int player, char * whiteStrPtr, char * blackStrPtr);
int playMax(uint** boardPtr, int ply, int* bestMovesCount, uint* bestMovesArr, int alpha, int beta);
int playMin(uint** boardPtr, int ply, int* bestMovesCount, uint* bestMovesArr, int alpha, int beta);
int evaluate(uint ** boardPtr);
uint ** makeMove(uint ** boardPtr, uint nextMove, uint player);
void placePiece(uint ** boardPtr, uint player, int x, int y);
uint ** duplicateBoard(uint ** boardPtr);
void freeBoard(uint ** boardPtr);
void printMovesArr(uint * MovesArr, uint count, int width);
void fprintMovesArr(FILE * ofp, uint * MovesArr, uint count, int width);
void readInputs(int argc, char ** argv);
void printBoard(uint ** boardPtr);

uint c_to_n(char c)
{
	uint n = -1;
	static const char * const alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	char *p = strchr(alphabet, toupper((unsigned char)c));

	if (p)
	{
		n = p - alphabet;
	}

	return n;
}

uint* to2d(uint i, int width) {
	uint* iarr = malloc(sizeof(uint) * 2);
	uint x = i%width;
	uint y = i / width;
	iarr[0] = x;
	iarr[1] = y;
	return iarr;
}

int to1d(int x, int y, int width) {
	return y*width + x;
}

char* mystrsep(char** stringp, const char* delim)
{
	char* start = *stringp;
	char* p;
	p = (start != NULL) ? strpbrk(start, delim) : NULL;
	if (p == NULL)
	{
		*stringp = NULL;
	}
	else
	{
		*p = '\0';
		*stringp = p + 1;
	}
	/*if (p == "") {
		return mystrsep(stringp, delim);
	}*/
	return start;
}

int availableMoves(uint** boardPtr, int* movesPtr, int player) {
	int i = 0;
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			if (countFlips(x, y, boardPtr, player) > 0) {
				movesPtr[i] = to1d(x, y, width);
				i++;
				//printf("%c%d is a valid move\n", alphabets[x],y);
			}
		}
	}
	return i;
}

int countFlips(int x, int y, uint** boardPtr, int player) {
	// if the box is not EMPTY then return 0 flips which is an invalid move
	if (boardPtr[x][y] != EMPTY) {
		return 0;
	}
	int flips = 0;
	int counter = 0;
	int temp_x;
	int temp_y;

	for (int dir = 0; dir < 8; dir++) {
		temp_x = x +DIRECTIONS[dir][0];
		temp_y = y +DIRECTIONS[dir][1];

		while (temp_x < width && temp_x >= 0 && temp_y < height && temp_y >= 0) {

			if (boardPtr[temp_x][temp_y] == player) {
				flips += counter;
				counter = 0;
				break;
			}
			if (boardPtr[temp_x][temp_y] == EMPTY) {
				counter = 0;
				break;
			}
			counter++;
			temp_x +=DIRECTIONS[dir][0];
			temp_y +=DIRECTIONS[dir][1];
		}
		counter = 0;
	}

	return flips;
}

/**
Creates board using the user defined starting positions. Take note that in this code, we start counting from 0 hence position A1 in the real world is A0 in these code, hence token[1] - '1'.
**/
uint** createBoard(int width, int height, int player, char* whiteStrPtr, char* blackStrPtr) {
	//fprintf(stderr,"Starting boardPtr creation\n");
	char* token;
	//This way i make sure i create my board such that the data is contiguous
	uint** boardPtr = malloc(sizeof(uint*)*width);
	uint* board = malloc(sizeof(uint)*height*width);
	for (int i = 0; i < width; i++) {
		boardPtr[i] = &board[i*height];
	}
	for (int i = 0; i < width; i++) {
		for (int j = 0; j < height; j++) {
			boardPtr[i][j] = EMPTY;
		}
	}
	while ((token = mystrsep(&whiteStrPtr, DELIM)) != NULL) {
		//fprintf(stderr,"Placing white token at %s\n", token);
		fprintf(stderr,"Placing white token at %d-%d\n", c_to_n(token[0]),token[1]-'1');
		boardPtr[c_to_n(token[0])][token[1] - '1'] = WHITE;
	}
	while ((token = mystrsep(&blackStrPtr, DELIM)) != NULL) {
		//fprintf(stderr,"Placing black token at %s\n", token);
		fprintf(stderr,"Placing black token at %d-%d\n", c_to_n(token[0]), token[1]-'1');
		boardPtr[c_to_n(token[0])][token[1] - '1'] = BLACK;
	}
	return boardPtr;
}

int playMax(uint** boardPtr, int ply, int* bestMovesCountPtr, uint* bestMovesArr, int alpha, int beta) {
	if (ply == 0 || numBoards >= maxBoards) {
		return evaluate(boardPtr);
	}
	else {
		int value = INT_MIN;
		uint* movesArr = malloc(height*width * sizeof(uint));
		int numMoves = availableMoves(boardPtr, movesArr, maxPlayer);
		if (numMoves == 0) {
			free(movesArr);
			return playMin(boardPtr, ply - 1, bestMovesCountPtr, bestMovesArr, alpha,beta);
		}
		int temp;
		int bestMovesCount;
		for (int i = 0; i < numMoves; i++) {
			uint** newBoard = makeMove(boardPtr, movesArr[i], maxPlayer);
			temp = playMin(newBoard, ply - 1, bestMovesCountPtr, bestMovesArr, alpha,beta);
			if (temp > value) {
				value = temp;
				bestMovesCount = 0;
			}
			if (temp >= value && ply == maxDepth) {
				bestMovesArr[bestMovesCount] = movesArr[i];
				bestMovesCount++;
				//fprintf(stderr,"Move %d has %d\n", movesArr[i], temp);
			}
			//fprintf(stderr, "BMC: %d\n", bestMovesCount);
			*bestMovesCountPtr = bestMovesCount;
			freeBoard(newBoard);
			if (ALPHA_BETA_PRUNING) {
				if (value >= beta) {
					free(movesArr);
					return value;
				}
				else {
					alpha = value > alpha ? value : alpha;
				}
			}
		}
		//printf("Done Max\n");
		free(movesArr);
		return value;
	}
}

int playMin(uint** boardPtr, int ply, int* bestMovesCountPtr, uint* bestMovesArr, int alpha, int beta) {
	if (ply == 0 || numBoards >= maxBoards) {
		return evaluate(boardPtr);
	}
	else {
		int value = INT_MAX;
		uint* movesArr = malloc(height*width * sizeof(uint));
		int numMoves = availableMoves(boardPtr, movesArr, !maxPlayer);
		if (numMoves == 0) {
			free(movesArr);
			return playMax(boardPtr, ply - 1, bestMovesCountPtr, bestMovesArr, alpha,beta);
		}
		int temp;
		for (int i = 0; i < numMoves; i++) {
			uint** newBoard = makeMove(boardPtr, movesArr[i], !maxPlayer);
			temp = playMax(newBoard, ply - 1, bestMovesCountPtr, bestMovesArr, alpha,beta);
			value = temp < value ? temp : value;
			freeBoard(newBoard);
			if (ALPHA_BETA_PRUNING) {
				if (value <= alpha) {
					free(movesArr);
					return value;
				}
				else {
					beta = value < beta ? value : beta;
				}
			}
			
		}
		//printf("Done Min\n");
		free(movesArr);
		return value;
	}
}

int evaluate(uint** boardPtr) {
	int score = 0;
	for (int i = 0; i < width; i++) {
		for (int j = 0; j < height; j++) {
			if (boardPtr[i][j] == maxPlayer) {
				score++;
			}
			//printf("%d-%d is %d\n", i, j, gptr->boardPtr[i][j]);
		}
	}

	return score;
}

uint** makeMove(uint** boardPtr, uint nextMove, uint player) {
	uint* move = to2d(nextMove, width);
	uint x = move[0];
	uint y = move[1];
	uint** newBoardPtr = duplicateBoard(boardPtr);

	placePiece(newBoardPtr, player, x, y);
	numBoards++;
	free(move);
	printBoard(boardPtr);
	fprintf(stderr, "Becomes this by making move %u-%u\n", to2d(nextMove, width)[0], to2d(nextMove, width)[1]);
	printBoard(newBoardPtr);
	return newBoardPtr;
}

void placePiece(uint** boardPtr, uint player, int x, int y) {
	int flipsMade = 0;
	int counter = 0;
	int temp_x = x;
	int temp_y = y;
	for (int dir = 0; dir < 8; dir++) {
		temp_x = x + DIRECTIONS[dir][0];
		temp_y = y + DIRECTIONS[dir][1];
		// fprintf(stderr, "x is %d and y is %d\n",DIRECTIONS[dir][0],DIRECTIONS[dir][1]);
		// fprintf(stderr, "temp_x  is %d and temp_y is %d\n",temp_x ,temp_y);
		while (temp_x < width && temp_x >= 0 && temp_y < height && temp_y >= 0) {
			if (boardPtr[temp_x][temp_y] == player) {
				flipsMade += counter;
				for (int flipsMadeForThisDir = 0; flipsMadeForThisDir <= counter+1; flipsMadeForThisDir++) {
					boardPtr[temp_x][temp_y] = player;
					temp_x -=DIRECTIONS[dir][0];
					temp_y -=DIRECTIONS[dir][1];
				}
				counter = 0;
				break;
			}
			if (boardPtr[temp_x][temp_y] == EMPTY) {
				counter = 0;
				break;
			}
			counter++;
			temp_x +=DIRECTIONS[dir][0];
			temp_y +=DIRECTIONS[dir][1];
		}
		counter = 0;
	}
}

uint** duplicateBoard(uint** boardPtr) {
	uint** newboardPtr = malloc(sizeof(uint*)*width);
	uint* newboard = malloc(sizeof(uint)*height*width);
	for (int i = 0; i < width; i++) {
		newboardPtr[i] = &newboard[i*height];
	}
	for (int i = 0; i < width; i++) {
		for (int j = 0; j < height; j++) {
			newboardPtr[i][j] = boardPtr[i][j];
		}
	}
	return newboardPtr;
}

void freeBoard(uint** boardPtr) {
	free(boardPtr[0]);
	free(boardPtr);
}

void printMovesArr(uint* MovesArr, uint count, int width) {
	fprintf(stdout,"Best Moves: { ");
	for (int i = 0; i < count; i++) {
		if (i > 0) {
			fprintf(stdout,", ");
		}
		uint* move = to2d(MovesArr[i], width);
		// move[1] + 1 because in the real world the board start from 1 and not 0
		fprintf(stdout,"%c%d", alphabets[move[0]], 1+move[1]);
	}
	fprintf(stdout," }\n");
}

void fprintMovesArr(FILE* ofp, uint* MovesArr, uint count, int width) {
	fprintf(ofp, "Best Moves: { ");
	for (int i = 0; i < count; i++) {
		if (i > 0) {
			fprintf(ofp, ", ");
		}
		uint* move = to2d(MovesArr[i], width);
		fprintf(ofp, "%c%d", alphabets[move[0]], move[1]+1);
	}
	fprintf(ofp, " }\n");
}

void readInputs(int argc, char**argv) {
	//checking for arguments
	//fprintf(stderr,"The argument supplied to %s are", argv[0]);
	for (int i = 1; i < argc - 1; i++) {
		//fprintf(stderr," %s ", argv[i]);
		if (i < argc - 1) {
			//fprintf(stderr,"and");
		}
	}
	//fprintf(stderr,". Check that the first two arguments are intitial board config and evaluation parameters respectively.\n");
	if (argc < 3) {
		printf("Too little arguments supplied, check README.\n");
		exit(1);
	}

	//Opening file inputs

	char *mode = "r";
	char outputFilename[30];
	char* opfp = outputFilename;
	char abPruning[10];

	brdtxt = fopen(argv[1], mode);
	evaltxt = fopen(argv[2], mode);

	if (brdtxt == NULL) {
		fprintf(stderr, "Can't open input file %s!\n", argv[1]);
		exit(1);
	}

	fscanf(brdtxt, "Size: %d,%d\n", &width, &height);
	fscanf(brdtxt, "White: { %s }\n", whiteStr);
	fscanf(brdtxt, "Black: { %s }\n", blackStr);

	if (evaltxt == NULL) {
		fprintf(stderr, "Can't open input file %s!\n", argv[2]);
		exit(1);
	}
	fscanf(evaltxt, "MaxDepth: %d\n", &maxDepth);
	fscanf(evaltxt, "MaxBoards: %ld\n", &maxBoards);
	fscanf(evaltxt, "MaxPlayer: %s\n", maxPlayerStr);
	fscanf(evaltxt, "abPruning: %s\n", abPruning);
	sprintf(opfp, "outSerial%dx%d.txt", width, height);
	ofp = fopen(outputFilename, "a");

	if (ofp == NULL) {
		fprintf(stderr, "Can't open output file %s!\n",
			outputFilename);
		exit(1);
	}

	if (strcmp(maxPlayerStrPtr, "WHITE") == 0) {
		maxPlayer = WHITE;
	}
	else if (strcmp(maxPlayerStrPtr, "BLACK") == 0) {
		maxPlayer = BLACK;
	}
	else {
		fprintf(stderr, "Can't tell who is the MaxPlayer: %s!\n", maxPlayerStr);
	}
	
	if (strcmp(abPruning, "TRUE") == 0) {
		ALPHA_BETA_PRUNING = 1;
	}
	else {
		ALPHA_BETA_PRUNING = 0;
	}
	fseek(brdtxt, 0, SEEK_SET);
	fseek(evaltxt, 0, SEEK_SET);
	fprintf(ofp, "########################################################################################################\n");
	char ch;
	while ((ch = fgetc(brdtxt)) != EOF)		fputc(ch, ofp);
	fputc('\n', ofp);
	while ((ch = fgetc(evaltxt)) != EOF)	fputc(ch, ofp);
	fputc('\n', ofp);
	fclose(evaltxt);
	fclose(brdtxt);
}

void saveRunResult(uint* bestMovesArr, int bestMovesCount) {
	fprintMovesArr(ofp, bestMovesArr, bestMovesCount, width);
	fprintf(stderr,"Number of boards assessed: %ld\n", numBoards);
	fprintf(ofp, "Number of boards assessed: %ld\n", numBoards);

	if (maxPlayer == BLACK) {

		fprintf(ofp, "Max Player: BLACK\n");
	}
	else {
		fprintf(ofp, "Max Player: WHITE\n");
	}
	fprintf(stderr, "computation_time = %ld clock ticks\n", comp_time);
	fprintf(ofp, "computation_time = %ld clock ticks\n", comp_time);

	fprintf(stderr, "computation_time = %ld mins %ld secs %ld msecs\n", comp_time / CLOCKS_PER_SEC / 60, comp_time / CLOCKS_PER_SEC % 60, comp_time * 1000 / CLOCKS_PER_SEC % 1000);
	fprintf(ofp, "computation_time = %ld mins %ld secs %ld msecs\n", comp_time / CLOCKS_PER_SEC / 60, comp_time / CLOCKS_PER_SEC % 60, comp_time * 1000 / CLOCKS_PER_SEC % 1000);
	fclose(ofp);
}

void printBoard(uint** boardPtr) {
	for (int i = height - 1; i >= 0; i--) {
		for (int j = 0; j < width; j++) {
			if (boardPtr[j][i] == BLACK) {
				fprintf(stderr, "B");
			}
			else if (boardPtr[j][i] == WHITE) {
				fprintf(stderr, "W");
			}
			else if(boardPtr[j][i] == 3){
				fprintf(stderr, "D");
			}
			else {
				fprintf(stderr, "-");
			}

		}
		fprintf(stderr, "\n");
	}
	fprintf(stderr, "\n");
}

int main(int argc, char **argv) {
	readInputs(argc, argv);
	//fprintf(stderr,"Started\n");
	uint** boardPtr = createBoard(width, height, maxPlayer, whiteStrPtr, blackStrPtr);
	//fprintf(stderr,"Board %d x %d created\n", width, height);

	//fprintf(stderr,"Game created\n");
	uint* bestMovesArr = malloc(height*width * sizeof(uint));
	before = clock();
	int bestMovesCount = 0;
	int* bestMovesCountPtr = &bestMovesCount;
	playMax(boardPtr, maxDepth, bestMovesCountPtr, bestMovesArr, INT_MIN, INT_MAX);
	after = clock();
	comp_time = after - before;

	saveRunResult(bestMovesArr, bestMovesCount);
	printMovesArr(bestMovesArr, bestMovesCount, width);
	fprintf(stderr,"Solved with %d best moves\n", *bestMovesCountPtr);
	free(bestMovesArr);
	freeBoard(boardPtr);
	return 0;
}
