# parallel-reversi

**Human vs. computer** or **computer vs. random move generator** command line game Reversi (https://en.wikipedia.org/wiki/Reversi) in C/C++. It was written for purposes of testing and timing execution times of the minimax (https://en.wikipedia.org/wiki/Minimax) algorithm parallelization with MPI and PThreads.

## Usage

To compile for human vs. computer:

    $ g++ -DUSER .\Reversi_PTHREADS.cpp

To compile for computer vs. random move generator:

    $ g++ .\Reversi_PTHREADS.cpp

Run with:

    $ ./a.out

For compilation of the MPI version, is to use mpicc instead of g++, for inclusion of the MPI libraries.

## Human vs computer pthreads output example

```
$ ./a.out
-------START-------
  -1-2-3-4-5-6-7-8
1|                |
2|                |
3|                |
4|       O X      |
5|       X O      |
6|                |
7|                |
8|                |
  ----------------

Player X:
  -1-2-3-4-5-6-7-8
1|                |
2|                |
3|       X        |
4|       X X      |
5|       X O      |
6|                |
7|                |
8|                |
  ----------------

Player O:
Input row and column ([1..8]<Enter>[1..8]):
3
3

  -1-2-3-4-5-6-7-8
1|                |
2|                |
3|     O X        |
4|       O X      |
5|       X O      |
6|                |
7|                |
8|                |
  ----------------

Player X:
  -1-2-3-4-5-6-7-8
1|                |
2|                |
3|   X X X        |
4|       O X      |
5|       X O      |
6|                |
7|                |
8|                |
  ----------------
```
