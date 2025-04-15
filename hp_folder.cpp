//
//  main.cpp
//  HP-folding
//
//  Created by Bob Hearn on 3/19/16.
//  Copyright © 2016 Bob Hearn. All rights reserved.
//
// Edited by Erik Strand on 11/16/2020.
// - implemented hexagonal grids
// - cleared out commented and incomplete code (mostly related to closed chains)
// - added explanatory comments
// TODO:
// - use more efficient data structures
// - parallelize
//

#include <array>
#include <iostream>
#include <vector>
#include <cassert>
#include <limits>
#include <stdlib.h>
#include <chrono>

using namespace std;

constexpr int MAXDIM = 120;

// This strucutre is used to represent partial and complete folded states. It's in a format that's
// easy to print, but not super space (or cache) efficient. Indexing is matrix style. So the first
// coordinate determines vertical position, and increasing it moves you from the top to the bottom.
// The second coordinate determines horizontal position, and increasing it moves you from the left
// to the right.
class SquareBoard {
public:
	// Boards are default constructible, so that they can be put in vectors and such without
	// incurring any extra initialization overhead.
	SquareBoard() {}

	// These allow us to write TheBoard(row, col).
	char operator()(int row, int col) const {
		if (row < 0 || MAXDIM <= row || col < 0 || MAXDIM <= col) {
			cout << "Error: TheBoard is too small. Increase MAXDIM.\n";
			exit(1);
		}
		return data[row][col];
	}
	char& operator()(int row, int col) {
		if (row < 0 || MAXDIM <= row || col < 0 || MAXDIM <= col) {
			cout << "Error: TheBoard is too small. Increase MAXDIM.\n";
			exit(1);
		}
		return data[row][col];
	}

	// This fills a board with spaces.
	// Note: memset would be faster (unless the compiler is already smart enough to call it for us).
	void clear() {
		for (int i = 0; i < MAXDIM; ++i) {
			for (int j = 0; j < MAXDIM; ++j) {
				data[j][i] = ' ';
			}
		}
	}

	// This just places a link character ('-' or '|') going in the specified direction from the
	// specified coordinates. This doesn't help the algorithm at all; it's just to make the ASCII
	// representation human readable.
	void draw_link(int y, int x, int dir) {
		int const link_y = y + DY[dir] / 2;
		int const link_x = x + DX[dir] / 2;
		data[link_y][link_x] = (dir % 2 ? '-' : '|');
	}

	// This removes a link character.
	void erase_link(int y, int x, int dir) {
		int const link_y = y + DY[dir] / 2;
		int const link_x = x + DX[dir] / 2;
		data[link_y][link_x] = ' ';
	}

	// Prints the board to cout as ASCII art.
	void print();

	static constexpr int NDirs = 4;

	// These provide the offsets (in x and y coordinates) for the four cardinal directions.
	// 0 is North, 1 is East, 2 is South, and 3 is West.
	static constexpr int DX[NDirs] = {0, 2, 0, -2};
	static constexpr int DY[NDirs] = {-2, 0, 2, 0};

	// Returns true if going from direction 0 to this direction requires a right turn.
	static bool is_right_turn(int dir) { return dir == 1; }

private:
	std::array<std::array<char, MAXDIM>, MAXDIM> data;
};

// Pre C++17, the compiler gets salty if we don't give explicit definitions somewhere.
constexpr int SquareBoard::NDirs;
constexpr int SquareBoard::DX[SquareBoard::NDirs];
constexpr int SquareBoard::DY[SquareBoard::NDirs];


// This has the same interface as SquareGrid, but it represents a triangular/hexagonal grid.
struct HexagonalBoard {
public:
	// Boards are default constructible, so that they can be put in vectors and such without
	// incurring any extra initialization overhead.
	HexagonalBoard() {}

	// These allow us to write TheBoard(row, col).
	// Our array of data is secretly four times as large as SquareBoard's. Otherwise there wouldn't
	// be room to draw the diagonal links. Really this should be rewritten so that the data
	// structure is minimal and printing does whatever transformations are needed to generate the
	// ASCII representation. It would make the algorithm run a good bit faster too. But this is a
	// quick hack to get things going.
	char operator()(int row, int col) const {
		row *= 2;
		col *= 2;
		if (row < 0 || 2 * MAXDIM <= row || col < 0 || 2 * MAXDIM <= col) {
			cout << "Error: TheBoard is too small. Increase MAXDIM.\n";
			exit(1);
		}
		return data[row][col];
	}
	char& operator()(int row, int col) {
		row *= 2;
		col *= 2;
		if (row < 0 || 2 * MAXDIM <= row || col < 0 || 2 * MAXDIM <= col) {
			cout << "Error: board is too smal. Increase MAXDIM.\n";
			exit(1);
		}
		return data[row][col];
	}

	// This fills a board with spaces.
	// Note: memset would be faster (unless the compiler is already smart enough to call it for us).
	void clear() {
		for (int i = 0; i < 2 * MAXDIM; ++i) {
			for (int j = 0; j < 2 * MAXDIM; ++j) {
				data[j][i] = ' ';
			}
		}
	}

	void draw_link(int y, int x, int dir);
	void erase_link(int y, int x, int dir);

	void print();

	static constexpr int NDirs = 6;

	// These provide the offsets (in x and y coordinates) for the six neighbors.
	// 0 is East, 1 is Northeast, 2 is Northwest, 3 is West, 4 is Southwest, 5 is Southeast.
	static constexpr int DX[NDirs] = {2,  1, -1, -2, -1, 1};
	static constexpr int DY[NDirs] = {0, -1, -1,  0,  1, 1};

	// Returns true if moving in direction 0 followed by this direction makes a right turn.
	static bool is_right_turn(int dir) { return dir == 4 || dir == 5; }

private:
	std::array<std::array<char, 2 * MAXDIM>, 2 * MAXDIM> data;
};

// Pre C++17, the compiler gets salty if we don't give explicit definitions somewhere.
constexpr int HexagonalBoard::DX[HexagonalBoard::NDirs];
constexpr int HexagonalBoard::DY[HexagonalBoard::NDirs];


// Changing this line changes the grid. Square and hexagonal boards are supported.
#ifdef GRID_TRIANGULAR
using Board = HexagonalBoard;
#else
using Board = SquareBoard;
#endif

// This is the global board that we modify as we run the algorithm.
Board TheBoard;

// This stores the amino acid sequence (e.g. "HPPHPHPH").
string Pattern;

// This records the highest score we've seen so far.
int MaxScore = 0;

// This records copies of all solutions we've found with the maximum score.
vector<Board> Solutions;

// 0 => no intermediate output
// 1 => print when we find a folding that beats our current best score
// 2 => print when we find a folding that beats or matches our current best score
// 3 => print subchains of a certain length (see below)
int verbosity = 1;

void Search(
	int index,        // index of the next amino acid to be placed
	int fromY,        // y coordinate of last placed amino acid
	int fromX,        // x coordinate of last placed amino acid
	int score,        // score of placed amino acids so far
	int potential,    // roughly the number of empty squares that neighbor H amino acids. only used for pruning
	bool turned       // indicates if the current pattern has broken mirror symmetry yet
);

// Counts the number of Hs and empty squares appearing in the four squares that neighbor (x, y).
// Note that this takes x first, then y, unlike the other methods in this file.
void CountNeighbors(int x, int y, int &numH, int &numEmpty);


int main(int argc, const char *argv[]) {
	// These are the default patterns. They're different based on the board type so that they both
	// take under 10 seconds to run, and don't produce thousands of results.
	if (Board::NDirs == 6) {
		// for a hexagonal grid
		Pattern = "HHPPHPPPHHPPHPHPHPH";
	} else {
		// for a square grid
		Pattern = "PPHPHHPHPPHPHPHPPHPHHPHPPHPHPH";
	}

	if (argc > 1) {
		Pattern = argv[1];
	}
	if (argc > 2) {
		// The target score is 0 by default, but supplying a higher number can enable the algorithm
		// to prune branches more greedily and thus run faster.
		MaxScore = atoi(argv[2]);
	}
	if (argc > 3) {
		verbosity = atoi(argv[3]);
	}
	cout << "Folding " << Pattern << " for scores >= " << MaxScore << endl;

	// Initialize the board. Every square starts blank.
	TheBoard.clear();

	// Place the first two amino acids. Note that this breaks rotational symmetry, so we won't get
	// rotated versions of the same solution. (Mirror symmetry is broken below.)
	int const y0 = MAXDIM / 2;
	int const x0 = MAXDIM / 2;
	int const dir0 = 0;
	int const y1 = y0 + Board::DY[dir0];
	int const x1 = x0 + Board::DX[dir0];
	TheBoard(y0, x0) = Pattern[0];
	TheBoard(y1, x1) = Pattern[1];
	TheBoard.draw_link(y0, x0, dir0);

	// Start the search.
	auto start = std::chrono::high_resolution_clock::now();
	Search(
		// We've placed amino acids 0 and 1; next up is 2.
		2,
		// These are the coordinates of amino acid 1 (i.e. the most recently placed amino acid).
		y1,
		x1,
		// H-H has score 1, all other two amino acid chains have score 0.
		(Pattern[0] == 'H' && Pattern[1] == 'H') ? 1 : 0,
		// Each amino acid could potentially have Board::NDirs - 1 additional H neighbors.
		(Board::NDirs - 1) * ((Pattern[0] == 'H' ? 1 : 0) + (Pattern[1] == 'H' ? 1 : 0)),
		// So far all amino acids are in a straight line, so we haven't broken mirror symmetry.
		false
	);
	auto stop = std::chrono::high_resolution_clock::now();
	float elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count();

	cout << "Found " << Solutions.size() << " optimal solutions (score " << MaxScore << "):\n" << endl;
	for (auto it = Solutions.begin(); it != Solutions.end(); ++it) {
		it->print();
	}
	cout << "Found " << Solutions.size() << " optimal solutions (score " << MaxScore << ").\n";
	cout << "Runtime: " << elapsed << " milliseconds\n\n";
}


void Search(
	int index,
	int fromY,
	int fromX,
	int score,
	int potential,
	bool turned
) {
	// Print some partial foldings if we're in verbose mode.
	if (verbosity >= 3 && index == 8) {
		cout << "Interim...\n";
		cout << "Score = " << score << "\n";
		cout << "Potential = " << potential << "\n";
		TheBoard.print();
	}

	// Check if we've placed all amino acids.
	if (index == Pattern.length()) {
		// Update MaxScore if this is a new record.
		if (score > MaxScore) {
			Solutions.clear();
			Solutions.push_back(TheBoard);
			MaxScore = score;
			if (verbosity >= 1) {
				cout << "New max score: " << MaxScore << '\n';
				TheBoard.print();
			}
		}

		// Save the solution if it achieves the maximum score.
		else if (score == MaxScore) {
			Solutions.push_back(TheBoard);
			if (verbosity >= 2) {
				cout << "Score: " << score << "\n";
				TheBoard.print();
			}
		}

		return;
	}

	// Examine all squares where we could place the next amino acid.
	for (int dir = 0; dir < Board::NDirs; ++dir) {
		int const newx = fromX + Board::DX[dir];
		int const newy = fromY + Board::DY[dir];

		// If the square is already occupied, we can't use it.
		// If we haven't turned before (i.e. all amino acids have been placed along direction 0),
		// then we require that this one be placed to the left. This prevents us from getting
		// mirror images of all our solutions.
		if (TheBoard(newy, newx) != ' ' || (!turned && Board::is_right_turn(dir))) {
			continue;
		}

		// Count this square's H and empty neighbors.
		int hneighbors, emptyneighbors;
		CountNeighbors(newx, newy, hneighbors, emptyneighbors);

		// The new score is the old one plus the number of H neighbors (if we're placing an H now).
		int newscore = score + hneighbors * (Pattern[index] == 'H');

		// The new potential is the old one with two changes.
		// - We subtract the number of H neighbors, since we are placing an amino acid now that
		//   either gains the point or doesn't.
		// - If this node is an H node, we add the number of empty neighbors.
		int newp = potential + emptyneighbors * (Pattern[index] == 'H') - hneighbors;

#ifdef USE_PRUNING
		// Check if we just isolated an empty space.
		// This reduces our new potential, since even though there's an empty cell next to an H, we
		// couldn't possibly fold the protein chain to fill it. A potential improvement to the
		// algorithm would be to check for empty spaces that are larger than one square.
		for (int d2 = 0; d2 < Board::NDirs; ++d2) {
			if (d2 == dir) {
				continue;
			}

			int const x = fromX + Board::DX[d2];
			int const y = fromY + Board::DY[d2];

			if (TheBoard(y, x) == ' ') {
				int h, e;
				CountNeighbors(x, y, h, e);
				// If so, subtract inaccessible potential.
				if (e == 0) {
					newp -= h;
				}
			}
		}

		// Compute an upper bound of the points we could still add to our score.
		// Note that we are double counting some things here.
		int degree = 0;
		for (int i = index + 1; i < Pattern.size(); ++i) {
			if (Pattern[i] == 'H') {
				degree += (Board::NDirs - 2) + (Pattern[i - 1] == 'H') +
					(i == Pattern.size() - 1 ? 1 : Pattern[i + 1] == 'H');
			}
		}

		// Only place an amino acid here if it looks possible to match or beat MaxScore.
		// TODO explain the condition
		int const delta = (degree < newp) ? degree : ((degree - newp) / 2 + newp);
		if (newscore + delta >= MaxScore) {
#endif
			// Place this amino acid.
			TheBoard(newy, newx) = Pattern[index];
			TheBoard.draw_link(fromY, fromX, dir);

			// Recurse.
			// We break mirror symmetry the first time we place an amino acid not along direction 0.
			// TODO: It would probably run faster if we converted recursion to iteration.
			Search(index + 1, newy, newx, newscore, newp, turned || dir != 0);

			// Remove this amino acid.
			TheBoard.erase_link(fromY, fromX, dir);
			TheBoard(newy, newx) = ' ';
#ifdef USE_PRUNING
		}
#endif
	}
}


void CountNeighbors(int x, int y, int &numH, int &numEmpty) {
	numH = 0;
	numEmpty = 0;

	for (int dir = 0; dir < Board::NDirs; ++dir) {
		int const newx = x + Board::DX[dir];
		int const newy = y + Board::DY[dir];

		if (TheBoard(newy, newx) == 'H') {
			++numH;
		}
		if (TheBoard(newy, newx) == ' ') {
			++numEmpty;
		}
	}
}


void SquareBoard::print() {
	auto const row_is_empty = [&](int row) {
		for (int i = 0; i < MAXDIM; ++i) {
			if (data[row][i] != ' ') {
				return false;
			}
		}
		return true;
	};

	auto const col_is_empty = [&](int col) {
		for (int i = 0; i < MAXDIM; ++i) {
			if (data[i][col] != ' ') {
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

	int minx = 0;
	while (col_is_empty(minx)) {
		++minx;
	}
	// It's good to have a border on the left and right, or things look cramped.
	if (minx > 0) {
		--minx;
	}

	int maxx = MAXDIM - 1;
	while (col_is_empty(maxx)) {
		--maxx;
	}
	if (maxx < MAXDIM - 1) {
		++maxx;
	}

	int const n_cols = maxx - minx + 1;

	cout << ' ' << string(n_cols, '-') << '\n';
	for (int y = miny; y <= maxy; ++y) {
		cout << '|';
		for (int x = minx; x <= maxx; ++x) {
			cout << data[y][x];
		}
		cout << "|\n";
	}
	cout << ' ' << string(n_cols, '-') << '\n';
}

void HexagonalBoard::print() {
	auto const row_is_empty = [&](int row) {
		for (int i = 0; i < 2 * MAXDIM; ++i) {
			if (data[row][i] != ' ') {
				return false;
			}
		}
		return true;
	};

	auto const col_is_empty = [&](int col) {
		for (int i = 0; i < 2 * MAXDIM; ++i) {
			if (data[i][col] != ' ') {
				return false;
			}
		}
		return true;
	};

	int miny = 0;
	while (row_is_empty(miny)) {
		++miny;
	}

	int maxy = 2 * MAXDIM - 1;
	while (row_is_empty(maxy)) {
		--maxy;
	}

	int minx = 0;
	while (col_is_empty(minx)) {
		++minx;
	}
	// It's good to have a border on the left and right, or things look cramped.
	if (minx > 0) {
		--minx;
	}

	int maxx = 2 * MAXDIM - 1;
	while (col_is_empty(maxx)) {
		--maxx;
	}
	if (maxx < 2 * MAXDIM - 1) {
		++maxx;
	}

	int const n_cols = maxx - minx + 1;

	cout << ' ' << string(n_cols, '-') << '\n';
	for (int y = miny; y <= maxy; ++y) {
		cout << '|';
		for (int x = minx; x <= maxx; ++x) {
			cout << data[y][x];
		}
		cout << "|\n";
	}
	cout << ' ' << string(n_cols, '-') << '\n';
}

void HexagonalBoard::draw_link(int y, int x, int dir) {
	y *= 2;
	x *= 2;
	switch (dir) {
		case 0:
			data[y][x + 1] = '-';
			data[y][x + 2] = '-';
			data[y][x + 3] = '-';
			break;
		case 1:
			data[y - 1][x + 1] = '/';
			break;
		case 2:
			data[y - 1][x - 1] = '\\';
			break;
		case 3:
			data[y][x - 1] = '-';
			data[y][x - 2] = '-';
			data[y][x - 3] = '-';
			break;
		case 4:
			data[y + 1][x - 1] = '/';
			break;
		case 5:
			data[y + 1][x + 1] = '\\';
			break;
	}
}

void HexagonalBoard::erase_link(int y, int x, int dir) {
	y *= 2;
	x *= 2;
	switch (dir) {
		case 0:
			data[y][x + 1] = ' ';
			data[y][x + 2] = ' ';
			data[y][x + 3] = ' ';
			break;
		case 1:
			data[y - 1][x + 1] = ' ';
			break;
		case 2:
			data[y - 1][x - 1] = ' ';
			break;
		case 3:
			data[y][x - 1] = ' ';
			data[y][x - 2] = ' ';
			data[y][x - 3] = ' ';
			break;
		case 4:
			data[y + 1][x - 1] = ' ';
			break;
		case 5:
			data[y + 1][x + 1] = ' ';
			break;
	}
}
