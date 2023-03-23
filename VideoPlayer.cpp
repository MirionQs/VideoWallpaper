#include "VideoPlayer.h"

VideoPlayer::VideoPlayer(QDialog* parent) : QDialog(parent) {
    setWindowFlags(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setFixedSize(QGuiApplication::primaryScreen()->size()); // 不考虑多显示器，因为我没有

    instance = nullptr;
    mediaList = nullptr;
    mediaPlayer = nullptr;
    mediaListPlayer = nullptr;
}

VideoPlayer::~VideoPlayer() {
    if (instance) {
        libvlc_release(instance);
    }
    if (mediaList) {
        libvlc_media_list_release(mediaList);
    }
    if (mediaPlayer) {
        libvlc_media_player_release(mediaPlayer);
    }
    if (mediaListPlayer) {
        libvlc_media_list_player_release(mediaListPlayer);
    }
}

bool VideoPlayer::init() {
    // 将 VideoPlayer 置于桌面图标和壁纸的中间
    // 参考 https://www.codeproject.com/articles/856020/draw-behind-desktop-icons-in-windows
    HWND hPlayer = reinterpret_cast<HWND>(winId());
    HWND hDesktop = FindWindow(L"Progman", nullptr);
    if (!hDesktop || !SendMessageTimeout(hDesktop, 0x52c, 0, 0, SMTO_NORMAL, 1000, nullptr)) {
        return false;
    }
    HWND hWorkerW = nullptr;
    HWND hDefView = nullptr;
    while (!hDefView) {
        hDefView = FindWindowEx(hWorkerW, nullptr, L"SHELLDLL_DefView", nullptr);
        hWorkerW = FindWindowEx(nullptr, hWorkerW, L"WorkerW", nullptr);
        if (!hWorkerW) {
            return false;
        }
    }
    ShowWindow(hWorkerW, SW_HIDE);
    if (!SetParent(hPlayer, hDesktop)) {
        return false;
    }

    // 将 libVLC Player 置于 VideoPlayer
    if (instance = libvlc_new(0, nullptr)) {
        mediaList = libvlc_media_list_new(instance);
        mediaPlayer = libvlc_media_player_new(instance);
        mediaListPlayer = libvlc_media_list_player_new(instance);
    }
    if (!instance || !mediaList || !mediaPlayer || !mediaListPlayer) {
        return false;
    }
    libvlc_media_list_player_set_media_list(mediaListPlayer, mediaList);
    libvlc_media_list_player_set_media_player(mediaListPlayer, mediaPlayer);
    libvlc_media_player_set_hwnd(mediaPlayer, hPlayer);

    // 播放结束时发出信号
    libvlc_event_attach(
        libvlc_media_player_event_manager(mediaPlayer),
        libvlc_MediaPlayerEndReached,
        [](auto, void* p_data) { emit static_cast<VideoPlayer*>(p_data)->playbackFinished(); },
        this
    );

    return true;
}

bool VideoPlayer::play(const QString& path) {
    if (libvlc_media_list_count(mediaList) > 0) {
        libvlc_media_list_remove_index(mediaList, 0);
    }
    libvlc_media_t* media = libvlc_media_new_path(instance, path.toLocal8Bit().replace('/', '\\'));
    if (!media) {
        return false;
    }
    libvlc_media_list_add_media(mediaList, media);
    libvlc_media_release(media);
    libvlc_media_list_player_play(mediaListPlayer);
    return true;
}

void VideoPlayer::pause() {
    libvlc_media_list_player_pause(mediaListPlayer);
}

void VideoPlayer::loop(bool enable) {
    if (enable) {
        libvlc_media_list_player_set_playback_mode(mediaListPlayer, libvlc_playback_mode_loop);
    }
    else {
        libvlc_media_list_player_set_playback_mode(mediaListPlayer, libvlc_playback_mode_default);
    }
}
