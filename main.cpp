#include <iostream>
#include <fstream>
#include <vector>
#include <omp.h>
#include <unistd.h>
#include <time.h>
#include <chrono>
#include <atomic>

using namespace std;
using namespace chrono;

atomic<int> k;

struct Point {
    Point(int x, int y): x(x), y(y) {}
    int x;
    int y;
};

int bestSteps = 1000;
vector<Point> bestPath;
vector<Point> path;

void moveTemp(int * temp, int iteration){
    switch (iteration){
        case 0: // move right
            temp[0]++;
            break;
        case 1: // move top-right
            temp[0]++;
            temp[1]++;
            break;
        case 2: // move top
            temp[1]++;
            break;
        case 3: //move top-left
            temp[0]--;
            temp[1]++;
            break;
        case 4: // move left
            temp[0]--;
            break;
        case 5: // move down-left
            temp[0]--;
            temp[1]--;
            break;
        case 6: // move down
            temp[1]--;
            break;
        case 7: // move down-right
            temp[0]++;
            temp[1]--;
            break;
        default:
            break;
    }
}

void deleteArr(int **arr, int size){
    for (int i = 0; i < size; ++i)
        delete [] arr[i];

    delete []arr;
}

int **parseFile(int &boundary, int &size, const char * filename){

    string s;
    ifstream file(filename);
    if (!file) {
      cerr << "Can't open " << filename << endl;
      exit(1);
    }
    file >> size;
    file.ignore(1, '\n');
    file >> boundary;
    file.ignore(size,'\n');
    int **board = new int * [size];

    for (int i = 0; i < size; i++) {
        board[i] = new int [size]();
    }

    int j = 0;
    while(getline(file, s)){
        for (int i = 0; i < s.length(); ++i) {
            board[j][i] =  (int) s.at(i) - 48;
        }
        j++;
    }

    file.close();
    return board;
}

void findQueen(int **board, int *queen, int size){

    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            if(board[i][j] == 3){ //check queen
                queen[0] = i;
                queen[1] = j;
                return;
            }
        }
    }
}

int **findNextSteps(int **board, int *queen, int &size, int &stepsCount, bool **visited){
    stepsCount = 0;// amount of steps for next turn

    int **delSteps = new int *[size*size];// if step will get black piece, coordinates of this step will store in this array
    for (int i = 0; i < size*size; i++) {
        delSteps[i] = new int [2]();
    }
    int **simpleSteps = new int *[size*size]; // other steps coordinates will store here
    for (int i = 0; i < size*size; i++) {
        simpleSteps[i] = new int [2]();
    }
    int **visitedSteps = new int *[size*size]; //if step was visited, it will store here
    for (int i = 0; i < size*size; i++) {
        visitedSteps[i] = new int [2]();
    }

    int simpleIndex = 0;
    int delIndex = 0;
    int visitedIndex = 0;
    int * temp = new int [2];
    temp[0] = queen[0];
    temp[1] = queen[1];
    int iter = 0;

    while (iter < 8) {
        moveTemp(temp, iter); // choose move direction

        if (!(temp[0] >= 0 && temp[0] < size && temp[1] >= 0 && temp[1] < size) || board[temp[0]][temp[1]] == 2) { // check boundary
            temp[0] = queen[0];
            temp[1] = queen[1];
            iter++;
            continue;
        } else {
            if(board[temp[0]][temp[1]] == 1){ // check if step will have black piece
                delSteps[delIndex][0] = temp[0];
                delSteps[delIndex][1] = temp[1];
                delIndex++;
            } else if(visited[temp[0]][temp[1]]){ // check if step was visited
                visitedSteps[visitedIndex][0] = temp[0];
                visitedSteps[visitedIndex][1] = temp[1];
                visitedIndex++;

            } else {
                simpleSteps[simpleIndex][0] = temp[0];
                simpleSteps[simpleIndex][1] = temp[1];
                simpleIndex++;
            }
        }
    }

    stepsCount = simpleIndex + delIndex + visitedIndex;
    int **nextSteps = new int *[stepsCount]; // array of all possible steps for a turn

    for (int i = 0; i < stepsCount; i++) {
        nextSteps[i] = new int [2]();
    }
    // fill the array, first will be steps, that have black pieces, and last will be steps that were visited
    for (int j = 0; j < delIndex; ++j) {
        for (int i = 0; i < 2; ++i) {
            nextSteps[j][i] = delSteps[j][i];
        }
    }
    for (int j = delIndex; j < (simpleIndex + delIndex); ++j) {
        nextSteps[j][0] = simpleSteps[j-delIndex][0];
        nextSteps[j][1] = simpleSteps[j-delIndex][1];
    }
    for(int j = (simpleIndex + delIndex); j < stepsCount; j++){
        nextSteps[j][0] = visitedSteps[j-(simpleIndex + delIndex)][0];
        nextSteps[j][1] = visitedSteps[j-(simpleIndex + delIndex)][1];
    }

    delete [] temp;
    deleteArr(simpleSteps, size*size);
    deleteArr(delSteps, size*size);
    deleteArr(visitedSteps, size*size);

    return nextSteps;
}

int calculateEnemy(int **board, int size){

    int enemyCount = 0;

    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            if(board[i][j] == 1){ // check black pieces
                enemyCount++;
            }
        }
    }
    return enemyCount;
}

void checkBoard(int **board, vector<Point> path){

    for (int i = 0; i < path.size(); ++i) {
        cout << " (" << path.at(i).x << "," << path.at(i).y << ")";
        if(board[path.at(i).x][path.at(i).y] == 1){
            cout << "*";
        }
    }
    cout << endl;
}

int **copyArr(int **arr, int size){

  int **newArr = new int *[size];
  for (int i = 0; i < size; i++) {
      newArr[i] = new int [size]();
  }

  for (int i = 0; i < size; ++i) {
      for (int j = 0; j < size; ++j) {
          newArr[i][j] = arr[i][j];
      }
  }
  return newArr;
}

void goDeeper(int **board, int size, int *queen, int boundary, int enemyCount, bool **visited) { //recursion function
    k--;
    if (!enemyCount && path.size() < bestSteps) {  //if found a better solution

        #pragma omp critical
        {
            if (path.size() < bestSteps) {
                bestSteps = path.size();
                bestPath = path;
              //  return;
            }
        }
    }

    int stepsCount = 0;
    int ** steps = findNextSteps(board, queen, size, stepsCount, visited); // find all possible steps for the next turn
    int chosenNode = 0;

    while (chosenNode < stepsCount) {

          if (path.size() + enemyCount < bestSteps && boundary > 0) { // check
              path.push_back(Point(steps[chosenNode][0], steps[chosenNode][1])); // add to path
              visited[steps[chosenNode][0]][steps[chosenNode][1]] = true; // make visited

              int *newQueen = new int[2]; // move queen
              newQueen[0] = steps[chosenNode][0];
              newQueen[1] = steps[chosenNode][1];
              int newEnemyCount = enemyCount;

              // make new board
          int**    newBoard = copyArr(board, size);

              if (board[newQueen[0]][newQueen[1]] == 1) { // check if has black piece
                  newBoard[newQueen[0]][newQueen[1]] = 0;
                  newEnemyCount--;
              }

              bool isTask =  (path.size() + newEnemyCount < bestSteps && (boundary-1) > 0);
              if(isTask){
                  if(k <= 2){
                      k++;
                      #pragma omp task shared(chosenNode, bestPath, bestSteps) firstprivate(path, newEnemyCount, boundary, newBoard, newQueen, visited)
                      {
                        goDeeper(newBoard, size, newQueen, boundary - 1, newEnemyCount, visited); // recursion
                      }
                      #pragma omp taskwait
                  } else {
                    goDeeper(newBoard, size, newQueen, boundary - 1, newEnemyCount, visited); // recursion
                  }
              }

              delete[] newQueen;
              deleteArr(newBoard, size);
              path.pop_back(); // delete from path

              visited[steps[chosenNode][0]][steps[chosenNode][1]] = false;
          }
          chosenNode++;
    }

    for (int i = 0; i < stepsCount; i++) {
        delete [] steps[i];
    }
    delete [] steps;
    return;
}


int solve(int **board, int size, int *queen, int boundary, int enemyCount) {
    // make visited array
    bool ** visited = new bool* [size];
    for (int i = 0; i < size; i++) {
        visited[i] = new bool[size]();
    }
    visited[queen[0]][queen[1]] = true;


    #pragma omp parallel shared(bestPath, bestSteps, size) firstprivate(board, queen, boundary, enemyCount, visited)
    {
        #pragma omp single
        {
          goDeeper(board, size, queen, boundary, enemyCount, visited); // start recursion
        }
    }

    for (int i = 0; i < size; i++) {
        delete [] visited[i];
    }
    delete [] visited;

    return bestSteps;
}

int findQueenMoves (const char * filename) {
    //clear global
    bestPath.clear();
    path.clear();
    bestSteps = 1000;
    int **board;
    int boundary, size;
    int queen[2]; // queen coordinates
    board = parseFile(boundary, size, filename); // parse file and make board
    findQueen(board, queen, size); // find queen coordinates
    int enemyCount = calculateEnemy(board, size); // calculate amount of black pieces
    int result = solve(board, size, queen, boundary, enemyCount);
    checkBoard(board, bestPath); // print path
    deleteArr(board, size);

    return result;
}

int main(int argc, char* argv[]){
  if (argc != 3) {
    cout << "Usage: " << argv[0] << " <thread_count> <test_file>" << endl;
    return 1;
  }

  int thread_count = atoi(argv[1]);
  cout << "Threads " << thread_count << endl;
  omp_set_num_threads(thread_count);

  auto start = std::chrono::high_resolution_clock::now();


  cout << findQueenMoves(argv[2]) << endl;

  auto stop = std::chrono::high_resolution_clock::now();

  cout << "Time " << std::chrono::duration_cast<std::chrono::microseconds>(stop-start).count()/1000. << '\n';
    return 0;
}
