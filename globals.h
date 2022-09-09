
#include "stdafx.h"

#define U64 unsigned __int64 
#define BITBOARD unsigned __int64 

const int MAXDEPTH   = 16;
const int BISHOPPAIR = 25; // Additional score is +0.25 for the bishop's pair

// http://www.billwallchess.com/articles/Value.htm
const int PAWN_VALUE   =   100;
const int KNIGHT_VALUE =   325;
const int BISHOP_VALUE =   350;
const int ROOK_VALUE   =   500;
const int QUEEN_VALUE  =   975;
const int KING_VALUE   = 10000;

const int A1 =  0;
const int B1 =  1;
const int C1 =  2;
const int D1 =  3;
const int E1 =  4;
const int F1 =  5;
const int G1 =  6;
const int H1 =  7;

const int A2 =  8;
const int B2 =  9;
const int C2 = 10;
const int D2 = 11;
const int E2 = 12;
const int F2 = 13;
const int G2 = 14;
const int H2 = 15;

const int A3 = 16;
const int B3 = 17;
const int C3 = 18;
const int D3 = 19;
const int E3 = 20;
const int F3 = 21;
const int G3 = 22;
const int H3 = 23;

const int A4 = 24;
const int B4 = 25;
const int C4 = 26;
const int D4 = 27;
const int E4 = 28;
const int F4 = 29;
const int G4 = 30;
const int H4 = 31;

const int A5 = 32;
const int B5 = 33;
const int C5 = 34;
const int D5 = 35;
const int E5 = 36;
const int F5 = 37;
const int G5 = 38;
const int H5 = 39;

const int A6 = 40;
const int B6 = 41;
const int C6 = 42;
const int D6 = 43;
const int E6 = 44;
const int F6 = 45;
const int G6 = 46;
const int H6 = 47;

const int A7 = 48;
const int B7 = 49;
const int C7 = 50;
const int D7 = 51;
const int E7 = 52;
const int F7 = 53;
const int G7 = 54;
const int H7 = 55;

const int A8 = 56;
const int B8 = 57;
const int C8 = 58;
const int D8 = 59;
const int E8 = 60;
const int F8 = 61;
const int G8 = 62;
const int H8 = 63;

const int NORTH = 0;
const int NE    = 1;
const int EAST  = 2;
const int SE    = 3;
const int SOUTH = 4;
const int SW    = 5;
const int WEST  = 6;
const int NW    = 7;

const int P     = 0;
const int N     = 1;
const int B     = 2;
const int R     = 3;
const int Q     = 4;
const int K     = 5;
const int EMPTY = 6;

const int White = 0;
const int Black = 1;

const int MAX_PLY = 64;
const int MOVE_STACK = 40000; // erhoeht um Faktor 10
const int GAME_STACK = 20000; // erhoeht um Faktor 10

const int HASH_SCORE = 100000000;
const int CAPTURE_SCORE = 10000000;

enum class FIXEDLEVEL { DEPTH, TIME, NODES };
extern FIXEDLEVEL fixedLevel;

extern const U64 MAXHASH; 
extern const U64 HASHSIZE;

extern bool gameIsRunning;
extern int move_start, move_dest, move_promote;

typedef struct {
	int start;
	int dest;
	int promote;
	int score;
  } move;

typedef struct {
	int start;
	int dest;
	int promote;
	int capture;
	int fifty;
	int castle;
	U64 hash;
	U64 lock;
} game;

extern int piece_value[6];
extern int pawn_mat[2];
extern int piece_mat[2];

extern int king_endgame[2][64];
extern int passed[2][64];

extern char piece_char[6];

extern int side,xside;
extern int fifty;
extern int ply,hply;
extern int castle;
extern int castle_mask[64];

extern U64 nodes;

extern int board[64];
extern int init_color[64];
extern int init_board[64];

extern int history[64][64];

extern int square_score[2][6][64];

extern const int row[64];
extern const int col[64];
extern const int nwdiag[64];
extern const int nediag[64];

extern int first_move[MAX_PLY];
extern move move_list[MOVE_STACK];
extern game game_list[GAME_STACK];

extern int hash_start,hash_dest;
extern U64 currentkey,currentlock;

extern U64 max_time;
extern U64 start_time;
extern U64 stop_time;
extern int max_depth;
extern U64 max_nodes;

extern int qrb_moves[64][9];
extern int knight_moves[64][9];
extern int king_moves[64][9];

extern U64 collisions;

extern int turn;

extern int Flip[64];

extern int rank[2][64];

extern U64 vectorbits[64][64];
extern U64 mask_vectors[64][8];
extern BITBOARD bit_between[64][64];
extern int pawndouble[2][64];
extern int pawnplus[2][64];
extern int pawnleft[2][64];
extern int pawnright[2][64];

extern BITBOARD bit_pawncaptures[2][64];
extern BITBOARD bit_pawndefends[2][64];
extern BITBOARD bit_left[2][64];
extern BITBOARD bit_right[2][64];
extern BITBOARD bit_pawnmoves[2][64];
extern BITBOARD bit_knightmoves[64];
extern BITBOARD bit_bishopmoves[64];
extern BITBOARD bit_rookmoves[64];
extern BITBOARD bit_queenmoves[64];
extern BITBOARD bit_kingmoves[64];

//current position
extern BITBOARD bit_pieces[2][7];
extern BITBOARD bit_units[2];
extern BITBOARD bit_all;

extern BITBOARD mask_passed[2][64];
extern BITBOARD mask_path[2][64];

extern BITBOARD mask_cols[64];

extern BITBOARD mask[64];
extern BITBOARD not_mask[64];

extern BITBOARD mask_isolated[64];

extern BITBOARD not_a_file;
extern BITBOARD not_h_file;

//init.cpp
void InitBoard();
void SetTables();
void NewPosition();
void Alg(int a,int b);
void Algebraic(int a);
void SetMoves();

//search.cpp
void DisplayBoardToFile(std::fstream& logfile, int flip);
void think();
int Search(int alpha, int beta, int depth);
int CaptureSearch(int alpha,int beta);
int ReCaptureSearch(int,const int);
int reps2();
void Sort(const int from);
void CheckUp();

//gen.cpp
void Gen(int,int);
void GenEp();
void GenCastle();
void AddMove(const int x,const int sq);
void AddMove(const int x,const int sq,const int score);
int GenRecapture(int);
void GenCaptures(const int,const int);
void AddCapture(const int x,const int sq,const int score);

//attack.cpp
bool Attack(const int s,const int sq);
int LowestAttacker(const int s,const int x);

//update.cpp
void UpdatePiece(const int s,const int p,const int start,const int dest);
void RemovePiece(const int s,const int p,const int sq);
void AddPiece(const int s,const int p,const int sq);
int MakeMove(const int,const int);
int TakeBack();
int MakeRecapture(const int,const int);
void UnMakeRecapture();

//eval.cpp
int Eval();

//hash.cpp
void RandomizeHash();
int Random(const int x);
void AddKey(const int,const int,const int);
U64 GetKey();
U64 GetLock();
void Free();
void AddHash(const int s, const move m);
bool LookUp(const int s);

void AddPawnHash(const int s1, const int s2, const int wq, const int wk, const int bq, const int bk);
int GetHashPawn0();
int GetHashPawn1();
int LookUpPawn();
void AddPawnKey(const int s,const int x);

//main.cpp
const std::string currentDateTime();
U64 GetTime();
char *MoveString(int from,int to,int promote);
void NewPosition();
int ParseMove(char *s);
void DisplayBoard();
int reps();

int GetHashDefence(const int s,const int half);

int NextBit(BITBOARD bb);

void PrintBitBoard(BITBOARD bb);




