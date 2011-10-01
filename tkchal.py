#!/usr/bin/python3

import tkinter
from tkinter.constants import *

import os
import subprocess

BHAGCHAL = os.getenv('BHAGCHAL')
if BHAGCHAL == None:
    BHAGCHAL = './bhagchal'

class Board:

    EMPTY=None
    TIGER=0
    SHEEP=1
    
    TURN_UNDEF=None
    TURN_TIGER=0
    TURN_SHEEP=1
    TURN_SETSHEEP=2

    TURNMAP = {
        'S': TURN_SETSHEEP,
        's': TURN_SHEEP,
        'T': TURN_TIGER,
        'u': TURN_UNDEF # used for next move possibilities, is added as first line by the protocol parser
        }

    OBJMAP = {
        'T': TIGER,
        'S': SHEEP,
        'o': EMPTY
        }

    ITURNMAP = {
        TURN_SETSHEEP: 'S',
        TURN_SHEEP: 's',
        TURN_TIGER: 'T',
        TURN_UNDEF: 'u' # used for next move possibilities, is added as first line by the protocol parser
        }

    IOBJMAP = {
        TIGER: 'T',
        SHEEP: 'S',
        EMPTY: 'o'
        }

    @classmethod
    def parse_board(clazz, text):
        board = clazz()
        board.data = []
        board.state = clazz.TURNMAP[text[0]]

        for char in text[1:]:
            if char.isspace():
                continue
            else:
                board.data.append(clazz.OBJMAP[char])

        if len(board.data) != 25:
            raise Exception('Invalid Board')

        return board

    def copy(self):
        board = Board()
        board.state = self.state
        board.data = self.data[:]
        return board

    def write_board(board):
        res = board.ITURNMAP[board.state] + '\n'
        i = 0
        for tp in board.data:
            i += 1
            res += board.IOBJMAP[tp]
            if i % 5 == 0:
                res += '\n'
        return res

class Game:
    def __init__(self, canvas, status):
        self.board = None
        self.game = None
        self.moves = []

        self.canvas = canvas
        self.status = status

        self.cids = []
        self.draw_board()

    def draw_board(self):
        self.canvas.create_rectangle(0, 0, 220, 220, fill='white')

        for x in [10, 60, 110, 160, 210]:
            self.canvas.create_line(x, 10, x, 210)
            self.canvas.create_line(x, 10, x, 210)
            self.canvas.create_line(x, 10, x, 210)
            self.canvas.create_line(x, 10, x, 210)

        for y in [10, 60, 110, 160, 210]:
            self.canvas.create_line(10, y, 210, y)
            self.canvas.create_line(10, y, 210, y)
            self.canvas.create_line(10, y, 210, y)
            self.canvas.create_line(10, y, 210, y)

        self.canvas.create_line(10, 10, 210, 210)
        self.canvas.create_line(10, 210, 210, 10)

        self.canvas.create_line(110, 10, 210, 110)
        self.canvas.create_line(210, 110, 110, 210)
        self.canvas.create_line(110, 210, 10, 110)
        self.canvas.create_line(10, 110, 110, 10)



    def draw(self):
        # here we could scale the canvas for a nice fit of the board to the window size

        self.canvas.unbind('<Button-1>')
        for cid in self.cids:
            self.canvas.delete(cid)

        if self.board.state == Board.TURN_SETSHEEP:
            self.canvas.bind('<Button-1>', self.insert_sheep)
        elif self.board.state == Board.TURN_SHEEP:
            self.canvas.bind('<ButtonPress-1>', self.move_sheep)
            self.canvas.bind('<ButtonRelease-1>', self.move_sheep2)
        elif self.board.state == Board.TURN_TIGER:
            self.canvas.bind('<ButtonPress-1>', self.move_tiger)
            self.canvas.bind('<ButtonRelease-1>', self.move_tiger2)

        

        for entry, (y, x) in zip(self.board.data, ((x, y) for x in [0, 50, 100, 150, 200] for y in [0, 50, 100, 150, 200])):
            if entry == Board.TIGER:
                self.cids.append(self.canvas.create_oval(x, y, x+20, y+20, fill='yellow'))

            elif entry == Board.SHEEP:
                self.cids.append(self.canvas.create_oval(x+5, y+5, x+15, y+15, fill='gray'))

    def apply_move(self, mymove):
        self.mymove = None
        if not self.moves:
            self.get_moves()

        k = 0
        for move in self.moves:
            if move.data == mymove.data:
                self.game.stdin.write(bytes(str(k) + '\n', encoding='US-ASCII'))
                break
            k += 1
        
        if k == len(self.moves):
            # this is supposed to reach the status bar one day
            # self.info('invalid move')

            # some text for debugging purposes
            # print("*****************************************")
            # print("My Board")
            # print(mymove.write_board())
            # print("Possible Boards")
            # for d in self.moves:
            #     print(d.write_board())
            # print("*****************************************")
            pass
        else:
            self.moves = []
            self.get_board()
            self.draw()

    def canvas_to_logical(self, x, y):
        i, j = (x + 25) // 50, (y + 25) // 50
        if i < 0: i = 0
        if j < 0: j = 0
        if i > 5: i = 5
        if j > 5: j = 5
        return i,j

    def move_sheep(self, ev):
        i, j = self.canvas_to_logical(ev.x, ev.y)

        self.mymove = self.board.copy()
        self.mymove.data[j * 5 + i] = Board.EMPTY

    def move_sheep2(self, ev):
        if self.mymove is not None:
            i, j = self.canvas_to_logical(ev.x, ev.y)
            self.mymove.data[j * 5 + i] = Board.SHEEP
            self.apply_move(self.mymove)
        

    def move_tiger(self, ev):
        i, j = self.canvas_to_logical(ev.x, ev.y)
        self.mymove = self.board.copy()
        self.mymove.pos = i, j
        self.mymove.data[j * 5 + i] = Board.EMPTY

    def move_tiger2(self, ev):
        if self.mymove is not None:
            i, j = self.canvas_to_logical(ev.x, ev.y)
            self.mymove.data[j * 5 + i] = Board.TIGER

            k,l = self.mymove.pos
            # the absolute values are neccessary as // rounds towards
            # negative infinite (not toward zero, as is needed here)
            if abs(k - i) // 2 != 0 or abs(l - j) // 2 != 0:
                self.mymove.data[(j + (l-j)//2) * 5 + (k - i) // 2 + i] = Board.EMPTY

            self.apply_move(self.mymove)

    def insert_sheep(self, ev):
        i, j = self.canvas_to_logical(ev.x, ev.y)

        mymove = self.board.copy()
        mymove.data[j * 5 + i] = Board.SHEEP
        self.apply_move(mymove)

    def get_board(self):
        if self.game.poll() is not None:
            raise Exception('Child process unexpectedly quit!')

        # read the current board from the pipe
        state = 'skip'
        text = ''
        while True:
            tmp = str(self.game.stdout.readline(), encoding='US-ASCII')
            if state == 'skip':
                if tmp.strip() == 'START':
                    state = 'board'
            elif state == 'board':
                if tmp.strip() == 'END':
                    break
                else:
                    text += tmp
            else:
                raise Exception('Invalid board reading state')

        self.board = Board.parse_board(text)

    def get_moves(self):
        self.moves = []

        if self.game.poll() is not None:
            raise Exception('Child process unexpectedly quit!')

        state = 'skip'
        cur = 'u\n'
        while True:
            tmp = str(self.game.stdout.readline(), encoding='US-ASCII')
            if tmp.strip() == 'Tigers win!':
                self.game.wait()
                self.status.text = 'Tigers win!'
                self.game = None
                return

            elif tmp.strip() == 'Sheep win!':
                self.game.wait()
                self.status.text = 'Sheep win!'
                self.game = None
                return

            if state == 'skip':
                if tmp.strip() == '#0':
                    state = 'reading'
            elif state == 'reading':
                if tmp.strip()[0] == '#':
                    self.moves.append(Board.parse_board(cur))

                    if tmp.strip() == '##':
                        break
                    else:
                        cur = 'u\n'
                        continue
                else:
                    cur += tmp

    def new(self):
        self.game = subprocess.Popen([BHAGCHAL, '-t'], stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=None)
        self.get_board()
        self.get_moves()
        self.draw()

    def undo(self):
        self.game.stdin.write(b'u\n')
        self.get_board()
        self.get_moves()
        self.draw()

tk = tkinter.Tk()
tk.title('Bhag Chal')

frame = tkinter.Frame(tk)
frame.pack(fill=BOTH, expand=1)

canvas = tkinter.Canvas(frame)
canvas.pack(fill=BOTH, expand=1)

menuframe = tkinter.Frame(frame)
menuframe.pack(expand=1, fill=X)

status = tkinter.Label(frame, text='foo', borderwidth=2, relief=RIDGE)
status.pack(expand=1, side=BOTTOM, fill=X)

game = Game(canvas, status)
new = tkinter.Button(menuframe, text='New Game', command=game.new)
new.pack(expand=1, side=LEFT, fill=X)

undo = tkinter.Button(menuframe, text='Undo', command=game.undo)
undo.pack(expand=1, side=LEFT, fill=X)

new = tkinter.Button(menuframe, text='Quit', command=tk.destroy)
new.pack(expand=1, side=LEFT, fill=X)

game.new()
tk.mainloop()
