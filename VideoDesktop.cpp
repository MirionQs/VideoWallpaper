#pragma execution_character_set("utf-8")
#include "VideoDesktop.h"

VideoDesktop::VideoDesktop(QWidget* parent) : QDialog(parent) {
    ui.setupUi(this);

    setWindowFlag(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);

	ui.groupBox->setFixedSize(size());

    layout = new VideoLayout();
    layout->setWindowFlag(Qt::Window);
    layout->show();

    lastPath = "";
    lastText = "启用";

    connect(ui.enable,   &QPushButton::clicked,    this, &VideoDesktop::enable_PushButton_clicked);
    connect(ui.min,      &QPushButton::clicked,    this, &VideoDesktop::min_PushButton_clicked);
    connect(ui.close,    &QPushButton::clicked,    this, &VideoDesktop::close_PushButton_clicked);
    connect(ui.getPath,  &QPushButton::clicked,    this, &VideoDesktop::getPath_PushButton_clicked);
    connect(ui.pathEdit, &QLineEdit::textChanged,  this, &VideoDesktop::pathEdit_LineEdit_textChanged);
    connect(ui.loop,     &QCheckBox::stateChanged, this, &VideoDesktop::loop_CheckBox_stateChanged);

    //connect(ui.pathEdit, &QLineEdit::textChanged,  this, &VideoDesktop::preview_undate); // 优化后再开启该功能
}

void VideoDesktop::enable_PushButton_clicked() {
	// bug: 如果关闭循环播放，那么播放结束后 ui.enable 依然会显示"暂停"
	if (ui.enable->text() == "继续") {
		layout->pause();
		ui.enable->setText("暂停");
	}
	else if (ui.enable->text() == "暂停") {
		layout->pause();
		ui.enable->setText("继续");
	}
	else {
		if (!checkPath(ui.pathEdit->text())) {
			return;
		}
		if (!layout->play(ui.pathEdit->text(), ui.enable->text() == "更换")) {
			return;
		}
		ui.enable->setText("暂停");
	}
	lastPath = ui.pathEdit->text();
	lastText = ui.enable->text();
}

void VideoDesktop::min_PushButton_clicked() {
    setWindowState(Qt::WindowMinimized);
}

void VideoDesktop::close_PushButton_clicked() {
    delete layout;
    close();
}

void VideoDesktop::pathEdit_LineEdit_textChanged() {
    if (!lastPath.isEmpty() && lastPath != ui.pathEdit->text()) {
        ui.enable->setText("更换");
    }
    else {
        ui.enable->setText(lastText);
    }
}

void VideoDesktop::getPath_PushButton_clicked() {
    ui.pathEdit->setText(QFileDialog::getOpenFileName(this, "选择视频文件", "./", "视频文件 (*)"));
}

void VideoDesktop::loop_CheckBox_stateChanged() {
    if (ui.loop->isChecked()) {
        layout->loopMode(true);
    }
    else {
        layout->loopMode(false);
    }
}

bool VideoDesktop::checkPath(const QString& videoPath) {
    if (videoPath.isEmpty()) {
        QMessageBox::critical(this, windowTitle(), "视频路径为空", QMessageBox::Ok);
        return false;
    }
    QFileInfo fileInfo(videoPath);
    if (!fileInfo.exists()) {
        QMessageBox::critical(this, windowTitle(), "视频路径不存在", QMessageBox::Ok);
        return false;
    }
    if (!fileInfo.isFile()) {
        QMessageBox::critical(this, windowTitle(), "请选择一个文件，而不是文件夹", QMessageBox::Ok);
        return false;
    }
    return true;
}

void VideoDesktop::mousePressEvent(QMouseEvent*) {
    if (ReleaseCapture()) {
        SendMessage((HWND)winId(), WM_SYSCOMMAND, SC_MOVE + HTCAPTION, NULL);
    }
}

void VideoDesktop::preview_undate() {
	// todo: 考虑多线程优化
	av_register_all();

	QString filePath = ui.pathEdit->text();
	QFileInfo fileInfo(filePath);
	if (filePath.isEmpty() || !fileInfo.exists() || !fileInfo.isFile()) {
		return;
	}

	QByteArray filePathArray = filePath.toLatin1();
	AVFormatContext* context = nullptr;
	char* videoPath = filePathArray.data();
	int videoStream = -1;

	if (avformat_open_input(&context, videoPath, NULL, NULL) != 0) {
		return;
	}
	if (avformat_find_stream_info(context, NULL) < 0) {
		return;
	}
	av_dump_format(context, 0, videoPath, 0);

	for (int i = 0; i < context->nb_streams; i++) {
		if (context->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
			videoStream = i;
			break;
		}
	}
	if (videoStream == -1) {
		return;
	}

	AVCodecContext* videoContext = context->streams[videoStream]->codec;
	AVCodec* codec = avcodec_find_decoder(videoContext->codec_id);
	if (codec == 0) {
		return;
	}
	AVCodecContext* videoContextCopy = avcodec_alloc_context3(codec);
	if (avcodec_copy_context(videoContextCopy, videoContext) != 0) {
		return;
	}
	if (avcodec_open2(videoContextCopy, codec, NULL) != 0) {
		return;
	}

	int videoWidth = videoContextCopy->width;
	int videoHeight = videoContextCopy->height;
	int videoFramerate = videoContextCopy->framerate.num / videoContextCopy->framerate.den;
	int videoFrame = context->duration * videoFramerate / AV_TIME_BASE;
	AVFrame* frame = av_frame_alloc();
	AVFrame* frameRGB = av_frame_alloc();

	if (frame == 0 || frameRGB == 0) {
		return;
	}
	uchar* buffer = (uchar*)av_malloc(av_image_get_buffer_size(AV_PIX_FMT_RGB24, videoWidth, videoHeight, 1) * sizeof(uchar));
	av_image_fill_arrays(frameRGB->data, frameRGB->linesize, buffer, AV_PIX_FMT_RGB24, videoWidth, videoHeight, 1);

	AVPacket packet;
	SwsContext* swsContext = sws_getContext(videoWidth, videoHeight, videoContextCopy->pix_fmt, videoWidth, videoHeight, AV_PIX_FMT_RGB24, SWS_BILINEAR, NULL, NULL, NULL);
	int i = 0;
	int frameFinished;

	while (av_read_frame(context, &packet) == 0) {
		if (packet.stream_index == videoStream) {
			avcodec_decode_video2(videoContextCopy, frame, &frameFinished, &packet);
			if (frameFinished) {
				sws_scale(swsContext, frame->data, frame->linesize, 0, videoHeight, frameRGB->data, frameRGB->linesize);
				if (++i == videoFrame / 2) {
					QImage image(frameRGB->data[0], videoWidth, videoHeight, QImage::Format_RGB888);
					ui.preview->setPixmap(QPixmap::fromImage(image).scaled(ui.preview->size(), Qt::KeepAspectRatio));
					av_packet_unref(&packet);
					break;
				}
				frameFinished = 0;
			}
		}
		av_packet_unref(&packet);
	}

	av_free(buffer);
	av_frame_free(&frame);
	av_frame_free(&frameRGB);
	avcodec_close(videoContext);
	avcodec_close(videoContextCopy);
	avformat_close_input(&context);
}