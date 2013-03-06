#!/usr/bin/python3

# Copyright 2012 Sebastian Riese

# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.

"""
A graphical user interface for the bagh chal board game
written in python with tkinter
"""

# TODO: all client server communication
# shall be encapsulated in a thread
# and interact with the GUI asynchronously

# alternative: Build request, response
# protocol in baghchal.c and then poll
# for output here

import tkinter
from tkinter.constants import *

import os
import subprocess

import configparser

import itertools

class Board:

    EMPTY=None
    TIGER=0
    SHEEP=1

    TURN_UNDEF=None
    TURN_TIGER=0
    TURN_SHEEP=1
    TURN_SETSHEEP=2
    TURN_SHEEP_WIN=3
    TURN_TIGER_WIN=4

    # data for the interface to the game binary
    TURNMAP = {
        'S': TURN_SETSHEEP,
        's': TURN_SHEEP,
        'T': TURN_TIGER,
        'D': TURN_TIGER_WIN,
        'Z': TURN_SHEEP_WIN,
        'u': TURN_UNDEF # used for next move possibilities, is added
                        # as first line by the protocol parser
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
        TURN_TIGER_WIN: 'D',
        TURN_SHEEP_WIN: 'Z',
        TURN_UNDEF: 'u' # used for next move possibilities, is added
                        # as first line by the protocol parser
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
        board.data = list(self.data)
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
    def __init__(self, canvas, statustext):
        self.board = None
        self.game = None
        self.moves = []
        self.win = ''

        self.mymove = None

        self.canvas = canvas
        self.statustext = statustext

        self.cids = []

        self.board_grid_x = [10,60,110,160,210]
        self.board_grid_y = [10,60,110,160,210]
        self.board_rect = [1,1,221,221]

        self.tiger_radius = 7
        self.sheep_radius = 5

        self.config = configparser.SafeConfigParser(defaults = {
                'sheepcolor' : 'gray',
                'tigercolor' : 'yellow'
                })
        self.config.add_section('ui')
        self.config.read(os.path.expanduser('~/.tkchal'))
        self.sheep_color = self.config.get('ui', 'sheepcolor')
        self.tiger_color = self.config.get('ui', 'tigercolor')

        self.draw_board()

    def calc_board_pos(self):
        w = self.canvas.winfo_width()
        h = self.canvas.winfo_height()

    def draw_board(self):
        self.calc_board_pos()

        self.canvas.create_rectangle(*self.board_rect, fill='white')

        board_min_x = self.board_grid_x[0]
        board_max_x = self.board_grid_x[-1]
        board_min_y = self.board_grid_y[0]
        board_max_y = self.board_grid_y[-1]
        board_center_x = self.board_grid_x[2]
        board_center_y = self.board_grid_y[2]

        for x in self.board_grid_x:
            self.canvas.create_line(x, board_min_y, x, board_max_y)

        for y in self.board_grid_y:
            self.canvas.create_line(board_min_x, y, board_max_x, y)

        self.canvas.create_line(board_min_x, board_min_y,
                                board_max_x, board_max_y)
        self.canvas.create_line(board_min_x, board_max_y,
                                board_max_x, board_min_y)

        self.canvas.create_line(board_center_x, board_min_y,
                                board_max_x, board_center_y)
        self.canvas.create_line(board_max_x, board_center_y,
                                board_center_x, board_max_y)
        self.canvas.create_line(board_center_x, board_max_y,
                                board_min_x, board_center_y)
        self.canvas.create_line(board_min_x, board_center_y,
                                board_center_x, board_min_y)



    def draw(self):
        # here we could scale the canvas for a nice fit of the board
        # to the window size
        self.canvas.unbind('<Button-1>')
        for cid in self.cids:
            self.canvas.delete(cid)

        if not self.win:
            self.statustext.set('your turn')

            if self.board.state == Board.TURN_SETSHEEP:
                self.canvas.bind('<Button-1>', self.insert_sheep)
            elif self.board.state == Board.TURN_SHEEP:
                self.canvas.bind('<ButtonPress-1>', self.move_sheep)
                self.canvas.bind('<ButtonRelease-1>', self.move_sheep2)
            elif self.board.state == Board.TURN_TIGER:
                self.canvas.bind('<ButtonPress-1>', self.move_tiger)
                self.canvas.bind('<ButtonRelease-1>', self.move_tiger2)


        tr = self.tiger_radius
        sr = self.sheep_radius
        for entry, (y, x) in zip(self.board.data,
                                 itertools.product(self.board_grid_x,
                                                   self.board_grid_y)):
            if entry == Board.TIGER:
                self.cids.append(self.canvas.create_oval(x-tr, y-tr,
                                                         x+tr, y+tr,
                                                         fill=self.tiger_color))

            elif entry == Board.SHEEP:
                self.cids.append(self.canvas.create_oval(x-sr, y-sr,
                                                         x+sr, y+sr,
                                                         fill=self.sheep_color))

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
            self.statustext.set('Invalid move')
        else:
            self.moves = []
            self.get_board()
            self.draw()

    def canvas_to_logical(self, x, y):
        ri, rj = None, None

        for i, gx in enumerate(self.board_grid_x):
            for j, gy in enumerate(self.board_grid_y):
                if (x - gx) ** 2 + (y - gy) ** 2 < 100:
                    ri, rj = i, j

        if (ri, rj) == (None, None):
            raise Exception()

        return ri, rj


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
                self.mymove.data[(j + (l-j)//2) * 5 + (k - i) // 2 + i] = \
                    Board.EMPTY

            self.apply_move(self.mymove)

    def insert_sheep(self, ev):
        i, j = self.canvas_to_logical(ev.x, ev.y)

        mymove = self.board.copy()
        mymove.data[j * 5 + i] = Board.SHEEP
        self.apply_move(mymove)

    def get_board(self):
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

        if self.board.state == Board.TURN_TIGER_WIN:
                self.game.wait()
                self.statustext.set('Tigers win!')
                self.game = None
                self.win = 'tigers'
                return

        elif self.board.state == Board.TURN_SHEEP_WIN:
                self.game.wait()
                self.statustext.set('Sheep win!')
                self.win = 'sheep'
                self.game = None
                return

    def get_moves(self):
        self.moves = []

        state = 'skip'
        cur = 'u\n'
        while True:
            tmp = str(self.game.stdout.readline(), encoding='US-ASCII')

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
        cmdline = ['baghchal']
        self.config = configparser.SafeConfigParser()
        self.config.add_section('game')
        self.config.add_section('engine')
        self.config.read(os.path.expanduser('~/.tkchal'))

        if self.config.has_option('engine', 'path'):
            cmdline[0] = os.path.expanduser(self.config.get('engine', 'path'))


        if os.getenv('BHAGCHAL') is not None:
            cmdline[0] = os.path.expanduser(os.getenv('BHAGCHAL'))

        if self.config.has_option('engine', 'args'):
            cmdline += self.config.get('engine', 'args').split()

        if self.config.has_option('game', 'ai'):
            if self.config.get('game', 'ai').lower() == 'sheep':
                cmdline.append('-s')
            elif self.config.get('game', 'ai').lower() == 'tiger':
                cmdline.append('-t')
        else:
            cmdline.append('-s')

        cmdline.append('-d')

        if self.config.has_option('game', 'aistrength'):
            cmdline.append(self.config.get('game', 'aistrength'))
        else:
            cmdline.append('3')

        self.game = subprocess.Popen(cmdline, stdin=subprocess.PIPE,
                                     stdout=subprocess.PIPE, stderr=None)
        self.win = ''
        self.get_board()
        self.get_moves()
        self.draw()

    def undo(self):
        self.game.stdin.write(b'u\n')
        self.get_board()
        self.get_moves()
        self.draw()

def configure():
    config = configparser.SafeConfigParser()
    config.add_section('game')
    config.read(os.path.expanduser('~/.tkchal'))

    conftoplevel = tkinter.Toplevel(tk)
    conftoplevel.title('Configuration')

    tkinter.Label(conftoplevel, text='Computer plays:').pack()

    aivar = tkinter.StringVar()

    if config.has_option('game', 'ai'):
        if config.get('game', 'ai').lower() == 'sheep':
            aivar.set('sheep')
        elif config.get('game', 'ai').lower() == 'tiger':
            aivar.set('tiger')

    sheep = tkinter.Radiobutton(conftoplevel, text='Sheep', variable=aivar,
                                value='sheep')
    tiger = tkinter.Radiobutton(conftoplevel, text='Tiger', variable=aivar,
                                value='tiger')

    sheep.pack()
    tiger.pack()

    tkinter.Label(conftoplevel, text='AI depth').pack()
    aistrengthvar = tkinter.IntVar()

    if config.has_option('game', 'aistrength'):
        aistrengthvar.set(config.getint('game', 'aistrength'))
    else:
        aistrengthvar.set(3)

    tkinter.Entry(conftoplevel, textvariable=aistrengthvar).pack()


    def save():
        config.set('game', 'ai', aivar.get())
        config.set('game', 'aistrength', str(aistrengthvar.get()))
        with open(os.path.expanduser('~/.tkchal'), 'w') as fp:
            config.write(fp)
        conftoplevel.destroy()

    def cancel():
        conftoplevel.destroy()

    savebutton = tkinter.Button(conftoplevel, text='Save', command=save)
    cancelbutton = tkinter.Button(conftoplevel, text='Cancel', command=cancel)
    savebutton.pack()
    cancelbutton.pack()

def about():
    abouttext = """\
This is the graphical frontend for the baghchal
baghchal engine."""
    toplevel = tkinter.Toplevel(tk)
    tkinter.Label(toplevel, text=abouttext, justify=LEFT).pack()

def rules():
    import webbrowser
    webbrowser.open("https://en.wikipedia.org/wiki/Bagh_Chal")

tk = tkinter.Tk()
tk.title('Bagh-Chal')

frame = tkinter.Frame(tk)
frame.pack(fill=BOTH, expand=1)

canvas = tkinter.Canvas(frame, width=220, height=220)
canvas.pack(fill=BOTH, expand=1, side=TOP, padx=1, pady=1)

statustext = tkinter.StringVar()
status = tkinter.Label(frame, textvariable=statustext,
                       borderwidth=2, relief=RIDGE)
status.pack(expand=1, side=BOTTOM, fill=X)

game = Game(canvas, statustext)
tk.bind('<Control-n>', lambda event: game.new())
tk.bind('<Control-u>', lambda event: game.undo())

menu = tkinter.Menu(tk)
tk['menu'] = menu

gamemenu = tkinter.Menu(menu, tearoff=0)
menu.add_cascade(label = 'Game', menu=gamemenu)

gamemenu.add_command(label='New Game', command=game.new)
gamemenu.add_separator()
gamemenu.add_command(label='Undo', command=game.undo)
gamemenu.add_separator()
gamemenu.add_command(label='Quit', command=tk.destroy)

settings = tkinter.Menu(menu, tearoff=0)
menu.add_cascade(label='Settings', menu=settings)

settings.add_command(label='Preferences', command=configure)

help = tkinter.Menu(menu, tearoff=0)
menu.add_cascade(labe = 'Help', menu=help)

help.add_command(label='Rules', command=rules)
help.add_command(label='About', command=about)


game.new()
tk.mainloop()
