#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <sys/time.h>
#include <string.h>
#include <ctype.h>
#include <mpi.h>
#include <time.h>



#define BLACK 1
#define WHITE 0
#define EMPTY 2
#define TRUE 1
#define FALSE 0
#define PLY 3
#define SSIZE 8
#define uchar unsigned char
#define uint unsigned int
#define delimiters " ,"
#define INT_MAX 2^32

long long before, after;
const uchar alphabets[26] = { 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z' };
const char DELIM[] = delimiters;
long long comm_time = 0;
long long comp_time = 0;
long long numBoards = 0;
long long maxBoards;
int slaves;
int slavesUsed;
#define MASTER_ID slaves

FILE *brdtxt, *evaltxt, *ofp;
char* whiteInitialPos;
char* blackInitialPos;
char whiteStr[26 * 26 * 3];
char blackStr[26 * 26 * 3];
char maxPlayerStr[6];
char* maxPlayerStrPtr = maxPlayerStr;
char* whiteStrPtr = whiteStr;
char* blackStrPtr = blackStr;

int config[5];
int* configPtr = config;
int width;
int height;
int maxDepth;
int maxPlayer;
int nprocs;

int myid;

void saveRunResult(uint * bestMovesArr, int bestMovesCount);
uint c_to_n(char c);
uint * to2d(uint i, int width);
int to1d(int x, int y, int width);
char * mystrsep(char ** stringp, const char * delim);
int availableMoves(uint ** boardPtr, int * movesPtr, int player);
int countFlips(int x, int y, uint ** boardPtr, int player);
uint ** createStandardBoard();
uint ** createBoard(int width, int height, int player, char * whiteStrPtr, char * blackStrPtr);
int solve(uint ** boardPtr, int ply, uint * bestMovesArr);
int playMax(uint ** boardPtr, int ply);
int playMin(uint ** boardPtr, int ply);
int evaluate(uint ** boardPtr);
uint ** makeMove(uint ** boardPtr, uint nextMove, uint player);
void placePiece(uint ** boardPtr, uint player, int x, int y);
uint ** duplicateBoard(uint ** boardPtr);
void freeBoard(uint ** boardPtr);
void printMovesArr(uint * MovesArr, uint count, int width);
void fprintMovesArr(FILE * ofp, uint * MovesArr, uint count, int width);
void readInputs(int argc, char ** argv);

long long wall_clock_time()
{
#ifdef LINUX
	struct timespec tp;
	clock_gettime(CLOCK_REALTIME, &tp);
	return (long long)(tp.tv_nsec + (long long)tp.tv_sec * 1000000000ll);
#else
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (long long)(tv.tv_usec * 1000 + (long long)tv.tv_sec * 1000000000ll);
#endif
}

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

	// Count right flips
	for (int i = x + 1; i < width; ++i) {
		if (boardPtr[i][y] == player) {
			flips += counter;
			break;
		}
		if (boardPtr[i][y] == EMPTY) {
			break;
		}
		//printf("Can flip at %d-%d for %d vs %d\n", i,y, boardPtr[i][y] , gptr->player);
		counter++;
	}
	counter = 0;

	//Count left flips
	for (int i = x - 1; i >= 0; --i) {
		if (boardPtr[i][y] == player) {
			flips += counter;
			break;
		}
		if (boardPtr[i][y] == EMPTY) {
			break;
		}
		counter++;
	}
	counter = 0;

	//Count top flips
	for (int j = y + 1; j < height; j++) {
		if (boardPtr[x][j] == player) {
			flips += counter;
			break;
		}
		if (boardPtr[x][j] == EMPTY) {
			break;
		}
		counter++;
	}
	counter = 0;

	//Count bottom flips
	for (int j = y - 1; j >= 0; j--) {
		if (boardPtr[x][j] == player) {
			flips += counter;
			break;
		}
		if (boardPtr[x][j] == EMPTY) {
			break;
		}
		counter++;
	}
	counter = 0;

	//Count top right flips
	for (int i = 1; i + x < width && i + y < height; i++) {
		if (boardPtr[x + i][y + i] == player) {
			flips += counter;
			break;
		}
		if (boardPtr[x + i][y + i] == EMPTY) {
			break;
		}
		counter++;
	}
	counter = 0;

	//Count bottom left flips
	for (int i = 1; x - i >= 0 && y - i >= 0; i++) {
		if (boardPtr[x - i][y - i] == player) {
			flips += counter;
			break;
		}
		if (boardPtr[x - i][y - i] == EMPTY) {
			break;
		}
		counter++;
	}
	counter = 0;

	//Count top left flips
	for (int i = 1; x - i >= 0 && i + y < height; i++) {
		if (boardPtr[x - i][y + i] == player) {
			flips += counter;
			break;
		}
		if (boardPtr[x - i][y + i] == EMPTY) {
			break;
		}
		counter++;
	}
	counter = 0;

	//Count bottom right flips
	for (int i = 1; x + i < width && y - i >= 0; i++) {
		if (boardPtr[x + i][y - i] == player) {
			flips += counter;
			break;
		}
		if (boardPtr[x + i][y - i] == EMPTY) {
			break;
		}
		counter++;
	}
	counter = 0;
	//printf("checking %d-%d has %d\n", x,y,flips);
	return flips;
}


uint** createStandardBoard() {
	printf("Starting boardPtr creation\n");
	uint** boardPtr = malloc(sizeof(uint*)*SSIZE);
	for (int i = 0; i < SSIZE; i++) {
		boardPtr[i] = malloc(sizeof(uint)*SSIZE);
	}
	for (int i = 0; i < SSIZE; i++) {
		for (int j = 0; j < SSIZE; j++) {
			boardPtr[i][j] = 0;
		}
	}
	boardPtr[4][4] = WHITE;
	boardPtr[5][5] = WHITE;
	boardPtr[5][4] = BLACK;
	boardPtr[4][5] = BLACK;
	return boardPtr;
}

uint** createBoard(int width, int height, int player, char* whiteStrPtr, char* blackStrPtr) {
	printf("Starting boardPtr creation\n");
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
		printf("Placing white token at %s\n", token);
		//printf("Placing white token at %d-%d\n", c_to_n(token[0]),token[1]-'0');
		boardPtr[c_to_n(token[0])][token[1] - '0'] = WHITE;
	}
	while ((token = mystrsep(&blackStrPtr, DELIM)) != NULL) {
		printf("Placing black token at %s\n", token);
		//printf("Placing white token at %d-%d\n", c_to_n(token[0]), token[1]-'0');
		boardPtr[c_to_n(token[0])][token[1] - '0'] = BLACK;
	}
	return boardPtr;
}

int solve(uint** boardPtr, int ply, uint* bestMovesArr) {
	if (ply == 0) {
		printf("ply == 0");
		return 0;
	}

	before = wall_clock_time();
	int value = -INT_MAX;
	uint* movesArr = malloc(height*width * sizeof(uint));
	int numMoves = availableMoves(boardPtr, movesArr, maxPlayer);
	slavesUsed = numMoves;
	if (numMoves == 0) {
		printf("No available moves");
		return 0;
	}
	int temp;
	int bestMovesCount;
	config[0] = height;
	config[1] = width;
	config[2] = maxPlayer;
	config[3] = ply - 1;
	config[4] = maxBoards;
	after = wall_clock_time();
	comp_time += after - before;

	for (int i = 0; i < numMoves; i++) {
		fprintf(stderr, "MASTER SENDING config TO PROCESS %d\n",i);

		before = wall_clock_time();
		MPI_Send(config, 5, MPI_INT, i, 1, MPI_COMM_WORLD);
		after = wall_clock_time();
		comm_time += after - before;

		before = wall_clock_time();
		uint** newBoardPtr = makeMove(boardPtr, movesArr[i], maxPlayer);
		after = wall_clock_time();
		comp_time += after - before;

		//temp = playMin(newBoard, ply - 1);
		fprintf(stderr, "MASTER SENDING newBoardPtr TO PROCESS %d\n", i);

		before = wall_clock_time();
		MPI_Send(newBoardPtr[0], height*width, MPI_UNSIGNED, i, !maxPlayer, MPI_COMM_WORLD);
		after = wall_clock_time();
		comm_time += after - before;

		//freeBoard(newBoardPtr);
	}
	for (int i = 0; i < numMoves; i++) {
		MPI_Status status;
		fprintf(stderr, "MASTER RECEIVING FROM PROCESS %d\n", i);

		before = wall_clock_time();
		MPI_Recv(&temp, 1, MPI_UNSIGNED, i, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
		after = wall_clock_time();
		comm_time += after - before;

		fprintf(stderr, "MASTER RECEIVED FROM PROCESS %d\n", i);
		before = wall_clock_time();
		if (temp > value) {
			value = temp;
			bestMovesCount = 0;
			bestMovesArr[bestMovesCount] = movesArr[i];
			bestMovesCount++;
			printf("Move %d has %d\n", movesArr[i], temp);
		}
		else if (temp == value) {
			value = temp;
			bestMovesArr[bestMovesCount] = movesArr[i];
			bestMovesCount++;
			printf("Move %d has %d\n", movesArr[i], temp);
		}
		after = wall_clock_time();
		comp_time += after - before;
	}
	free(movesArr);
	return bestMovesCount;
}

int playMax(uint** boardPtr, int ply) {
	if (ply == 0 || numBoards == maxBoards) {
		return evaluate(boardPtr);
	}
	else {
		//fprintf(stderr, "PROCESS %d PLAYING FOR MAX %d\n", myid, ply);
		int value = -INT_MAX;
		uint* movesArr = malloc(height*width * sizeof(uint));
		int numMoves = availableMoves(boardPtr, movesArr, maxPlayer);
		if (numMoves == 0) {
			return playMin(boardPtr, ply - 1);
		}
		int temp;
		for (int i = 0; i < numMoves; i++) {
			uint** newBoard = makeMove(boardPtr, movesArr[i], maxPlayer);
			temp = playMin(newBoard, ply - 1);
			if (temp > value) {
				value = temp;
			}
			freeBoard(newBoard);
		}
		//printf("Done Max\n");
		free(movesArr);

		return value;
	}
}

int playMin(uint** boardPtr, int ply) {
	if (ply == 0 || numBoards == maxBoards) {
		return evaluate(boardPtr);
	}
	else {
		//fprintf(stderr, "PROCESS %d PLAYING FOR MIN %d\n", myid, ply);
		int value = INT_MAX;
		uint* movesArr = malloc(height*width * sizeof(uint));
		int numMoves = availableMoves(boardPtr, movesArr, !maxPlayer);
		if (numMoves == 0) {
			return playMax(boardPtr, ply - 1);
		}
		int temp;
		for (int i = 0; i < numMoves; i++) {
			uint** newBoard = makeMove(boardPtr, movesArr[i], !maxPlayer);
			temp = playMax(newBoard, ply - 1);
			if (temp < value) {
				value = temp;
			}
			freeBoard(newBoard);
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
	++numBoards;
	free(move);
	return newBoardPtr;
}

void placePiece(uint** boardPtr, uint player, int x, int y) {
	int counter = 0;
	for (int i = x + 1; i < width; ++i) {
		if (boardPtr[i][y] == player) {
			for (int j = 0; j <= counter; j++) {
				boardPtr[x + j][y] = player;
			}
			break;
		}
		if (boardPtr[i][y] == EMPTY) {
			break;
		}
		//printf("Can flip at %d-%d for %d vs %d\n", i,y, boardPtr[i][y] , gptr->player);
		counter++;
	}
	counter = 0;

	//left flips
	for (int i = x - 1; i >= 0; --i) {
		if (boardPtr[i][y] == player) {
			for (int j = 0; j <= counter; j++) {
				boardPtr[x - j][y] = player;
			}
			break;
		}
		if (boardPtr[i][y] == EMPTY) {
			break;
		}
		counter++;
	}
	counter = 0;

	// top flips
	for (int i = y + 1; i < height; i++) {
		if (boardPtr[x][i] == player) {
			for (int j = 0; j <= counter; j++) {
				boardPtr[x][y + j] = player;
			}
			break;
		}
		if (boardPtr[x][i] == EMPTY) {
			break;
		}
		counter++;
	}
	counter = 0;

	// bottom flips
	for (int i = y - 1; i >= 0; i--) {
		if (boardPtr[x][i] == player) {
			for (int j = 0; j <= counter; j++) {
				boardPtr[x][y - j] = player;
			}
			break;
		}
		if (boardPtr[x][i] == EMPTY) {
			break;
		}
		counter++;
	}
	counter = 0;

	// top right flips
	for (int i = 1; i + x < width && i + y < height; i++) {
		if (boardPtr[x + i][y + i] == player) {
			for (int j = 0; j <= counter; j++) {
				boardPtr[x + j][y + j] = player;
			}
			break;
		}
		if (boardPtr[x + i][y + i] == EMPTY) {
			break;
		}
		counter++;
	}
	counter = 0;

	// bottom left flips
	for (int i = 1; x - i >= 0 && y - i >= 0; i++) {
		if (boardPtr[x - i][y - i] == player) {
			for (int j = 0; j <= counter; j++) {
				boardPtr[x - j][y - j] = player;
			}
			break;
		}
		if (boardPtr[x - i][y - i] == EMPTY) {
			break;
		}
		counter++;
	}
	counter = 0;

	// top left flips
	for (int i = 1; x - i >= 0 && i + y < height; i++) {
		if (boardPtr[x - i][y + i] == player) {
			for (int j = 0; j <= counter; j++) {
				boardPtr[x - j][y + j] = player;
			}
			break;
		}
		if (boardPtr[x - i][y + i] == EMPTY) {
			break;
		}
		counter++;
	}
	counter = 0;

	// bottom right flips
	for (int i = 1; x + i < width && y - i >= 0; i++) {
		if (boardPtr[x + i][y - i] == player) {
			for (int j = 0; j <= counter; j++) {
				boardPtr[x + j][y - j] = player;
			}
			break;
		}
		if (boardPtr[x + i][y - i] == EMPTY) {
			break;
		}
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
	printf("Best Moves: { ");
	for (int i = 0; i < count; i++) {
		if (i > 0) {
			printf(", ");
		}
		uint* move = to2d(MovesArr[i], width);
		printf("%c%d", alphabets[move[0]], move[1]);
	}
	printf(" }\n");
}

void fprintMovesArr(FILE* ofp, uint* MovesArr, uint count, int width) {
	fprintf(ofp, "Best Moves: { ");
	for (int i = 0; i < count; i++) {
		if (i > 0) {
			fprintf(ofp, ", ");
		}
		uint* move = to2d(MovesArr[i], width);
		fprintf(ofp, "%c%d", alphabets[move[0]], move[1]);
	}
	fprintf(ofp, " }\n");
}

void readInputs(int argc, char**argv) {
	//checking for arguments
	printf("The argument supplied to %s are", argv[0]);
	for (int i = 1; i < argc - 1; i++) {
		printf(" %s ", argv[i]);
		if (i < argc - 1) {
			printf("and");
		}
	}
	printf(". Check that the first two arguments are intitial board config and evaluation parameters respectively.\n");
	if (argc < 3) {
		printf("Too little arguments supplied, check README.\n");
		exit(1);
	}

	//Opening file inputs

	char *mode = "r";
	char outputFilename[30];
	char* opfp = outputFilename;

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
	fscanf(evaltxt, "MaxBoards: %lld\n", &maxBoards);
	fscanf(evaltxt, "MaxPlayer: %s\n", maxPlayerStr);

	sprintf(opfp, "outMPI%dx%d.txt", width, height);
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
	printf("Number of boards assessed: %lld\n", numBoards);
	fprintf(ofp, "Number of boards assessed: %lld\n", numBoards);

	if (maxPlayer == BLACK) {

		fprintf(ofp, "Max Player: BLACK\n");
	}
	else {
		fprintf(ofp, "Max Player: WHITE\n");
	}
	printf("communication_time = %lld mins %lld secs %lld msecs\n", comm_time / 1000000000 / 60, comm_time / 1000000000 % 60, comm_time * 1000 / 1000000000 % 1000);
	fprintf(ofp, "communication_time = %lld mins %lld secs %lld msecs\n", comm_time / 1000000000/ 60, comm_time / 1000000000 % 60, comm_time * 1000 / 1000000000 % 1000);

	printf("computation_time = %lld mins %lld secs %lld msecs\n", comp_time / 1000000000 / 60, comp_time / 1000000000 % 60, comp_time * 1000 / 1000000000 % 1000);
	fprintf(ofp, "computation_time = %lld mins %lld secs %lld msecs\n", comp_time / 1000000000 / 60, comp_time / 1000000000 % 60, comp_time * 1000 / 1000000000 % 1000);
	fclose(ofp);
}

void reportRunResult() {
	MPI_Send(&numBoards, 1, MPI_LONG , MASTER_ID, 1, MPI_COMM_WORLD );
	long long timings[2] = { comm_time, comp_time };
	MPI_Send(timings, 2, MPI_LONG_LONG, MASTER_ID, 2, MPI_COMM_WORLD);
}

void retrieveRunResults() {
	long long nB;
	long long timings[2];
	MPI_Status status;
	fprintf(stderr, "MASTER----communication_time = %lld mins %lld secs %lld msecs\n", comm_time / 1000000000 / 60, comm_time / 1000000000 % 60, comm_time * 1000 / 1000000000 % 1000);
	fprintf(stderr, "MASTER----computation_time = %lld mins %lld secs %lld msecs\n", comp_time / 1000000000 / 60, comp_time / 1000000000 % 60, comp_time * 1000 / 1000000000 % 1000);
	for (int i = 0; i < slavesUsed; i++) {
		MPI_Recv(&nB, 1, MPI_LONG_LONG, i, 1, MPI_COMM_WORLD, &status);
		MPI_Send(timings, 2, MPI_LONG_LONG, i, 2, MPI_COMM_WORLD);
		fprintf(stderr, "Process %d----numBoards = %lld\n", i, nB);
		fprintf(stderr, "Process %d----communication_time = %lld mins %lld secs %lld msecs\n",i, timings[0] / 1000000000 / 60, timings[0] / 1000000000 % 60, timings[0] * 1000 / 1000000000 % 1000);
		fprintf(stderr, "Process %d----computation_time = %lld mins %lld secs %lld msecs\n",i, timings[1] / 1000000000 / 60, timings[1] / 1000000000 % 60, timings[1] * 1000 / 1000000000 % 1000);
		numBoards += nB;
		comm_time += timings[0];
		comp_time += timings[1];
	}
}

void slave() {
	int player;
	int result;
	
	MPI_Status status;
	fprintf(stderr, "PROCESS %d ready to receive config\n", myid);

	before = wall_clock_time();
	MPI_Recv(config, 5, MPI_INT, MASTER_ID, 1, MPI_COMM_WORLD, &status);
	after = wall_clock_time();
	comm_time += after - before;

	fprintf(stderr, "PROCESS %d RECEIVED config ", myid);
	fprintf(stderr, " {%d,%d,%d,%d}FROM MASTER\n", config[0], config[1],config[2],config[3]);

	before = wall_clock_time();
	height = config[0];
	width = config[1];
	maxPlayer = config[2];
	maxDepth = config[3];
	maxBoards = config[4];
	uint** boardPtr = malloc(sizeof(uint*)*width);
	uint* board = malloc(sizeof(uint)*width*height);
	for (int i = 0; i < width; i++) {
		boardPtr[i] = &board[i*height];
	}
	after = wall_clock_time();
	comp_time += after - before;

	fprintf(stderr, "PROCESS %d RECEIVING FROM MASTER\n", myid);

	before = wall_clock_time();
	MPI_Recv(boardPtr[0], height*width, MPI_UNSIGNED, MASTER_ID, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
	after = wall_clock_time();
	comm_time += after - before;

	fprintf(stderr, "PROCESS %d RECEIVED FROM MASTER\n", myid);
	before = wall_clock_time();
	player = status.MPI_TAG;
	fprintf(stderr, "PROCESS %d RECEIVED TAG %d\n", myid, player);
	if (player == maxPlayer) {
		fprintf(stderr, "PROCESS %d PLAYING MAX WITH %d ply\n", myid, maxDepth);
		result = playMax(boardPtr, maxDepth);

	}
	else {
		fprintf(stderr, "PROCESS %d PLAYING MIN WITH %d ply\n", myid, maxDepth);
		result = playMin(boardPtr, maxDepth);
	}
	after = wall_clock_time();
	comp_time += after - before;

	fprintf(stderr, "PROCESS %d SENDING RESULT %d\n", myid, result);

	before = wall_clock_time();
	MPI_Send(&result, 1, MPI_UNSIGNED, MASTER_ID, player, MPI_COMM_WORLD);
	after = wall_clock_time();
	comm_time += after - before;
}

int main(int argc, char **argv) {
	int nprocs;
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
	MPI_Comm_rank(MPI_COMM_WORLD, &myid);

	slaves = nprocs - 1;
	

	if (myid == MASTER_ID) {
		readInputs(argc, argv);
		printf("Started\n");
	
		fprintf(stderr, " +++ Process %d is master\n", myid);
		uint** boardPtr = createBoard(width, height, maxPlayer, whiteStrPtr, blackStrPtr);
		printf("Board %d x %d created\n", width, height);

		//printf("Game created\n");
		uint* bestMovesArr = malloc(height*width * sizeof(uint));
		
		int bestMovesCount = solve(boardPtr, maxDepth, bestMovesArr);
		retrieveRunResults();
		saveRunResult(bestMovesArr, bestMovesCount);

		printf("Solved!\n");
		free(bestMovesArr);
		freeBoard(boardPtr);
	}
	else {
		fprintf(stderr, " --- Process %d is slave\n", myid);
		slave();
		fprintf(stderr, " --- Process %d is done\n", myid);
		reportRunResult();
	}
	MPI_Finalize();
	return 0;
}