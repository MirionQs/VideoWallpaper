#pragma once

#include <QDialog>
#include "ui_VideoDesktop.h"
#include "VideoLayout.h"

// Qt
#include <QMouseEvent>
#include <QPaintEvent>
#include <QMessageBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QPixmap>

// WinAPI
#include <windows.h>
#include <windowsx.h>

// ffmpeg  用以截取视频某一帧作为预览画面
extern "C" {
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
}

class VideoDesktop : public QDialog {
    Q_OBJECT

public:
    VideoDesktop(QWidget* parent = nullptr);

    void enable_PushButton_clicked();
    void min_PushButton_clicked();
    void close_PushButton_clicked();
    void getPath_PushButton_clicked();
    void pathEdit_LineEdit_textChanged();
    void loop_CheckBox_stateChanged();

    void preview_undate();
    bool checkPath(const QString& videoPath);

protected:
    void mousePressEvent(QMouseEvent* event);

private:
    Ui::VideoDesktopClass ui;
    VideoLayout* layout;
    QString lastPath;
    QString lastText;
};
