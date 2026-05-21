#include "SudokuBoard.h"
#include <QGridLayout>
#include <QRandomGenerator>
#include <QRegularExpressionValidator>
#include <algorithm>
#include <numeric>
#include <functional>


static const char *C_CELL_EMPTY = "#FFFFFF";
static const char *C_CELL_GIVEN = "#EDECE8";
static const char *C_CELL_SELECTED = "#18181B";
static const char *C_CELL_RELATED = "#F2F1EE";
static const char *C_CELL_MATCH = "#DBEAFE";

static const char *C_TEXT_GIVEN = "#18181B";
static const char *C_TEXT_USER = "#2563EB";
static const char *C_TEXT_SEL = "#FFFFFF";
static const char *C_TEXT_ERROR = "#DC2626";
static const char *C_TEXT_ERR_SEL = "#FCA5A5";


SudokuCell::SudokuCell(QWidget *parent) : QWidget(parent) {
    setFixedSize(54, 54);

    QGridLayout *layout = new QGridLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    lineEdit = new QLineEdit(this);
    lineEdit->setFixedSize(54, 54);
    lineEdit->setMaxLength(1);
    lineEdit->setValidator(new QRegularExpressionValidator(
        QRegularExpression("^[1-9]$"), this));
    lineEdit->setFrame(false);

    connect(lineEdit, &QLineEdit::textChanged, this, &SudokuCell::textChanged);

    layout->addWidget(lineEdit, 0, 0);
    setLayout(layout);
}

void SudokuCell::setReadOnly(bool ro) {
    lineEdit->setReadOnly(ro);
    if (ro) lineEdit->clearFocus();
}

bool SudokuCell::isReadOnly() const {
    return lineEdit->isReadOnly();
}

void SudokuCell::setValue(int val) {
    if (val == 0) lineEdit->clear();
    else lineEdit->setText(QString::number(val));
}

int SudokuCell::value() const {
    QString t = lineEdit->text().trimmed();
    return t.isEmpty() ? 0 : t.toInt();
}

void SudokuCell::setCellFocus() {
    lineEdit->setFocus();
    lineEdit->selectAll();
}

void SudokuCell::applyStyle(const QString &bgColor,
                            const QString &textColor,
                            const QString &borderStyle,
                            bool isBold) {
    lineEdit->setStyleSheet(QString(
            "QLineEdit {"
            "  background-color: %1; color: %2; %3"
            "  font-family: 'Outfit';"
            "  font-size: 20pt;"
            "  font-weight: %4;"
            "  qproperty-alignment: 'AlignCenter';"
            "}")
        .arg(bgColor, textColor, borderStyle,
             isBold ? "600" : "400"));
}


void SudokuBoard::createBoard() {
    QGridLayout *layout = new QGridLayout(this);
    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);

    setFixedSize(CELL_SIZE * BOARD_SIZE, CELL_SIZE * BOARD_SIZE);

    cells.resize(BOARD_SIZE);
    given.resize(BOARD_SIZE);
    answer.resize(BOARD_SIZE);

    for (int r = 0; r < BOARD_SIZE; ++r) {
        cells[r].resize(BOARD_SIZE);
        given[r].resize(BOARD_SIZE);
        answer[r].resize(BOARD_SIZE);

        for (int c = 0; c < BOARD_SIZE; ++c) {
            SudokuCell *cell = new SudokuCell(this);
            cell->installEventFilter(this);
            cell->getLineEdit()->installEventFilter(this);
            connect(cell, &SudokuCell::textChanged,
                    this, &SudokuBoard::onCellTextChanged);
            layout->addWidget(cell, r, c);
            cells[r][c] = cell;
        }
    }
    setLayout(layout);
}

SudokuBoard::SudokuBoard(QWidget *parent) : QWidget(parent) {
    createBoard();
}


bool SudokuBoard::eventFilter(QObject *watched, QEvent *event) {
    SudokuCell *clickedCell = qobject_cast<SudokuCell *>(watched);
    if (!clickedCell) {
        QLineEdit *le = qobject_cast<QLineEdit *>(watched);
        if (le) clickedCell = qobject_cast<SudokuCell *>(le->parentWidget());
    }

    if (clickedCell) {
        if (event->type() == QEvent::MouseButtonPress)
            clickedCell->setCellFocus();

        if (event->type() == QEvent::FocusIn) {
            for (int r = 0; r < BOARD_SIZE; ++r) {
                for (int c = 0; c < BOARD_SIZE; ++c) {
                    if (cells[r][c] == clickedCell) {
                        updateSubtleHighlight(r, c);
                        return QWidget::eventFilter(watched, event);
                    }
                }
            }
        }
    }
    return QWidget::eventFilter(watched, event);
}


void SudokuBoard::updateSubtleHighlight(int focusedRow, int focusedCol) {
    m_lastFocusedRow = focusedRow;
    m_lastFocusedCol = focusedCol;

    int focusedNum = cells[focusedRow][focusedCol]->value();
    int boxStartRow = (focusedRow / 3) * 3;
    int boxStartCol = (focusedCol / 3) * 3;

    for (int r = 0; r < BOARD_SIZE; ++r) {
        for (int c = 0; c < BOARD_SIZE; ++c) {
            SudokuCell *cell = cells[r][c];
            bool isGiven = cell->isReadOnly();
            int curNum = cell->value();
            bool isWrong = !isGiven && curNum > 0 && curNum != answer[r][c];

            QString bg;
            bool isSelected = (r == focusedRow && c == focusedCol);
            bool isMatch = focusedNum > 0 && curNum == focusedNum && !isSelected;
            bool isRelated = (r == focusedRow || c == focusedCol ||
                              (r >= boxStartRow && r < boxStartRow + 3 &&
                               c >= boxStartCol && c < boxStartCol + 3));

            if (isSelected) bg = C_CELL_SELECTED;
            else if (isMatch) bg = C_CELL_MATCH;
            else if (isRelated) bg = C_CELL_RELATED;
            else bg = isGiven ? C_CELL_GIVEN : C_CELL_EMPTY;

            QString fg;
            if (isSelected) {
                fg = isWrong ? C_TEXT_ERR_SEL : C_TEXT_SEL;
            } else if (isWrong) {
                fg = C_TEXT_ERROR;
            } else {
                fg = isGiven ? C_TEXT_GIVEN : C_TEXT_USER;
            }


            int L = (c % 3 == 0) ? 2 : 1;
            int T = (r % 3 == 0) ? 2 : 1;
            int R = (c == 8) ? 2 : ((c % 3 == 2) ? 2 : 1);
            int B = (r == 8) ? 2 : ((r % 3 == 2) ? 2 : 1);

            auto edgeColor = [&](int px) -> QString {
                return (px == 2) ? "#18181B" : "#D4D4D8";
            };

            QString borderStr = QString(
                        "border-left:   %1px solid %2;"
                        "border-top:    %3px solid %4;"
                        "border-right:  %5px solid %6;"
                        "border-bottom: %7px solid %8;")
                    .arg(L).arg(edgeColor(L))
                    .arg(T).arg(edgeColor(T))
                    .arg(R).arg(edgeColor(R))
                    .arg(B).arg(edgeColor(B));

            cell->applyStyle(bg, fg, borderStr, isGiven);
        }
    }
}


void SudokuBoard::onCellTextChanged() {
    if (m_blockSignals) return;

    SudokuCell *typedCell = qobject_cast<SudokuCell *>(sender());
    if (!typedCell) return;

    int typedRow = -1, typedCol = -1;
    for (int r = 0; r < BOARD_SIZE && typedRow == -1; ++r) {
        for (int c = 0; c < BOARD_SIZE && typedRow == -1; ++c) {
            if (cells[r][c] == typedCell) {
                typedRow = r;
                typedCol = c;
            }
        }
    }

    if (typedRow != -1) {
        int num = typedCell->value();
        if (num > 0 && num != answer[typedRow][typedCol]) {
            emit wrongMove();
        } else if (num > 0 && num == answer[typedRow][typedCol]) {
            if (emptyCellCount() == 0 && checkSolution())
                emit puzzleSolved();
        }
    }

    int hr = (m_lastFocusedRow != -1) ? m_lastFocusedRow : 0;
    int hc = (m_lastFocusedCol != -1) ? m_lastFocusedCol : 0;
    updateSubtleHighlight(hr, hc);
    emit boardChanged();
}


void SudokuBoard::loadPuzzle(const QVector<QVector<int> > &puzzle,
                             const QVector<QVector<int> > &solution) {
    m_blockSignals = true;
    m_lastFocusedRow = -1;
    m_lastFocusedCol = -1;
    given = puzzle;
    answer = solution;

    for (int r = 0; r < BOARD_SIZE; ++r) {
        for (int c = 0; c < BOARD_SIZE; ++c) {
            cells[r][c]->setValue(puzzle[r][c]);
            cells[r][c]->setReadOnly(puzzle[r][c] != 0);
        }
    }
    m_blockSignals = false;
    updateSubtleHighlight(0, 0);
}

void SudokuBoard::newGame(Difficulty difficulty) {
    QVector<QVector<int> > full = generateFull();
    QVector<QVector<int> > puzzle = makePuzzle(full, static_cast<int>(difficulty));
    loadPuzzle(puzzle, full);
}


bool SudokuBoard::checkSolution() {
    for (int r = 0; r < BOARD_SIZE; ++r)
        for (int c = 0; c < BOARD_SIZE; ++c)
            if (cells[r][c]->value() != answer[r][c]) return false;
    return true;
}

bool SudokuBoard::giveHint() {
    QVector<QPair<int, int> > empty;
    for (int r = 0; r < BOARD_SIZE; ++r)
        for (int c = 0; c < BOARD_SIZE; ++c)
            if (cells[r][c]->value() == 0) empty.append({r, c});

    if (empty.isEmpty()) return false;

    int idx = QRandomGenerator::global()->bounded(empty.size());
    int r = empty[idx].first;
    int c = empty[idx].second;

    m_blockSignals = true;
    cells[r][c]->setValue(answer[r][c]);
    cells[r][c]->setReadOnly(true);
    m_blockSignals = false;

    if (emptyCellCount() == 0 && checkSolution()) {
        emit puzzleSolved();
    } else {
        int hr = (m_lastFocusedRow != -1) ? m_lastFocusedRow : 0;
        int hc = (m_lastFocusedCol != -1) ? m_lastFocusedCol : 0;
        updateSubtleHighlight(hr, hc);
    }
    emit boardChanged();
    return true;
}

int SudokuBoard::emptyCellCount() const {
    int n = 0;
    for (int r = 0; r < BOARD_SIZE; ++r)
        for (int c = 0; c < BOARD_SIZE; ++c)
            if (cells[r][c]->value() == 0) ++n;
    return n;
}


bool SudokuBoard::findEmpty(const QVector<QVector<int> > &b, int &row, int &col) {
    for (row = 0; row < 9; ++row)
        for (col = 0; col < 9; ++col)
            if (b[row][col] == 0) return true;
    return false;
}

bool SudokuBoard::isValid(const QVector<QVector<int> > &b, int row, int col, int num) {
    for (int i = 0; i < 9; ++i) {
        if (b[row][i] == num) return false;
        if (b[i][col] == num) return false;
    }
    int br = (row / 3) * 3, bc = (col / 3) * 3;
    for (int dr = 0; dr < 3; ++dr)
        for (int dc = 0; dc < 3; ++dc)
            if (b[br + dr][bc + dc] == num) return false;
    return true;
}

bool SudokuBoard::solve(QVector<QVector<int>> &b)
{
    int bestRow = -1, bestCol = -1, bestCount = 10;

    for (int r = 0; r < 9; ++r) {
        for (int c = 0; c < 9; ++c) {
            if (b[r][c] != 0) continue;
            int cnt = 0;
            for (int n = 1; n <= 9; ++n)
                if (isValid(b, r, c, n)) ++cnt;
            if (cnt == 0) return false;
            if (cnt < bestCount) {
                bestCount = cnt;
                bestRow   = r;
                bestCol   = c;
            }
        }
    }

    if (bestRow == -1) return true;

    for (int n = 1; n <= 9; ++n) {
        if (isValid(b, bestRow, bestCol, n)) {
            b[bestRow][bestCol] = n;
            if (solve(b)) return true;
            b[bestRow][bestCol] = 0;
        }
    }
    return false;
}


int SudokuBoard::countSolutions(QVector<QVector<int>> b, int limit)
{
    int bestRow = -1, bestCol = -1, bestCount = 10;

    for (int r = 0; r < 9; ++r) {
        for (int c = 0; c < 9; ++c) {
            if (b[r][c] != 0) continue;
            int cnt = 0;
            for (int n = 1; n <= 9; ++n)
                if (isValid(b, r, c, n)) ++cnt;
            if (cnt == 0) return 0;
            if (cnt < bestCount) {
                bestCount = cnt;
                bestRow   = r;
                bestCol   = c;
            }
        }
    }

    if (bestRow == -1) return 1;

    int count = 0;
    for (int n = 1; n <= 9; ++n) {
        if (isValid(b, bestRow, bestCol, n)) {
            b[bestRow][bestCol] = n;
            count += countSolutions(b, limit);
            if (count >= limit) return count;
        }
    }
    return count;
}


QVector<QVector<int> > SudokuBoard::generateFull() {
    QVector<QVector<int> > board(9, QVector<int>(9, 0));

    std::function<bool(QVector<QVector<int> > &)> fill =
            [&](QVector<QVector<int> > &b) -> bool {
        int row, col;
        if (!findEmpty(b, row, col)) return true;

        QVector<int> nums = {1, 2, 3, 4, 5, 6, 7, 8, 9};
        for (int i = 8; i > 0; --i) {
            int j = QRandomGenerator::global()->bounded(i + 1);
            std::swap(nums[i], nums[j]);
        }
        for (int n: nums) {
            if (isValid(b, row, col, n)) {
                b[row][col] = n;
                if (fill(b)) return true;
                b[row][col] = 0;
            }
        }
        return false;
    };

    fill(board);
    return board;
}

QVector<QVector<int> > SudokuBoard::makePuzzle(QVector<QVector<int> > full, int clues) {
    QVector<int> positions(81);
    std::iota(positions.begin(), positions.end(), 0);
    for (int i = 80; i > 0; --i) {
        int j = QRandomGenerator::global()->bounded(i + 1);
        std::swap(positions[i], positions[j]);
    }

    int removed = 0;
    int target = 81 - clues;

    for (int pos: positions) {
        if (removed >= target) break;

        int r = pos / 9;
        int c = pos % 9;
        int backup = full[r][c];
        full[r][c] = 0;


        if (countSolutions(full, 2) == 1) {
            ++removed;
        } else {
            full[r][c] = backup;
        }
    }

    return full;
}
