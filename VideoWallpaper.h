#pragma once

#include <QDialog>
#include "ui_VideoWallpaper.h"

#include "VideoPlayer.h"

#include <QFileInfo>
#include <QFileDialog>
#include <QMessageBox>

class VideoWallpaper : public QDialog {
    Q_OBJECT

public:
    VideoWallpaper(QWidget* parent = nullptr);
    ~VideoWallpaper();

    void pathEditTextChanged();
    void openButtonClicked();
    void playButtonClicked();
    void loopButtonClicked();
    void playerPlaybackFinished();

private:
    Ui::VideoWallpaperClass ui;
    VideoPlayer* player;
    enum {
        idle, playing, paused
    } state;
    bool isLooping;
};
