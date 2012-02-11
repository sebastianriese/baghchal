#include "bhagchal.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include <time.h>

int genmoves_sheep(state st, state *res) {
  int moves = 0;
  if (st.setsheep < MAXSHEEP) {
    for (int i = 0; i < BOARDPLACES; i++) {
      if (! ((1ULL << i) & (st.sheep | st.tiger))) {
	res[moves] = st;
	res[moves].turn = TURN_TIGER;
	res[moves].setsheep++;
	res[moves++].sheep |= (1ULL << i);
	assert(moves <= 64);
      }
    }
  } else {
    for (int i = 0; i < BOARDPLACES; i++) {
      // there has to be a sheep to move
      if ((1ULL << i) & st.sheep) {
	for (int p = 0; p < 8; p++) {
	  // there has to be a connection
	  if (CONNECTIONS[i] & (1u << p)) {
	    // the target has to be empty
	    int newplace = (i + BOARDPLACES + SHIFT[p]) % BOARDPLACES;

	    if (! ((1ULL << newplace) & (st.sheep | st.tiger))) {
	      res[moves] = st;
	      res[moves].turn = TURN_TIGER;
	      res[moves].sheep &= ~(1ULL << i);
	      res[moves++].sheep |= (1ULL << newplace);
	      assert(moves <= 64);
	    }
	  }
	}
      }
    }
  }

  return moves;
}

int blocked_tigers(state st) {
  int blocked = 0;
  
  // there has to be a tiger to move
  for (int i = 0; i < BOARDPLACES; i++) {
    if ((1ULL << i) & st.tiger) {
      int isblocked = 1;
      for (int p = 0; p < 8; p++) {
	// there has to be a connection
	if (CONNECTIONS[i] & (1u << p)) {
	  // the target has to be tigerfree
	  int newplace = (i + BOARDPLACES + SHIFT[p]) % BOARDPLACES;

	  if (! ((1ULL << newplace) & (st.tiger | st.sheep))) {
	    isblocked = 0;
	  } else if ((1ULL << newplace) & st.sheep) {
	    // if there is a sheep, it may be possible to jump
	    if (CONNECTIONS[newplace] & (1u << p)) { // jumps are straigh on in the same direction
	      int jumpplace = (newplace + BOARDPLACES + SHIFT[p]) % BOARDPLACES;
	      if (! ((1ULL << jumpplace) & (st.tiger | st.sheep))) { // the jumpplace has to be empty
		isblocked = 0;
	      }
	    }
	  }
	}
      }
      if (isblocked)
	blocked++;
    }
  }
  assert(blocked <= 4);
  return blocked;
}

int genmoves_tiger(state st, state *res) {
  int moves = 0;
  
  // there has to be a tiger to move
  for (int i = 0; i < BOARDPLACES; i++) {
    if ((1ULL << i) & st.tiger) {
      for (int p = 0; p < 8; p++) {
	// there has to be a connection
	if (CONNECTIONS[i] & (1u << p)) {
	  // the target has to be tigerfree
	  int newplace = (i + BOARDPLACES + SHIFT[p]) % BOARDPLACES;

	  if (! ((1ULL << newplace) & (st.tiger | st.sheep))) {
	    res[moves] = st;
	    res[moves].turn = TURN_SHEEP;
	    res[moves].tiger &= ~(1ULL << i);
	    res[moves++].tiger |= 1ULL << newplace;
	    assert(moves <= 64);
	  } else if ((1ULL << newplace) & st.sheep) {
	    // if there is a sheep, it may be possible to jump
	    if (CONNECTIONS[newplace] & (1u << p)) { // jumps are straigh on in the same direction
	      int jumpplace = (newplace + BOARDPLACES + SHIFT[p]) % BOARDPLACES;
	      if (! ((1ULL << jumpplace) & (st.tiger | st.sheep))) { // the jumpplace has to be empty
		res[moves] = st;
		res[moves].turn = TURN_SHEEP;
		res[moves].sheep &= ~(1ULL << newplace);
		res[moves].tiger &= ~(1ULL << i);
		res[moves++].tiger |= 1ULL << jumpplace;
		assert(moves <= 64);
	      }
	    }
	  }
	}
      }
    }
  }
  return moves;
}

// compute all tiger reachable positions
// by a tri-colour algorithm
// imagine a wavefront ...
// hope this isn't too slow
// well it is still quite expensive
// (and not exact, just an approximation from below
// due to the tri-colour logic: a sheep may be jumped
// and we may fail to consider other sheep, that may be
// jumped as a result of this event)
// think about the accurate calculator ...
int locked_fields(state st) {
  // first calculate all fields reachable by
  // tigers
  state states[64];
  state black, gray, newblack;
  black = st;
  black.tiger = 0;
  newblack = black;
  gray  = st;

  while (hamming(gray.tiger)) {
    int n = genmoves_tiger(gray, states);
    for (int i = 0; i < n; i++) {
      newblack.tiger |= states[i].tiger;
      newblack.sheep &= states[i].sheep;
    }
    gray.tiger = newblack.tiger & ~black.tiger;
    gray.sheep = newblack.sheep;
    black = newblack;
  }

  // where no tiger can go, and no sheep is placed,
  // there must be a locked field ...
  return hamming(~black.tiger & ~black.sheep);
}

// the caller must ensure, that res is sufficiently large,
// the actual number of possible moves is returned
// NOTE: it is simple to obtain a safe bound for the length of res
int genmoves(state st, state *res) {
  if (st.turn == TURN_SHEEP) {
    return genmoves_sheep(st, res);
  } else {
    return genmoves_tiger(st, res);
  }
}

void draw_board(state st, FILE *to) {
  for (int j = 0; j < BOARDLEN; j++) {
    // draw the connection lines
    if (j == 0) {
      fputs("  a   b   c   d   e\n", to);
    } else {
      if (j % 2) {
	fputs("  | \\ | / | \\ | / |\n", to);
      } else {
	fputs("  | / | \\ | / | \\ |\n", to);
      }
    }

    // draw the figures
    for (int i = 0; i < BOARDLEN; i++) {
      if (i == 0) {
	fprintf(to, "%d ", j + 1);
      } else {
	fputs("---", to);
      }

      if (sheep_at(st, i, j)) {
	fputs("S", to);
      } else if (tiger_at(st, i, j)) {
	fputs("T", to);
      } else {
	fputs("o", to);
      }
    }
    fputs("\n", to);
  }

  if (st.turn) {
    fputs("Tigers to move\n", to);
  } else {
    if (st.setsheep != MAXSHEEP) {
      fprintf(to, "Sheep to move (%d to place)\n", MAXSHEEP - st.setsheep);
    } else {
      fputs("Sheep to move\n", to);
    }
  }
}

void write_sheep_win(FILE *to) {
  fputs("Z\n", to);
}

void write_tigers_win(FILE *to) {
  fputs("D\n", to);
}

void write_turn(state st, FILE *to) {
  if (st.turn == TURN_TIGER) {
    fputs("T\n", to);
  } else {
    if (st.setsheep != MAXSHEEP) {
      fprintf(to, "S\n");
    } else {
      fprintf(to, "s\n");
    }
  }
}

void write_board(state st, FILE *to) {
  for (int j = 0; j < BOARDLEN; j++) {

    // draw the figures
    for (int i = 0; i < BOARDLEN; i++) {
      if (sheep_at(st, i, j)) {
	fputs("S", to);
      } else if (tiger_at(st, i, j)) {
	fputs("T", to);
      } else {
	fputs("o", to);
      }
    }
    fputs("\n", to);
  }
}

// todo design the breadth first search ... this will need some datastructures
// to remember the currently considered configurations of the board
// An intelligent stop criterion (primarily considering spent time, but also
// searching on if for example winning is probable) would increasy the strength
// notably

// minimax move selector
state ai_move_rec(state cur, state *space, int *score, int depth, int tiger, int *moves) {
  if (depth == 0) {
    // fprintf(stderr, "SHEEP %d\n", hamming(states[0].sheep));
    int blocked = blocked_tigers(cur);
    if (blocked == 4) {
      *score = MAXSCORE; // if we win nothing else matters, so a win gets max score
    } else {
      *score = SHEEPWEIGHT * hamming(cur.sheep) + TRAPPEDWEIGHT * blocked + LOCKEDWEIGHT * locked_fields(cur);
    }
    return cur;
  } else {
    int best = 0;
    if (tiger)
      *score = MAXSCORE + depth; // prefer to win fast!
    else
      *score = 0 - depth; // prefer to win fast!

    int k = genmoves(cur, space);
    *moves += k;
    for (int i = 0; i < k; i++) {
      int tmp;

      ai_move_rec(space[i], &space[k], &tmp, depth - 1, !tiger, moves);

      if ((!tiger && tmp > *score) || (tiger && tmp < *score)) {
	*score = tmp;
	best = i;
      }
    }
    // free(nstates);
    if (k == 0) {
      return cur; // there is no possible move, so the new state can only be the old state
    }

    return space[best];
  }
}

state ai_move(state st, int depth, int limit) {
  int score; // dummy
  state *states = (state *) malloc(sizeof(state) * 64 * (depth + 4));
  int moves = 0;
  state res = ai_move_rec(st, states, &score, depth, st.turn == TURN_TIGER, &moves);
  free(states);

  if (moves <= limit) {
    return ai_move(st, depth+RECURSE_INC, LIMIT_MULT*limit);
  }

  /* printf("Considered %d moves, depth %d\n", moves, depth); */
 return res;
}

// should be plenty of space:
// I guess the maximal number of possible moves is (four sheep on the 8-moves positions,
// the others placed not to block their movement possibilities)
// 36, but i am too lazy to check ... so just add a security margin
// if the 64 does not hold (for MAXSHEEP == 20 and four tigers), 
// feel free to flame me intensely ;)
// probably this should be done properly, with one large chunk of memory 
// and stack like allocation deallocation, shouldn't be much slower, but therefore
// much safer and more beautiful
static state news[64];
static int cap;
static int turn;
static state *game; // the entire game is recorded, this way undo is possible as well as replays and complete saves

static void apply_move(state st) {
  if (turn == cap) {
    cap *= 2;
    game = (state *) realloc(game, sizeof(state) * cap);

    if (game == NULL) {
      fputs("Out of mem!", stderr);
      abort();
    }
  }

  game[turn++] = st;
}

static void undo_move() {
  if (turn > 1) {
    turn -= 1;
  }
}

void gameloop(FILE *in, FILE *out, int verb, int cm, int ait, int ais, int ai_depth) {
  while (1) {
  TOP:
     assert((game[turn-1].sheep & game[turn-1].tiger) == 0);

     // check for win situation
     if (game[turn-1].turn == TURN_TIGER) {
       if (blocked_tigers(game[turn-1]) == 4) {
         if (verb) {
           fputs("Sheep win!\n", out);
           draw_board(game[turn-1], out);
         } else {
           fputs("START\n", out);
           write_sheep_win(out);
           write_board(game[turn-1], out);
           fputs("END\n", out);
         }
	 return;
       }
     } else {
       if (game[turn-1].setsheep - hamming(game[turn-1].sheep) >= 5) {
         if (verb) {
           fputs("Tigers win!\n", out);
           draw_board(game[turn-1], out);
         } else {
           fputs("START\n", out);
           write_tigers_win(out);
           write_board(game[turn-1], out);
           fputs("END\n", out);
         }
	 return;
       }
     }

     if (game[turn-1].turn == TURN_SHEEP && ais) {
       if (cm) {
	 write_turn(game[turn-1], out);
	 write_board(game[turn-1], out);
       }

       apply_move(ai_move(game[turn-1], ai_depth, RECURSE_LIMIT_SHEEP));
     }
     else if (game[turn-1].turn == TURN_TIGER && ait) {
       if (cm) {
	 write_turn(game[turn-1], out);
	 write_board(game[turn-1], out);
       }

       apply_move(ai_move(game[turn-1], ai_depth, RECURSE_LIMIT_TIGER));
     }
     else {
       if (verb) {
	 draw_board(game[turn-1], out);
       } else {
	 fputs("START\n", out);
	 write_turn(game[turn-1], out);
	 write_board(game[turn-1], out);
	 fputs("END\n", out);
	 fflush(out);
       }

       int nmoves = genmoves(game[turn-1], news);
       if (verb) {
	 for (int m = 0; m < nmoves; m++) {
	   fprintf(out, "Move #%d\n", m);
	   draw_board(news[m], out);
	 }
       } else {
	 for (int m = 0; m < nmoves; m++) {
	   fprintf(out, "#%d\n", m);
	   write_board(news[m], out);
	 }
	 fputs("##\n", out);
	 fflush(out);
       }

       int cmd;
       fflush(in);
       while ((cmd = fgetc(in)) != EOF) {
      
	 if (isspace(cmd)) {
	   continue;
	 }

	 if (cmd == 'h') {
	   fputs("Sorry, no help available yet\n", out);
	   continue;
	 }

	 if (cmd == 'u') {
           if (ait || ais) {
             undo_move();
           }

	   undo_move();
	   goto TOP;
	 }

	 if (cmd == 'q') {
	   fputs("Goodbye!\n", out);
	   exit(0);
	 }

	 if (isdigit(cmd)) {
	   int move;
	   ungetc(cmd, in);
	   fscanf(in, "%d", &move);
	   if (move >= nmoves) {
	     fputs("Invalid turn!\n", out);
	     continue;
	     
	   } else {
	     apply_move(news[move]);
	     break;
	   }
	 }

	 fputs("Unknown command!\n", out);
       }

       if (cmd == EOF) {
	 fputs("Input closed: abort game!\n", out);
	 exit(1);
       }
     }
  }
}

void usage() {
  fputs("usage: bhagchal [-vhtsa] [-d NUM]\n", stdout);
}

void version() {
  fputs("bhagchal 0.1\n", stdout);
}

void help() {
  usage();
  version();
  fputs(
  "written by Sebastian Riese <sebastian.riese.mail@web.de>\n"
  "This is a the bhag chal board game.\n"
  "The interface is crappy currently, but there\n"
  "is a simple Python/Tk frontend available called tkchal.py\n"
  "The interface will be improved Real Soon Now.\n"
  "Options:\n"
  "-v    -- print out boards more human readable\n"
  "-s/-t -- sheep/tiger are played by computer\n"
  "-a    -- print the board when the computer has moved as well\n"
  "-d N  -- set AI strength (default 5)\n"
  "-h    -- print this help and exit\n", stdout);
}

int main(int argc, char *argv[]) {
  int verbose = 0,
    ait = 0,
    ais = 0,
    cm = 0,
    ai_depth = AI_DEPTH_DEFAULT;

  srand(time(NULL));

  for (int i = 1; i < argc; i++) {
    if (argv[i][0] == '-') {
      char *cur = argv[i];
      for (int j = 1; cur[j] != 0; j++) {
	switch (cur[j]) {
	case 'h':
	  help();
	  exit(0);
	  break;
	case 'v':
	  verbose = 1;
	  break;
	case 'a':
	  cm = 1;
	  break;
        case 'd':
          i++;
          if (i == argc) {
            fputs("Missing argument for option -d!\n", stderr);
            usage();
            exit(1);
          }
          ai_depth = atoi(argv[i]);
          break;
	case 't':
	  ait = 1;
	  break;
	case 's':
	  ais = 1;
	  break;
	default:
	  fputs("Unknown option!\n", stderr);
	  usage();
	  exit(1);
	}
      }
    } else {
      // this is a positional argument
      fputs("No positional arguments accepted!\n", stderr);
      usage();
      return 1;
    }
  }

  cap = 128;
  turn = 1;
  game = (state *) malloc(sizeof(state) * cap);
  game[0] = START;

  gameloop(stdin, stdout, verbose, cm, ait, ais, ai_depth);
  return 0;
}
