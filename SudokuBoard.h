#ifndef SUDOKUBOARD_H
#define SUDOKUBOARD_H

#include <QWidget>
#include <QVector>
#include <QLineEdit>
#include <QLabel>
#include <QGridLayout>
#include <QStackedLayout>
#include <QKeyEvent>

// ============================================================
// КАСТОМНЫЙ ВИДЖЕТ ЯЧЕЙКИ С ПОДДЕРЖКОЙ ЗАМЕТОК
// ============================================================
class SudokuCell : public QWidget {
    Q_OBJECT
public:
    explicit SudokuCell(QWidget *parent = nullptr);

    void setReadOnly(bool ro);
    bool isReadOnly() const;

    void setValue(int val);
    int value() const;

    void applyStyle(const QString &bgColor, const QString &textColor, const QString &borderStyle, bool isBold);

    void setCellFocus();
    QLineEdit* getLineEdit() const { return lineEdit; }

    signals:
        void textChanged();

private:
    QLineEdit *lineEdit;
};
// ============================================================
// ОБНОВЛЕННЫЙ КЛАСС DOSKI
// ============================================================
class SudokuBoard : public QWidget {
    Q_OBJECT
public:
    enum Difficulty { Easy = 42, Medium = 32, Hard = 24 };

    explicit SudokuBoard(QWidget *parent = nullptr);
    ~SudokuBoard() override = default;

    void newGame(Difficulty difficulty);
    bool checkSolution();
    bool giveHint();
    int emptyCellCount() const;
    void resetHighlight() { m_lastFocusedRow = -1; m_lastFocusedCol = -1; }

    // Метод для включения/выключения заметок на всей доске
    void setGlobalNotesMode(bool enabled);
    bool isGlobalNotesMode() const { return m_globalNotesMode; }

signals:
    void boardChanged();
    void wrongMove();
    void puzzleSolved();

private slots:
    void onCellTextChanged();

private:
    bool eventFilter(QObject *watched, QEvent *event) override;
    void createBoard();
    void updateSubtleHighlight(int focusedRow, int focusedCol);
    void loadPuzzle(const QVector<QVector<int>>& puzzle, const QVector<QVector<int>>& solution);

    bool findEmpty(const QVector<QVector<int>>& b, int& r, int& c);
    bool isValid(const QVector<QVector<int>>& b, int r, int c, int n);
    bool solve(QVector<QVector<int>>& b);
    QVector<QVector<int>> generateFull();
    QVector<QVector<int>> makePuzzle(QVector<QVector<int>> full, int clues);

    // ВАЖНО: заменяем QLineEdit* на SudokuCell*
    QVector<QVector<SudokuCell*>> cells;
    QVector<QVector<int>> given;
    QVector<QVector<int>> answer;

    bool m_blockSignals = false;
    bool m_globalNotesMode = false; // Хранит текущий режим доски
    int m_lastFocusedRow = -1;
    int m_lastFocusedCol = -1;

    static const int BOARD_SIZE = 9;
    static const int CELL_SIZE  = 54; // Чуть увеличим для удобства заметок
};

#endif // SUDOKUBOARD_H