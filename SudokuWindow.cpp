#include "SudokuWindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QTimer>
#include <QMessageBox>
#include <QSettings>

SudokuWindow::SudokuWindow(QWidget *parent) : QMainWindow(parent) {
    stack = new QStackedWidget(this);
    setCentralWidget(stack);

    setupMenuPage();
    setupGamePage();

    setMinimumSize(520, 680);
    setWindowTitle("Судоку Pro");
}

void SudokuWindow::setupMenuPage() {
    QWidget *page = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(page);
    layout->setContentsMargins(40, 50, 40, 50);

    QLabel *title = new QLabel("СУДОКУ");
    title->setStyleSheet("font-size: 52px; font-weight: bold; color: #1a2740; letter-spacing: 4px;");
    title->setAlignment(Qt::AlignCenter);

    statsLabel = new QLabel();
    statsLabel->setAlignment(Qt::AlignCenter);
    statsLabel->setStyleSheet("font-size: 15px; color: #4A5568; line-height: 140%; padding: 15px; background: #EDF2F7; border-radius: 8px;");
    updateStatsLabels();

    QPushButton *btnEasy = new QPushButton("ЛЕГКИЙ УРОВЕНЬ");
    QPushButton *btnMed  = new QPushButton("СРЕДНИЙ УРОВЕНЬ");
    QPushButton *btnHard = new QPushButton("СЛОЖНЫЙ УРОВЕНЬ");

    // Красивый QSS-стиль для кнопок меню (без свойства filter!)
    QString btnStyle =
        "QPushButton {"
        "  height: 48px; font-size: 16px; font-weight: bold; color: white;"
        "  border: none; border-radius: 6px; padding: 0 20px;"
        "}"
        "QPushButton:hover { background-color: rgba(255, 255, 255, 30); }"
        "QPushButton:pressed { background-color: rgba(0, 0, 0, 40); }";

    btnEasy->setStyleSheet("QPushButton { background-color: #2E7D32; }" + btnStyle);
    btnMed->setStyleSheet("QPushButton { background-color: #1565C0; }" + btnStyle);
    btnHard->setStyleSheet("QPushButton { background-color: #C62828; }" + btnStyle);

    connect(btnEasy, &QPushButton::clicked, [this]{ currentDiff = SudokuBoard::Easy; startGame(); });
    connect(btnMed,  &QPushButton::clicked, [this]{ currentDiff = SudokuBoard::Medium; startGame(); });
    connect(btnHard, &QPushButton::clicked, [this]{ currentDiff = SudokuBoard::Hard; startGame(); });

    layout->addWidget(title);
    layout->addStretch();
    layout->addWidget(statsLabel);
    layout->addSpacing(30);
    layout->addWidget(btnEasy);
    layout->addSpacing(10);
    layout->addWidget(btnMed);
    layout->addSpacing(10);
    layout->addWidget(btnHard);
    layout->addStretch();

    stack->addWidget(page);
}

void SudokuWindow::setupGamePage() {
    QWidget *page = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(page);
    layout->setContentsMargins(15, 15, 15, 15);

    // ВЕРХНЯЯ ПАНЕЛЬ
    QHBoxLayout *topLayout = new QHBoxLayout();

    // Простая текстовая кнопка возврата
    QPushButton *btnBack = new QPushButton("В меню");
    btnBack->setStyleSheet(
        "QPushButton { background: #64748B; color: white; border: none; border-radius: 4px; padding: 6px 12px; font-weight: bold; }"
        "QPushButton:hover { background: #475569; }"
    );

    // Жизни по центру верхней панели в текстовом формате
    livesLabel = new QLabel("Ошибки: 0/3");
    livesLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: #C62828;");

    // Таймер справа
    timerLabel = new QLabel("00:00");
    timerLabel->setStyleSheet("font-size: 20px; font-weight: bold; color: #1e293b; font-family: monospace;");

    topLayout->addWidget(btnBack);
    topLayout->addStretch();
    topLayout->addWidget(livesLabel);
    topLayout->addStretch();
    topLayout->addWidget(timerLabel);

    // Игровое поле
    board = new SudokuBoard(this);

    // НИЖНЯЯ ПАНЕЛЬ
    QHBoxLayout *bottomLayout = new QHBoxLayout();

    // Кнопка-переключатель режима


    QPushButton *btnHint = new QPushButton("Подсказать ячейку");
    btnHint->setStyleSheet(
        "QPushButton { background: #D97706; color: white; height: 44px; font-size: 15px; font-weight: bold; border: none; border-radius: 6px; }"
        "QPushButton:hover { background: #B45309; }"
    );

    bottomLayout->addWidget(btnHint);

    layout->addLayout(topLayout);
    layout->addSpacing(10);
    layout->addWidget(board, 0, Qt::AlignCenter);
    layout->addSpacing(15);
    layout->addLayout(bottomLayout);

    gameTimer = new QTimer(this);

    connect(gameTimer, &QTimer::timeout, this, &SudokuWindow::updateTimer);
    connect(btnBack, &QPushButton::clicked, this, &SudokuWindow::switchToMenu);
    connect(btnHint, &QPushButton::clicked, [this]() {
        if (!board->giveHint()) {
            QMessageBox::information(this, "Подсказка", "Все доступные ячейки уже заполнены верно!");
        }
    });
    connect(board, &SudokuBoard::puzzleSolved, this, &SudokuWindow::onSolved);
    connect(board, &SudokuBoard::wrongMove, this, &SudokuWindow::onWrongMove);
    stack->addWidget(page);
}

void SudokuWindow::startGame() {
    elapsedSeconds = 0;
    livesCount = 3; // Сброс жизней
    livesLabel->setText("Ошибки: 0/3");
    timerLabel->setText("00:00");
    board->resetHighlight();
    board->newGame(currentDiff);
    stack->setCurrentIndex(1);
    gameTimer->start(1000);
}

void SudokuWindow::switchToMenu() {
    gameTimer->stop();
    updateStatsLabels();
    stack->setCurrentIndex(0);
}

void SudokuWindow::updateTimer() {
    elapsedSeconds++;
    int m = elapsedSeconds / 60;
    int s = elapsedSeconds % 60;
    timerLabel->setText(QString("%1:%2").arg(m, 2, 10, QChar('0')).arg(s, 2, 10, QChar('0')));
}

void SudokuWindow::onSolved() {
    gameTimer->stop();
    saveResult(elapsedSeconds);
    QMessageBox::information(this, "Победа! 🎉",
        QString("Поздравляем! Вы успешно решили судоку.\nВаше время: %1").arg(timerLabel->text()));
    switchToMenu();
}

void SudokuWindow::updateStatsLabels() {
    QSettings s("MyCompany", "SudokuGame");
    int total = s.value("totalSolved", 0).toInt();
    QString bestE = s.value("best_Easy", "--:--").toString();
    QString bestM = s.value("best_Medium", "--:--").toString();
    QString bestH = s.value("best_Hard", "--:--").toString();

    statsLabel->setText(QString("<b>Решено всего головоломок:</b> %1<br><br>"
                                "<b>ЛИЧНЫЕ РЕКОРДЫ ВРЕМЕНИ:</b><br>"
                                "Легко: %2<br>"
                                "Средне: %3<br>"
                                "Сложно: %4")
                        .arg(total).arg(bestE).arg(bestM).arg(bestH));
}

void SudokuWindow::saveResult(int sec) {
    QSettings s("MyCompany", "SudokuGame");
    s.setValue("totalSolved", s.value("totalSolved", 0).toInt() + 1);

    QString diffKey = (currentDiff == SudokuBoard::Easy) ? "Easy" :
                      (currentDiff == SudokuBoard::Medium) ? "Medium" : "Hard";

    int currentBest = s.value("best_" + diffKey + "_sec", 999999).toInt();

    if (sec < currentBest) {
        s.setValue("best_" + diffKey + "_sec", sec);
        s.setValue("best_" + diffKey, timerLabel->text());
    }
}
void SudokuWindow::onWrongMove() {
    livesCount--;

    // Считаем количество допущенных ошибок
    int errorsMade = 3 - livesCount;
    livesLabel->setText(QString("Ошибки: %1/3").arg(errorsMade));

    if (livesCount <= 0) {
        gameTimer->stop();
        QMessageBox::critical(this, "Игра окончена", "Вы совершили 3 ошибки. Попробуйте еще раз!");
        switchToMenu();
    }
}