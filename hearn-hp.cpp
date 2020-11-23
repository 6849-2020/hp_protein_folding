//
//  main.cpp
//  HP-folding
//
//  Created by Bob Hearn on 3/19/16.
//  Copyright © 2016 Bob Hearn. All rights reserved.
//
// Edited by Erik Strand on 11/16/2020.
// - cleared out commented and incomplete code (mostly related to closed chains)
// - added explanatory comments
// - changed the board representation from strings to std::arrays of chars (first step toward a
//   representation optimized for the algorithm rather than printing)
//

#include <array>
#include <iostream>
#include <vector>
#include <cassert>
#include <limits>
#include <stdlib.h>
#include <chrono>

using namespace std;

constexpr int MAXDIM = 100;

// When true, we print intermediate results. It can be changed here or on the command line.
bool verbose = false;

// This strucutre is used to represent partial and complete folded states. It's in a format that's
// easy to print, but not super space (or cache) efficient. Indexing is matrix style. So the first
// coordinate determines vertical position, and increasing it moves you from the top to the bottom.
// The second coordinate determines horizontal position, and increasing it moves you from the left
// to the right.
using Board = std::array<std::array<char, MAXDIM>, MAXDIM>;
Board TheBoard;

// This records the highest score we've seen so far.
int MaxScore = 0;

// This records all solutions we've found with the maximum score.
vector<Board> Solutions;

//string Pattern = "PHPPHPPHPPHP";
//string Pattern = "PHPPHPHPPHPPHPPHPHPPHP";
//string Pattern = "PHPPHPHPPHPPHPHPPHPPHPPHPHPHPPHP";
//string Pattern = "PHPPHPHPPHPPHPHPHPPHPPHPPHPHPPHPPHPHPHPPHP";
//string Pattern = "PHPPHPHPPHHPHPPHPHPPHH";
//string Pattern = "PPPP";
string Pattern =   "PPHPHHPHPPHPHPHPPHPHHPHPPHPHPH";		// score = 15
//string Pattern =   "PPHHPHPHPPHPHPPHHPHPHPPHPH";				// score = 13
//string Pattern = "HPPHPHPHHPHPHPPHPHPHPHPPHPHPHHPHPHPPHPHPHP";
//string Pattern = "HPPHPHHPPHPHPHPPHHPHPPHPHPHPPHHPHHPPHPHP";
//string Pattern = "PPHPHPHPHPHPPPHPHPHPHPPHPHPHPHPHPPPHPHPHPH";
//string Pattern = "PHPPHPHPPHPPHPHPHPPHPPHPPHPHPHPPHPPHPPHPHPPHPHPHPPHP";

void Search(
	int index,        // index of the next amino acid to be placed
	int fromY,        // x coordinate of last placed amino acid
	int fromX,        // y coordinate of last placed amino acid
	int score,        // score of placed amino acids so far
	int potential,    // roughly the number of empty squares that neighbor H amino acids
	bool turned       // indicates if the current pattern has broken mirror symmetry yet
);

// Counts the number of Hs and empty squares appearing in the four squares that neighbor (x, y).
void CountNeighbors(int x, int y, int &numH, int &numEmpty);

// Prints an ASCII representation of the board. (Not much transformation needed, since the board
// stores ASCII characters.)
void PrintBoard(Board const& board);
void PrintBoard() { PrintBoard(TheBoard); }

int main(int argc, const char *argv[]) {
	if (argc > 1) {
		Pattern = argv[1];
	}
	if (argc > 2) {
		MaxScore = atoi(argv[2]);
	}
	if (argc > 3) {
		// If any third argument is supplied, we interpret that as asking for verbose output.
		verbose = true;
	}
	cout << "Folding " << Pattern << " for scores >= " << MaxScore << endl;

	// Initialize the board. Every square starts blank.
	for (int i = 0; i < MAXDIM; ++i) {
		for (int j = 0; j < MAXDIM; ++j) {
			TheBoard[j][i] = ' ';
		}
	}

	// Place the first two amino acids. We always position the second right below the first. Note
	// that this breaks rotational symmetry, so we won't get rotated versions of the same solution.
	// (Mirror symemtry is broken below, search for "turned".)
	TheBoard[MAXDIM / 2][MAXDIM / 2] = Pattern[0];
	TheBoard[MAXDIM / 2 + 1][MAXDIM / 2] = '|';
	TheBoard[MAXDIM / 2 + 2][MAXDIM / 2] = Pattern[1];

	// Start the search.
	auto start = std::chrono::high_resolution_clock::now();
	Search(
		// We've placed two amino acids so far.
		2,
		// These are the coordinates of the second amino acid in the chain.
		MAXDIM / 2 + 2,
		MAXDIM / 2,
		// H-H has score 1, all other two amino acid chains have score 0.
		(Pattern[0] == 'H' && Pattern[1] == 'H') ? 1 : 0,
		// Each amino acid could potentially have three additional H neighbors.
		3 * (Pattern[0] == 'H') + 3 * (Pattern[1] == 'H'),
		// So far all amino acids are in a straight line, so we haven't broken mirror symmetry.
		false
	);
	auto stop = std::chrono::high_resolution_clock::now();
	float elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count();

	cout << "Found " << Solutions.size() << " optimal solutions (score " << MaxScore << "):\n" << endl;
	for (auto it = Solutions.begin(); it != Solutions.end(); ++it) {
		PrintBoard(*it);
	}
	cout << "Found " << Solutions.size() << " optimal solutions (score " << MaxScore << ").\n" << endl;
	cout << "Runtime: " << elapsed << " milliseconds\n";
}


// These provide the offsets (in x and y coordinates) for the four cardinal directions.
// Assuming the directions are indexed as N, E, S, W, this means positive y is down the screen.
int DX[] = { 0, 1, 0, -1 };
int DY[] = { -1, 0, 1, 0 };


void Search(
	int index,
	int fromY,
	int fromX,
	int score,
	int potential,
	bool turned
) {
	// Print some intermediate chains if the verbose flag is set.
	if (verbose && index == 8) {
		cout << "Interim...\n";
		cout << "Score = " << score << "\n";
		cout << "Potential = " << potential << "\n";
		//cout << "Score upper bound = " << score + (degree < potential ? degree : ((degree - potential) / 2 + potential)) << "\n";
		PrintBoard();
	}

	// Check if we've placed all amino acids.
	if (index == Pattern.length()) {
		// Update MaxScore if this is a new record.
		if (score > MaxScore) {
			Solutions.clear();
			MaxScore = score;
			if (verbose) {
				cout << "New max score: " << MaxScore << '\n';
			}
		}

		// Save the solution if it achieves the maximum score.
		if (score == MaxScore) {
			Solutions.push_back(TheBoard);
			if (verbose) {
				cout << "Score = " << score << "\n";
				PrintBoard();
			}
		}

		return;
	}

	// Examine all squares where we could place the next amino acid.
	for (int dir = 0; dir < 4; ++dir) {
		int const newx = fromX + 2 * DX[dir];
		int const newy = fromY + 2 * DY[dir];

		// If the square is already occupied, we can't use it.
		// If we haven't turned before (i.e. all amino acids have been placed in a straight line),
		// then we require that this one be placed to the right. This prevents us from getting
		// mirror images of all our solutions.
		if (TheBoard[newy][newx] != ' ' || (!turned && dir == 3)) {
			continue;
		}

		// Count this square's H and empty neighbors.
		int hneighbors, emptyneighbors;
		CountNeighbors(newx, newy, hneighbors, emptyneighbors);

		// The new potential is the old one with two changes.
		// - We subtract the number of H neighbors, since we are placing an amino acid now that
		//   either gains the point or doesn't.
		// - If this node is an H node, we add the number of empty neighbors.
		int newp = potential + emptyneighbors * (Pattern[index] == 'H') - hneighbors;

		// The new score is the old one plus the number of H neighbors (if we're placing an H now).
		int newscore = score + hneighbors * (Pattern[index] == 'H');

		// Check if we just isolated an empty space.
		// This reduces our new potential, since even though there's an empty cell next to an H, we
		// couldn't possibly fold the protein chain to fill it. A potential improvement to the
		// algorithm would be to check for empty spaces that are larger than one square.
		for (int d2 = 0; d2 < 4; ++d2) {
			if (d2 == dir) {
				continue;
			}

			int const x = fromX + 2 * DX[d2];
			int const y = fromY + 2 * DY[d2];

			if (TheBoard[y][x] == ' ') {
				int h, e;
				CountNeighbors(x, y, h, e);
				// If so, subtract inaccessible potential.
				if (e == 0) {
					newp -= h;
				}
			}
		}

		// Compute the maximum possible number of points we could still add to our score.
		// (Or at least something similar to this.)
		// Q: Aren't we double counting H-H links within the sequence?
		int degree = 0;
		for (int i = index + 1; i < Pattern.size(); ++i) {
			if (Pattern[i] == 'H') {
				degree += 2 + (Pattern[i - 1] == 'H') + (Pattern[(i + 1) % Pattern.size()] == 'H');
			}
		}

		// Only place an amino acid here if it looks possible to match or beat MaxScore.
		// Q: Are there false negatives? I.e. configurations that we throw away that would achieve
		//    the max score?
		// Q: How can we come up with a tighter bound?
		int const delta = (degree < newp) ? degree : ((degree - newp) / 2 + newp);
		if (newscore + delta >= MaxScore) {
			// Place the amino acid.
			TheBoard[newy][newx] = Pattern[index];
			TheBoard[fromY + DY[dir]][fromX + DX[dir]] = (dir % 2 ? '-' : '|');

			// Recurse.
			// We break symmetry the first time we stop placing amino acids in a straight line.
			Search(index + 1, newy, newx, newscore, newp, turned || dir != 2);

			// Remove this amino acid.
			TheBoard[fromY + DY[dir]][fromX + DX[dir]] = ' ';
			TheBoard[newy][newx] = ' ';
		}
	}
}


void CountNeighbors(int x, int y, int &numH, int &numEmpty) {
	numH = 0;
	numEmpty = 0;

	for (int dir = 0; dir < 4; ++dir) {
		int const newx = x + 2 * DX[dir];
		int const newy = y + 2 * DY[dir];

		if (TheBoard[newy][newx] == 'H') {
			++numH;
		}
		if (TheBoard[newy][newx] == ' ') {
			++numEmpty;
		}
	}
}


void PrintBoard(Board const& board) {
	auto const row_is_empty = [&](int row) {
		for (int i = 0; i < MAXDIM; ++i) {
			if (board[row][i] != ' ') {
				return false;
			}
		}
		return true;
	};

	int miny = 0;
	while (row_is_empty(miny)) {
		++miny;
	}

	int maxy = MAXDIM - 1;
	while (row_is_empty(maxy)) {
		--maxy;
	}

	for (int y = miny; y <= maxy; ++y) {
		for (int x = 0; x < MAXDIM; ++x) {
			cout << board[y][x];
		}
		cout << '\n';
	}

	cout << string(MAXDIM, '-') << "\n";
}
