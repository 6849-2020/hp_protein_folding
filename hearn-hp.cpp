//
//  main.cpp
//  HP-folding
//
//  Created by Bob Hearn on 3/19/16.
//  Copyright © 2016 Bob Hearn. All rights reserved.
//

#include <iostream>
#include <vector>
#include <queue>
#include <set>
#include <cassert>
#include <limits>
#include <stdlib.h>

using namespace std;

// This is used to display patterns.
typedef vector<string> Board;
Board TheBoard;

set<Board> Solutions;
int NumSols;
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

//const int MAXDIM = 2 * (int) Pattern.size();
const int MAXDIM = 100;

void Search(
	int index,        // ?
	int fromY,        // positions the pattern
	int fromX,        // positions the pattern
	int score,
	int potential,
	bool turned,
	int distEstimate
);

// Counts the number of Hs and empty squares appearing in the four squares that neighbor (x, y).
void CountNeighbors(int x, int y, int &numH, int &numEmpty);

void PrintBoard();

// score = # of HH pairs
// potential = # of H-empty pairs
float FScore[MAXDIM][MAXDIM], GScore[MAXDIM][MAXDIM];

struct Spot {
	int x, y;

	Spot(int x, int y) : x(x), y(y)	{}
	bool operator>(const Spot &s) const {
		//		cout << "FScore[" << y << "][" << x << "] = " << FScore[y][x] << ", " << "FScore[" << s.y << "][" << s.x << "] = " << FScore[s.y][s.x]
		//		  << ", operator> = " << (FScore[y][x] > FScore[s.y][s.x]) << "\n";
		return FScore[y][x] > FScore[s.y][s.x];
	}
};


// priority_queue returns largest element as top(). We want the smallest, so we use std::greater<Spot> for the Compare function
// (default = std::less<Spot>).

priority_queue<Spot, vector<Spot>, greater<Spot> > OpenSet;
bool Open[MAXDIM][MAXDIM];

vector<Spot> ClosedList;
bool Closed[MAXDIM][MAXDIM];


int main(int argc, const char *argv[]) {
	if (argc > 1) {
		Pattern = argv[1];
	}
	if (argc > 2) {
		MaxScore = atoi(argv[2]);
	}
	cout << "Folding " << Pattern << " for scores >= " << MaxScore << endl;

	for (int i = 0; i < MAXDIM; ++i) {
		TheBoard.push_back(string(MAXDIM, ' '));
	}

	TheBoard[MAXDIM / 2][MAXDIM / 2] = Pattern[0];
	TheBoard[MAXDIM / 2 + 1][MAXDIM / 2] = '|';
	TheBoard[MAXDIM / 2 + 2][MAXDIM / 2] = Pattern[1];
	Search(
		2,
		MAXDIM / 2 + 2,
		MAXDIM / 2,
		Pattern[0] == 'H' && Pattern[1] == 'H',
		3 * (Pattern[0] == 'H') + 3 * (Pattern[1] == 'H'),
		false,
		3
	);

	//	TheBoard[MAXDIM / 2 + 3][MAXDIM / 2] = '|';
	//	TheBoard[MAXDIM / 2 + 4][MAXDIM / 2] = Pattern[2];

	//	Search(3, MAXDIM / 2 + 4, MAXDIM / 2, 0, 2, true, 4);

	cout << "Found " << Solutions.size() << " optimal solutions (score " << MaxScore << "):\n" << endl;
	for (set<Board>::iterator it = Solutions.begin(); it != Solutions.end(); ++it) {
		TheBoard = *it;
		PrintBoard();
	}
	cout << "Found " << Solutions.size() << " optimal solutions (score " << MaxScore << ").\n" << endl;
}


// These provide the offsets (in x and y coordinates) for the four cardinal directions.
// Assuming the directions are indexed as N, E, S, W, this means positive y is down the screen.
int DX[] = { 0, 1, 0, -1 };
int DY[] = { -1, 0, 1, 0 };

int NumSteps;


// distEstimate is a guess (generally high) for path length from fromX, fromY back to start via empty squares
// We use it as a heuristic for pruning. We will only prune if the estimate is too far, and the actual distance
// (computed in that case) turns out to be too far.
void Search(
	int index,
	int fromY,
	int fromX,
	int score,
	int potential,
	bool turned,
	int distEstimate
) {
	//	if (0)
	if (index == 32)
	//	if (!(++NumSteps % 100000))
	//	if (!(++NumSteps % 10000000))
	{
		cout << "Interim...\n";
		cout << "Score = " << score << "\n";
		cout << "Potential = " << potential << "\n";
		//		cout << "Score upper bound = " << score + (degree < potential ? degree : ((degree - potential) / 2 + potential)) << "\n";
		PrintBoard();
	}

	if (index == Pattern.length()) {
		// check score
		if (score > MaxScore) {
			Solutions.clear();
			MaxScore = score;
		}

		// Found a good solution
		if (score == MaxScore) {
			Solutions.insert(TheBoard);
			cout << "Score = " << score << "\n";
			PrintBoard();
		}

		return;
	}

	for (int dir = 0; dir < 4; ++dir) {
		int const newx = fromX + 2 * DX[dir];
		int const newy = fromY + 2 * DY[dir];

		if (!(TheBoard[newy][newx] == ' ') || !(turned || dir != 3)) {
			continue;
		}

		int hneighbors, emptyneighbors;

		CountNeighbors(newx, newy, hneighbors, emptyneighbors);

		int newp = potential + emptyneighbors * (Pattern[index] == 'H') - hneighbors;
		int newscore = score + hneighbors * (Pattern[index] == 'H');

		// Did we just isolate an empty space?
		for (int d2 = 0; d2 < 4; ++d2) {
			int const x = fromX + 2 * DX[d2];
			int const y = fromY + 2 * DY[d2];

			if (dir != d2 && TheBoard[y][x] == ' ') {
				int e, h;
				CountNeighbors(x, y, h, e);
				if (!e)
				newp -= h;					// If so, subtract inaccessible potential
			}
		}

		int degree = 0;

		for (int i = index + 1; i < Pattern.size(); ++i)
		if (Pattern[i] == 'H')
		degree += 2 + (Pattern[i - 1] == 'H') + (Pattern[(i + 1) % Pattern.size()] == 'H');

		if (newscore + (degree < newp ? degree : ((degree - newp) / 2 + newp)) >= MaxScore) {
			TheBoard[newy][newx] = Pattern[index];
			TheBoard[fromY + DY[dir]][fromX + DX[dir]] = (dir % 2 ? '-' : '|');
			Search(index + 1, newy, newx, newscore, newp, turned || dir != 2, distEstimate + 2);	// typically add 2 to path?
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


void PrintBoard() {
	int miny = 0;

	while (TheBoard[miny] == string(MAXDIM, ' ')) {
		++miny;
	}

	int maxy = MAXDIM - 1;

	while (TheBoard[maxy] == string(MAXDIM, ' ')) {
		--maxy;
	}

	for (int y = miny; y <= maxy; ++y) {
		cout << TheBoard[y] << "\n";
	}

	cout << string(MAXDIM, '-') << "\n";
}
