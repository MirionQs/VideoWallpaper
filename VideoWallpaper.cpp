#include "VideoWallpaper.h"

VideoWallpaper::VideoWallpaper(QWidget* parent) : QDialog(parent) {
    ui.setupUi(this);

    player = new VideoPlayer;
    if (!player->init()) {
        QMessageBox::critical(this, "错误", "播放器初始化失败。");
    }
    player->show();
    state = idle;
    isLooping = true;

    connect(ui.pathEdit, &QLineEdit::textChanged, this, &VideoWallpaper::pathEditTextChanged);
    connect(ui.open, &QToolButton::clicked, this, &VideoWallpaper::openButtonClicked);
    connect(ui.play, &QToolButton::clicked, this, &VideoWallpaper::playButtonClicked);
    connect(ui.loop, &QToolButton::clicked, this, &VideoWallpaper::loopButtonClicked);
    connect(player, &VideoPlayer::playbackFinished, this, &VideoWallpaper::playerPlaybackFinished);
}

VideoWallpaper::~VideoWallpaper() {
    delete player;
}

void VideoWallpaper::pathEditTextChanged() {
    if (QFileInfo(ui.pathEdit->text()).isFile()) {
        ui.pathEdit->setStyleSheet("");
    }
    else {
        ui.pathEdit->setStyleSheet("border-color: #ff99a4;");
    }
    if (state == playing) {
        player->pause();
        ui.play->setIcon(QIcon(":/VideoWallpaper/icon/Play.svg"));
    }
    state = idle;
}

void VideoWallpaper::openButtonClicked() {
    QString path = QFileDialog::getOpenFileName(this, "打开视频文件");
    if (!path.isEmpty()) {
        ui.pathEdit->setText(path);
    }
}

void VideoWallpaper::playButtonClicked() {
    if (!QFileInfo(ui.pathEdit->text()).isFile()) {
        return;
    }
    switch (state) {
    case idle:
        player->loop(isLooping);
        if (player->play(ui.pathEdit->text())) {
            state = playing;
            ui.play->setIcon(QIcon(":/VideoWallpaper/icon/Pause.svg"));
        }
        else {
            ui.pathEdit->setStyleSheet("border-color: #ff99a4;");
        }
        break;
    case playing:
        player->pause();
        state = paused;
        ui.play->setIcon(QIcon(":/VideoWallpaper/icon/Play.svg"));
        break;
    case paused:
        player->pause();
        state = playing;
        ui.play->setIcon(QIcon(":/VideoWallpaper/icon/Pause.svg"));
        break;
    }
}

void VideoWallpaper::loopButtonClicked() {
    isLooping = !isLooping;
    if (isLooping) {
        ui.loop->setIcon(QIcon(":/VideoWallpaper/icon/Loop.svg"));
    }
    else {
        ui.loop->setIcon(QIcon(":/VideoWallpaper/icon/NoLoop.svg"));
    }
    player->loop(isLooping);
}

void VideoWallpaper::playerPlaybackFinished() {
    if (isLooping) {
        return;
    }
    state = idle;
    ui.play->setIcon(QIcon(":/VideoWallpaper/icon/Play.svg"));
}
