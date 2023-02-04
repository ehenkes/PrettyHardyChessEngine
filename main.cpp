/*
  Stockfish, a UCI chess playing engine derived from Glaurung 2.1
  Copyright (C) 2004-2022 The Stockfish developers (see AUTHORS file)

  Stockfish is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Stockfish is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <iostream>

#include "bitboard.h"
#include "endgame.h"
#include "position.h"
#include "psqt.h"
#include "search.h"
#include "syzygy/tbprobe.h"
#include "thread.h"
#include "tt.h"
#include "uci.h"

#if defined _MyCode_
    #include "windows.h"
    #include "evaluate.h" // for Eval::trace...(const Position& pos)
#endif

using namespace Stockfish;

#if defined _MyCode_
    static void SetWindow(short Width, short Height, short ScrollBuffer); 
#endif

int main(int argc, char* argv[]) 
{
    std::cout << engine_info() << std::endl;

#if defined _MyCode_
  /*for (int i = 1; i < argc; i++) 
  {

      if (!strcmp(argv[i], "-ShowHelpForPrettyHardy")) 
      {
          sync_cerr << "Usage: if Parameter -usePrettyHardy exists\nthen Console prompt up\n" << sync_endl;
      }
      else if (!strcmp(argv[i], "-usePrettyHardy")) 
      {
      */
          ::DeleteFileA("output.txt");
          ::DeleteFileA("chessboard.txt");
#if defined _NoAdditionalConsole_  
          //do nothing
#else
          ::AllocConsole();
          
          freopen_s((FILE**)stderr, "CONOUT$", "w", stderr); //connect to stderr (std::cerr)
          ::SetConsoleTextAttribute(::GetStdHandle(STD_ERROR_HANDLE), 14); //yellow text
          setvbuf(stdout, NULL, _IONBF, 0); //turn off buffering for stdout
          setvbuf(stderr, NULL, _IONBF, 0); //turn off buffering for stdcerr
          ::SetWindow(120, 60, 1000);
          ::ShowWindow(::GetConsoleWindow(), SW_SHOW);
          ::UpdateWindow(::GetConsoleWindow());
          sync_cerr << "04.02.2023: Hier gibt es real-time Kommentare von SF 15.1.\n" << sync_endl;
#endif
     /* }
      else 
      {
          if (i == argc - 1) break;
      }
  }*/
#endif

  CommandLine::init(argc, argv);
  UCI::init(Options);
  Tune::init();
  PSQT::init();
  Bitboards::init();
  Position::init();
  Bitbases::init();
  Endgames::init();
  Threads.set(size_t(Options["Threads"]));
  Search::clear(); // After threads are up
  Eval::NNUE::init();

  UCI::loop(argc, argv);

  Threads.set(0);
  return 0;
}

#if defined _MyCode_
    static void SetWindow(short Width, short Height, short ScrollBuffer)
    {
        COORD coord;
        coord.X = Width;
        coord.Y = max(Height, ScrollBuffer);
        SMALL_RECT Rect;
        Rect.Top = 0;
        Rect.Left = 0;
        Rect.Bottom = Height - 1;
        Rect.Right = Width - 1;
        HANDLE Handle = GetStdHandle(STD_ERROR_HANDLE);
        ::SetConsoleScreenBufferSize(Handle, coord);
        ::SetConsoleWindowInfo(Handle, true, &Rect);
    }
#endif