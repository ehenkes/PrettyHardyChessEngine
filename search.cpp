#include "stdafx.h"
#include "globals.h"

#define DEBUG // zur Ausgabe in ein Logfile

jmp_buf env;
bool stop_search;
int currentmax;
int move_start, move_dest, move_promote;
int LowestAttacker(const int s, const int xs,const int sq);
void SetHashMove();
void DisplayPV(int i);

#ifdef DEBUG
static int debugDepth = 0; //DEBUG
static bool moveListFlag = false; //DEBUG
static int oldHply = 0; //DEBUG
std::fstream streamToLogSearch("logSearch.txt", std::ios::out); //DEBUG

void DisplayBoardToFile(std::fstream& logfile, int flip)
{
	int i, c, x = 0;

	if (flip == 0)
		logfile << "\n8 ";
	else
		logfile << "\n1 ";

	for (int j = 0; j < 64; ++j)
	{
		if (flip == 0)
		{
			i = Flip[j];
		}
		else
		{
			i = 63 - Flip[j];
		}

		c = EMPTY;
		if (bit_units[White] & mask[i]) c = White;
		if (bit_units[Black] & mask[i]) c = Black;
		switch (c)
		{
		case EMPTY:
			logfile << "  ";
			break;
		case White:
			logfile << " " << (char)(piece_char[board[i]]);
			break;
		case Black:
			logfile << " " << (char)(piece_char[board[i]] + ('a' - 'A'));
			break;
		default:
			logfile << " " << (int)c;
			break;
		}

		if ((bit_all & mask[i]) && board[i] == 6)
			if (x == 0)
				logfile << " " << (int)c;
			else
				logfile << (int)c << " ";

		if (board[i] < 0 || board[i]>6)
			if (x == 0)
			{
				logfile << " " << board[i];
			}
			else
			{
				logfile << board[i] << " ";
			}

		if (flip == 0)
		{
			if ((j + 1) % 8 == 0 && j != 63)
				logfile << "\n" << row[i] << " ";
		}
		else
		{
			if ((j + 1) % 8 == 0 && row[i] != 7)
				logfile << "\n" << row[j] + 2 << " ";
		}
	}

	if (flip == 0)
		logfile << "\n\n   a b c d e f g h\n\n";
	else
		logfile << "\n\n   h g f e d c b a\n\n";
}

int ShowCastleRights(std::fstream& logfile)
{
	if (castle & 1 || castle & 2) logfile << "w: ";
	if (castle & 1) logfile << "0-0  ";
	if (castle & 2) logfile << " 0-0-0  ";
	if (castle & 4 || castle & 8) logfile << "b: ";
	if (castle & 4) logfile << "0-0  ";
	if (castle & 8) logfile << " 0-0-0";
	return castle;
}
#endif

/*

think() iterates until the maximum depth for the move is reached or until the allotted time
has run out. After each iteration the principal variation is then displayed.

*/
void think()
{
#ifdef DEBUG
	debugDepth = 0; //DEBUG
	moveListFlag = false; //DEBUG
	streamToLogSearch << "\n" << "-------------" << std::endl; //DEBUG
#endif

	int x;
	stop_search = false;
	setjmp(env); //If the allotted time for the move runs out, program flow jumps to this line
				 // and restores the game position and ends the search.

	if (stop_search)
	{
		while (ply)
			TakeBack();
		return;
	}

	// If a fixed time for each move has not been set, 
	// then the allotted time is halved for moves that are check evasions or likely recaptures.
	if (fixedLevel != FIXEDLEVEL::TIME)
	{
		if (Attack(xside, NextBit(bit_pieces[side][K])))
			//max_time /= 2;
			max_time /= 4; //acceleration test
	}

	start_time = GetTime(); //The starting time is stored.
	stop_time = start_time + max_time;

	ply = 0;
	nodes = 0;

	NewPosition(); // NewPosition gets the board ready before the computer starts to think.
	memset(history, 0, sizeof(history)); //History tables are cleared.
	
	std::cout << "ply    score         time\t\t\t nodes\t principal variation" << std::endl;
	for (int i = 1; i <= max_depth; ++i)
	{
		// It iterates until the maximum depth for the move is reached or until the allotted time has run out. 
		currentmax = i;
		if (fixedLevel != FIXEDLEVEL::DEPTH && max_depth > 1)
		{
			if (fixedLevel == FIXEDLEVEL::NODES)
			{
				if (nodes > max_nodes)
				{
					stop_search = true;
					return;
				}
			}
			else if (fixedLevel == FIXEDLEVEL::TIME)
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

		// Each iteration it calls search(). alpha is set low while beta is set high. 
		// alpha increases while beta decreases. They move closer together.
		x = Search(-10000, 10000, i); // alpha, beta, depth 

		// output for UCI
		if (LookUp(side))
		{
			std::cout << "info depth " << i << " score cp " << x << " nodes " << nodes << " pv";
			DisplayPV(i);
			std::cout << std::endl;
		}

		/*
		printf("%0*d\t%*d\t  %*llu\t",
			3, i, //depth
			4, x, //score
			7, (GetTime() - start_time) / 10, //time
			20, nodes); //nodes
		*/

		if (LookUp(side))
		{
			DisplayPV(i);
		}
		else
		{
			move_start = 0;
			move_dest = 0;
		}
		std::cout << std::endl;

		if (x > 9000 || x < -9000) // If the score is greater than 9000 it means a forced mate has been found, 
		{
			std::cout << "info string A forced mate has been found!" << std::endl;
			break; //in which case it stops searching.
		}
	}
}

/*
search is the main part of the search.
*/
int Search(int alpha, int beta, int depth)
{
	if (ply && reps2()) // If the position is repeated we don't need to look any further.
		return 0;
		
	if (depth < 1) // If depth has run out, the capture search is done.
		return CaptureSearch(alpha,beta);

	nodes++;
	if ((nodes & 4095) == 0) // Every 4,000 positions approx the time is checked.
		CheckUp(); // checkup checks to see if the time has run out. If so, the search ends.
	
	if (ply > MAX_PLY-2)
		return Eval();
	
	// Moves are generated. The moves are looped through in order of their score.
	move bestmove;
	int bestscore = -10001;
	int check = 0;

	if (Attack(xside,NextBit(bit_pieces[side][K]))) // Schach dem König? 
		check = 1;
	
	Gen(side,xside);

	if(LookUp(side))
	  SetHashMove();

	int c = 0, x, d;
		
	for (int i = first_move[ply]; i < first_move[ply + 1]; ++i) 
	{
		Sort(i);

		if (!MakeMove(move_list[i].start,move_list[i].dest)) // If a move is illegal
		{
			continue; // then it is skipped over.
		}
		c++;
	
		if (Attack(xside,NextBit(bit_pieces[side][K]))) // If the move is check, we extend by one ply.
		{									// This is done by not changing depth in the call to search.
			d = depth - 1; // Auf normale Suche abgeändert, da dies bei der Suche nach legalen Zügen massiv stört!!!
		}
		else
		{
		    d = depth - 3; 
			if(move_list[i].score > CAPTURE_SCORE || c==1 || check==1) // If it has a score greater than zero, 
				                                                       // ply is one or the move number is less than 12
			{                                                          // a normal search is done. 
			    d = depth - 1; // This is done by subtracting 1 from depth.
			}
			else if(move_list[i].score > 0) // Otherwise we reduce by one ply.
			{
				d = depth - 2; // This is done by subtracting 2 from depth. 
			}
		}
				
		x = -Search(-beta, -alpha, d);
		TakeBack(); // The move is taken back.
		
		if(x > bestscore)
		{
			bestscore = x;
			bestmove = move_list[i];
			move_promote = bestmove.promote; //TEST
		}

		if (x > alpha) // if it is greater than alpha, alpha is changed.
		{
			// If the score from search is greater than beta, a beta cutoff happens.
		    // No need to search any moves at this depth.
			if (x >= beta)
			{
				if (!(mask[move_list[i].dest] & bit_all))
					history[move_list[i].start][move_list[i].dest] += depth;
				AddHash(side, move_list[i]);
				return beta;
			}
		alpha = x;
		}
	}	
	
	// If there were no legal moves it is either checkmate or stalemate.
	if (c == 0) 
	{
		if (Attack(xside,NextBit(bit_pieces[side][K]))) 
		{
			return -10000 + ply;
		}
		else
			return 0;
	    }
	
	if (fifty >= 100) // 50-Züge-Remis-Regel
		return 0;
	AddHash(side, bestmove);

		
#ifdef DEBUG
	//Alle Züge (an dieser Stelle auch noch illegale!!!) eines Halbzuges werden in die Log-Datei "logSearch.txt" ausgegeben:
	if (depth > debugDepth)
	{
		if (!moveListFlag)
		{			
			//The move list for ply 1 starts at 0 and ends just before first_move[1].
			//The move list for ply 2 starts at first_move[1] and ends just before first_move[2].
			//The move list for ply 3 starts at first_move[2] and ends just before first_move[3]. etc. 
			//So the move list for any ply is between first_move[ply] and first_move[ply + 1].
			//Jordan, FM Bill. How to Write a Bitboard Chess Engine: How Chess Programs Work
			
			bool illegalFlag = false;
			
			int savedPly   = ply;
			int savedHPly  = hply;
			int savedSide  = side;
			int savedXSide = xside;
			int savedFifty = fifty;

			streamToLogSearch << "hply: " << hply << std::endl;
			
			for (int i = 0; i < hply; i++)
			{
				if ((i > 0) && (i % 20 == 0))
				{
					streamToLogSearch << "\n";
				}
				streamToLogSearch << MoveString(game_list[i].start, game_list[i].dest, game_list[i].promote) << " ";									
			}
			streamToLogSearch << "\n\n-------------------------" << std::endl;

			int countLegalMoves = 0;

			for (int i = first_move[ply]; i < first_move[ply+1]; ++i) 
			{
				if (MakeMove(move_list[i].start, move_list[i].dest))
				{
					// legaler Zug
					countLegalMoves++;
					streamToLogSearch << "legal(" << std::setw(2) << i + 1 << "): " << "\t"
						<< MoveString(move_list[i].start, move_list[i].dest, move_list[i].promote) << " " << move_list[i].score
						<< std::endl;
					
					int retVal = TakeBack(); 
					// if (retVal == 1) {streamToLogSearch << "Figurenschlagen wurde rückgängig gemacht." << std::endl;}
					// als Beispiel zur Kontrolle von TakeBack()
				}
				else
				{
					// illegaler Zug (nicht notwendig für NN, nur zur Kontrolle)
					streamToLogSearch << ">> illegal(" << std::setw(2) << i + 1 << "): " << "\t"
						          << MoveString(move_list[i].start, move_list[i].dest, move_list[i].promote) 
					              << std::endl;	
					
					illegalFlag = true;					
				}
				moveListFlag = true;
			}
			streamToLogSearch << std::endl;
			
			if (illegalFlag) 				             
			{
				ply   = savedPly;
				hply  = savedHPly;
				side  = savedSide;
				xside = savedXSide;
				fifty = savedFifty;
				
				illegalFlag = false;
			}

			streamToLogSearch << "\nAnzahl legaler Züge (Mobilität und Komplexität): " << countLegalMoves << std::endl; 
			std::cout << "info string countLegalMoves " << countLegalMoves << std::endl;

			DisplayBoardToFile(streamToLogSearch, White);
			streamToLogSearch << std::endl;
			ShowCastleRights(streamToLogSearch);
			streamToLogSearch << std::endl << std::endl;
		}
		
		debugDepth++;
		streamToLogSearch << "depth " << std::setw(3) << depth << "\t"
						  << MoveString(bestmove.start, bestmove.dest, bestmove.promote)
				          << "\t" << " score " << std::setw(9) << bestmove.score << std::endl;		
	}
#endif

	return alpha;
}
/*

CaptureSearch evaluates the position. If the position is more than a queen less than
alpha (the best score that side can do) it doesn't search.
It generates all captures and does a recapture search to see if material is won.
If so, the material gain is added to the score.

*/
int CaptureSearch(int alpha, int beta)
{
	nodes++;

	int x = Eval();

	if (x > alpha)
	{
		if (x >= beta)
		{
			return beta;
		}
		alpha = x;
	}
	else if (x + 900 < alpha)
		return alpha;

	int score = 0, bestmove = 0;
	int best = 0;

	GenCaptures(side, xside);

	for (int i = first_move[ply]; i < first_move[ply + 1]; ++i)
	{
		Sort(i);

		if (x + piece_value[board[move_list[i].dest]] < alpha)
		{
			continue;
		}

		score = ReCaptureSearch(move_list[i].start, move_list[i].dest);

		if (score > best)
		{
			best = score;
			bestmove = i;
		}
	}

	if (best > 0)
	{
		x += best;
	}
	if (x > alpha)
	{
		if (x >= beta)
		{
			if (best > 0)
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
int ReCaptureSearch(int a, const int sq)
{
	int b;
	int c = 0;
	int t = 0;
	int score[12];

	memset(score, 0, sizeof(score));
	score[0] = piece_value[board[sq]];
	score[1] = piece_value[board[a]];

	int total_score = 0;

	while (c < 10)
	{
		if (!MakeRecapture(a, sq))
			break;
		t++;
		nodes++;
		c++;

		b = LowestAttacker(side, xside, sq);
		if (b > 63)
			b = LowestAttacker(side, xside, sq);

		if (b > -1)
		{
			score[c + 1] = piece_value[board[b]];
			if (score[c] > score[c - 1] + score[c + 1])
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

	while (c > 1)
	{
		if (score[c - 1] >= score[c - 2])
			c -= 2;
		else
			break;
	}

	for (int x = 0; x < c; x++)
	{
		if (x % 2 == 0)
			total_score += score[x];
		else
			total_score -= score[x];
	}

	while (t)
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
	if( (gameIsRunning == false) || (GetTime() >= stop_time) || ((max_time<50 && ply>1) && ((fixedLevel != FIXEDLEVEL::DEPTH) && ply>1)) )
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
	for (int x = first_move[ply]; x < first_move[ply + 1]; x++)
	{
		if (move_list[x].start == hash_start && move_list[x].dest == hash_dest)
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
		std::cout << " ";
		Alg(hash_start,hash_dest); // The function Algebraic displays a square
		MakeMove(hash_start,hash_dest);
	}
	
	while (ply)
		TakeBack(); // TakeBack is the opposite of MakeMove
}