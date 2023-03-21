#pragma once

#include <QDialog>
#include <QGuiApplication>
#include <QScreen>

// WinAPI
#include <Windows.h>

// libVLC
using ssize_t = __int64;
#include <vlc/vlc.h>

class VideoPlayer : public QDialog {
    Q_OBJECT

public:
    VideoPlayer(QDialog* parent = nullptr);
    ~VideoPlayer();

    bool init();
    bool play(const QString& path);
    void pause();
    void loop(bool enable = true);

signals:
    void playbackFinished();

private:
    libvlc_instance_t* instance;
    libvlc_media_list_t* mediaList;
    libvlc_media_player_t* mediaPlayer;
    libvlc_media_list_player_t* mediaListPlayer;
};
