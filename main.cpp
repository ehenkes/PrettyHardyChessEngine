#include "stdafx.h"
#include "globals.h"


void ShowHelp();
void SetUp();
void UCI();

void ProcessMoves(char  line[2048], int& m, std::fstream& streamToLog, bool fen=false);

void ProcessMove(int& m, char  line[2048], int last_space, std::fstream& streamToLog);

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
int ParseFEN(const char* ts, bool checkSide);
char ts[200];

void CloseDiagram();

FILE* diagram_file;
char fen_name[256];

int flip = 0;

int computer_side;
int player[2];

FIXEDLEVEL fixedLevel;

U64 max_time;
U64 start_time;
U64 stop_time;
int max_depth;
U64 max_nodes;
int turn = 0;

void PrintResult();
void NewGame();
void SetMaterial();
void SetBits();

bool gameIsRunning;

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
    gameIsRunning = true;
    SetBits();
    std::cout << "Pretty Hardy Chess Master\n" << ("Version 0.5\n") << std::endl;

    char s[256];
    
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
    /*
    HANDLE hConsole;
    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    int text = 15;

    int i;
    int x = 0;
    int c;

    if (flip == 0)
        std::cout << "\n8 ";
    else
        std::cout << "\n1 ";

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

            std::cout << "  ";
            SetConsoleTextAttribute(hConsole, 15);
            break;
        case White:
            if (board_color[i] == White)
                text = 126;
            else
                text = 46;
            SetConsoleTextAttribute(hConsole, text);
            std::cout << " " << (char)piece_char[board[i]]; 
            SetConsoleTextAttribute(hConsole, 15);
            break;

        case Black:
            if (board_color[i] == White)
                text = 112;
            else
                text = 32;
            SetConsoleTextAttribute(hConsole, text);
            std::cout << " " << (char)(piece_char[board[i]] + ('a' - 'A'));
            SetConsoleTextAttribute(hConsole, 15);
            break;

        default:
            std::cout << " " << c;
            break;
        }

        if ((bit_all & mask[i]) && board[i] == 6)
            if (x == 0)
                std::cout << " " << c;
            else
                std::cout << c << " ";
        if (board[i] < 0 || board[i]>6)
            if (x == 0)
                std::cout << " " << board[i];
            else
                std::cout << board[i] << " ";

        if (flip == 0)
        {
            if ((j + 1) % 8 == 0 && j != 63)
                std::cout << "\n" << row[i] << " "; 
        }
        else
        {
            if ((j + 1) % 8 == 0 && row[i] != 7)
                std::cout << "\n" << row[j] + 2 << " ";
        }
    }
    if (flip == 0)
        std::cout << "\n\n   a b c d e f g h\n\n";
    else
        std::cout << "\n\n   h g f e d c b a\n\n";
        */
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
                move_list[i].promote = N;
            if (s[4] == 'b' || s[4] == 'B')
                move_list[i].promote = B;
            if (s[4] == 'r' || s[4] == 'R')
                move_list[i].promote = R;
            if (s[4] == 'q' || s[4] == 'Q')
                move_list[i].promote = Q;
            return i;
        }
    return -1;
}
    
void splitString(std::string parameters, std::list<std::string>& list)
{
    std::string delimiter = " ";
    std::string token;
    size_t pos = 0;
    while ((pos = parameters.find(delimiter)) != std::string::npos)
    {
        token = parameters.substr(0, pos);
        list.push_back(token);
        parameters.erase(0, pos + delimiter.length());
    }
    list.push_back(parameters);
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
    bool optionalCommand = false;
    float timeDivider = 20;

    NewGame();
    std::stringstream ss;

    ss << "id name PrettyHardyChessmaster v0.5 May 2022\n"
       << "id author Erhard Henkes, Paul Puntschart (inspired by code of Bill Jordan)\n"
       << "option name UCI_Chess960 0\n"
       << "option name Threads 1\n"
       << "option name Hash type spin default 128 min 16 max " << MAXHASH/1000000 << " \n"
       << "option name Syzygy50MoveRule 1\n"
       << "option name Ponder 0\n"
       << "uciok" << std::endl;
    std::cout << ss.str();
    std::fstream streamToLog;
    streamToLog.open("log.txt", std::ios::out);
    streamToLog << currentDateTime() << ": ENGINE:\n" << ss.str() << std::endl;
    ss.str("");

    std::fstream streamToTest;
    streamToTest.open("bestmove_in_main.txt", std::ios::out);
    
    while (gameIsRunning)
    {        
        fflush(stdout);

        if (!optionalCommand) 
        {
            std::cin >> command;
        }
        else
        {
        optionalCommand ^= 1;
        }

        streamToLog << currentDateTime() << ": " << command << std::endl;

        if (!strcmp(command, "isready"))
        {
            ss << "readyok";
            std::cout << ss.str() << std::endl;
            streamToLog << currentDateTime() << ": ENGINE: " << ss.str() << std::endl;
            ss.str("");
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

                if (strcmp(parameter, "moves"))
                {
                    strcpy(command, parameter);
                    computer_side = White;
                    optionalCommand = true;
                    continue;
                }
                else 
                {
                    if (!fgets(line, 2048, stdin)) 
                    {
                        gameIsRunning = false;
                        break;
                    }

                    if (line[0] == '\n')
                        continue;
                    streamToLog << "\tstartpos + move sequence: " << line << std::endl;
                    ProcessMoves(line, m, streamToLog);
                }
            }

            /// 
            /// Forsyth-Edwards-Notation (FEN) auslesen
            /// 
            if (!strcmp(parameter, "fen"))
            {
                InitBoard(); //Important!!

                std::string fen, input, str2("moves ");
                getline(std::cin, input);
                std::size_t pos = input.find(str2); // position of "moves" 
                std::string str3 = ""; 
                bool checkSide;

                // fen is separated
                if (pos != std::string::npos)
                { 
                    checkSide = false;
                    str3 = input.substr(pos); // part of input until "moves"
                    fen = input.substr(1, input.length() - str3.length() - 1); // vorne wird ein space eingelesen 
                }
                else
                {
                    checkSide = true;
                    fen = input.substr(1, input.length() - 1); // vorne wird ein space eingelesen 
                }
                fen = input.substr(1, input.length() - str3.length() - 1); // vorne wird ein space eingelesen 
                streamToLog << "\t" << fen << std::endl;

                // fen umsetzen in Position, Rochade-Optionen, Seite am Zug ...
                // https://de.wikipedia.org/wiki/Forsyth-Edwards-Notation#:~:text=Die%20Forsyth%2DEdwards%2DNotation%20(,Jahrhundert%20popul%C3%A4r.
                ParseFEN(fen.c_str(), checkSide); 
                streamToLog << "\t" << str3 << std::endl; // str3 contains fen 
                if (str3 != "") 
                {
                    strcpy(parameter, str3.c_str());
                    ProcessMoves(const_cast<char*>((str3.substr(5)+"\n").c_str()), m, streamToLog, 1); // last parameter 1 means moves ... behind fen position
                }                
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
                std::cout << " Engine received movetime ";
                max_time = value; // values are given in [ms]
                fixedLevel = FIXEDLEVEL::TIME;
                max_depth = MAX_PLY;
            }
            else if (!strcmp(parameter, "depth"))
            {
                int value;
                std::cin >> value;
                streamToLog << currentDateTime() << ": "
                    << "Depth wurde auf '" << value << "' Halbzuege gesetzt." << std::endl;
                std::cout << " Engine received depth ";
                max_depth = value; // values are given in plies (Halbzuege)
                fixedLevel = FIXEDLEVEL::DEPTH;
            }
            else if (!strcmp(parameter, "nodes"))
            {
                int value;
                std::cin >> value;
                streamToLog << currentDateTime() << ": "
                    << "Nodes wurde auf '" << value << "' gesetzt." << std::endl;
                std::cout << " Engine received nodes ";
                max_depth = MAX_PLY; // Halbzuege
                fixedLevel = FIXEDLEVEL::NODES;
                max_nodes = value;
            }
            else if (!strcmp(parameter, "infinite"))
            {
                streamToLog << currentDateTime() << ": "
                    << "Depth und Movetime wurde auf 'infinite' gesetzt." << std::endl;
                std::cout << " Engine received infinite. We use MAXDEPTH.";
                max_depth = MAXDEPTH; // values are given in plies (Halbzuege)
                fixedLevel = FIXEDLEVEL::DEPTH;
            }                
            else if (!strcmp(parameter, "wtime"))
            {
                streamToLog << "\t" << parameter << std::endl;
                
                timeDivider = 20.0f; // valid for both sides

                int value;
                std::cin >> value;
                streamToLog << "\tZeit fuer Weiss wurde auf '" << value << "' ms gesetzt." << std::endl;
                std::cout << " Engine received wtime ";
                //ToDo: Zeit für Weiss setzen
                // 
                // Erste Idee:
                if (computer_side == White)
                {
                    max_time = (U64)((float)value / timeDivider); // values are given in [ms]
                    fixedLevel = FIXEDLEVEL::TIME;
                    max_depth = MAX_PLY;   
                    if (max_time < 2500)
                        max_time =  500;
                }                
            }                
            else
            {
                strcpy(command, parameter);
                optionalCommand = true;
            }
            
            std::string parameters = "";
            std::getline(std::cin,parameters);
                
            std::list<std::string> parameterList;
            std::list<std::string>::iterator it;
            splitString(parameters, parameterList);

            for (it = parameterList.begin(); it != parameterList.end(); ++it)
            {
                streamToLog << "\t" << *it << " ";
                
                if (!strcmp(it->c_str(), "btime"))
                {
                    int value = std::stoi(*(++it));
                    streamToLog << "\tZeit fuer Schwarz wurde auf '" << value << "' ms gesetzt." << std::endl;
                    std::cout << " Engine received btime ";

                    if (computer_side == Black)
                    {
                        max_time = (U64)((float)value / timeDivider); // values are given in [ms]
                        fixedLevel = FIXEDLEVEL::TIME;
                        max_depth = MAX_PLY;
                        if (max_time < 2500)
                            max_time =  500;
                    }
                }
                
                if (!strcmp(it->c_str(), "winc"))
                {
                    int value = std::stoi(*(++it));
                    streamToLog << "\tInkrement pro Zug für Weiss wurde auf '" << value << "' ms gesetzt." << std::endl;
                    std::cout << " Engine received winc ";
                    if (computer_side == White)
                    { 
                        max_time += value; // values are given in [ms]; Inkrement wird pro Zug addiert 
                        if (max_time < 3000)
                            max_time =  500;
                        streamToLog << "\tZeit pro Zug für Weiss wurde (incl.Inkrement) auf '"  << max_time << "' ms erhoeht." << std::endl;  
                    }
                }
                
                if (!strcmp(it->c_str(), "binc"))
                {
                    int value = std::stoi(*(++it));
                    streamToLog << "\tInkrement pro Zug wurde auf '" << value << "' ms gesetzt." << std::endl;
                    std::cout << " Engine received binc ";
                    if (computer_side == Black)
                    {
                        max_time += value; // values are given in [ms]; Inkrement wird pro Zug addiert   
                        if (max_time < 3000)
                            max_time =  500;
                        streamToLog << "\tZeit für Schwarz pro Zug wurde (incl.Inkrement) auf '" << max_time << "' ms erhoeht." << std::endl;
                    }
                }
            }
            streamToLog << std::endl;
        }//if
        
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
                std::cout << " lookup=0 ";

            move_list[0].start = hash_start;
            move_list[0].dest = hash_dest;
            
            if (board[hash_start] == P && (row[hash_dest] == 0 || row[hash_dest] == 7))
            {
                move_promote = Q;
            }
            
            std::cout << "bestmove " << MoveString(hash_start, hash_dest, move_promote) << "\n"; // promote eingefügt anstelle immer 0 ////TEST////
            streamToTest << "bestmove " << MoveString(hash_start, hash_dest, move_promote) << "\n"; ////TEST////
            MakeMove(hash_start, hash_dest);
            
            
            ply = 0;
            Gen(side, xside);
            PrintResult();
            continue;
        }

        if (!strcmp(command, "ponderhit")) 
        {
            // Noch nicht implementiert
            continue;
        }

        if (!strcmp(command, "quit") || !strcmp(command, "stop"))
        {            
            gameIsRunning = false; 
            break;
        }            
    }//while

    streamToLog.close(); // file fuer log-Datei schliessen
}

void ProcessMoves(char line[2048], int& m, std::fstream& streamToLog, bool fen)
{
    int last_space = 0;
    if (!fen)
    {
        NewGame();
        computer_side = EMPTY;
    }
    else
    {
        // with fen we do not want a new move sequence from startposition        
    }

    for (int i = 0; i < 2048; i++)
    {
        if (line[i] == '\n')
        {
            break;
        }
        if (line[i] == ' ')
        {
            last_space = i;
            ProcessMove(m, line, last_space, streamToLog);
        }
    }
}

void ProcessMove(int& m, char line[2048], int last_space, std::fstream& streamToLog)
{
    ply = 0;
    first_move[0] = 0;
    Gen(side, xside);
    m = ParseMove(&line[last_space + 1]);
    
    if (m == -1 || !MakeMove(move_list[m].start, move_list[m].dest))
    {
        MoveString(move_list[m].start, move_list[m].dest, move_list[m].promote);
    }
    
    //if (game_list[hply - 1].promote > 0 && (row[move_list[m].dest] == 0 || row[move_list[m].dest] == 7))
    if ((move_list[m].promote > 0) && (row[move_list[m].dest] == 0 || row[move_list[m].dest] == 7))
    {
        RemovePiece(xside, Q, move_list[m].dest);
        if (line[4] == 'n' || line[4] == 'N')
            AddPiece(xside, N, move_list[m].dest);
        else 
            if (line[4] == 'b' || line[4] == 'B')
                AddPiece(xside, B, move_list[m].dest);
            else 
                if (line[4] == 'r' || line[4] == 'R')
                    AddPiece(xside, R, move_list[m].dest);
                else 
                    AddPiece(xside, Q, move_list[m].dest);
    }
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
        std::cout << "info string 1/2-1/2 {Stalemate}\n"; //UCI

        NewGame();
        computer_side = EMPTY;
        return;
    }
    if (i == first_move[1] && flag == 0)
    {
        Gen(side, xside);
        DisplayBoard();
        
        std::cout << "info string end of game\n"; //UCI

        if (Attack(xside, NextBit(bit_pieces[side][K])))
        {
            if (side == 0)
            {
                std::cout << "info string 0-1 {Black mates}\n"; //UCI
            }
            else
            {
                std::cout << "info string 1-0 {White mates}\n"; //UCI
            }
        }
        else
        {
            std::cout << "info string 1/2-1/2 {Stalemate}\n"; //UCI
        }
        NewGame();
        computer_side = EMPTY;
    }
    else if (reps() >= 3)
    {
        std::cout << "info string 1/2-1/2 {Draw by repetition}\n"; //UCI
        NewGame();
        computer_side = EMPTY;
    }
    else if (fifty >= 100)
    {
        std::cout << "info string 1/2-1/2 {Draw by fifty move rule}\n"; //UCI
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
        std::cout << "Diagram missing." << std::endl;
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
    std::cout << " diagram # " << num + count << "\n";
    count++;
    if (side == 0)
        std::cout << "White to move\n";
    else
        std::cout << "Black to move\n";
    std::cout << " " << ts << std::endl;
    return 0;
}

int ParseFEN(const char* ts, bool checkSide)
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
            if (checkSide)
            {
                side = 0; xside = 1;
            }                
        }
        if (ts[c + 1] == 'b') // Schwarz zieht
        {
            if (checkSide)
            {
                side = 1; xside = 0;
            }                
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
    
    count++;
    if (side == 0)
    {
        std::cout << "info string White to move\n"; //UCI
    }
    else
    {
        std::cout << "info string Black to move\n"; //UCI
    }        
    std::cout << "info string " << ts << std::endl; //UCI
    return 0;
}

void CloseDiagram()
{
    if (diagram_file)
        fclose(diagram_file);
    diagram_file = NULL;
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

    if (promote > 0) 
    {
        switch (promote) 
        {
        case N:
            c = 'n';
            break;
        case B:
            c = 'b';
            break;
        case R:
            c = 'r';
            break;
        case Q:
            c = 'q';
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
    {
        sprintf_s(str, "%c%d%c%d",
            col[start] + 'a',
            row[start] + 1,
            col[dest] + 'a',
            row[dest] + 1);
    }
    return str;
}