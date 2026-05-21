#include "SudokuBoard.h"
#include <QGridLayout>
#include <QIntValidator>
#include <QRandomGenerator>
#include <algorithm>
#include <numeric>
#include <QRegularExpressionValidator>
// ============================================================
// РЕАЛИЗАЦИЯ ОЧИЩЕННОЙ ЯЧЕЙКИ
// ============================================================
SudokuCell::SudokuCell(QWidget *parent) : QWidget(parent) {
    // Жестко фиксируем размер самого виджета ячейки
    setFixedSize(54, 54);

    QGridLayout *layout = new QGridLayout(this);
    layout->setContentsMargins(0, 0, 0, 0); // Убираем отступы вокруг текстового поля
    layout->setSpacing(0);

    lineEdit = new QLineEdit(this);
    lineEdit->setFixedSize(54, 54); // Жестко фиксируем размер поля ввода внутри ячейки
    lineEdit->setMaxLength(1);
    lineEdit->setValidator(new QRegularExpressionValidator(QRegularExpression("^[1-9]$"), this));

    // Отключаем внутренние рамки Qt по умолчанию, чтобы работал наш CSS-стиль границ
    lineEdit->setFrame(false);

    connect(lineEdit, &QLineEdit::textChanged, this, &SudokuCell::textChanged);

    layout->addWidget(lineEdit, 0, 0);
    setLayout(layout);
}

void SudokuCell::setReadOnly(bool ro) {
    lineEdit->setReadOnly(ro);
    if (ro) {
        lineEdit->clearFocus();
    }
}
bool SudokuCell::isReadOnly() const {
    return lineEdit->isReadOnly();
}
void SudokuCell::setValue(int val) {
    if (val == 0) {
        lineEdit->clear();
    } else {
        lineEdit->setText(QString::number(val));
    }
}

int SudokuCell::value() const {
    QString txt = lineEdit->text().trimmed();
    return txt.isEmpty() ? 0 : txt.toInt();
}

void SudokuCell::setCellFocus() {
    lineEdit->setFocus();
    lineEdit->selectAll();
}

void SudokuCell::applyStyle(const QString &bgColor, const QString &textColor, const QString &borderStyle, bool isBold) {
    QString weight = isBold ? "bold" : "normal";
    lineEdit->setStyleSheet(QString(
        "QLineEdit { "
        "  background-color: %1; color: %2; %3 "
        "  font-size: 22pt; qproperty-alignment: 'AlignCenter'; font-weight: %4;"
        "}"
    ).arg(bgColor).arg(textColor).arg(borderStyle).arg(weight));
}

// ============================================================
// ОБНОВЛЕНИЕ СВЯЗЕЙ НА ИГРОВОМ ПОЛЕ
// ============================================================
void SudokuBoard::createBoard() {
    QGridLayout *layout = new QGridLayout(this);
    layout->setSpacing(0);           // Убираем пустые промежутки МЕЖДУ ячейками
    layout->setContentsMargins(0, 0, 0, 0); // Убираем рамку вокруг всей доски судоку

    // Вычисляем точный размер доски: 9 ячеек по 54 пикселя = 486 пикселей
    setFixedSize(54 * BOARD_SIZE, 54 * BOARD_SIZE);

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

            connect(cell, &SudokuCell::textChanged, this, &SudokuBoard::onCellTextChanged);

            layout->addWidget(cell, r, c);
            cells[r][c] = cell;
        }
    }
    setLayout(layout);
}
// ============================================================
// ОБНОВЛЕННАЯ РЕАЛИЗАЦИЯ SUDOKUBOARD
// ============================================================
SudokuBoard::SudokuBoard(QWidget *parent) : QWidget(parent) {
    createBoard();
}





bool SudokuBoard::eventFilter(QObject *watched, QEvent *event) {
    SudokuCell *clickedCell = qobject_cast<SudokuCell*>(watched);

    // Если событие пришло от QLineEdit внутри нашей ячейки
    if (!clickedCell) {
        QLineEdit *le = qobject_cast<QLineEdit*>(watched);
        if (le) clickedCell = qobject_cast<SudokuCell*>(le->parentWidget());
    }

    if (clickedCell) {
        if (event->type() == QEvent::MouseButtonPress) {
            clickedCell->setCellFocus();
        }
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
            SudokuCell* cell = cells[r][c];
            bool isGivenCell = cell->isReadOnly();
            int currentNum = cell->value();

            // ЦВЕТ ШРИФТА
            QString textColor = isGivenCell ? "#1a2740" : "#2E7D32";
            if (!isGivenCell && currentNum > 0 && currentNum != answer[r][c]) {
                textColor = "#C62828"; // Ошибка пользователя красным
            }

            // ФОН
            QString baseBg = isGivenCell ? "#EEF2F7" : "#FFFFFF";

            bool inSameRow = (r == focusedRow);
            bool inSameCol = (c == focusedCol);
            bool inSameBox = (r >= boxStartRow && r < boxStartRow + 3 && c >= boxStartCol && c < boxStartCol + 3);

            if (r == focusedRow && c == focusedCol) {
                baseBg = "#1565C0"; // Выделенная (Синий)
                textColor = (!isGivenCell && currentNum > 0 && currentNum != answer[r][c]) ? "#FFCDD2" : "#FFFFFF";
            }
            else if (focusedNum > 0 && currentNum == focusedNum) {
                baseBg = "#4FC3F7"; // Одинаковые числа (Голубой)
                if (currentNum != answer[r][c]) textColor = "#C62828";
                else textColor = "#000000";
            }
            else if (inSameRow || inSameCol || inSameBox) {
                baseBg = "#E3F2FD"; // Крест и квадрат (Светло-голубой)
            }

            int L = (c % 3 == 0) ? 3 : 1;
            int T = (r % 3 == 0) ? 3 : 1;
            int R = (c == 8)     ? 3 : ((c % 3 == 2) ? 3 : 1);
            int B = (r == 8)     ? 3 : ((r % 3 == 2) ? 3 : 1);

            QString borderStr = QString("border-left: %1px solid #555; border-top: %2px solid #555; border-right: %3px solid #555; border-bottom: %4px solid #555;")
                                .arg(L).arg(T).arg(R).arg(B);

            cell->applyStyle(baseBg, textColor, borderStr, isGivenCell);
        }
    }
}

void SudokuBoard::onCellTextChanged() {
    if (m_blockSignals) return;

    SudokuCell* typedCell = qobject_cast<SudokuCell*>(sender());
    if (!typedCell) return;

    int typedRow = -1, typedCol = -1;
    for (int r = 0; r < BOARD_SIZE; ++r) {
        for (int c = 0; c < BOARD_SIZE; ++c) {
            if (cells[r][c] == typedCell) {
                typedRow = r; typedCol = c; break;
            }
        }
    }

    if (typedRow != -1 && typedCol != -1) {
        int enteredNum = typedCell->value();
        if (enteredNum > 0 && enteredNum != answer[typedRow][typedCol]) {
            emit wrongMove();
        } else if (enteredNum > 0 && enteredNum == answer[typedRow][typedCol]) {
            // Если ввели ПРАВИЛЬНУЮ цифру, автоматически очищаем заметки в этой строке/столбце/квадрате!

            if (emptyCellCount() == 0 && checkSolution()) {
                emit puzzleSolved();
            }
        }
    }

    if (m_lastFocusedRow != -1 && m_lastFocusedCol != -1) {
        updateSubtleHighlight(m_lastFocusedRow, m_lastFocusedCol);
    } else {
        updateSubtleHighlight(0, 0);
    }
    emit boardChanged();
}

void SudokuBoard::loadPuzzle(const QVector<QVector<int>>& puzzle, const QVector<QVector<int>>& solution) {
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
    QVector<QVector<int>> full = generateFull();
    QVector<QVector<int>> puzzle = makePuzzle(full, static_cast<int>(difficulty));
    loadPuzzle(puzzle, full);
}

bool SudokuBoard::checkSolution() {
    for (int r = 0; r < BOARD_SIZE; ++r) {
        for (int c = 0; c < BOARD_SIZE; ++c) {
            if (cells[r][c]->value() != answer[r][c]) {
                return false;
            }
        }
    }
    return true;
}

bool SudokuBoard::giveHint() {
    QVector<QPair<int,int>> emptyCells;
    for (int r = 0; r < BOARD_SIZE; ++r) {
        for (int c = 0; c < BOARD_SIZE; ++c) {
            if (cells[r][c]->value() == 0) emptyCells.append({r, c});
        }
    }
    if (emptyCells.isEmpty()) return false;
    int idx = QRandomGenerator::global()->bounded(emptyCells.size());
    int r = emptyCells[idx].first, c = emptyCells[idx].second;

    m_blockSignals = true;
    cells[r][c]->setValue(answer[r][c]);
    m_blockSignals = false;
    if (emptyCellCount() == 0 && checkSolution()) {
        emit puzzleSolved();
    } else {
        // Обновляем подсветку поля, чтобы новая цифра окрасилась правильно
        if (m_lastFocusedRow != -1 && m_lastFocusedCol != -1) {
            updateSubtleHighlight(m_lastFocusedRow, m_lastFocusedCol);
        }
    }
    emit boardChanged();
    return true;
}

int SudokuBoard::emptyCellCount() const {
    int count = 0;
    for (int r = 0; r < BOARD_SIZE; ++r) {
        for (int c = 0; c < BOARD_SIZE; ++c) {
            if (cells[r][c]->value() == 0) {
                count++;
            }
        }
    }
    return count;
}

// ============================================================
//  ---- АЛГОРИТМЫ ----
// ============================================================

bool SudokuBoard::findEmpty(const QVector<QVector<int> > &board,
                            int &row, int &col) {
    for (row = 0; row < 9; ++row)
        for (col = 0; col < 9; ++col)
            if (board[row][col] == 0)
                return true;
    return false;
}

bool SudokuBoard::isValid(const QVector<QVector<int> > &board,
                          int row, int col, int num) {
    // Строка
    for (int c = 0; c < 9; ++c)
        if (board[row][c] == num) return false;
    // Столбец
    for (int r = 0; r < 9; ++r)
        if (board[r][col] == num) return false;
    // Блок 3×3
    int br = (row / 3) * 3, bc = (col / 3) * 3;
    for (int dr = 0; dr < 3; ++dr)
        for (int dc = 0; dc < 3; ++dc)
            if (board[br + dr][bc + dc] == num) return false;
    return true;
}

bool SudokuBoard::solve(QVector<QVector<int> > &board) {
    int row, col;
    if (!findEmpty(board, row, col)) return true; // всё заполнено

    for (int num = 1; num <= 9; ++num) {
        if (isValid(board, row, col, num)) {
            board[row][col] = num;
            if (solve(board)) return true;
            board[row][col] = 0;
        }
    }
    return false;
}

// Генерируем полностью заполненную доску (randomised backtracking)
QVector<QVector<int> > SudokuBoard::generateFull() {
    QVector<QVector<int> > board(9, QVector<int>(9, 0));

    // Рекурсивная лямбда с перемешанными вариантами
    std::function<bool(QVector<QVector<int> > &)> fill =
            [&](QVector<QVector<int> > &b) -> bool {
        int row, col;
        if (!findEmpty(b, row, col)) return true;

        QVector<int> nums = {1, 2, 3, 4, 5, 6, 7, 8, 9};
        // Перемешиваем
        for (int i = 8; i > 0; --i) {
            int j = QRandomGenerator::global()->bounded(i + 1);
            std::swap(nums[i], nums[j]);
        }

        for (int num: nums) {
            if (isValid(b, row, col, num)) {
                b[row][col] = num;
                if (fill(b)) return true;
                b[row][col] = 0;
            }
        }
        return false;
    };

    fill(board);
    return board;
}

// Убираем ячейки из полной доски так, чтобы осталось ровно `clues` подсказок
QVector<QVector<int> > SudokuBoard::makePuzzle(QVector<QVector<int> > full, int clues) {
    // Список всех 81 позиций в случайном порядке
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

        int r = pos / 9, c = pos % 9;
        int backup = full[r][c];
        full[r][c] = 0;

        // Проверяем единственность решения: пытаемся решить копию
        QVector<QVector<int> > copy = full;
        if (solve(copy)) {
            // Решение найдено и единственное — оставляем ячейку пустой
            ++removed;
        } else {
            // Без этого числа решений нет — возвращаем
            full[r][c] = backup;
        }
    }

    return full;
}

