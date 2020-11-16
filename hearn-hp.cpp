//
//  main.cpp
//  HP-folding
//
//  Created by Bob Hearn on 3/19/16.
//  Copyright © 2016 Bob Hearn. All rights reserved.
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

// This strucutre is used to represent folded states. It's in a format that's easy to print, but not
// super space (or cache) efficient. We always index by y and then x coordinate.
using Board = std::array<std::array<char, MAXDIM>, MAXDIM>;
Board TheBoard;

vector<Board> Solutions;
int MaxScore = 0;


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
	int index,        // basically how many amino acids have been placed
	int fromY,        // x coordinate of last placed amino acid
	int fromX,        // y coordinate of last placed amino acid
	int score,        // score of placed amino acids
	int potential,    // number of empty squares that neighbor H amino acids
	bool turned       // ?
);

// Counts the number of Hs and empty squares appearing in the four squares that neighbor (x, y).
void CountNeighbors(int x, int y, int &numH, int &numEmpty);

void PrintBoard(Board const& board);
void PrintBoard() { PrintBoard(TheBoard); }

int main(int argc, const char *argv[]) {
	if (argc > 1) {
		Pattern = argv[1];
	}
	if (argc > 2) {
		MaxScore = atoi(argv[2]);
	}
	cout << "Folding " << Pattern << " for scores >= " << MaxScore << endl;

	for (int i = 0; i < MAXDIM; ++i) {
		for (int j = 0; j < MAXDIM; ++j) {
			TheBoard[j][i] = ' ';
		}
	}

	// We start with the first two amino acids placed.
	auto start = std::chrono::high_resolution_clock::now();
	TheBoard[MAXDIM / 2][MAXDIM / 2] = Pattern[0];
	TheBoard[MAXDIM / 2 + 1][MAXDIM / 2] = '|';
	TheBoard[MAXDIM / 2 + 2][MAXDIM / 2] = Pattern[1];
	Search(
		2,
		MAXDIM / 2 + 2,
		MAXDIM / 2,
		// H-H has score 1, all other two amino acid chains have score 0
		(Pattern[0] == 'H' && Pattern[1] == 'H') ? 1 : 0,
		// each amino acid could potentially have three additional H neighbors
		3 * (Pattern[0] == 'H') + 3 * (Pattern[1] == 'H'),
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
	/*
	// Uncomment this to print intermediate chains.
	if (index == 8) {
		cout << "Interim...\n";
		cout << "Score = " << score << "\n";
		cout << "Potential = " << potential << "\n";
		//		cout << "Score upper bound = " << score + (degree < potential ? degree : ((degree - potential) / 2 + potential)) << "\n";
		PrintBoard();
	}
	*/

	// Check if we've placed all amino acids.
	if (index == Pattern.length()) {
		// Update MaxScore if this is a new record.
		if (score > MaxScore) {
			Solutions.clear();
			MaxScore = score;
		}

		// Save the solution if it achieves the maximum score.
		if (score == MaxScore) {
			Solutions.push_back(TheBoard);
			//cout << "Score = " << score << "\n";
			//PrintBoard();
		}

		return;
	}

	for (int dir = 0; dir < 4; ++dir) {
		int const newx = fromX + 2 * DX[dir];
		int const newy = fromY + 2 * DY[dir];

		if (TheBoard[newy][newx] != ' ' || !(turned || dir != 3)) {
			continue;
		}

		int hneighbors, emptyneighbors;
		CountNeighbors(newx, newy, hneighbors, emptyneighbors);

		// The new potential is the old one with two changes.
		// We subtract the number of H neighbors, since are placing an amino acid in that spot.
		// If this node is an H node, we add the number of empty neighbors.
		int newp = potential + emptyneighbors * (Pattern[index] == 'H') - hneighbors;
		// The new score is the old one plus the number of H neighbors (if are placing an H now).
		int newscore = score + hneighbors * (Pattern[index] == 'H');

		// Check if we just isolated an empty space.
		// Q: What about larger regions of empty space?
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

		int degree = 0;
		for (int i = index + 1; i < Pattern.size(); ++i) {
			if (Pattern[i] == 'H') {
				// Q: Why do we allow ourselves to wrap around here?
				degree += 2 + (Pattern[i - 1] == 'H') + (Pattern[(i + 1) % Pattern.size()] == 'H');
			}
		}

		int const delta = (degree < newp) ? degree : ((degree - newp) / 2 + newp);
		if (newscore + delta >= MaxScore) {
			TheBoard[newy][newx] = Pattern[index];
			TheBoard[fromY + DY[dir]][fromX + DX[dir]] = (dir % 2 ? '-' : '|');
			Search(index + 1, newy, newx, newscore, newp, turned || dir != 2);
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
