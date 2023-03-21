#include "VideoWallpaper.h"
#include <QtWidgets/QApplication>

int main(int argc, char* argv[]) {
    QApplication a(argc, argv);
    VideoWallpaper w;
    w.show();
    return a.exec();
}
