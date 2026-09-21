#include <qdir.h>
#include <qstring.h>
#include <qlist.h>
#include <qstrlist.h>
#include <qpixmap.h>
#include <qimage.h>
#include <qbitmap.h>

#include <kapp.h>

#include "levels.h"

Levels *leV = 0;

Levels::Levels()
{
    leV = this;

    QString levelDir = KApplication::kde_datadir();
    levelDir.append("/ksnake/levels/");

    QDir d(levelDir);
    d.setNameFilter("room*"); // Only match room files
    d.setFilter( QDir::Files );
    d.setSorting( QDir::Name );

    QString *s = new QString("dummy");
    list.insert(0, s);

    for ( unsigned int i = 0; i < d.count(); i++ ) {
        QString *path = new QString(d.absFilePath(d[i]));
        list.append(path);
    }

    QDir userDir(QDir::homeDirPath() + "/.kde/share/apps/ksnake/levels");
    if ( userDir.exists() ) {
        d.setNameFilter("room*"); // Only match room files
        userDir.setFilter( QDir::Files );
        userDir.setSorting( QDir::Name );
        for ( unsigned int i = 0; i < userDir.count(); i++ ) {
            QString *path = new QString(userDir.absFilePath(userDir[i]));
            list.append(path);
        }
    }
}

int Levels::max()
{
    return ( list.count() - 1 );
}

QImage Levels::getImage(int at)
{
    QString *path = list.at(at);
    if (!path) {
        fprintf(stderr, "Levels: No level path found at index %d\n", at);
        return QImage();
    }

    QBitmap bitmap(path->latin1());
    if (bitmap.isNull()) {
        fprintf(stderr, "Levels: Failed to load level bitmap from: %s\n", path->latin1());
    }

    QImage image = bitmap.convertToImage();
    return image;
}

QPixmap Levels::getPixmap(int at)
{
    QString *path = list.at(at);
    if (!path) {
        fprintf(stderr, "Levels: No level path found at index %d\n", at);
        return QPixmap();
    }

    QPixmap pixmap(path->latin1());
    if (pixmap.isNull()) {
        fprintf(stderr, "Levels: Failed to load level pixmap from: %s\n", path->latin1());
    }
    return pixmap;
}
