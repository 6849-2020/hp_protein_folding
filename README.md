# HP Protein Folding

## Building

Assuming you have `g++` and `make` installed, you can just run `make`. If you'd like to use a
different C++ compiler, edit the first line of the [Makefile](Makefile). And if you don't want to
use `make`, you can compile it yourself like this:

```
g++ -std=c++11 -Wall -O3 -o hearn-hp hearn-hp.cpp
```

## Running

By default, the Makefile produces one optimized (-O3) and one debug executable. They're different
versions of the same code &mdash; just different compiler flags. The optimized version is called
`hearn-hp` and should give you output like this:

```
./hearn-hp
Folding HHPPHPPPHHPPHPHPHPH for scores >= 0

[lots of lines omitted]

 -----------------
|       P         |
|      / \        |
| P---H   H   P   |
|  \         / \  |
|   H   H---H   P |
|  /           /  |
| P---H   H   H   |
|    /   / \   \  |
|   P---P   H   P |
|          /   /  |
|         P---P   |
 -----------------
 -------------------
|       P           |
|      / \          |
| P---H   H   P     |
|  \         / \    |
|   H   H---H   P   |
|  /           /    |
| P---H   H   H     |
|    /   / \   \    |
|   P---P   H   P   |
|            \   \  |
|             P---P |
 -------------------
Found 99 optimal solutions (score 16).
Runtime: 9538 milliseconds

```

The program takes three command line arguments.

1. the amino acid sequence (e.g. "PHPHHHPPH")
2. the target score (branches that can't achieve this score will be ignored)
3. verbosity (0 prints nothing but the optimal foldings, 1 is the default and prints when a new max
   score is achieved, 2 prints when the current max score is matched or beaten, 3 prints lots of
   intermediate stuff too)

So for example, you could run `hearn-hp PHHPPHPHPH 6 2`.

If you want to use a square grid instead of a triangular one, change lines 163-164.

## TODO

There are a lot of remaining easy improvements.

- add a command line argument to change the grid type
- optimize the data structure for running fast, instead of being easy to display
- parallelize
