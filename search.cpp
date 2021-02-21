#include <setjmp.h>
#include <iostream>
#include <iomanip> 
#include <fstream>

#include "globals.h"

void SetHashMove();	
void DisplayPV(int i);

jmp_buf env;
bool stop_search;

int currentmax;

int move_start,move_dest;

int LowestAttacker(const int s, const int xs,const int sq);

static int debugDepth = 0; //DEBUG
static bool moveListFlag = false; //Debug
std::fstream streamToLogSearch("logSearch.txt", std::ios::out); //DEBUG

/*

think() iterates until the maximum depth for the move is reached or until the allotted time
has run out.
After each iteration the principal variation is then displayed.

*/
void think()
{
	debugDepth = 0; //DEBUG
	moveListFlag = false; //DEBUG
	streamToLogSearch << "\n" << "-------------" << std::endl; //DEBUG

	int x;
	stop_search = false;
	setjmp(env);

	if (stop_search)
	{
		while (ply)
			TakeBack();
		return;
	}

	if (fixed_time == 0)
	{
		if (Attack(xside, NextBit(bit_pieces[side][K])))
			max_time = max_time / 2;
	}

	start_time = GetTime();
	stop_time = start_time + max_time;

	ply = 0;
	nodes = 0;

	NewPosition();
	memset(history, 0, sizeof(history));
	printf("ply    score         time\t\t\t nodes\t principal variation\n");
	for (int i = 1; i <= max_depth; ++i)
	{
		currentmax = i;
		if (fixed_depth == 0 && max_depth > 1)
		{
			if (fixed_nodes == 1)
			{
				if (nodes > max_nodes)
				{
					stop_search = true;
					return;
				}
			}
			else if (fixed_time == 1)
			{
				if (GetTime() >= start_time + max_time)
				{
					stop_search = true;
					return;
				}
			}
			else if (GetTime() >= start_time + max_time / 4)
			{
				stop_search = true;
				return;
			}
		}						

		x = Search(-10000, 10000, i);

		// output for UCI
		if (LookUp(side))
		{
			std::cout << "info depth " << i << " score cp " << x
				<< " nodes " << nodes << " pv";
			DisplayPV(i);
			std::cout << std::endl;
		}

		printf("%0*d\t%*d\t  %*llu\t  %*d\t", 3, i,
			4, x,
			7, (GetTime() - start_time) / 10,
			20, nodes);

		if (LookUp(side))
		{
			DisplayPV(i);
		}
		else
		{
			move_start = 0;
			move_dest = 0;
		}
		printf("\n");
		fflush(stdout);

		if (x > 9000 || x < -9000)
		{
			break;
		}
	}
}
/*

search is the main part of the search.
If the position is repeated we don't need to look any further.
If depth has run out, the capture search is done.
Every 4,000 positions approx the time is checked.
Moves are generated.
The moves are looped through in order of their score.
If a move is illegal (for example, if it attempts to move a pinned piece)
then it is skipped over.
If the move is check, we extend by one ply. This is done by not changing depth in the call to search.
If it has a score greater than zero, ply is one or the move number is less than 12
a normal search is done. This is done by subtracting 1 from depth. 
Otherwise we reduce by one ply. This is done by subtracting 2 from depth. 
The move is taken back.

If the score from search is greater than beta, a beta cutoff happens. No need to
search any moves at this depth.
Otherwise, if it is greater than alpha, alpha is changed. 

If there were no legal moves it is either checkmate or stalemate.

*/

int Search(int alpha, int beta, int depth)
{
	if (ply && reps2())
	{
		return 0;
	}

	if (depth < 1)
		return CaptureSearch(alpha,beta);

	nodes++;
	
	if ((nodes & 4095) == 0)
	{
		CheckUp();
	}

	if (ply > MAX_PLY-2)
		return Eval();

	move bestmove;

	int bestscore = -10001;

	int check = 0;

	if (Attack(xside,NextBit(bit_pieces[side][K]))) 
	{
		check = 1;
	}
	Gen(side,xside);

	if(LookUp(side))
	  SetHashMove();

	int c = 0;
	int x;
	int d;
		
	for (int i = first_move[ply]; i < first_move[ply + 1]; ++i) 
	{
			Sort(i);

			if (!MakeMove(move_list[i].start,move_list[i].dest))
			{
				continue;
			}
			c++;
	
		if (Attack(xside,NextBit(bit_pieces[side][K]))) 
		{
			d = depth;
		}
		else
		{
		    d = depth - 3;
			if(move_list[i].score > CAPTURE_SCORE || c==1 || check==1)
			{
			    d = depth - 1;
			}
			else if(move_list[i].score > 0) 
			{
				d = depth - 2;
			}
		}

		x = -Search(-beta, -alpha, d);
	
		TakeBack();

		if(x > bestscore)
		{
			bestscore = x;
			bestmove = move_list[i];
		}
		if (x > alpha) 
		{
			if (x >= beta)
			{
				if(!(mask[move_list[i].dest] & bit_all))
					history[move_list[i].start][move_list[i].dest] += depth;
					AddHash(side, move_list[i]);
				return beta;
			}
		alpha = x;
		}
	}	
	
	if (c == 0) 
	{
		if (Attack(xside,NextBit(bit_pieces[side][K]))) 
		{
			return -10000 + ply;
		}
		else
			return 0;
	    }
	
	if (fifty >= 100)
		return 0;
	AddHash(side, bestmove);

	
	//DEBUG	///////////////////////////////////////////////	
	if (depth > debugDepth)
	{
		if (!moveListFlag)
		{
			for (int i = 0;; i++)
			{
				if (move_list[i].start == 0 || move_list[i].dest == 0)
					break;
				streamToLogSearch << "move_list(" << i + 1 << "): " << "\t"
					<< MoveString(move_list[i].start, move_list[i].dest, move_list[i].promote) << std::endl;
			}
			streamToLogSearch << std::endl;
			moveListFlag = true;
		}
		
		debugDepth++;
		streamToLogSearch << "depth " << std::setw(3) << depth << "\t"
						  << MoveString(bestmove.start, bestmove.dest, bestmove.promote)
				          << "\t" << " score " << std::setw(9) << bestmove.score << std::endl;
	}
	//DEBUG	///////////////////////////////////////////////

	return alpha;
}
/*

CaptureSearch evaluates the position. If the position is more than a queen less than
alpha (the best score that side can do) it doesn't search.
It generates all captures and does a recapture search to see if material is won.
If so, the material gain is added to the score.

*/
int CaptureSearch(int alpha,int beta)
{
nodes++;

int x = Eval();

if (x > alpha)
{
	if(x >= beta)
	{	
		return beta;
	}
	alpha = x;
}
else if(x + 900 < alpha)
	return alpha;

int score = 0, bestmove = 0;
int best = 0;

GenCaptures(side,xside);

for (int i = first_move[ply]; i < first_move[ply + 1]; ++i) 
{
		Sort(i);

		if(x + piece_value[board[move_list[i].dest]] < alpha)
		{
			continue;
		}

	    score = ReCaptureSearch(move_list[i].start, move_list[i].dest);
		
		if(score>best)
		{
			best = score;
			bestmove = i;
		}
}

if(best>0)
{
	x += best;
}
if (x > alpha) 
{
	if (x >= beta)
	{	
		if(best>0)
			AddHash(side, move_list[bestmove]);
		return beta;
	}
	return x;
}
return alpha;
}
/*

ReCaptureSearch searches the outcome of capturing and recapturing on the same square.
It stops searching if the value of the capturing piece is more than that of the
captured piece and the next attacker. For example, a White queen could take a rook, but a
bishop could take the queen. Even if White could take the bishop, its not worth exchanging a
queen for rook and bishop.

*/
int ReCaptureSearch(int a,const int sq)
{				
int b;
int c = 0;
int t = 0;
int score[12];

memset(score,0,sizeof(score));
score[0] = piece_value[board[sq]]; 
score[1] = piece_value[board[a]];

int total_score = 0;

while(c < 10)
{
	if(!MakeRecapture(a,sq))
			break;
	t++;
	nodes++;
	c++;

	b = LowestAttacker(side,xside,sq);
	if(b>63)
		b = LowestAttacker(side,xside,sq);

	if(b>-1)
	{
		score[c + 1] = piece_value[board[b]]; 
		if(score[c] > score[c - 1] + score[c + 1])
		{
			c--;
			break;
		}
	}
	else
	{
		break;
	}
	a = b;
}

while(c>1)
{
	if(score[c-1] >= score[c-2])
		c -= 2;
	else
		break;
}

for(int x=0; x < c; x++)
{
if(x%2 == 0)
	total_score += score[x];
else
	total_score -= score[x];
}

while(t)
{
	UnMakeRecapture();
	t--;
}

return total_score;
}
/*

reps2() searches backwards for an identical position.
A positions are identical if the key and lock are the same.
'fifty' represents the number of moves made since the last pawn move or capture.

*/
int reps2()
{
	for (int i = hply-4; i >= hply-fifty; i-=2)
	{
		if (game_list[i].hash == currentkey && game_list[i].lock == currentlock)
			return 1;
	}
	return 0;
}

/*

Sort searches the move list for the move with the highest score.
It is moved to the top of the list so that it will be played next.

*/
void Sort(const int from)
{
	move g;

	int bs = move_list[from].score;
	int bi = from;
	for (int i = from + 1; i < first_move[ply + 1]; ++i)
		if (move_list[i].score > bs) 
		{
			bs = move_list[i].score;
			bi = i;
		}
	
	g = move_list[from];
	move_list[from] = move_list[bi];
	move_list[bi] = g;
}
/*

checkup checks to see if the time has run out.
If so, the search ends.

*/
void CheckUp()
{
	if( (GetTime() >= stop_time || (max_time<50 && ply>1)) && fixed_depth==0 && ply>1)
	{
		stop_search = true;
		longjmp(env, 0);
	}
}
/*

SetHashMove searches the move list for the move from the Hash Table.
If it finds it, it sets the move a high score so that it will be played first.

*/
void SetHashMove()
{

for(int x=first_move[ply];x < first_move[ply+1];x++)
{
 if(move_list[x].start == hash_start && move_list[x].dest == hash_dest)
 {
	move_list[x].score = HASH_SCORE;
	return;
  }
}

}
/*

DisplayPV displays the principal variation(PV). This is the best line of play by both sides.
Firstly it displays the best move at the root. 
It plays this move so that the current hash key and lock will be correct.
It looks up the Hash Table and finds the best move at the greater depth and
continues until no more best moves can be found.
Lastly, it takes back the moves, returning to the original position.

*/
void DisplayPV(int i)
{
	move_start = hash_start;
	move_dest = hash_dest;

	for(int x=0;x < i;x++)
	{
		if(LookUp(side)==false)
			break;
		printf(" ");
		Alg(hash_start,hash_dest); // The function Algebraic displays a square
		MakeMove(hash_start,hash_dest);
	}
	while (ply)
		TakeBack(); // TakeBack is the opposite of MakeMove
}