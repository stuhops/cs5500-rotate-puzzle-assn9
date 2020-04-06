/* Project 1 Rotation Puzzle.cpp : Defines the entry point for the console application.
 * 
 * Author: Stuart Hopkins (A02080107)
 * Last updated: 1/24/2018
*/

#include <mpi.h>
#include <iostream>
#include <string>
#include <vector>
#include "Board.h"
#include "Board.cpp"

using namespace std;

#define MCW MPI_COMM_WORLD


// ------------------------------- Functions ----------------------------------------
Board initialize();

vector<Board> prioritizeQueue(vector<Board> queue);

void print(string message, vector<int> arr);
void print(string message, int x);
void printQueue(vector<Board> queue);
void printQueueRanks(vector<Board> queue);
void printBreak();


// --------------------------------- Main -------------------------------------------
int main(int argc, char **argv) {
	const int ROWS = 3;
	const int DIRECTIONS = 4;  // N, E, S, W
	const int ITERS = 1;

  int rank, size;
  int data[ROWS * ROWS];
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MCW, &rank);
  MPI_Comm_size(MCW, &size);
	MPI_Request request;
	MPI_Status status;

	int finished = 0;
	int primary_board_sum = 0;
	int num_of_levels = 0;
	int state1 = 0;
	int again = 0;
	std::string history_string = "";
	int board_int = 0;
	Board primary_board;
	Board curr_board;
	vector<Board> queue;

	if(!rank) {
		for(int i = 0; i < size; i++) {
			MPI_Irecv(&finished, 1, MPI_INT, MPI_ANY_SOURCE, 1, MCW, &request);
		}

		primary_board = initialize();
		queue.push_back(primary_board);

		if (!primary_board.getRank()) {
			std::cout << "You started with a perfect board." << endl;
		}
		else {
			while (true) {
				num_of_levels++;
				for (int i = 0; i < ROWS; i++) { 
					for (int j = 0; j < DIRECTIONS; j++) {
						state1++;
						curr_board = queue[0];
						curr_board.move(DIRECTIONS * i + j);
						// std::cout << "State " << state1 << " from state " << queue[0].getState() << " History " << curr_board.history() << endl
						// 					<< curr_board.getRank() << endl
						// 					<< curr_board.toString() << endl;
						queue.push_back(curr_board);
						if (!curr_board.getRank()) {
							break;
						}
					}
					if (!curr_board.getRank()) break;
				}
				queue.erase(queue.begin());
				if (!curr_board.getRank()) {
					finished = 1;
					for(int j = 1; j < size; j++) {
						MPI_Send(&finished, 1, MPI_INT, j, 1, MCW);
					}
					break;
				}

				queue = prioritizeQueue(queue);

				// ------------------------------- Send Work Out ------------------------------------
				if(!(num_of_levels % ITERS)) {
					if(num_of_levels != 1) {
						// ------------------ Receive --------------------
						for(int i = 0; i < size; i++) {
							MPI_Test(&request, &finished, &status);
							if(finished) {
								std::cout << "YOU WIN!!! Original Board:" << endl << primary_board.toString() << endl;
								for(int j = 1; j < size; j++) {
									MPI_Send(&finished, 1, MPI_INT, j, 1, MCW);
								}
								break;
							}

							for(int j = 0; j < size; j++) {
								MPI_Recv(&data, ROWS * ROWS, MPI_INT, i, 0, MCW, MPI_STATUS_IGNORE);
								queue.push_back(Board(data));
							}
						}
						queue = prioritizeQueue(queue);
					}

					// --------------------- Send ----------------------
					vector<int> curr;
					for(int i = 1; i < size; i++) {
						curr = queue[0].toVect();
						queue.erase(queue.begin());

						int to_send[curr.size()];
						for(int j = 0; j < queue.size(); j++) {
							to_send[j] = curr[j];
						}
						MPI_Send(&to_send, ROWS * ROWS, MPI_INT, i, 0, MCW);
					}

					curr_board = queue[0];
					queue.clear();
					queue.push_back(curr_board);
				}
				if(finished) break;
			}
		}
		std::cout << "YOU WIN!!! Original Board:" << endl << primary_board.toString() << endl;
		queue.clear();
		int to_send = 0;
	}

	// ----------------------------------- Slave Processes -------------------------------------
	else {
		MPI_Irecv(&finished, 1, MPI_INT, 0, 1, MCW, &request);
		while(true) {
			MPI_Test(&request, &finished, &status);
			if(finished) 
				break;

			queue.clear();
			MPI_Recv(&data, ROWS * ROWS, MPI_INT, 0, 0, MCW, MPI_STATUS_IGNORE);
			cout << rank << " Data Received" << endl;
			queue.push_back(Board(data));

			for(int iters = 0; iters < ITERS; iters++) {
				for (int i = 0; i < ROWS; i++) { 
					for (int j = 0; j < DIRECTIONS; j++) {
						state1++;
						curr_board = queue[0];
						curr_board.move(DIRECTIONS * i + j);
						// std::cout << "State " << state1 << " from state " << queue[0].getState() << " History " << curr_board.history() << endl
						// 					<< curr_board.getRank() << endl
						// 					<< curr_board.toString() << endl;
						queue.push_back(curr_board);
						if (!curr_board.getRank()) {
							break;
						}
					}
					if (!curr_board.getRank()) break;
				}
				queue.erase(queue.begin());
				if (!curr_board.getRank()) {
					finished = 1;
					MPI_Send(&finished, 1, MPI_INT, 0, 1, MCW);
					break;
				}

				queue = prioritizeQueue(queue);
			}
			if (!curr_board.getRank()) break;


			vector<int> curr;
			for(int i = 0; i < size; i++) {
				curr = queue[0].toVect();
				queue.erase(queue.begin());

				int to_send[curr.size()];
				for(int j = 0; j < queue.size(); j++) {
					to_send[j] = curr[j];
				}
				MPI_Send(&to_send, ROWS * ROWS, MPI_INT, 0, 0, MCW);
			}

		}
	}

	MPI_Finalize();
	return 0;
}


// --------------------------------------- Functions -----------------------------------------
Board initialize() {
	int input = 0;
	Board start1;
	Board start2;
	Board start3;
	Board start4;

	//start1 set up
	start1.rotateEast(1);
	start1.rotateEast(2);

	//start2 set up
	start2.rotateNorth(0);
	start2.rotateSouth(2);

	//start 3 set up
	start3.rotateNorth(2);
	start3.rotateSouth(0);
	start3.rotateEast(0);
	start3.rotateEast(2);

	//start 4 set up
	start4.makeBoard(4);
	//Print out the to the terminal to give the user options of boards to start from.
	std::cout << endl << "Option 1: " << endl << start1.toString() << endl;
	std::cout << endl << "Option 2: " << endl << start2.toString() << endl;
	std::cout << endl << "Option 3: " << endl << start3.toString() << endl;
	std::cout << endl << "Option 4: " << endl << start4.toString() << endl;
		
	std::cout << "Please enter the number of the board you want to start with: " << endl;
	while (true) {
		std::cin >> input;

		if (input >= 1 && input <= 4)
			break;
		std::cout << "You have entered an incorrect value please enter a valid number of option: ";
	}
	switch (input)
	{
		case 1:  return start1;
		case 2:  return start2;
		case 3:  return start3;
		case 4:  return start4;
	}
	return start1;
}


vector<Board> prioritizeQueue(vector<Board> queue) {
	bool flag = false;
	vector<Board> new_queue;
	new_queue.push_back(queue[0]);
	for(int i = 1; i < queue.size(); i++) {
		flag = false;
		for(int j = 0; j < new_queue.size(); j++) {
			// cout << endl << queue[i].getRank() << endl;
			if(queue[i].getRank() < new_queue[j].getRank()) {
				flag = true;
				new_queue.insert(new_queue.begin() + j, queue[i]);
				break;
			}
		}
		if(!flag) {
			new_queue.push_back(queue[i]);
		}
	}
	return new_queue;
}


// Print functions
void print(string message, vector<int> vect) {
  cout << message << ": " << endl;

  cout << vect[0];
  for(int i = 1; i < vect.size(); i++) {
    cout << ", " << vect[i];
  }
  cout << endl;
}

void print(string message, int x) {
  cout << message << ": " << x << endl;
}


void printQueue(vector<Board> queue) {
	printBreak();
	cout << "Queue Ranks and Boards: " << endl;
	for(int i = 0; i < queue.size(); i++) {
		cout << queue[i].getRank() << endl
				 << queue[i].history() << endl
				 << queue[i].toString() << endl << endl;
	}
	printBreak();
}

void printQueueRanks(vector<Board> queue) {
	printBreak();
	cout << endl << "Queue Ranks: " << endl;
	for(int i = 0; i < queue.size(); i++) {
		cout << queue[i].getRank() << ", ";
	}
	printBreak();
}

void printBreak() {
  cout << "\n--------------------------------\n";
}
