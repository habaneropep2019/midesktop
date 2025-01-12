#include "kpopmenu.h"
#include "kpopmenu.h"
#include <qpainter.h>
#include <klocale.h>
#include <kapp.h>

KPopupMenu::KPopupMenu(QWidget *parent, const char *name)
    : QPopupMenu(parent, name)
{
    initialize(klocale->translate("Untitled"));
}

KPopupMenu::KPopupMenu(const char *title, QWidget *parent, const char *name)
    : QPopupMenu(parent, name)
{
    initialize(title);
}


KPopupMenu::~KPopupMenu()
{
}


const char *KPopupMenu::title() const
{
    return text(0);
}

void KPopupMenu::setTitle(const char *title)
{
    changeItem(title, 0);
}
    
void KPopupMenu::initialize(const char *title)
{
    insertItem(title);
    insertSeparator();
    insertSeparator();
}

#include "kpopmenu.moc"
