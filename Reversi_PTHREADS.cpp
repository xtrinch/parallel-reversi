#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include<pthread.h>

//Stevilo MPI procesov
#define NPROC 4
//Definiramo velikost igralnega poljas
#define BOARD_HEIGHT 8
#define BOARD_WIDTH 8
#define BOARD_SIZE (BOARD_HEIGHT*BOARD_WIDTH)

//Mozne barve polja
#define BLACK (-1)
#define WHITE 1
#define EMPTY 0

//Maksimalna globina iskanja Minimax algoritma
#define MAX_DEPTH 4

//Podatkovne strukture potrebne pri postavljanju novega ploscka 
#define DIRECTIONS 8
const int direction[DIRECTIONS][2] = { { 0, 1 }, { 0, -1 }, { 1, 0 }, { -1, 0 }, { 1, 1 }, { 1, -1 }, { -1, 1 }, { -1, -1 } };


#define MAX(x, y) (((x) > (y)) ? (x) : (y))

int numLeaves;
char *boards;
int phase = 0;
int* scores;

struct params{
	int scoreIndex;	//indeks niti
	char *L;		//kazalec na seznam		
	int count;	// stevilo boardov
	int *scores;
	int depth;
};

//Inicializiramo zacetno igralno polje
void initBoard(char *board){

	for (int i = 0; i < BOARD_HEIGHT; i++)
	{

		for (int j = 0; j < BOARD_WIDTH; j++)
		{
			board[i*BOARD_WIDTH + j] = EMPTY;
		}
	}

	board[27] = WHITE;
	board[28] = BLACK;
	board[35] = BLACK;
	board[36] = WHITE;

}

//Izrisi igralno polje
void printBoard(char *board){
	printf("  ");
	for (int j = 0; j < BOARD_WIDTH; j++)
		printf("-%d", j + 1);
	printf("\n");
	for (int i = 0; i < BOARD_HEIGHT; i++){
		printf("%d| ", i + 1);
		for (int j = 0; j < BOARD_WIDTH; j++)
		{
			char c = ' ';
			if (board[i*BOARD_WIDTH + j] == WHITE) c = 'O';
			if (board[i*BOARD_WIDTH + j] == BLACK) c = 'X';
			printf("%c ", c);
		}
		printf("\b|\n");
	}
	printf("  ");
	for (int j = 0; j < BOARD_WIDTH; j++)
		printf("--");
	printf("\n");
}

//Preveri, ce je poteza veljavna
int isValidMove(int player, char *board, int row, int column){

	//Ce polje ni prazno potem poteza ni veljavna
	if (board[row*BOARD_WIDTH + column] != EMPTY)
		return 0;

	//V vse smeri iz trenutnega polja pogledamo, ce preko nasprotnikovih plosckov pridemo do svojega ploscka
	int valid = 0;
	for (int i = 0; i < DIRECTIONS; i++){
		int metOpponent = 0;
		for (int di = row + direction[i][0], dj = column + direction[i][1]; di >= 0 && di < BOARD_HEIGHT && dj >= 0 && dj < BOARD_WIDTH; di += direction[i][0], dj += direction[i][1]){
			if (board[di*BOARD_WIDTH + dj] == player){
				if (!metOpponent)
					break;
				else{
					//Ce najdemo tako pot je poteza veljavna
					valid = 1;
					return valid;
				}
			}
			else if (board[di*BOARD_WIDTH + dj] == -player)
				metOpponent = 1;
			else
				break;
		}

	}
	return valid;

}

//Preverimo ce lahko trenutni igralec postavi svoj ploscek
int canPlayerMakeMove(int player, char *board){

	for (int i = 0; i < BOARD_HEIGHT; i++)
	for (int j = 0; j < BOARD_WIDTH; j++)
	if (isValidMove(player, board, i, j))
		return 1;

	return 0;

}

//Preverimo ce je ze konec igre
int isGameOver(char *board){
	//Ce noben od igralcev ne more narediti poteze je igre konec
	return (!canPlayerMakeMove(WHITE, board) && !canPlayerMakeMove(BLACK, board));

}
//Postavimo ploscek na polje in pobarvamo ustrezne ploscke v svojo barvo
void placePiece(int row, int column, int player_color, char * board){

	for (int i = 0; i < DIRECTIONS; i++){
		int metOpponent = 0;
		for (int di = row + direction[i][0], dj = column + direction[i][1]; di >= 0 && di < BOARD_HEIGHT && dj >= 0 && dj < BOARD_WIDTH; di += direction[i][0], dj += direction[i][1]){

			if (board[di*BOARD_WIDTH + dj] == player_color){
				if (!metOpponent)
					break;
				else{
					int count = MAX(abs(di - row), abs(dj - column));
					for (int c = 0; c < count; c++)
						board[(row + c*direction[i][0])*BOARD_WIDTH + column + c*direction[i][1]] = player_color;
					break;
				}
			}
			else if (board[di*BOARD_WIDTH + dj] == -player_color)
				metOpponent = 1;
			else
				break;
		}

	}


}

//Cloveski igralec preberemo vrstico in stolpec, kamor naj se postavi ploscek
int humanMove(char *board, int player_color) {
	int row = -1;
	int column = -1;
	if (isGameOver(board))
		return 0;
	do {
		printf("Input row and column ([1..8]<Enter>[1..8]):\n");
		scanf_s("%d", &row);
		scanf_s("%d", &column);
		printf("\n");
		row -= 1;
		column -= 1;
	} while (row >= 9 || row < 0 || column >= 9 || column >= 9 || !isValidMove(player_color, board, row, column));
	placePiece(row, column, player_color, board);
	return 1;
}

int evaluateBoard(char *board){
	int sum = 0;
	//Izracunamo razliko med stevilom belih in crnih plosckov
	for (int i = 0; i < BOARD_HEIGHT; i++)
	{
		for (int j = 0; j < BOARD_WIDTH; j++)
			sum += board[i*BOARD_WIDTH + j];
	}
	return sum;

}


//Prestejemo koliko je vseh plosckov na igralnem polju
int countPieces(char *board){
	int sum = 0;

	for (int i = 0; i < BOARD_HEIGHT; ++i)
	for (int j = 0; j < BOARD_WIDTH; ++j)
	if (board[i*BOARD_WIDTH + j] != 0)
		sum++;

	return sum;

}

//Rekurzivni minimax algoritem
int minimax(char *board, int player_color, int max_depth, int depth, int row, int column){

	//Nastavimo najmanjse mozno zacetno stevilo tock
	int score = -INT_MAX*player_color;

	//Pogledamo ce smo ze prisli do najvecje mozne globine rekurzije
	if (depth > max_depth)
	{
		if (phase == 0)
		{
			// save board
			if (numLeaves < NPROC || numLeaves < 128)
			{
				memcpy(&boards[numLeaves * 64], board, 64);
				numLeaves++;
			}
		}
		else if (phase == 2)
		{
			score = scores[numLeaves];
			numLeaves++;
		}
		else {
			score = evaluateBoard(board);
		}
		return score;
	}
	//Generiramo vse mozne poteze nasprotnega igralca in se spustimo en nivo nizje
	int moved = 0;
	char tempBoard[64];
	for (int i = 0; i < BOARD_HEIGHT; i++)
	for (int j = 0; j < BOARD_WIDTH; j++)
		//Preverimo ce je nasprotnikova poteza veljavna
	if (isValidMove(-player_color, board, i, j)) {
		moved = 1;
		//Ustvarimo si zacasno igralno polje
		memcpy(tempBoard, board, BOARD_SIZE);
		//Postavimo ploscek
		placePiece(i, j, -player_color, tempBoard);
		//Spustimo se en nivo nizje po rekurziji in odigramo potezo
		int tempScore = minimax(tempBoard, -player_color, max_depth, depth + 1, i, j);
		//Pogledamo ce je poteza smiselna
		if (tempScore*player_color > score*player_color){
			score = tempScore;
		}
	}
	//Ce ni bilo mozno narediti nobene poteze evalviramo igralno polje in koncamo
	if (!moved)
	{
		if (phase == 0)
		{
			// save board
			if (numLeaves < NPROC || numLeaves < 128)
			{
				memcpy(&boards[numLeaves * 64], board, 64);
				numLeaves++;
			}
		}
		else if (phase == 2)
		{
			numLeaves++;
			score = evaluateBoard(board);

		}
		else {
			score = evaluateBoard(board);
		}

	}

	return score;



}


//Racunalniski igralec
int computerMove(char * board, int player_color, int max_depth){


	int row = -1;
	int column = -1;
	int score = -INT_MAX*player_color;
	//Preverimo ce je ze konec igre
	if (isGameOver(board))
		return 0;
	char tempBoard[BOARD_SIZE];
	//Generiramo in izvedemo vse mozne poteze racunalnika
	for (int i = 0; i < BOARD_HEIGHT; i++) {
		for (int j = 0; j < BOARD_WIDTH; j++){
			if (board[i*BOARD_WIDTH + j] == EMPTY && isValidMove(player_color, board, i, j)) {
				memcpy(tempBoard, board, BOARD_SIZE);
				placePiece(i, j, player_color, tempBoard);
				int tempScore = minimax(tempBoard, player_color, max_depth, 1, i, j);

				//Najdemo najboljso potezo
				if (tempScore*player_color > score*player_color) {
					score = tempScore;
					row = i;
					column = j;

				}
			}
		}
	}

	//Izvedemo najbolje ocenjeno potezo
	if (row >= 0 && column >= 0)
		placePiece(row, column, player_color, board);
	return 1;
}

int computerGenerate(char * board, int player_color, int max_depth){


	//Preverimo ce je ze konec igre
	if (isGameOver(board))
		return -500;
	int score = -INT_MAX*player_color;
	//Generiramo in izvedemo vse mozne poteze racunalnika
	for (int i = 0; i < BOARD_HEIGHT; i++) {
		for (int j = 0; j < BOARD_WIDTH; j++){
			if (board[i*BOARD_WIDTH + j] == EMPTY && isValidMove(player_color, board, i, j)) {
				char tempBoard[BOARD_SIZE];
				memcpy(tempBoard, board, BOARD_SIZE);
				placePiece(i, j, player_color, tempBoard);
				int tempScore = minimax(tempBoard, player_color, max_depth, 1, i, j);
				if (tempScore*player_color > score*player_color) {
					score = tempScore;
				}
			}
		}
	}

	return score;
}


//Nakljucni racunalniski igralec
int computerRandomMove(char * board, int player_color, int max_depth = 0){
	int row = -1;
	int column = -1;
	//Preverimo ce je ze konec igre
	if (isGameOver(board))
		return 0;

	struct move{
		int row;
		int column;
	};
	struct move available_moves[64];
	//Generiramo vse mozne poteze racunalnika
	int n_moves = 0;
	for (int i = 0; i < BOARD_HEIGHT; i++) {
		for (int j = 0; j < BOARD_WIDTH; j++){
			if (board[i*BOARD_WIDTH + j] == EMPTY && isValidMove(player_color, board, i, j)) {
				available_moves[n_moves].row = i;
				available_moves[n_moves].column = j;
				n_moves++;
			}
		}
	}

	//Izberemo nakljucno potezo
	if (n_moves>0){
		int move_index = rand() % n_moves;
		row = available_moves[move_index].row;
		column = available_moves[move_index].column;
		if (row >= 0 && column >= 0)
			placePiece(row, column, player_color, board);
	}
	return 1;
}


void *parallelMinimax(void * arg)
{
	int player1 = BLACK; //Player X
	int player2 = WHITE; //Player O

	phase = 1;

	int numToProcess = ((params *)arg)->count;
	int scoreIndex = ((params *)arg)->scoreIndex;
	char *recvbuf = ((params *)arg)->L;
	int *scores = ((params *)arg)->scores;
	int depth = ((params *)arg)->depth;
	int myScore;

	for (int i = 0; i < numToProcess; i++)
	{
		if ((depth) % 2 == 0)
		{
			myScore = minimax(recvbuf + i * 64, player1, MAX_DEPTH, depth + 1, 0, 0);
		}
		else
		{
			myScore = minimax(recvbuf + i * 64, player2, MAX_DEPTH, depth + 1, 0, 0);
		}

		// add to array
		scores[scoreIndex + i] = myScore;
	}

	return NULL;
}

int main(int argc, char* argv[]) {

	char *myBoard;
	int player1 = BLACK; //Player X
	int player2 = WHITE; //Player O
	int *res = (int*)malloc(2 * sizeof(int));

	// pthread variables
	pthread_t t[NPROC];
	params p[NPROC];

	double avgTime = 0.0;
	int nWhiles = 0;

	//Inicializiramo zacetno polje
	char *board = (char *)calloc(64, sizeof(char));

	// starting at depth 1
	int seqDepth = 1;
	int myScore;
	int breakWhile = 0;

	srand(time(NULL));

	initBoard(board);

	// array for boards
	if (NPROC < 128) {
		scores = (int *)malloc(128 * sizeof(int));
		boards = (char *)malloc(128 * sizeof(char)* 64);
	}
	else {
		scores = (int *)malloc(NPROC*sizeof(int));
		boards = (char *)malloc(NPROC*sizeof(char)* 64);
	}
	printf("-------START-------\n");
	printBoard(board);
	
	int numWhiles = 0;
	while (1){
		numWhiles++;

		double t1, t2;

		t1 = clock();
		// first execution of sequential part
		phase = 0;

		numLeaves = 0;
		while (numLeaves < NPROC && seqDepth <= MAX_DEPTH)
		{
			seqDepth++;
			// number of leaves
			numLeaves = 0;
			if (computerGenerate(board, player1, seqDepth) == -500)
				break;

			if (numLeaves >= NPROC)
			{
				numLeaves = 0;
				seqDepth--;
				computerGenerate(board, player1, seqDepth);

				break;
			}

		}

		res[0] = numLeaves;
		res[1] = seqDepth;

		int nLeaves = res[0];
		int depth = res[1];
		int nEl = nLeaves / NPROC;
		int *sendcnts = 0;
		int *displs = 0;

		sendcnts = (int *)malloc(NPROC*sizeof(int));

		int nOst = nLeaves % NPROC;
		int tmpCnt = 1;
		for (int i = 0; i < NPROC; i++) {
			sendcnts[i] = nEl * 64;
			if (nOst != 0) {
				sendcnts[i] += 64;
				nOst--;
			}
		}

		displs = (int *)malloc(NPROC*sizeof(int));
		displs[0] = 0;
		for (int i = 1; i < NPROC; i++) {
			if (sendcnts[i] != 0)
				displs[i] = displs[i - 1] + sendcnts[i - 1];
			else {
				displs[i] = 0;
				sendcnts[i] = 1;
			}
		}

		int scoreIndex = 0;
		for (int i = 0; i < NPROC; i++){
			if (i>0)
			scoreIndex = scoreIndex + (sendcnts[i-1] / 64);
			p[i].L = boards + displs[i];
			p[i].scoreIndex = scoreIndex;
			p[i].count = sendcnts[i] / 64;
			p[i].scores = scores;
			p[i].depth = seqDepth;
			pthread_create(&t[i], NULL, parallelMinimax, (void *)&p[i]);
		}

		for (int i = 0; i<NPROC; i++)
			pthread_join(t[i], NULL);

		// run sequential part of program again, read scores
		phase = 2;
		numLeaves = 0;
		printf("\nPlayer X:\n");
		if (!computerMove(board, player1, seqDepth)) {
			breakWhile = 1;
		}
		else {

			t2 = clock();
			//printf("Parallel minimax on %d processes took %f seconds.\n", NPROC, (double)(t2 - t1) / CLOCKS_PER_SEC);
			// time measurements
			t2 = clock();
			double time = (double)(t2 - t1) / CLOCKS_PER_SEC;
			avgTime += time;
			nWhiles++;

			printBoard(board);
			printf("\nPlayer O:\n");
			if (!computerRandomMove(board, player2, 1)) {
				breakWhile = 1;
			}
			printBoard(board);
		}

		if (breakWhile == 1)
			break;


	}

	printf("-----GAME OVER-----\n");
	printBoard(board);
	int score = evaluateBoard(board);
	int total = countPieces(board);
	printf("SCORE: Player X:%d Player O:%d\n", (total - score) / 2, (total - score) / 2 + score);

	avgTime /= nWhiles;
	printf("Search of depth %d  on %d processes took an average of %f seconds.\n", MAX_DEPTH, NPROC, avgTime);
	return 0;
}
