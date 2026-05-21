#ifndef SUDOKUWINDOW_H
#define SUDOKUWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include "SudokuBoard.h"

class QLabel;
class QTimer;

class SudokuWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit SudokuWindow(QWidget *parent = nullptr);

    ~SudokuWindow() override = default;

private slots:
    void switchToMenu();

    void onWrongMove();

    void startGame();

    void updateTimer();

    void onSolved();

    void updateStatsLabels();

private:
    QLabel *livesLabel;
    int livesCount = 3;

    void setupMenuPage();

    void setupGamePage();

    void saveResult(int seconds);

    QStackedWidget *stack;
    SudokuBoard *board;

    // Элементы UI Главного меню
    QLabel *statsLabel;

    // Элементы UI Экрана игры
    QLabel *timerLabel;
    QTimer *gameTimer;
    int elapsedSeconds = 0;
    SudokuBoard::Difficulty currentDiff;
};

#endif // SUDOKUWINDOW_H
