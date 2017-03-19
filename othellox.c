#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stddef.h>

#define BLACK 2
#define WHITE 1
#define EMPTY 0
#define TRUE 1
#define FALSE 0
#define PLY 3
#define SSIZE 8
#define uchar unsigned char
#define uint unsigned int
#define delimiters " ,"

const uchar alphabets[26] = { 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z' };
const char DELIM[] = delimiters;


typedef struct {
	uint** board; //Using uchar because it is the smallest suitable data type 
	uchar player;
	uint height;
	uint width;
}Game;

int c_to_n(char c)
{
	int n = -1;
	static const char * const alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	char *p = strchr(alphabet, toupper((unsigned char)c));

	if (p)
	{
		n = p - alphabet;
	}

	return n;
}

int* to2d(int i, int width) {
	int* iarr = (int*)malloc(sizeof(i)*2);
	int x = i%width;
	int y = i / width;
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

int availableMoves(Game* gptr){
	for(int y=0; y<gptr->height; y++){
		for(int x=0; x<gptr->width; x++){
			if(countFlips(x,y,gptr)>0){
				printf("%c%d is a valid move\n", alphabets[x],y);
				// evaluate(x,y,game->board);
			}
		}
	}
}

int countFlips(int x, int y, Game* gptr){
	// if the box is not EMPTY then return 0 flips which is an invalid move
	if (gptr->board[x][y] != EMPTY) {
		return 0;
	}
	int flips=0;
	int counter=0;
	uint** board = gptr->board;
	
	// Count right flips
	for(int i=x+1;i<gptr->width;++i){
		if(board[i][y]==gptr->player){
			flips += counter;
			break;
		}
		if(board[i][y]==EMPTY){
			break;
		}
		//printf("Can flip at %d-%d for %d vs %d\n", i,y, board[i][y] , gptr->player);
		counter++;
	}
	counter=0;

	//Count left flips
	for(int i=x-1;i>=0;--i){
		if(board[i][y]==gptr->player){
			flips += counter;
			break;
		}
		if(board[i][y]==EMPTY){
			break;
		}
		counter++;
	}
	counter=0;

	//Count top flips
	for(int j=y+1;j<gptr->height;j++){
		if(board[x][j]==gptr->player){
			flips += counter;
			break;
		}
		if(board[x][j]==EMPTY){
			break;
		}
		counter++;
	}
	counter=0;

	//Count bottom flips
	for(int j=y-1;j>=0;j--){
		if(board[x][j]==gptr->player){
			flips += counter;
			break;
		}
		if(board[x][j]==EMPTY){
			break;
		}
		counter++;
	}
	counter = 0;

	//Count top right flips
	for (int i = 1; i + x < gptr->width && i + y < gptr->height; i++) {
		if (board[x + i][y + i] == gptr->player) {
			flips += counter;
			break;
		}
		if (board[x + i][y + i] == EMPTY) {
			break;
		}
		counter++;
	}
	counter = 0;

	//Count bottom left flips
	for (int i = 1; x-i >=0 && y-i >=0; i++) {
		if (board[x - i][y - i] == gptr->player) {
			flips += counter;
			break;
		}
		if (board[x - i][y - i] == EMPTY) {
			break;
		}
		counter++;
	}
	counter = 0;

	//Count top left flips
	for (int i = 1; x-i >= 0 && i + y <gptr->height; i++) {
		if (board[x - i][y + i] == gptr->player) {
			flips += counter;
			break;
		}
		if (board[x - i][y + i] == EMPTY) {
			break;
		}
		counter++;
	}
	counter = 0;

	//Count bottom right flips
	for (int i = 1; x + i <gptr->width && y-i >= 0; i++) {
		if (board[x + i][y - i] == gptr->player) {
			flips += counter;
			break;
		}
		if (board[x + i][y - i] == EMPTY) {
			break;
		}
		counter++;
	}
	counter = 0;
	//printf("checking %d-%d has %d\n", x,y,flips);
	return flips;
}

Game* newGame(uchar player, uint height, uint width, uint** board){
	Game* gptr;
	gptr = (Game*) malloc(sizeof(Game));
	gptr->board = board; //(uint**) malloc(sizeof(uint)*height*width);
	gptr->height = height;
	gptr->width = width;
	gptr->player = player;
	
	return gptr;
} 


uint** createStandardBoard(){
	printf("Starting board creation\n");
	uint** board = (uint**) malloc(sizeof(uint*)*SSIZE);
	for(int i =0; i<SSIZE; i++){
		board[i]= (uint*) malloc(sizeof(uint)*SSIZE);
	}
	for (int i = 0; i < SSIZE; i++) {
		for (int j = 0; j < SSIZE; j++) {
			board[i][j] = 0;
		}
	}
	board[4][4]= WHITE;
	board[5][5]=WHITE;
	board[5][4]=BLACK;
	board[4][5]=BLACK;
	return board;
}

uint** createBoard(int width, int height, int player, char* whiteStrPtr, char* blackStrPtr) {
	printf("Starting board creation\n");
	char* token;
	uint** board = (uint**)malloc(sizeof(uint*)*width);
	for (int i = 0; i<width; i++) {
		board[i] = (uint*)malloc(sizeof(uint)*height);
	}
	for (int i = 0; i < width; i++) {
		for (int j = 0; j < height; j++) {
			board[i][j] = 0;
		}
	}
	while ((token = mystrsep(&whiteStrPtr, DELIM)) != NULL) {
		printf("Placing white token at %s\n", token);
		//printf("Placing white token at %d-%d\n", c_to_n(token[0]),token[1]-'0');
		board[c_to_n(token[0])][token[1]-'0'] = WHITE;
	}
	while ((token = mystrsep(&blackStrPtr, DELIM)) != NULL) {
		printf("Placing black token at %s\n", token);
		//printf("Placing white token at %d-%d\n", c_to_n(token[0]), token[1]-'0');
		board[c_to_n(token[0])][token[1]-'0'] = BLACK;
	}
	return board;
}


int main(int argc, char **argv) {
	//checking for arguments
	if (argc == 3) {
		printf("The argument supplied is %s and %s\n", argv[1], argv[2]);
	}
	else if (argc > 3) {
		printf("Too many arguments supplied.\n");
	}
	else {
		printf("Two argument expected for othellox program, check README.\n");
	}

	//Opening file inputs
	FILE *brdtxt, *evaltxt, *ofp;
	char *mode = "r";
	char outputFilename[] = "out.list";

	brdtxt= fopen(argv[1], mode);
	evaltxt = fopen(argv[2], mode);

	if (brdtxt == NULL) {
		fprintf(stderr, "Can't open input file %s!\n", argv[1]);
		exit(1);
	}

	char* whiteInitialPos;
	char* blackInitialPos;
	char whiteStr[26 * 26 * 3];
	char blackStr[26 * 26 * 3];
	char* whiteStrPtr = whiteStr;
	char* blackStrPtr = blackStr;

	int width;
	int height;

	fscanf(brdtxt, "Size: %d,%d\n", &width, &height);
	fscanf(brdtxt, "White: { %s }\n", whiteStr);
	fscanf(brdtxt, "Black: { %s }\n", blackStr);

	if (evaltxt == NULL) {
		fprintf(stderr, "Can't open input file %s!\n", argv[2]);
		exit(1);
	}

	ofp = fopen(outputFilename, "w");

	if (ofp == NULL) {
		fprintf(stderr, "Can't open output file %s!\n",
			outputFilename);
		exit(1);
	}

	printf("Started\n");
	uint** board = createBoard(width, height, BLACK, whiteStrPtr, blackStrPtr);
	printf("board created\n");
	
	//printf("checking if placed at %d\n", board[4][4]);
	Game* gptr = newGame(BLACK , width , height, board);
	availableMoves(gptr
);

	return 0;
}