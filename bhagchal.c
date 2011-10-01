#include "bhagchal.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

int genmoves_sheep(state st, state *res) {
  int moves = 0;
  if (st.setsheep < MAXSHEEP) {
    for (int i = 0; i < BOARDPLACES; i++) {
      if (! ((1ULL << i) & (st.sheep | st.tiger))) {
	res[moves] = st;
	res[moves].turn = TURN_TIGER;
	res[moves].setsheep++;
	res[moves++].sheep |= (1ULL << i);
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
	    }
	  }
	}
      }
    }
  }
  return moves;
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
	      }
	    }
	  }
	}
      }
    }
  }

  return moves;
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

int ai_move_rec(state *states, int n, int *sheepscore, int *tigerscore, int depth) {
  if (depth == 0) {
    // now comes the scoring
    *sheepscore = 0;
    *tigerscore = 0;
    int index = 0;
    for (int i = 0; i < n; i++) {
      // additionally we should give bonus for trapped tigers
      if (states[i].turn == TURN_SHEEP) {
	int tmp = hamming(states[i].sheep);
	if (tmp > *sheepscore) {
	  index = i;
	  *sheepscore = tmp;
	}
      } else {
	// rate movability and number of eaten sheep
	int tmp = MAXSHEEP - hamming(states[i].sheep) + n / 2;
	if (tmp > *tigerscore) {
	  index = i;
	  *tigerscore = tmp;
	}
      }
    }
    return index;
  } else {
    state *mystates = (state *) malloc(sizeof(state) * 64);
    int best = 0;
    int *score = NULL;

    for (int i = 0; i < n; i++) {
      int myn = genmoves(states[i], mystates);
      int myscore;
      ai_move_rec(mystates, myn, &myscore, &myscore, depth - 1);
      if (myscore > *score) {
	best = i;
	*sheepscore = myscore;
      }
    }
    free(mystates);
    return best;
  }
}

state ai_move(state st, int depth) {
  // todo: one huge chunk of memory, to which the states are written
  // continuously ... that should work pretty well
  // wait isn't that exactly what obstacks do ... portability?
  state *states = (state *) malloc(sizeof(state) * 64);
  int n = genmoves(st, states);
  int sscore, tscore;
  int best = ai_move_rec(states, n, &sscore, &tscore, depth);
  state res = states[best];
  free(states);
  return res;
}

// should be plenty of space:
// I guess the maximal number of possible moves is (four sheep on the 8-moves positions,
// the others placed not to block their movement possibilities)
// 36, but i am too lazy to check ... so just add a security margin
// if the 64 does not hold (for MAXSHEEP == 20 and four tigers), 
// feel free flame me intensely ;)
static state news[64];
static int cap;
static int turn;
static state *game; // the entire game is recorded, this way undo is possible

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

void gameloop(FILE *in, FILE *out, int verb, int ait, int ais) {
  while (1) {
  TOP:
     assert((game[turn-1].sheep & game[turn-1].tiger) == 0);

     if (game[turn-1].turn == TURN_SHEEP && ais) {
       apply_move(ai_move(game[turn-1], 4));
     }
     else if (game[turn-1].turn == TURN_TIGER && ait) {
       apply_move(ai_move(game[turn-1], 4));
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
  fputs("usage: bhagchal [-vhts]\n", stdout);
}

void help() {
  fputs(
  "written by Sebastian Riese <sebastian.riese.mail@web.de>\n"
  "This is a the bhag chal board game.\n"
  "The interface is crappy currently, but there\n"
  "is a simple Python/Tk frontend available called tkchal.py\n"
  "The interface will change Real Soon Now.\n"
  "Options:\n"
  "-v    -- print out boards more human readable\n"
  "-s/-t -- sheep/tiger are played by computer\n"
  "-h    -- print this help and exit\n", stdout);
}

int main(int argc, char *argv[]) {
  int verbose = 0,
    ait = 0,
    ais = 0;

  for (int i = 1; i < argc; i++) {
    if (argv[i][0] == '-') {
      for (int j = 1; argv[i][j] != 0; j++) {
	switch (argv[i][j]) {
	case 'h':
	  help();
	  exit(0);
	  break;
	case 'v':
	  verbose = 1;
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

  gameloop(stdin, stdout, verbose, ais, ait);
  return 0;
}
