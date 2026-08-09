#include "preview.h"

Preview::Preview()
  : QPushButton(0)
{

}

Preview::~Preview()
{

}

void Preview::closeEvent(QCloseEvent *e)
{
  emit clicked();
}

#include "preview.moc"
