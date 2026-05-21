#include "SudokuWindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QTimer>
#include <QMessageBox>
#include <QSettings>
#include <QFrame>


static const QString BG        = "#F7F6F3";
static const QString INK       = "#18181B";
static const QString MUTED     = "#71717A";
static const QString RULE      = "#D4D4D8";
static const QString CARD_BG   = "#FFFFFF";

static const QString CLR_EASY  = "#16A34A";
static const QString CLR_MED   = "#2563EB";
static const QString CLR_HARD  = "#DC2626";
static const QString CLR_TIMER = "#18181B";
static const QString CLR_ERR   = "#DC2626";


static QPushButton *makeMenuButton(const QString &label, const QString &accent)
{
    QPushButton *btn = new QPushButton(label);
    btn->setFixedHeight(52);
    btn->setCursor(Qt::PointingHandCursor);
    btn->setStyleSheet(QString(
        "QPushButton {"
        "  font-family: 'Outfit'; font-size: 13px; font-weight: 600;"
        "  letter-spacing: 2px; color: %1;"
        "  background: %2;"
        "  border: 1px solid %3;"
        "  border-left: 3px solid %4;"
        "  border-radius: 6px;"
        "  padding-left: 18px;"
        "}"
        "QPushButton:hover   { background: %5; }"
        "QPushButton:pressed { background: %6; }")
        .arg(INK, CARD_BG, RULE, accent)
        .arg("#EFEFEC")
        .arg("#E5E4E0")
    );
    return btn;
}

static QFrame *makeDivider()
{
    QFrame *f = new QFrame();
    f->setFrameShape(QFrame::HLine);
    f->setFixedHeight(1);
    f->setStyleSheet(QString("background: %1; border: none;").arg(RULE));
    return f;
}


SudokuWindow::SudokuWindow(QWidget *parent) : QMainWindow(parent)
{
    stack = new QStackedWidget(this);
    setCentralWidget(stack);

    setStyleSheet(QString("QMainWindow, QWidget#page { background: %1; }").arg(BG));
    setMinimumSize(520, 680);
    setWindowTitle("Судоку");

    setupMenuPage();
    setupGamePage();
}


void SudokuWindow::setupMenuPage()
{
    QWidget *page = new QWidget();
    page->setObjectName("page");
    page->setStyleSheet(QString("background: %1;").arg(BG));

    QVBoxLayout *vl = new QVBoxLayout(page);
    vl->setContentsMargins(48, 60, 48, 60);
    vl->setSpacing(0);


    QLabel *title = new QLabel("СУДОКУ");
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet(QString(
        "font-family: 'Outfit'; font-size: 54px; font-weight: 300;"
        "color: %1; letter-spacing: 12px; background: transparent;")
        .arg(INK));


    statsLabel = new QLabel();
    statsLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    statsLabel->setWordWrap(true);
    statsLabel->setStyleSheet(QString(
        "font-family: 'Outfit'; font-size: 13px; color: %1;"
        "background: %2; border: 1px solid %3;"
        "border-radius: 8px; padding: 16px 20px; line-height: 180%;")
        .arg(MUTED, CARD_BG, RULE));
    updateStatsLabels();

    QPushButton *btnEasy = makeMenuButton("ЛЁГКИЙ",   CLR_EASY);
    QPushButton *btnMed  = makeMenuButton("СРЕДНИЙ",  CLR_MED);
    QPushButton *btnHard = makeMenuButton("СЛОЖНЫЙ",  CLR_HARD);

    connect(btnEasy, &QPushButton::clicked, [this]{ currentDiff = SudokuBoard::Easy;   startGame(); });
    connect(btnMed,  &QPushButton::clicked, [this]{ currentDiff = SudokuBoard::Medium; startGame(); });
    connect(btnHard, &QPushButton::clicked, [this]{ currentDiff = SudokuBoard::Hard;   startGame(); });

    vl->addWidget(title);
    vl->addSpacing(6);
    vl->addSpacing(36);
    vl->addWidget(makeDivider());
    vl->addSpacing(24);
    vl->addWidget(statsLabel);
    vl->addStretch(1);
    vl->addWidget(btnEasy);
    vl->addSpacing(8);
    vl->addWidget(btnMed);
    vl->addSpacing(8);
    vl->addWidget(btnHard);

    stack->addWidget(page);
}


void SudokuWindow::setupGamePage()
{
    QWidget *page = new QWidget();
    page->setObjectName("page");
    page->setStyleSheet(QString("background: %1;").arg(BG));

    QVBoxLayout *vl = new QVBoxLayout(page);
    vl->setContentsMargins(20, 18, 20, 20);
    vl->setSpacing(0);


    QHBoxLayout *topRow = new QHBoxLayout();
    topRow->setSpacing(0);

    QPushButton *btnBack = new QPushButton("← Меню");
    btnBack->setCursor(Qt::PointingHandCursor);
    btnBack->setStyleSheet(QString(
        "QPushButton { font-family:'Outfit'; font-size:13px; font-weight:500;"
        "  color:%1; background:transparent; border:none; padding:0; }"
        "QPushButton:hover { color:%2; }")
        .arg(MUTED, INK));

    livesLabel = new QLabel("● ● ●");
    livesLabel->setAlignment(Qt::AlignCenter);
    livesLabel->setStyleSheet(QString(
        "font-family:'Outfit'; font-size:18px; font-weight:400; color:%1;"
        "background:transparent;").arg(CLR_EASY));

    timerLabel = new QLabel("00:00");
    timerLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    timerLabel->setStyleSheet(QString(
        "font-family:'Outfit'; font-size:15px; font-weight:500;"
        "color:%1; letter-spacing:1px; background:transparent;").arg(CLR_TIMER));

    topRow->addWidget(btnBack,    0, Qt::AlignVCenter);
    topRow->addStretch();
    topRow->addWidget(livesLabel, 0, Qt::AlignVCenter);
    topRow->addStretch();
    topRow->addWidget(timerLabel, 0, Qt::AlignVCenter);


    board = new SudokuBoard(this);


    QPushButton *btnHint = new QPushButton("Подсказать");
    btnHint->setFixedHeight(48);
    btnHint->setCursor(Qt::PointingHandCursor);
    btnHint->setStyleSheet(QString(
        "QPushButton {"
        "  font-family:'Outfit'; font-size:13px; font-weight:600;"
        "  letter-spacing:1.5px; color:%1;"
        "  background:transparent; border:1px solid %2;"
        "  border-radius:6px;"
        "}"
        "QPushButton:hover   { background:%3; }"
        "QPushButton:pressed { background:%4; }")
        .arg(INK, RULE)
        .arg("#EFEFEC").arg("#E5E4E0")
    );

    vl->addLayout(topRow);
    vl->addSpacing(14);
    vl->addWidget(makeDivider());
    vl->addSpacing(20);
    vl->addWidget(board, 0, Qt::AlignHCenter);
    vl->addSpacing(20);
    vl->addWidget(makeDivider());
    vl->addSpacing(16);
    vl->addWidget(btnHint);


    gameTimer = new QTimer(this);
    connect(gameTimer, &QTimer::timeout, this, &SudokuWindow::updateTimer);


    connect(btnBack, &QPushButton::clicked, this, &SudokuWindow::switchToMenu);
    connect(btnHint, &QPushButton::clicked, [this]() {
        if (!board->giveHint())
            QMessageBox::information(this, "Подсказка", "Все ячейки уже заполнены.");
    });
    connect(board, &SudokuBoard::puzzleSolved, this, &SudokuWindow::onSolved);
    connect(board, &SudokuBoard::wrongMove,    this, &SudokuWindow::onWrongMove);

    stack->addWidget(page);
}


void SudokuWindow::startGame()
{
    elapsedSeconds = 0;
    livesCount = 3;
    livesLabel->setText("● ● ●");
    livesLabel->setStyleSheet(QString(
        "font-family:'Outfit'; font-size:18px; color:%1; background:transparent;").arg(CLR_EASY));
    timerLabel->setText("00:00");
    board->resetHighlight();
    board->newGame(currentDiff);
    stack->setCurrentIndex(1);
    gameTimer->start(1000);
}

void SudokuWindow::switchToMenu()
{
    gameTimer->stop();
    updateStatsLabels();
    stack->setCurrentIndex(0);
}

void SudokuWindow::updateTimer()
{
    ++elapsedSeconds;
    timerLabel->setText(QString("%1:%2")
        .arg(elapsedSeconds / 60, 2, 10, QChar('0'))
        .arg(elapsedSeconds % 60, 2, 10, QChar('0')));
}

void SudokuWindow::onSolved()
{
    gameTimer->stop();
    saveResult(elapsedSeconds);
    QMessageBox::information(this, "Победа!",
        QString("Судоку решено.\nВремя: %1").arg(timerLabel->text()));
    switchToMenu();
}

void SudokuWindow::onWrongMove()
{
    --livesCount;
    int errors = 3 - livesCount;

    QString dots;
    for (int i = 0; i < 3; ++i) {
        if (i > 0) dots += " ";
        dots += (i < livesCount) ? "●" : "○";
    }
    livesLabel->setText(dots);

    if (livesCount <= 0) {
        livesLabel->setStyleSheet(QString(
            "font-family:'Outfit'; font-size:18px; color:%1; background:transparent;").arg(CLR_ERR));
        gameTimer->stop();
        QMessageBox::critical(this, "Игра окончена", "Допущено 3 ошибки. Попробуйте снова!");
        switchToMenu();
    } else if (errors >= 2) {
        livesLabel->setStyleSheet(QString(
            "font-family:'Outfit'; font-size:18px; color:%1; background:transparent;").arg(CLR_ERR));
    } else {
        livesLabel->setStyleSheet(QString(
            "font-family:'Outfit'; font-size:18px; color:%1; background:transparent;").arg(CLR_EASY));
    }
}


void SudokuWindow::updateStatsLabels()
{
    QSettings s("MyCompany", "SudokuGame");
    int total   = s.value("totalSolved", 0).toInt();
    QString bE  = s.value("best_Easy",   "--:--").toString();
    QString bM  = s.value("best_Medium", "--:--").toString();
    QString bH  = s.value("best_Hard",   "--:--").toString();

    statsLabel->setText(
        QString("<b style='color:#18181B;'>Решено головоломок:</b> %1<br><br>"
                "<span style='letter-spacing:1px;'>ЛУЧШЕЕ ВРЕМЯ</span><br>"
                "Лёгкий:&nbsp;&nbsp;%2<br>"
                "Средний:&nbsp;%3<br>"
                "Сложный:&nbsp;%4")
        .arg(total).arg(bE).arg(bM).arg(bH));
}

void SudokuWindow::saveResult(int sec)
{
    QSettings s("MyCompany", "SudokuGame");
    s.setValue("totalSolved", s.value("totalSolved", 0).toInt() + 1);

    QString key = (currentDiff == SudokuBoard::Easy)   ? "Easy"   :
                  (currentDiff == SudokuBoard::Medium)  ? "Medium" : "Hard";

    int currentBest = s.value("best_" + key + "_sec", 999999).toInt();
    if (sec < currentBest) {
        s.setValue("best_" + key + "_sec", sec);
        s.setValue("best_" + key, timerLabel->text());
    }
}