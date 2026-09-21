#include <qcolor.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qbitmap.h>
#include <qrect.h>
#include <qstring.h>

#include <kapp.h>

#include "pixServer.h"
#include "board.h"


QPixmap loadPixmap( const char *fileName )
{
    QPixmap pm(fileName);
    return pm;
}

PixServer::PixServer( Board *b, QWidget *parent)
{
    pixDir = KApplication::kde_datadir();
    pixDir.append("/ksnake/pics/");

    board = b;
    initPixmaps();
    initBrickPixmap();
    initbackPixmaps();
    initRoomPixmap();
    w = parent;
}

void PixServer::erase(int pos)
{
    if (!board->isEmpty(pos))
        return;

    QRect rect = board->rect(pos);
    bitBlt( w, rect.x(), rect.y(), &backPix,
            rect.x(), rect.y(), rect.width(), rect.height());
}

void PixServer::restore(int pos)
{
    QRect rect = board->rect(pos);
    bitBlt( w, rect.x(), rect.y(), &roomPix,
            rect.x(), rect.y(), rect.width(), rect.height());
}

void PixServer::draw(int pos, PixMap pix, int i)
{
    // Allocate a 16x16 buffer directly (p.resize() on a null pixmap does nothing in Qt2/Osiris)
    QPixmap p(16, 16);

    QRect rect = board->rect(pos);

    if (! plainColor)
        bitBlt( &p, 0, 0, &backPix,
                rect.x(), rect.y(), rect.width(), rect.height());
        else
            p.fill(backgroundColor);

    switch (pix) {
        case SamyPix:        bitBlt(&p ,0,0, &samyPix[i]);
        break;
        case CompuSnakePix:  bitBlt(&p ,0,0, &compuSnakePix[i]);
        break;
        case ApplePix:       bitBlt(&p ,0,0, &applePix[i]);
        break;
        case BallPix:        bitBlt(&p ,0,0, &ballPix[i]);
        break;
        default:
            break;
    }

    bitBlt(w, rect.x(), rect.y(), &p);
}


/** Debug section for snake not showing up
void PixServer::draw(int pos, PixMap pix, int i)
{
    QRect rect = board->rect(pos);

    // TEMPORARY TEST: Print coordinates and fill with solid red
    fprintf(stderr, "PixServer::draw called: pos=%d, pix=%d, i=%d, rect=(%d, %d, %d, %d), w=%p\n",
            pos, (int)pix, i, rect.x(), rect.y(), rect.width(), rect.height(), (void*)w);

    QPixmap p(16, 16);
    p.fill(Qt::red); // Solid red test box

    bitBlt(w, rect.x(), rect.y(), &p);
}

*/

void PixServer::initPixmaps()
{
    QPixmap PIXMAP;

    // 1. CompuSnake
    PIXMAP.load((pixDir + "snake1.xpm").latin1());
    const QBitmap *mask1 = PIXMAP.mask();
    for (int x = 0 ; x < 18; x++){
        compuSnakePix[x] = QPixmap(16, 16);
        bitBlt(&compuSnakePix[x] ,0,0, &PIXMAP,x*16, 0, 16, 16, Qt::CopyROP, TRUE);
        if (mask1) {
            QBitmap tileMask(16, 16);
            bitBlt(&tileMask, 0, 0, mask1, x*16, 0, 16, 16, Qt::CopyROP, TRUE);
            compuSnakePix[x].setMask(tileMask);
        } else {
            compuSnakePix[x].setMask(compuSnakePix[x].createHeuristicMask());
        }
    }

    // 2. Samy (Player Snake)
    PIXMAP.load((pixDir + "snake2.xpm").latin1());
    const QBitmap *mask2 = PIXMAP.mask();
    for (int x = 0 ; x < 18; x++){
        samyPix[x] = QPixmap(16, 16);
        bitBlt(&samyPix[x] ,0,0, &PIXMAP,x*16, 0, 16, 16, Qt::CopyROP, TRUE);
        if (mask2) {
            QBitmap tileMask(16, 16);
            bitBlt(&tileMask, 0, 0, mask2, x*16, 0, 16, 16, Qt::CopyROP, TRUE);
            samyPix[x].setMask(tileMask);
        } else {
            samyPix[x].setMask(samyPix[x].createHeuristicMask());
        }
    }

    // 3. Ball
    PIXMAP.load((pixDir + "ball.xpm").latin1());
    const QBitmap *mask3 = PIXMAP.mask();
    for (int x = 0 ; x < 4; x++){
        ballPix[x] = QPixmap(16, 16);
        bitBlt(&ballPix[x] ,0,0, &PIXMAP,x*16, 0, 16, 16, Qt::CopyROP, TRUE);
        if (mask3) {
            QBitmap tileMask(16, 16);
            bitBlt(&tileMask, 0, 0, mask3, x*16, 0, 16, 16, Qt::CopyROP, TRUE);
            ballPix[x].setMask(tileMask);
        } else {
            ballPix[x].setMask(ballPix[x].createHeuristicMask());
        }
    }

    // 4. Apples
    PIXMAP.load((pixDir + "apples.xpm").latin1());
    const QBitmap *mask4 = PIXMAP.mask();
    for (int x = 0 ; x < 2; x++){
        applePix[x] = QPixmap(16, 16);
        bitBlt(&applePix[x] ,0,0, &PIXMAP,x*16, 0, 16, 16, Qt::CopyROP, TRUE);
        if (mask4) {
            QBitmap tileMask(16, 16);
            bitBlt(&tileMask, 0, 0, mask4, x*16, 0, 16, 16, Qt::CopyROP, TRUE);
            applePix[x].setMask(tileMask);
        } else {
            applePix[x].setMask(applePix[x].createHeuristicMask());
        }
    }
}

void PixServer::initbackPixmaps()
{
    QString path;
    plainColor = FALSE;

    int red, green, blue;
    red = green = blue = 0;

    KConfig *conf = kapp->getConfig();
    if(conf != NULL) {
        int i = conf->readNumEntry("Background", 2);
        if(i == 1) {
            QString s = conf->readEntry("BackgroundColor");
            if ( s.length() > 0)
                sscanf(s.latin1(), "%d %d %d", &red, &green, &blue);
            backgroundColor = (QColor(red, green, blue));
            plainColor = TRUE;
        } else if(i == 2) {
            path = KApplication::kde_datadir();
            path.append("/ksnake/backgrounds/");
            path.append("Green_Carpet.xpm");
            path = conf->readEntry("BackgroundPixmap", path.latin1());
        }
    }

    QPixmap PIXMAP;
    int pw, ph;

    backPix = QPixmap(560, 560);

    if (! plainColor) {
        PIXMAP = loadPixmap(path.latin1());

        if (!PIXMAP.isNull()) {
            pw = PIXMAP.width();
            ph = PIXMAP.height();

            for (int x = 0; x <= 560; x+=pw)
                for (int y = 0; y <= 560; y+=ph)
                    bitBlt(&backPix, x, y, &PIXMAP);
        }
        else  {
            printf("error loading background image :%s\n", path.latin1() );
            backgroundColor = (QColor(red, green, blue));
            plainColor = TRUE;
        }
    }
    if ( plainColor)
        backPix.fill(backgroundColor);
}

void PixServer::initBrickPixmap()
{
    QPixmap PIXMAP;

    PIXMAP.load((pixDir + "brick.xpm").latin1());
    if (PIXMAP.isNull()) {
        printf(klocale->translate("error loading %s, aborting\n"), (pixDir + "brick.xpm").latin1());
        kapp->quit();
    }
    int pw = PIXMAP.width() > 0 ? PIXMAP.width() : 16;
    int ph = PIXMAP.height() > 0 ? PIXMAP.height() : 16;

    offPix = QPixmap(560, 560);
    for (int x = 0; x <= 560; x+=pw)
        for (int y = 0; y <= 560; y+=ph)
            bitBlt(&offPix, x, y, &PIXMAP);
}

void PixServer::initRoomPixmap()
{
    QPainter paint;
    roomPix = QPixmap(560, 560);
    bitBlt(&roomPix,0,0, &backPix);
    paint.begin(&roomPix);

    for (unsigned int x = 0; x < board->size(); x++) {
        if (board->isBrick(x))
            drawBrick(&paint, x);
    }
    paint.end();
}

void PixServer::drawBrick(QPainter *p ,int i)
{
    QColor light("gray90");
    QColor dark("gray40");

    int topSq   = board->getNext(N, i);
    int botSq   = board->getNext(S, i);
    int rightSq = board->getNext(E ,i);
    int leftSq  = board->getNext(W, i);

    QRect rect = board->rect(i);

    int x = rect.x();
    int y = rect.y();

    int X = x;
    int Y = y;
    int width, height;
    int Width, Height;

    int highlight = 3;

    Width = Height = width = height = rect.width();

    p->fillRect(x, y, width, height, light);

    if(!board->isBrick(topSq))    {Y+=highlight; Height-=highlight;}
    if(!board->isBrick(leftSq))   {X+=highlight; Width-=highlight;}
    if(!board->isBrick(botSq))     Height-=highlight;
    if(!board->isBrick(rightSq))   Width-=highlight;

    QRect xyz(X,Y,Width,Height);

    bitBlt(&roomPix, xyz.x(), xyz.y(), &offPix,
           xyz.x(), xyz.y(), xyz.width(), xyz.height());

    if (board->isBrick(topSq) && board->isBrick(rightSq)
        && !board->isBrick(board->getNext(NE ,i)))
        p->fillRect(x+width-highlight, y, highlight, highlight, dark);

    if (board->isBrick(botSq) && board->isBrick(rightSq)
        && !board->isBrick(board->getNext(SE, i)))
        p->fillRect(x+width -highlight, y+height-highlight, highlight, highlight, dark);

    if (board->isBrick(topSq) && board->isBrick(leftSq)
        && !board->isBrick(board->getNext(NW, i)))
        p->fillRect(x, y, highlight, highlight, light);

    if (board->isBrick(leftSq) && board->isBrick(botSq)
        && !board->isBrick(board->getNext(SW ,i)))
        p->fillRect(x, y+height-highlight, highlight, highlight, light);

    QBrush brush( dark);
    p->setBrush(brush);
    p->setPen(Qt::NoPen);

    QPointArray a;

    if(!board->isBrick(topSq) && !board->isBrick(rightSq)){
        a.setPoints( 4,
                     x+width-highlight, y+height-1,
                     x+width-1, y+height-1,
                     x+width-1, y+1,
                     x+width-highlight, y+highlight);
        p->drawPolygon(a);
    }
    else if(!board->isBrick(rightSq))
        p->fillRect(x + width -highlight, y, highlight, height, dark);

    if(!board->isBrick(leftSq) && !board->isBrick(botSq)){
        a.setPoints( 4,
                     x+width-1, y+height-highlight,
                     x+width-1, y+height-1,
                     x+1, y+height-1,
                     x+highlight, y+height-highlight);
        p->drawPolygon(a);
    }
    else if(!board->isBrick(botSq))
        p->fillRect(x , y+height-highlight, width, highlight, dark);
}
