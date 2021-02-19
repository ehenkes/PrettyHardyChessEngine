﻿#include "stdafx.h"
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <signal.h>
#include <windows.h>
#include <iostream>
#include <fstream>
#include <sys/timeb.h>
#include <time.h>
#include "globals.h"

void ShowHelp();
void SetUp();
void UCI();

int board_color[64] =
{
  1, 0, 1, 0, 1, 0, 1, 0,
  0, 1, 0, 1, 0, 1, 0, 1,
  1, 0, 1, 0, 1, 0, 1, 0,
  0, 1, 0, 1, 0, 1, 0, 1,
  1, 0, 1, 0, 1, 0, 1, 0,
  0, 1, 0, 1, 0, 1, 0, 1,
  1, 0, 1, 0, 1, 0, 1, 0,
  0, 1, 0, 1, 0, 1, 0, 1
};

int LoadDiagram(char* file, int);

/// <summary>
///  Eigene ParseFEN Funktion
/// </summary>
/// <param name="ts"></param>
int ParseFEN(const char* ts);
char ts[200];

void CloseDiagram();

FILE* diagram_file;
char fen_name[256];

int flip = 0;

int computer_side;
int player[2];

int fixed_time;
int fixed_depth;
U64 max_time;
U64 start_time;
U64 stop_time;
int max_depth;
int turn = 0;

void PrintResult();
void NewGame();
void SetMaterial();
void SetBits();

extern int move_start, move_dest;

const std::string currentDateTime() {
    time_t     now = time(0);
    struct tm  tstruct;
    char       buf[80];
    tstruct = *localtime(&now);
    strftime(buf, sizeof(buf), "%Y-%m-%d %X", &tstruct);
    return buf;
}

int main()
{
    SetBits();
    printf("Pretty Hardy Chess Engine\n");
    printf("Version 0.1\n");

    char s[256];
    //char sFen[256];  // wird nirgends verwendet
    //char sText[256]; // wird nirgends verwendet

    //int m;           // wird nirgends verwendet
    //int turns = 0;   // wird nirgends verwendet
    //int t;           // wird nirgends verwendet
    //int lookup;      // wird nirgends verwendet

    //double nps;      // wird nirgends verwendet

    fixed_time = 0;

    SetUp(); // setzt z.B. maximale Halbzugtiefe

    while (true)
    {
        if (scanf("%s", s) == EOF)
            return 0;

        if (!strcmp(s, "uci")) // Universal Chess Interface
        {
            UCI();
            break;
        }
    }
    Free();
    return 0;
}

/*
DisplayBoard() displays the board
The console object is only used to display in colour.
*/
void DisplayBoard()
{
    HANDLE hConsole;
    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    int text = 15;

    int i;
    int x = 0;
    int c;

    if (flip == 0)
        printf("\n8 ");
    else
        printf("\n1 ");

    for (int j = 0; j < 64; ++j)
    {
        if (flip == 0)
            i = Flip[j];
        else
            i = 63 - Flip[j];
        c = EMPTY;
        if (bit_units[White] & mask[i]) c = White;
        if (bit_units[Black] & mask[i]) c = Black;
        switch (c)
        {
            //https://docs.microsoft.com/de-de/windows/console/console-screen-buffers#character-attributes
        case EMPTY:
            if (board_color[i] == 0)
                text = 127;
            else
                text = 34;
            SetConsoleTextAttribute(hConsole, text);

            printf("  ");
            SetConsoleTextAttribute(hConsole, 15);
            break;
        case White:
            if (board_color[i] == White)
                text = 126;
            else
                text = 46;
            SetConsoleTextAttribute(hConsole, text);
            printf(" %c", piece_char[board[i]]);
            SetConsoleTextAttribute(hConsole, 15);
            break;

        case Black:
            if (board_color[i] == White)
                text = 112;
            else
                text = 32;
            SetConsoleTextAttribute(hConsole, text);
            printf(" %c", piece_char[board[i]] + ('a' - 'A'));
            SetConsoleTextAttribute(hConsole, 15);
            break;

        default:
            printf(" %d.", c);
            break;

        }
        if ((bit_all & mask[i]) && board[i] == 6)
            if (x == 0)
                printf(" %d", c);
            else
                printf("%d ", c);
        if (board[i] < 0 || board[i]>6)
            if (x == 0)
                printf(" %d.", board[i]);
            else
                printf("%d ", board[i]);
        if (flip == 0)
        {
            if ((j + 1) % 8 == 0 && j != 63)
                printf("\n%d ", row[i]);
        }
        else
        {
            if ((j + 1) % 8 == 0 && row[i] != 7)
                printf("\n%d ", row[j] + 2);
        }
    }
    if (flip == 0)
        printf("\n\n   a b c d e f g h\n\n");
    else
        printf("\n\n   h g f e d c b a\n\n");
}

int ParseMove(char* s)
{
    int start, dest, i;

    if (s[0] < 'a' || s[0] > 'h' ||
        s[1] < '0' || s[1] > '9' ||
        s[2] < 'a' || s[2] > 'h' ||
        s[3] < '0' || s[3] > '9')
        return -1;

    start = s[0] - 'a';
    start += ((s[1] - '0') - 1) * 8;
    dest = s[2] - 'a';
    dest += ((s[3] - '0') - 1) * 8;

    for (i = 0; i < first_move[1]; ++i)
        if (move_list[i].start == start && move_list[i].dest == dest)
        {
            if (s[4] == 'n' || s[4] == 'N')
                move_list[i].promote = 1;
            if (s[4] == 'b' || s[4] == 'B')
                move_list[i].promote = 2;
            else if (s[4] == 'r' || s[4] == 'R')
                move_list[i].promote = 3;
            return i;
        }
    return -1;
}

/*
  UCI Implementation
*/
void UCI()
{
    char line[2048], command[256], parameter[256];
    int m;
    int post = 0;
    int analyze = 0;
    int lookup;
    bool gameIsRunning = true;
    bool optionalCommand = false;

    signal(SIGINT, SIG_IGN);
    printf("\n");
    NewGame();
    fixed_time = 0;

    std::cout << "id name PrettyHardyChessmaster Feb 2021\n"
        << "id author E.Henkes, P.Puntschart (inspired by code of Bill Jordan)\n"
        << "option name UCI_Chess960 0\n"
        << "option name Threads 1\n"
        << "option name Hash type spin default 128 min 16 max 2048\n"
        << "option name Syzygy50MoveRule 1\n"
        << "option name Ponder 0\n"
        << "uciok" << std::endl;

    std::fstream streamToLog;
    streamToLog.open("log.txt", std::ios::out);

    while (gameIsRunning)
    {
        fflush(stdout);

        if (!optionalCommand) {
            std::cin >> command;
        }
        else {
            optionalCommand ^= 1;
        }

        streamToLog << currentDateTime() << ": " << command << std::endl;

        if (!strcmp(command, "isready"))
        {
            std::cout << "readyok" << std::endl;
            continue;
        }

        if (!strcmp(command, "setoption"))
        {
            std::cin >> parameter;
            streamToLog << "\t" << parameter << std::endl;
            continue;
        }

        if (!strcmp(command, "ucinewgame"))
        {
            NewGame();
            computer_side = EMPTY;
            continue;
        }

        if (!strcmp(command, "position"))
        {
            std::cin >> parameter;
            streamToLog << "\t" << parameter << std::endl;

            if (!strcmp(parameter, "startpos"))
            {
                std::cin >> parameter;
                streamToLog << "\t" << parameter << std::endl;

                if (strcmp(parameter, "moves")) {
                    strcpy(command, parameter);
                    computer_side = White;
                    optionalCommand = true;
                    continue;
                }
                else {
                    if (!fgets(line, 2048, stdin)) {
                        gameIsRunning = false;
                        break;
                    }

                    if (line[0] == '\n')
                        continue;

                    int last_space = 0;

                    for (int i = 0; i < 2048; i++) {
                        if (line[i] == '\n') {
                            break;
                        }
                        if (line[i] == ' ') {
                            last_space = i;
                        }
                    }

                    ply = 0;
                    first_move[0] = 0;
                    Gen(side, xside);
                    m = ParseMove(&line[last_space + 1]);
                    if (m == -1 || !MakeMove(move_list[m].start, move_list[m].dest))
                    {
                        printf("The engine did not understand the given moves: \n");
                        printf(line);
                        printf("\n");
                        MoveString(move_list[m].start, move_list[m].dest, move_list[m].promote);
                    }
                    if (game_list[hply - 1].promote > 0 && (row[move_list[m].dest] == 0 || row[move_list[m].dest] == 7))
                    {
                        RemovePiece(xside, Q, move_list[m].dest);
                        if (line[4] == 'n' || line[4] == 'N')
                            AddPiece(xside, N, move_list[m].dest);
                        else if (line[4] == 'b' || line[4] == 'B')
                            AddPiece(xside, B, move_list[m].dest);
                        else if (line[4] == 'r' || line[4] == 'R')
                            AddPiece(xside, R, move_list[m].dest);
                        else AddPiece(xside, Q, move_list[m].dest);
                    }
                }
            }

            /// 
            /// Forsyth-Edwards-Notation (FEN) auslesen
            /// 
            if (!strcmp(parameter, "fen"))
            {
                std::string fen, input;
                getline(std::cin, input);
                fen = input.substr(1, fen.length() - 1); // vorne wird ein space eingelesen
                streamToLog << "\t" << fen << std::endl;

                // fen umsetzen in Position, Rochade-Optionen, Seite am Zug ...
                // https://de.wikipedia.org/wiki/Forsyth-Edwards-Notation#:~:text=Die%20Forsyth%2DEdwards%2DNotation%20(,Jahrhundert%20popul%C3%A4r.

                ParseFEN(fen.c_str()); 
            }            

            continue;
        }

        if (!strcmp(command, "go"))
        {
            computer_side = side;
            std::cin >> parameter;
            if (!strcmp(parameter, "movetime"))
            {
                int value;
                std::cin >> value;
                streamToLog << currentDateTime() << ": "
                    << "Movetime wurde auf '" << value << "' ms gesetzt." << std::endl;
                printf(" Engine received movetime ");
                max_time = value; // values are given in [ms]
                fixed_time = 1;
                max_depth = MAX_PLY;
            }
            else if (!strcmp(parameter, "depth"))
            {
                int value;
                std::cin >> value;
                streamToLog << currentDateTime() << ": "
                    << "Depth wurde auf '" << value << " Halbzuege gesetzt." << std::endl;
                printf(" Engine received depth ");
                max_depth = value; // values are given in plies (Halbzuege)
                fixed_depth = 1;                
            }
            else if (!strcmp(parameter, "infinite")) 
            {
                streamToLog << currentDateTime() << ": "
                    << "Depth und Movetime wurde auf infinite gesetzt." << std::endl;
                printf(" Engine received infinite. We use MAXDEPTH.");
                max_depth = MAXDEPTH; // values are given in plies (Halbzuege)
                fixed_depth = 1;                
            }
            else 
            {
                strcpy(command, parameter);
                optionalCommand = true;
            }
        }

        if (side == computer_side)
        {
            think();
            SetMaterial();
            Gen(side, xside);
            currentkey = GetKey();
            currentlock = GetLock();
            lookup = LookUp(side);

            if (move_start != 0 || move_dest != 0)
            {
                hash_start = move_start;
                hash_dest = move_dest;
            }
            else
                printf(" lookup=0 ");

            move_list[0].start = hash_start;
            move_list[0].dest = hash_dest;
            printf("bestmove %s\n", MoveString(hash_start, hash_dest, 0));

            MakeMove(hash_start, hash_dest);

            ply = 0;
            Gen(side, xside);
            PrintResult();
            continue;
        }

        if (!strcmp(command, "ponderhit")) {
            // Noch nicht implementiert
            continue;
        }

        if (!strcmp(command, "quit") || !strcmp(command, "stop"))
        {
            gameIsRunning = false;            
        }            
    }//while

    streamToLog.close(); // file fuer log-Datei schliessen
}

void PrintResult()
{
    int i;
    int flag = 0;

    SetMaterial();
    Gen(side, xside);
    for (i = 0; i < first_move[1]; ++i)
        if (MakeMove(move_list[i].start, move_list[i].dest))
        {
            TakeBack();
            flag = 1;
            break;
        }

    if (pawn_mat[0] == 0 && pawn_mat[1] == 0 && piece_mat[0] <= 300 && piece_mat[1] <= 300)
    {
        printf("1/2-1/2 {Stalemate}\n");

        NewGame();
        computer_side = EMPTY;
        return;
    }
    if (i == first_move[1] && flag == 0)
    {
        Gen(side, xside);
        DisplayBoard();
        printf(" end of game ");

        if (Attack(xside, NextBit(bit_pieces[side][K])))
        {
            if (side == 0)
            {
                printf("0-1 {Black mates}\n");
            }
            else
            {
                printf("1-0 {White mates}\n");
            }
        }
        else
        {
            printf("1/2-1/2 {Stalemate}\n");
        }
        NewGame();
        computer_side = EMPTY;
    }
    else if (reps() >= 3)
    {
        printf("1/2-1/2 {Draw by repetition}\n");
        NewGame();
        computer_side = EMPTY;
    }
    else if (fifty >= 100)
    {
        printf("1/2-1/2 {Draw by fifty move rule}\n");
        NewGame();
        computer_side = EMPTY;
    }
}

int reps()
{
    int r = 0;

    for (int i = hply; i >= hply - fifty; i -= 2)
        if (game_list[i].hash == currentkey && game_list[i].lock == currentlock)
            r++;
    return r;
}

int LoadDiagram(char* file, int num) // wird nicht verwendet
{
    int x, n = 0;
    static int count = 1;
    char ts[200];

    diagram_file = fopen(file, "r");
    if (!diagram_file)
    {
        printf("Diagram missing.\n");
        return -1;
    }

    strcpy_s(fen_name, file);

    for (x = 0; x < num; x++)
    {
        fgets(ts, 256, diagram_file);
        if (!ts) break;
    }

    for (x = 0; x < 64; x++)
    {
        board[x] = EMPTY;
    }
    memset(bit_pieces, 0, sizeof(bit_pieces));
    memset(bit_units, 0, sizeof(bit_units));
    bit_all = 0;

    int c = 0, i = 0, j;

    while (ts)
    {
        if (ts[c] >= '0' && ts[c] <= '8')
            i += ts[c] - 48;
        if (ts[c] == '\\')
            continue;
        j = Flip[i];

        switch (ts[c])
        {
        case 'K': AddPiece(0, 5, j); i++; break;
        case 'Q': AddPiece(0, 4, j); i++; break;
        case 'R': AddPiece(0, 3, j); i++; break;
        case 'B': AddPiece(0, 2, j); i++; break;
        case 'N': AddPiece(0, 1, j); i++; break;
        case 'P': AddPiece(0, 0, j); i++; break;
        case 'k': AddPiece(1, 5, j); i++; break;
        case 'q': AddPiece(1, 4, j); i++; break;
        case 'r': AddPiece(1, 3, j); i++; break;
        case 'b': AddPiece(1, 2, j); i++; break;
        case 'n': AddPiece(1, 1, j); i++; break;
        case 'p': AddPiece(1, 0, j); i++; break;
        }
        c++;
        if (ts[c] == ' ')
            break;
        if (i > 63)
            break;
    }
    if (ts[c] == ' ' && ts[c + 2] == ' ')
    {
        if (ts[c + 1] == 'w')
        {
            side = 0; xside = 1;
        }
        if (ts[c + 1] == 'b')
        {
            side = 1; xside = 0;
        }
    }

    castle = 0;
    while (ts[c])
    {
        switch (ts[c])
        {
        case '-': break;
        case 'K':if (bit_pieces[0][K] & mask[E1]) castle |= 1; break;
        case 'Q':if (bit_pieces[0][K] & mask[E1]) castle |= 2; break;
        case 'k':if (bit_pieces[1][K] & mask[E8]) castle |= 4; break;
        case 'q':if (bit_pieces[1][K] & mask[E8]) castle |= 8; break;
        default:break;
        }
        c++;
    }

    CloseDiagram();
    DisplayBoard();
    NewPosition();
    Gen(side, xside);
    printf(" diagram # %d \n", num + count);
    count++;
    if (side == 0)
        printf("White to move\n");
    else
        printf("Black to move\n");
    printf(" %s \n", ts);
    return 0;
}

int ParseFEN(const char* ts)
{
    int x, n = 0;
    static int count = 1;    

    for (x = 0; x < 64; x++)
    {
        board[x] = EMPTY;
    }
    memset(bit_pieces, 0, sizeof(bit_pieces));
    memset(bit_units, 0, sizeof(bit_units));
    bit_all = 0;

    int c = 0, i = 0, j;

    while (ts)
    {
        if (ts[c] >= '0' && ts[c] <= '8')
            i += ts[c] - 48;
        if (ts[c] == '\\')
            continue;
        j = Flip[i];

        switch (ts[c])
        {
            case 'K': AddPiece(0, 5, j); i++; break;
            case 'Q': AddPiece(0, 4, j); i++; break;
            case 'R': AddPiece(0, 3, j); i++; break;
            case 'B': AddPiece(0, 2, j); i++; break;
            case 'N': AddPiece(0, 1, j); i++; break;
            case 'P': AddPiece(0, 0, j); i++; break;
            case 'k': AddPiece(1, 5, j); i++; break;
            case 'q': AddPiece(1, 4, j); i++; break;
            case 'r': AddPiece(1, 3, j); i++; break;
            case 'b': AddPiece(1, 2, j); i++; break;
            case 'n': AddPiece(1, 1, j); i++; break;
            case 'p': AddPiece(1, 0, j); i++; break;
        }
        c++;
        if (ts[c] == ' ')
            break;
        if (i > 63)
            break;
    }
    
    // Welche Seite ist am Zug?
    if (ts[c] == ' ' && ts[c + 2] == ' ')
    {
        if (ts[c + 1] == 'w') // Weiß zieht
        {
            side = 0; xside = 1;
        }
        if (ts[c + 1] == 'b') // Schwarz zieht
        {
            side = 1; xside = 0;
        }
    }

    // Welche Rochaden sind bei Weiß und Schwarz erlaubt?
    castle = 0;
    while (ts[c])
    {
        switch (ts[c])
        {
        case '-': break;
        case 'K':if (bit_pieces[0][K] & mask[E1]) castle |= 1; break;
        case 'Q':if (bit_pieces[0][K] & mask[E1]) castle |= 2; break;
        case 'k':if (bit_pieces[1][K] & mask[E8]) castle |= 4; break;
        case 'q':if (bit_pieces[1][K] & mask[E8]) castle |= 8; break;
        default:break;
        }
        c++;
    }

    CloseDiagram();
    DisplayBoard();
    NewPosition();
    Gen(side, xside);
    // printf(" diagram # %d \n", num + count);
    count++;
    if (side == 0)
        printf("White to move\n");
    else
        printf("Black to move\n");
    printf(" %s \n", ts);
    return 0;
}

void CloseDiagram()
{
    if (diagram_file)
        fclose(diagram_file);
    diagram_file = NULL;
}

void ShowHelp()
{
    printf("d - Displays the board.\n");
    printf("f - Flips the board.\n");
    printf("go - Starts the engine.\n");
    printf("help - Displays help on the commands.\n");
    printf("moves - Displays of list of possible moves.\n");
    printf("new - Starts a new game .\n");
    printf("off - Turns the computer player off.\n");
    printf("on or p - The computer plays a move.\n");
    printf("sb - Loads a fen diagram.\n");
    printf("sd - Sets the search depth.\n");
    printf("st - Sets the time limit per move in seconds.\n");
    printf("sw - Switches sides.\n");
    printf("quit - Quits the program.\n");
    printf("undo - Takes back the last move.\n");
    printf("xboard - Starts xboard.\n");
}

void SetUp()
{
    RandomizeHash();
    SetTables();
    SetMoves();
    InitBoard();
    computer_side = EMPTY;
    player[0] = 0;
    player[1] = 0;
    max_time = 1 << 25;
    max_depth = MAXDEPTH;
}

void NewGame()
{
    InitBoard();
    Gen(side, xside);
}

void SetMaterial()
{
    int c;
    pawn_mat[0] = 0;
    pawn_mat[1] = 0;
    piece_mat[0] = 0;
    piece_mat[1] = 0;
    for (int x = 0; x < 64; x++)
    {
        if (board[x] < 6)
        {
            if (bit_units[0] & mask[x])
                c = 0;
            else
                c = 1;
            if (board[x] == 0)
                pawn_mat[c] += 100;
            else
                piece_mat[c] += piece_value[board[x]];
        }
    }
}

U64 GetTime() // integer als return value erscheint unpassend!?
{
    struct timeb timebuffer;
    ftime(&timebuffer);
    return (timebuffer.time * 1000) + timebuffer.millitm;
}

char* MoveString(int start, int dest, int promote)
{
    static char str[6];

    char c;

    if (promote > 0) {
        switch (promote) {
        case N:
            c = 'n';
            break;
        case B:
            c = 'b';
            break;
        case R:
            c = 'r';
            break;
        default:
            c = 'q';
            break;
        }
        sprintf_s(str, "%c%d%c%d%c",
            col[start] + 'a',
            row[start] + 1,
            col[dest] + 'a',
            row[dest] + 1,
            c);
    }
    else
        sprintf_s(str, "%c%d%c%d",
            col[start] + 'a',
            row[start] + 1,
            col[dest] + 'a',
            row[dest] + 1);
    return str;
}