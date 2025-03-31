# HP Protein Folding

This C++ code implements a brute-force search to solve the
[***HP protein folding problem***](https://en.wikipedia.org/wiki/Hydrophobic-polar_protein_folding_model):
given a string of Hs and Ps
(representing hydrophilic and hydrophobic amino acids, respectively),
find foldings of the chain that maximize the number of neighboring pairs of Hs
(the ***score***).
For example:

```

                 ---------
                |   P-P   |
                |   | |   |
                | P-H H-P |
PHPPHPPHPPHP -> | |     | |  unique optimal folding
                | P-H H-P |  (score 4)
                |   | |   |
                |   P P   |
                 ---------
```

This code was originally written by Bob Hearn in March 2016.
During the [MIT class 6.849 in Fall 2020](https://courses.csail.mit.edu/6.849/fall20/),
Erik Strand edited the code in particular to add support for triangular grids.

## Building

Assuming you have `g++` and `make` installed, you can just run `make`. If you'd like to use a
different C++ compiler, edit the first line of the [Makefile](Makefile). And if you don't want to
use `make`, you can compile it yourself like this:

```sh
# square grid
g++ -std=c++11 -Wall -O3 -o hp_folder hp_folder.cpp
# triangular grid
g++ -std=c++11 -Wall -O3 -o tri_hp_folder -DGRID_TRIANGULAR hp_folder.cpp
```

## Running

By default, the Makefile produces four executables,
all from the same source file `hp_folder.cpp`:

- `hp_folder` is the optimized version of the code for the square grid.
- `tri_hp_folder` is the optimized version of the code for the triangular grid.
- `debug_*` are debug executables for the same (with different compiled flags).

For example, `hp_folder` should give you output like this:

```
./hp_folder
Folding PPHPHHPHPPHPHPHPPHPHHPHPPHPHPH for scores >= 0

[lots of lines omitted]

 -----------
|     P-H   |
|     |     |
|   P-H H-P |
|   |   | | |
| P-H H-P P |
| |   |     |
| P-H H-P   |
|   |   |   |
|   P-H H-P |
|     |   | |
|   P-H H-P |
|   |   |   |
| P-H H-P   |
| |   |     |
| P-H-P     |
 -----------
 -----------
| P-H-P     |
| |   |     |
| P-H H-P   |
|   |   |   |
|   P-H H   |
|     |     |
|   P-H H-P |
|   |   | | |
| P-H H-P P |
| |   |     |
| P-H H-P   |
|   |   |   |
|   P-H H-P |
|     |   | |
|     P-H-P |
 -----------
Found 6 optimal solutions (score 15).
Runtime: 2244 milliseconds
```

And `tri_hp_folder` should give you output like this:

```
./tri_hp_folder
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
Runtime: 3763 milliseconds

```

The programs take three command-line arguments:

1. the amino acid sequence (e.g. `PHPHHHPPH`)
2. the target score (branches that can't achieve this score will be ignored)
3. verbosity (0 prints nothing but the optimal foldings, 1 is the default and prints when a new max
   score is achieved, 2 prints when the current max score is matched or beaten, 3 prints lots of
   intermediate stuff too)

So for example, you could run `./tri_hp_folder PHHPPHPHPH 6 2`.

## TODO

There are a lot of remaining easy improvements.

- optimize the data structure for running fast, instead of being easy to display
- parallelize
- interactive web app
