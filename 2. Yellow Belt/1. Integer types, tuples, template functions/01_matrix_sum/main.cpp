#include <iostream>
#include <stdexcept>
#include <vector>

using namespace std;

// Implement here
// * Matrix class
// * input operator for the Matrix class from istream
// * output operator of the Matrix class to ostream
// * operator to check the equality of two objects of the Matrix class
// * operator to add two objects of the Matrix class

class Matrix {
public:
    Matrix() : numRows(0), numCols(0) { elements.resize(0, vector<int>(0)); }

    Matrix(int newNumRows, int newNumCols) {
        if (newNumRows < 0 || newNumCols < 0) {
            throw out_of_range("");
        }

        numRows = newNumRows;
        numCols = newNumCols;
        Reset(numRows, numCols);
    }

    void Reset(int newNumRows, int newNumCols) {
        if (newNumRows < 0 || newNumCols < 0) {
            throw out_of_range("");
        }

        numRows = newNumRows;
        numCols = newNumCols;

        if (numRows * numCols == 0) {
            return;
        }

        elements.resize(numRows, vector<int>(numCols, 0));
        for (int i = 0; i < numRows; i++) {
            for (int j = 0; j < numCols; j++) {
                elements[i][j] = 0;
            }
        }
    }

    [[nodiscard]] int At(int row, int col) const {
        if (row < 0 || row >= numRows || col < 0 || col >= numCols) {
            throw out_of_range("");
        }

        return elements[row][col];
    }

    int &At(int row, int col) {
        if (row < 0 || row >= numRows || col < 0 || col >= numCols) {
            throw out_of_range("");
        }

        return elements[row][col];
    }

    [[nodiscard]] int GetNumRows() const { return numRows; }

    [[nodiscard]] int GetNumColumns() const { return numCols; }

private:
    int numRows, numCols;
    vector<vector<int>> elements;
};

istream &operator>>(istream &stream, Matrix &matrix) {
    int numRows, numCols;
    stream >> numRows >> numCols;
    matrix = Matrix(numRows, numCols);
    for (int i = 0; i < numRows; i++) {
        for (int j = 0; j < numCols; j++) {
            stream >> matrix.At(i, j);
        }
    }

    return stream;
}

ostream &operator<<(ostream &stream, const Matrix &matrix) {
    int numRows = matrix.GetNumRows();
    int numCols = matrix.GetNumColumns();
    stream << numRows << " " << numCols << endl;
    for (int i = 0; i < numRows; i++) {
        for (int j = 0; j < numCols; j++) {
            stream << matrix.At(i, j);
            if (j < numCols - 1)
                stream << " ";
        }
        stream << endl;
    }

    return stream;
}

bool operator==(const Matrix &a, const Matrix &b) {
    int numRows = a.GetNumRows();
    int numCols = a.GetNumColumns();

    if (numRows * numCols == 0 && b.GetNumRows() * b.GetNumColumns() == 0) {
        return true;
    }

    if (numRows != b.GetNumRows() || numCols != b.GetNumColumns()) {
        return false;
    }

    for (int i = 0; i < numRows; i++) {
        for (int j = 0; j < numCols; j++) {
            if (a.At(i, j) != b.At(i, j)) {
                return false;
            }
        }
    }

    return true;
}

Matrix operator+(const Matrix &a, const Matrix &b) {
    int numRows = a.GetNumRows();
    int numCols = a.GetNumColumns();

    if (numRows * numCols == 0 && b.GetNumRows() * b.GetNumColumns() == 0) {
        return {};
    }

    if (numRows != b.GetNumRows() || numCols != b.GetNumColumns()) {
        throw invalid_argument("");
    }

    Matrix c = Matrix(numRows, numCols);
    for (int i = 0; i < numRows; i++) {
        for (int j = 0; j < numCols; j++) {
            c.At(i, j) = a.At(i, j) + b.At(i, j);
        }
    }

    return c;
}

int main() {
    Matrix one;
    Matrix two;

    cin >> one >> two;
    cout << one + two << endl;
    return 0;
}
