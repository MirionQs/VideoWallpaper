#pragma once

#include <QWidget>
#include "ui_VideoLayout.h"

// Qt
#include <QScreen>
#include <QMessageBox>

// WinAPI
#include "windows.h"
#include "windowsx.h"

// libVLC  因为 QMediaPlayer 切换时会短暂黑屏，所以重新选择了 libVLC
#include <vlc/vlc.h>

void setParentDesktop(QWidget* pWidget);

class VideoLayout : public QWidget {
	Q_OBJECT

public:
	VideoLayout(QWidget* parent = nullptr);
	~VideoLayout();

	bool play(const QString& videoPath, bool change = false);
	void pause();
	void loopMode(bool enable = true);

private:
	Ui::VideoLayout ui;
	libvlc_instance_t* instance;
	libvlc_media_list_t* mediaList;
	libvlc_media_player_t* mediaPlayer;
	libvlc_media_list_player_t* mediaListPlayer;
};
