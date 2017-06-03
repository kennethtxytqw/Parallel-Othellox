#include "board.h"
#include "solver.h"
#include <mpi.h>

#define TYPE "MPI"

long long before, after;
long long comm_time = 0;
long long comp_time = 0;

char whiteStr[26 * 26 * 3];
char blackStr[26 * 26 * 3];
int width = -1;
int* widthPtr = &width;
int height = -1;
int* heightPtr = &height;
int maxDepth = -1;
int* maxDepthPtr = &maxDepth;
int maxPlayer = -1;
int* maxPlayerPtr = &maxPlayer;
int abPruning = 1;
int* abPruningPtr = &abPruning;
int numBoards = 0;
int* numBoardsPtr = &numBoards;
int maxBoards = -1;
int* maxBoardsPtr = &maxBoards;
char* whiteStrPtr = whiteStr;
char* blackStrPtr = blackStr;
int timeout = 0;
int* timeoutPtr = &timeout;
uint bestMovesArr[26];
int bestMovesCount = 0;
int* bestMovesCountPtr = &bestMovesCount;

// MPI needed variables
int slaves;
int slavesUsed;
#define MASTER_ID slaves
#define CONFIG_SIZE 6
int config[CONFIG_SIZE];
int* configPtr = config;
int myid;

int player;
int result;
uint** boardPtr;

int* solve(uint*** listOf_boardsPtr, int nB, int ply, int currPlayer);

int master(uint** boardPtr, int ply, uint* bestMovesArr) {
	if (ply == 0) {
		//fprintf(stderr,"ply == 0");
		return 0;
	}
	uint* movesArr = malloc(100 * sizeof(uint));
	int* values;
	uint*** listOf_boardsPtr = malloc(sizeof(uint**));
	listOf_boardsPtr[0] = boardPtr;
	//printBoard(listOf_boardsPtr[0],width,height);
	int numMoves = availableMoves(boardPtr, movesArr, maxPlayer, width, height);
	values = solve(listOf_boardsPtr, 1, maxDepth, maxPlayer);
	int currMax = INT_MIN;
	for (int i = 0; i < numMoves; i++) {
		//fprintf(stderr, "Move %d scores %d pts\n", movesArr[i], values[i]);
		if (currMax < values[i]) {
			currMax = values[i];
			bestMovesCount = 0;
		}
		if (currMax <= values[i]) {
			bestMovesArr[bestMovesCount] = movesArr[i];
			bestMovesCount++;
			
		}
	}
	
	

	//free(movesArr);
	//free(listOf_boardsPtr);
	return bestMovesCount;
}

void master_distribute(uint*** listOf_boardsPtr, int* numMoves_of_each_board, int currPlayer, int ply, int nB, int* recvB, MPI_Request* requests) {
	before = wall_clock_time();
	int bestMovesCount;
	config[0] = height;
	config[1] = width;
	config[2] = maxPlayer;
	config[3] = ply;
	config[4] = maxBoards;
	config[5] = currPlayer; // This should be the player you want the slaves to play as.
	after = wall_clock_time();
	comp_time += after - before;
	
	int bCtr = 0;
	int mCtr = 0;

	for (int i = 0; i < slaves; i++) {
		//fprintf(stderr, "MASTER SENDING config TO PROCESS %d\n", i);

		before = wall_clock_time();
		MPI_Isend(config, CONFIG_SIZE, MPI_INT, i, 1, MPI_COMM_WORLD, &(requests[i]));
		after = wall_clock_time();
		comm_time += after - before;
	}

}

int* master_taskpooling(uint*** listOf_boardsPtr, int* numMoves_of_each_board, int currPlayer, int* recvB, int numMoves, int nB, MPI_Request* requests) {
	int sent = 0;
	int received = 0;
	int bCtr = 0;
	int mCtr = 0;

	int* values = malloc(sizeof(int)*numMoves);
	for (int i = 0; i < numMoves; i++) {
		if (currPlayer == maxPlayer) {
			values[i] = INT_MIN;
		}
		else {
			values[i] = INT_MAX;
		}
		
	}

	for(int i =0; i<slaves;i++){
		//fprintf(stderr, "MASTER SENDING newBoardPtr TO PROCESS %d\n", i);
		//fprintf(stderr, "sending bCtr %d with mCtr %d\n", bCtr, mCtr);
		before = wall_clock_time();
		MPI_Isend(listOf_boardsPtr[bCtr][0], height*width, MPI_UNSIGNED, i, bCtr * 1000 + mCtr, MPI_COMM_WORLD, &(requests[i]));
		//MPI_Isend(boardPtr[0], height*width, MPI_UNSIGNED, i, mCtr, MPI_COMM_WORLD, &(requests[i]));
		//printBoard(listOf_boardsPtr[bCtr],width,height);
		//freeBoard(newBoardPtr);
		MPI_Irecv(&(recvB[i]), 1, MPI_INT, i, bCtr, MPI_COMM_WORLD, &(requests[i]));
		after = wall_clock_time();
		comm_time += after - before;
		if (numMoves_of_each_board[bCtr] - 1 > mCtr) {
			mCtr++;

		}
		else {
			mCtr = 0;
			bCtr++;
		}
		sent++;
		
	}
	while (sent < numMoves || received < numMoves) {
		MPI_Status status;
		// p keeps track of who has completed so that we can reassign
		int p, pval;
		//fprintf(stderr, "MASTER Waiting for any report\n");
		MPI_Waitany(slaves, requests, &p, &status);

		// move dictates which board is this value for, AKA the bCtr it was
		int parentBrdIdx = status.MPI_TAG;
		pval = recvB[p];
		//fprintf(stderr, "MASTER RECEIVED FROM PROCESS %d\n", p);
		received++;
		if (sent < numMoves) {
			before = wall_clock_time();
			//fprintf(stderr, "MASTER SENDING ANOTHER TASK %d of numMoves %d TO PROCESS %d\n", sent, numMoves,p);
			//fprintf(stderr, "sending bCtr %d with mCtr %d\n", bCtr, mCtr);
			//printBoard(listOf_boardsPtr[bCtr],width,height);
			MPI_Isend(listOf_boardsPtr[bCtr][0], height*width, MPI_UNSIGNED, p, bCtr*1000+mCtr, MPI_COMM_WORLD, &(requests[p]));
			MPI_Irecv(&(recvB[p]), 1, MPI_INT, p, bCtr, MPI_COMM_WORLD, &(requests[p]));
			after = wall_clock_time();
			comm_time += after - before;
			sent++;
			if (numMoves_of_each_board[bCtr] - 1 > mCtr) {
				mCtr++;

			}
			else {
				//fprintf(stderr, "Board %d has %d moves\n", bCtr, numMoves_of_each_board[bCtr]);
				mCtr = 0;
				bCtr++;
			}
		}
		before = wall_clock_time();
		//fprintf(stderr, "Move %d can get %d compared to %d\n", parentBrdIdx, pval, values[parentBrdIdx]);
		if (currPlayer == maxPlayer && pval > values[parentBrdIdx]) {
			
			values[parentBrdIdx] = pval;
		}
		else if (currPlayer != maxPlayer && pval < values[parentBrdIdx]) {
			values[parentBrdIdx] = pval;
		}
		after = wall_clock_time();
		comp_time += after - before;
	}

	for (int i = 0; i < slaves; i++) {
		MPI_Isend(boardPtr[0], width*height, MPI_UNSIGNED, i, INT_MAX, MPI_COMM_WORLD, &(requests[i]));
	}
	return values;
}

int* solve(uint*** listOf_boardsPtr, int nB, int ply, int currPlayer) {
	int numMoves = 0;
	int* numMoves_of_each_board = malloc(sizeof(int)*nB);
	uint** listOf_movesArr = malloc(nB* sizeof(uint*));
	uint* movesArr = malloc(100 * nB * sizeof(uint));
	
	int* recvB = malloc(sizeof(int)*numMoves*3);
	int* values;
	for (int i = 0; i < nB; i++) {
		listOf_movesArr[i] = &movesArr[i*nB];
		//printBoard(listOf_boardsPtr[i], width, height);
		numMoves_of_each_board[i] = availableMoves(listOf_boardsPtr[i], listOf_movesArr[i], currPlayer, width, height);
		numMoves += numMoves_of_each_board[i];
		//fprintf(stderr, "Board %d has %d moves\n", i, numMoves_of_each_board[i]);
	}

	MPI_Request* requests = malloc(sizeof(MPI_Request) * slaves);
	if (numMoves >= slaves*2) {
		master_distribute(listOf_boardsPtr, numMoves_of_each_board, currPlayer, ply - 1, nB, recvB, requests);
		values = master_taskpooling(listOf_boardsPtr, numMoves_of_each_board,currPlayer, recvB, numMoves, nB, requests);
		//free(numMoves_of_each_board);
		free(movesArr);
		free(listOf_movesArr);
		//free(recvB);
		return values;
	}
	else {
		uint*** newlistof_boardsptr = malloc(sizeof(uint**)*numMoves);
		int bCtr = 0;
		int mCtr = 0;
		for (int i = 0; i < numMoves; i++) {

			newlistof_boardsptr[i] = makeMove(listOf_boardsPtr[bCtr], listOf_movesArr[bCtr][mCtr], currPlayer, width, height);
			numBoards++;
			if (numMoves_of_each_board[bCtr] - 1 > mCtr) {
				mCtr++;
			}
			else {
				mCtr = 0;
				bCtr++;
			}
		}
		values = solve(newlistof_boardsptr, numMoves, ply - 1,!currPlayer);
		//free(numMoves_of_each_board);
		free(movesArr);
		free(listOf_movesArr);
		//free(recvB);
		//for (int i = 0; i < numMoves; i++) {
		//	free(newlistof_boardsptr[i]);
		//}
		//free(newlistof_boardsptr);
		return values;
	}

}

void reportRunResult() {


	MPI_Send(&numBoards, 1, MPI_INT , MASTER_ID, INT_MAX-1, MPI_COMM_WORLD );
	long long timings[2] = { comm_time, comp_time };
	MPI_Send(&(timings[0]), 2, MPI_LONG_LONG, MASTER_ID, INT_MAX-2, MPI_COMM_WORLD);
	fprintf(stderr, "MYID %d----numBoards = %d\n", myid, numBoards);
	fprintf(stderr, "MYID %d----communication_time = %lld mins %lld secs %lld msecs\n", myid, comm_time / 1000000000 / 60, comm_time / 1000000000 % 60, comm_time * 1000 / 1000000000 % 1000);
	fprintf(stderr, "MYID %d----computation_time = %lld mins %lld secs %lld msecs\n", myid, comp_time / 1000000000 / 60, comp_time / 1000000000 % 60, comp_time * 1000 / 1000000000 % 1000);

}

void retrieveRunResults() {
	int nB;
	long long timings[2];
	MPI_Status status;
	fprintf(stderr, "MASTER----communication_time = %lld mins %lld secs %lld msecs\n", comm_time / 1000000000 / 60, comm_time / 1000000000 % 60, comm_time * 1000 / 1000000000 % 1000);
	fprintf(stderr, "MASTER----computation_time = %lld mins %lld secs %lld msecs\n", comp_time / 1000000000 / 60, comp_time / 1000000000 % 60, comp_time * 1000 / 1000000000 % 1000);
	for (int i = 0; i < slaves; i++) {
		MPI_Recv(&nB, 1, MPI_INT, i, INT_MAX-1, MPI_COMM_WORLD, &status);
		//fprintf(stderr, "MPI_TAG: %d, MPI_SOURCE: %d MPI_ERROR: %d\n", status.MPI_TAG, status.MPI_SOURCE, status.MPI_ERROR);
		MPI_Recv(timings, 2, MPI_LONG_LONG, i, INT_MAX-2, MPI_COMM_WORLD, &status);
		//fprintf(stderr, "Process %d----numBoards = %d\n", i, nB);
		//fprintf(stderr, "Process %d----communication_time = %lld mins %lld secs %lld msecs\n",i, timings[0] / 1000000000 / 60, timings[0] / 1000000000 % 60, timings[0] * 1000 / 1000000000 % 1000);
		//fprintf(stderr, "Process %d----computation_time = %lld mins %lld secs %lld msecs\n",i, timings[1] / 1000000000 / 60, timings[1] / 1000000000 % 60, timings[1] * 1000 / 1000000000 % 1000);
		numBoards += nB;
		comm_time += timings[0];
		comp_time += timings[1];
	}
}

void slave_receive_config() {
	MPI_Status status;
	//fprintf(stderr, "PROCESS %d ready to receive config\n", myid);

	before = wall_clock_time();
	MPI_Recv(config, CONFIG_SIZE, MPI_INT, MASTER_ID, 1, MPI_COMM_WORLD, &status);
	after = wall_clock_time();
	comm_time += after - before;

	//fprintf(stderr, "PROCESS %d RECEIVED config ", myid);
	//fprintf(stderr, " {h:%d,w:%d,maxPlayer:%d,maxDepth:%d,maxB:%d,currPlayer:%d}FROM MASTER\n", config[0], config[1], config[2], config[3], config[4], config[5]);

	before = wall_clock_time();
	height = config[0];
	width = config[1];
	maxPlayer = config[2];
	maxDepth = config[3];
	maxBoards = config[4];
	player = config[5];
	after = wall_clock_time();
	comp_time += after - before;
}

int slave_receive_board() {
	MPI_Status status;
	uint** boardPtrBuf = createBoard(width, height);
	//fprintf(stderr, "PROCESS %d RECEIVING BOARD FROM MASTER\n", myid);

	before = wall_clock_time();
	//printBoard(boardPtrBuf, width, height);
	MPI_Recv(boardPtrBuf[0], height*width, MPI_UNSIGNED, MASTER_ID, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
	//printBoard(boardPtrBuf, width, height);
	after = wall_clock_time();
	comm_time += after - before;

	
	if (status.MPI_TAG == INT_MAX) {
		return -1;
	}
	before = wall_clock_time();
	int* movesArr = malloc(sizeof(int) * 100);
	availableMoves(boardPtrBuf, movesArr, player, width, height);
	boardPtr = makeMove(boardPtrBuf, movesArr[status.MPI_TAG%1000] , player, width, height);
	player = !player;
	//printBoard(boardPtr, width, height);
	numBoards++;
	after = wall_clock_time();
	comp_time += after - before;

	free(movesArr);
	freeBoard(boardPtrBuf);
	//fprintf(stderr, "PROCESS %d RECEIVED BOARD %d TO PLAY MOVE %d FROM MASTER\n", myid, status.MPI_TAG/1000, status.MPI_TAG%1000);
	before = wall_clock_time();
	//fprintf(stderr, "PROCESS %d RECEIVED Player %d\n", myid, player);
	return status.MPI_TAG/1000;
}

void slave() {
	slave_receive_config();
	int parentBrdIdx = slave_receive_board();
	while (parentBrdIdx != -1) {
		before = wall_clock_time();
		if (player == maxPlayer) {
			//fprintf(stderr, "PROCESS %d STARTING MAX WITH %d ply on player %d \n", myid, maxDepth, player);
			result = playMax(boardPtr, maxDepth, bestMovesCountPtr, bestMovesArr, INT_MIN, INT_MAX, width, height, maxBoards, maxDepth, abPruning, maxPlayer, numBoardsPtr);

		}
		else {
			//fprintf(stderr, "PROCESS %d STARTING MIN WITH %d ply on player %d \n", myid, maxDepth, player);
			result = playMin(boardPtr, maxDepth, bestMovesCountPtr, bestMovesArr, INT_MIN, INT_MAX, width, height, maxBoards, maxDepth, abPruning, maxPlayer, numBoardsPtr);
		}
		after = wall_clock_time();
		comp_time += after - before;

		//fprintf(stderr, "PROCESS %d SENDING RESULT %d FOR BOARD %d\n", myid, result, parentBrdIdx);

		before = wall_clock_time();
		MPI_Send(&result, 1, MPI_INT, MASTER_ID, parentBrdIdx, MPI_COMM_WORLD);
		after = wall_clock_time();
		comm_time += after - before;
		player = !player;
		parentBrdIdx = slave_receive_board();
	}
}

int main(int argc, char **argv) {
	int nprocs;
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
	MPI_Comm_rank(MPI_COMM_WORLD, &myid);

	slaves = nprocs - 1;
	

	if (myid == MASTER_ID) {
		long long start = wall_clock_time();
		readInputs(argc, argv, widthPtr, heightPtr, maxDepthPtr, maxBoardsPtr, maxPlayerPtr, abPruningPtr, whiteStrPtr, blackStrPtr, timeoutPtr, TYPE);
		//fprintf(stderr,"Started\n");
	
		//fprintf(stderr, " +++ Process %d is master\n", myid);
		boardPtr = initBoard(width, height, maxPlayer, whiteStrPtr, blackStrPtr);
		//fprintf(stderr,"Board %d x %d created\n", width, height);

		//printf("Game created\n");
		
		bestMovesCount = master(boardPtr, maxDepth, bestMovesArr);
		long long end = wall_clock_time();
		long long total_time = end - start;
		retrieveRunResults();
		printMovesArr(bestMovesArr, bestMovesCount, width);
		saveRunResult(bestMovesArr, bestMovesCount, width, numBoards, comp_time, comm_time, total_time, slaves+1);

		//fprintf(stderr,"Solved!\n");
		//free(bestMovesArr);
		//freeBoard(boardPtr);
	}
	else {
		//fprintf(stderr, " --- Process %d is slave\n", myid);
		slave();
		//fprintf(stderr, " --- Process %d is done\n", myid);
		reportRunResult();
	}
	MPI_Finalize();
	return 0;
}