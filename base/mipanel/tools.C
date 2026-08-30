/* tool.C - Part of the KDE project.
  
  Copyright (C) 1996,97 Matthias Ettrich
  
  Modifications to this program were made by Corel Corporation,
  November, 1999.  All such modifications are copyright (C) 1999
  Corel Corporation and are licensed under the terms of the GNU
  General Public License.
 			 
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

  **************************************************************************

  Modifications to this program were made by Alec Bloss,
  based on the original code and modifications made by Corel Corporations.
  All modifications are (C) Copyright 2026 Alec Bloss and are licensed under
  the terms of the GNU General Public License, or, may be used under the MIT
  license.

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include <stdio.h>
#include <unistd.h>
#include <time.h>

#include "mipanel.h"
#include "kfm.h"
#include <kbutton.h>
#include <kprocess.h>

#include <qapp.h>
#include <qmsgbox.h>
#include <qpmcache.h>
#include <qmenudta.h>
#include <qkeycode.h>
#include <qbitmap.h>
#include <qtooltip.h>
#include <qscrollbar.h>
#include <qlayout.h>
#include <qwmatrix.h>
#include <qapp.h>
#include <qfileinfo.h>
#include <qevent.h>

int myPopupMenu::keyStatus = 0;

//
// Constructors
//
myPopupMenu::myPopupMenu( QWidget *parent, const char *name )
  : QPopupMenu( parent, name ), id(-1), parentMenu(NULL) 
{
	setMouseTracking(true);
}

myPopupMenu::myPopupMenu( PMenu* _parentMenu )
  : QPopupMenu( 0, 0 ), id(-1), parentMenu(_parentMenu) 
{
	setMouseTracking(true);
}

/////////////////////////////////////////////////////////////////////////////
bool myPopupMenu::x11Event( XEvent * xe)
{
  if (xe->type == UnmapNotify)
    if (parentMenu && parentMenu->inherits("PFileMenu"))
      ((PFileMenu *)parentMenu)->deactivated(id);

  return QPopupMenu::x11Event(xe);
}

/////////////////////////////////////////////////////////////////////////////
void myPopupMenu::mousePressEvent ( QMouseEvent *e )
{
  myPopupMenu::keyStatus = e->state();
  QPopupMenu::mousePressEvent(e);
}

/////////////////////////////////////////////////////////////////////////////
int myPopupMenu::entryHeight()
{
  return itemHeight(0);
}

/////////////////////////////////////////////////////////////////////////////
int myPopupMenu::maxEntriesOnScreen()
{
  QWidget *desktop = QApplication::desktop();
  int sh = desktop->height();			// screen height

  int entryh = entryHeight();

  return sh / entryh;

}

//////////////////////////////////////////////////////////////////////////////

int myPopupMenu::height()
{
  return QPopupMenu::height();
}

int myPopupMenu::width()
{
  return QPopupMenu::width();
}


myPushButton::myPushButton(QWidget *parent, const char* name)
  : QPushButton( parent, name )
{
    setMouseTracking(true);
    setFocusPolicy(NoFocus);
    flat = True;
    never_flat = False;
    flat_means_down = False;
    draw_down = False;
    swallowed_window = None;
    last_button = 0;
}

myPushButton::~myPushButton () 
{
  if (most_recent_pressed == this)
    most_recent_pressed = 0;
}

myPushButton* myPushButton::most_recent_pressed = 0;

void myPushButton::enterEvent( QEvent * )
{
  if (!flat)
    return;
  flat = False;
  if (!never_flat)
	{
    repaint();
  }
}

void myPushButton::leaveEvent( QEvent * )
{
  if (flat)
    return;
  flat = True;
  if (!never_flat)
    repaint();
}

void myPushButton::paint(QPainter *painter)
{
  draw_down = (isDown() || (isOn()) && (never_flat || !flat));
  if (flat_means_down && !never_flat && flat)
    draw_down = true;
	
  drawButtonLabel(painter);

  if (draw_down ) 
	{
    if ( style() == WindowsStyle )
      qDrawWinButton( painter, 0, 0, width(),
		      height(), colorGroup(), true );
    else
			qDrawShadePanel( painter, 0, 0, width(),
	  	       height(), colorGroup(), true, 2, 0L );
  }
  else if (!flat || never_flat) 
	{
    if ( style() == WindowsStyle )
      qDrawWinButton( painter, 0, 0, width(), height(),
		      colorGroup(), false );
    else 
		{
      qDrawShadePanel( painter, 0, 0, width(), height(),
		       colorGroup(), false, 2, 0L );
//       painter->setPen(black);
//       painter->drawRect(0,0,width(),height());
    }
  }
}

void myPushButton::drawButtonLabel(QPainter *painter)
{
  if ( pixmap() ) 
	{
    int dx = ( width() - pixmap()->width() ) / 2;
    int dy = ( height() - pixmap()->height() ) / 2;
    if ( draw_down && style() == WindowsStyle ) {
      dx++;
      dy++;
    }
    painter->drawPixmap( dx, dy, *pixmap() );
  }
}


void myPushButton::mousePressEvent( QMouseEvent *e)
{
  if ( isDown())
    return;

  bool hit = hitButton( e->pos() );
  if ( hit ){
    most_recent_pressed = this;
    last_button = e->button();
    setDown( true );
    if (style() == WindowsStyle && swallowed_window != None)
      XMoveWindow(qt_xdisplay(), swallowed_window, 4, 4);
    repaint( false );
    emit pressed();
  }
}

void myPushButton::mouseReleaseEvent( QMouseEvent *e)
{
  if ( !isDown() ){
    last_button = 0;
    releaseMouse();
    return;
  }
  bool hit = hitButton( e->pos() );
  if (style() == WindowsStyle && swallowed_window != None)
    XMoveWindow(qt_xdisplay(), swallowed_window, 3, 3);
  setDown( false );
  if ( hit ){
    if ( isToggleButton() )
      setOn( !isOn() );
    repaint( false );
    if ( isToggleButton() )
      emit toggled( isOn() );
    emit released();
    emit clicked();
  }
  else {
    repaint();
    emit released();
  }
  last_button = 0;
  releaseMouse();
}

void myPushButton::mouseMoveEvent( QMouseEvent *e )
{
  if (!last_button)
    return;

  if ( !(e->state() & LeftButton) &&
       !(e->state() & MidButton) &&
       !(e->state() & RightButton))
    return;

  bool hit = hitButton( e->pos() );
  if ( hit ) {
    if ( !isDown() ) {
      setDown(true);
      if (style() == WindowsStyle && swallowed_window != None)
	XMoveWindow(qt_xdisplay(), swallowed_window, 4, 4);
      repaint(false);
      emit pressed();
    }
  } else {
    if ( isDown() ) {
      setDown(false);
      if (style() == WindowsStyle && swallowed_window != None)
	XMoveWindow(qt_xdisplay(), swallowed_window, 3, 3);
      repaint();
      emit released();
    }
  }
}

myTaskButton* myTaskButton::active = 0;

myTaskButton::myTaskButton(QWidget *parent, const char* name)
  :myPushButton(parent, name)
{
    win = None;
    virtual_desktop = 1;
    never_flat = True;
    flat_means_down = True;
		m_bSticky = false;

		connect(this, SIGNAL(clicked()), this, SLOT(btnClicked()));
		connect(this, SIGNAL(pressed()), this, SLOT(btnPressed()));
}

myTaskButton::~myTaskButton () 
{
  if (active == this)
    active = 0;
}

bool myTaskButton::isActive() const
{
   return this == active;
}

void myTaskButton::setActive(bool value)
{
  if (value){
    myTaskButton* tmp = active;
    active = this;
    if (tmp && tmp != this)
      tmp->setActive(false);
    never_flat = false;
    flat = !(rect().contains(mapFromGlobal(QCursor::pos()), true));
    repaint();
  }
  else {
    never_flat = true;
    flat = !(rect().contains(mapFromGlobal(QCursor::pos()), true));
    repaint();
  }
}

void myTaskButton::setNoActive()
{
  myTaskButton* buttontolower = active;
  active = 0;
  if (buttontolower!=0)
    buttontolower->setActive(FALSE);
}

void myTaskButton::setText(const char* arg)
{
  if (s != arg){
    s = arg;
    QToolTip::add(this, s);
    repaint();
  }
}

void myTaskButton::drawButtonLabel( QPainter *painter )
{
  if (this == active){
    if (QApplication::style() == WindowsStyle)
      painter->fillRect(rect(), QBrush( white, Dense4Pattern ));
    else
      painter->fillRect(rect(), QBrush( colorGroup().mid(), Dense4Pattern ));
  }

  if ( pixmap() ) {
    int dx = ( 32 - pixmap()->width() ) / 2;
    int dy = ( height() - pixmap()->height() ) / 2;
    if ( draw_down && style() == WindowsStyle ) {
      dx++;
      dy++;
    }
    painter->drawPixmap( dx, dy, *pixmap() );
  }
  painter->setPen(colorGroup().text());
  if (!s.isNull()){
    QString s2 = s;
    if (fontMetrics().width(s2) > width()-32){
      //s2.detach(); <-- No longer needed in Qt2 as QString automatically detaches now
      while (s2.length()>0 &&
	     fontMetrics().width(s2) > width()-32-fontMetrics().width("...")){
	s2.truncate(s2.length()-1);
      }
      s2.append("...");
    }
    if ( draw_down && style() == WindowsStyle )
      painter->drawText( 33, 1, width()-31, height()+1, AlignLeft|AlignVCenter, s2);
    else
      painter->drawText( 32, 0, width()-32, height(), AlignLeft|AlignVCenter, s2);
  }
}

void myTaskButton::btnClicked()
{
	emit clicked(this);
}

void myTaskButton::btnPressed()
{
	emit pressed(this);
}

void execute(const char* cmd)
{
  KShellProcess proc;
  proc << cmd;
  proc.start(KShellProcess::DontCare);
}


void kPanel::init_popup(QPopupMenu* popup)
{
  popup->installEventFilter( this );
}

void kPanel::generateWindowlist(QPopupMenu* p)
{
  Window *w;
  int i = 0;
  int nw = kwmmapp->windows.count();
  p->clear();
  callbacklist = new Window[nw];
  for (w = kwmmapp->windows.first(); w; w = kwmmapp->windows.next())
    callbacklist[i++]=*w;
  int d = 1;
  int nd = KWM::numberOfDesktops();
  int cd = KWM::currentDesktop();
  Window active_window = KWM::activeWindow();
  if (nd>1){
    for (d=1; d<=nd; d++){
      p->insertItem(QString("&")+KWM::getDesktopName(d), 1000+d);
      if (!active_window && d == cd)
	p->setItemChecked(1000+d, true);
      for (i=0; i<nw;i++){
	if (
	    (KWM::desktop(callbacklist[i]) == d && !KWM::isSticky(callbacklist[i]))
	    || (d == cd && KWM::isSticky(callbacklist[i]))
	    ){
	    QPixmap pm = KWM::miniIcon(callbacklist[i], 16, 16);
	    p->insertItem(pm.isNull()?defaultPixmap:pm,
			  QString("   ")+KWM::titleWithState(callbacklist[i]),i);
	  if (callbacklist[i] == active_window)
	    p->setItemChecked(i, true);
	}
      }
      if (d < nd)
	p->insertSeparator();
    }
  }
  else {
    for (i=0; i<nw;i++){
      if (
	  (KWM::desktop(callbacklist[i]) == d && !KWM::isSticky(callbacklist[i]))
	  || (d == cd && KWM::isSticky(callbacklist[i]))
	  ){
	    QPixmap pm = KWM::miniIcon(callbacklist[i], 16, 16);
	    p->insertItem(pm.isNull()?defaultPixmap:pm,
			  KWM::titleWithState(callbacklist[i]),i);
	    if (callbacklist[i] == active_window)
		p->setItemChecked(i, true);
      }
    }
  }
}


void kPanel::windowlistActivated(int item)
{
  if (item>1000){
    KWM::switchToDesktop(item-1000);
  }
  else {
    Window w = callbacklist[item];
    delete [] callbacklist;
    KWM::activate(w);
  }
}

int kPanel::show_popup(QPopupMenu* popup, QWidget* button, bool isTaskButton)
{
	int xp;
	int yp;
	
	if (popup == windowlist)
		generateWindowlist(windowlist);
	
	popup->move(-1000,-1000);
	popup->show();
	
	if (!isTaskButton)
	{
		if (orientation == Qt::Vertical)
		{
			yp = button->mapToGlobal(QPoint(0,0)).y();
			if (position == top_left)
				xp = button->mapToGlobal(QPoint(0,0)).x() + button->width();
			else
				xp = button->mapToGlobal(QPoint(0,0)).x() - popup->width();
		}
		else 
		{
			xp = button->mapToGlobal(QPoint(0,0)).x();
			if (position == top_left)
				yp = button->mapToGlobal(QPoint(0,0)).y() + button->height();
			else
			yp = button->mapToGlobal(QPoint(0,0)).y() - popup->height();
		}
		if (yp + popup->height() > QApplication::desktop()->height())
			yp = button->y()+button->height()-popup->height();
		if (yp < 0)
			yp = QApplication::desktop()->height()-popup->height()+button->height();
	}
	else 
	{
		if (taskbar_position == taskbar_top_left)
		{
			xp = button->mapToGlobal(QPoint(0,0)).x() + button->width();
			yp = button->mapToGlobal(QPoint(0,0)).y();
		}
		else 
		{
			xp = button->mapToGlobal(QPoint(0,0)).x();
			if (taskbar_position == top)
				yp = button->mapToGlobal(QPoint(0,0)).y() + button->height();
			else
				yp = button->mapToGlobal(QPoint(0,0)).y() - popup->height();
		}
	}
	
	popup->move(QPoint(xp, yp));
	int v = popup->exec();
	QEvent ev(QEvent::Leave);
	QMouseEvent mev (QEvent::MouseButtonRelease, QCursor::pos(), LeftButton, LeftButton);
	QApplication::sendEvent(button, &ev);
	QApplication::sendEvent(button, &mev);
	return v;
}

void kPanel::set_button_text(QButton* button, const char* s)
{
  QToolTip::add(button, s);
  QPixmap pm(button->width(), button->height());
  pm.fill(button->backgroundColor());

  QPainter p;
  int a = 2+ (pm.height() - button->fontMetrics().height())/2;
  if (a > button->fontMetrics().height()/2)
    a = button->fontMetrics().height()/2;
  int b = (pm.height() + button->fontMetrics().height())/2-button->fontMetrics().descent();
  if (a<3) a = 3;

  p.begin( &pm );
  p.setFont(button->font());
  p.setPen(colorGroup().text());
  p.drawText(a,b,s);
  p.end();
  QBitmap bm(pm.width(), pm.height());
  bm.fill( color0 );
  p.begin(&bm);
  p.setFont(button->font());
  p.drawText(a,b,s);
  p.end();
  pm.setMask(bm);
  button->setPixmap(pm);
}

QPixmap kPanel::create_arrow_pixmap(QPixmap pm)
{
  QColorGroup colgrp = QColorGroup( colorGroup().text(),
				    backgroundColor(),
				    white, black, black,
  				    black, white );
  QColorGroup colgrp2 = QColorGroup( color1, color0, color1, color1, color1,
 				     color1, color1 );

  QPixmap pm2 = QPixmap(box_width, box_height);
  QBitmap bm = QBitmap(box_width, box_height);
  bm.fill(color0);
  pm2.setMask(bm);

  QPainter p;
  QPainter p2;

  pm2.fill( backgroundColor() );
	  	
  p.begin( &pm2 );

  p2.begin( pm2.mask());
	
  if (!pm.isNull()){
    p.drawPixmap( (pm2.width()-pm.width())/2,
		  (pm2.height()-pm.height())/2,
		  pm, 0, 0, pm.width(), pm.height());
    if (pm.mask())
      p2.drawPixmap( (pm2.width()-pm.mask()->width())/2,
		     (pm2.height()-pm.mask()->height())/2,
		     *(pm.mask()), 0, 0, pm.mask()->width(), pm.mask()->height());
  }

  if (orientation == Qt::Horizontal){
    if (position == top_left){
      qDrawArrow( &p, DownArrow, WindowsStyle, false,
		  box_width-6, box_height-5, 0, 0, colgrp, true);
      qDrawArrow( &p2, DownArrow, WindowsStyle, false,
 		  box_width-6, box_height-5, 0, 0, colgrp2, true);
    }
    else{
      qDrawArrow( &p, UpArrow, WindowsStyle, false,
		  box_width-6, 4, 0, 0, colgrp, true);
      qDrawArrow( &p2, UpArrow, WindowsStyle, false,
		  box_width-6, 4, 0, 0, colgrp2, true);
    }
  }
  else{ // position == vertical
    if (position == top_left){
      qDrawArrow( &p, RightArrow, WindowsStyle, false,
		  box_width-5, box_height-6, 0, 0, colgrp, true);
      qDrawArrow( &p2, RightArrow, WindowsStyle, false,
 		  box_width-5, box_height-6, 0, 0, colgrp2, true);
    }
    else{
      qDrawArrow( &p, LeftArrow, WindowsStyle, false,
		  5, box_height-6, 0, 0, colgrp, true);
      qDrawArrow( &p2, LeftArrow, WindowsStyle, false,
		  5, box_height-6, 0, 0, colgrp2, true);
    }
  }
  p.end();
  p2.end();
  return pm2;
}


void arrow_on_pixmap(QWidget *w, QPixmap* pm, Qt::ArrowType rt)
{
  QColorGroup colgrp = QColorGroup( w->colorGroup().text(),
				    w->backgroundColor(),
				    Qt::white, Qt::black, Qt::black,
				    Qt::black, Qt::white );
  QColorGroup colgrp2 = QColorGroup( Qt::color1, Qt::color0, Qt::color1, Qt::color1, Qt::color1,
				     Qt::color1, Qt::color1 );
  QPainter paint;
  QPainter paint2;
  paint.begin(pm);
  paint2.begin(pm->mask());
  qDrawArrow( &paint, rt, Qt::WindowsStyle, false,
	      pm->width()/2, pm->height()/2,
	      0, 0,
	      colgrp, true);
  qDrawArrow( &paint2, rt, Qt::WindowsStyle, false,
	      pm->width()/2, pm->height()/2,
	      0, 0,
	      colgrp2, true);
  paint.end();
  paint2.end();
}

void kPanel::arrow_on_pixmap(QPixmap* pm, ArrowType rt)
{
	::arrow_on_pixmap(this, pm, rt);
}

QPixmap kPanel::load_pixmap(const char* name, bool is_folder){
  QPixmap pm;
  pm.resize(0,0);
  if (!name || name[0]=='\0')
    name = is_folder?"folder.xpm":"exec.xpm";

  if (box_width-10 > 16 || box_height-10 > 16){
    pm =  kapp->getIconLoader()
      ->loadApplicationIcon(name, box_width-4, box_height-4);
    if (pm.isNull())
      pm = kapp->getIconLoader()
	->loadApplicationIcon(is_folder?"folder.xpm":"exec.xpm",
			      box_width-4, box_height-4);
  }
  else {
    pm =  kapp->getIconLoader()
      ->loadApplicationMiniIcon(name, box_width-4, box_height-4);
    if (pm.isNull())
      pm = kapp->getIconLoader()
	->loadApplicationMiniIcon(is_folder?"folder.xpm":"exec.xpm",
				  box_width-4, box_height-4);
  }
  return pm;
}


void kPanel::set_label_date()
{
  struct tm *loctime;
  char dayline[256];
  char timeline[256];
  char dateline[256];
  time_t curtime;

  curtime=time(0);

  if( clockBeats )
    loctime=gmtime(&curtime);
  else
    loctime=localtime(&curtime);

  strftime(dayline,256,"%a\n",loctime);

  if (clockBeats) 
	{
    long iTime = (((loctime->tm_hour*3600 + loctime->tm_min*60 + loctime->tm_sec)+3600)*1000)/86400;

    if( iTime >= 1000 )
      iTime -= 1000;
    else if( iTime < 0 )
      iTime += 1000;

    sprintf(timeline,"@%.3d", (int)iTime );
  }
  else if( clockAmPm )
    strftime(timeline,256,"%I:%M%p",loctime);
  else
    strftime(timeline,256,"%H:%M",loctime);
    //kartiks: remove the leading '0' if any exist
    if(timeline[0] == '0')
      strcpy(timeline, &timeline[1]);

  strftime(dateline,256,i18n("%b %d\n"),loctime);

  QToolTip::add(label_date, QString(dayline)+QString(dateline)+QString(timeline));

  if (label_date->fontMetrics().lineSpacing() * 3 <= label_date->height())
    label_date->setText(QString(dayline)+QString(dateline)+QString(timeline));
  else if (label_date->fontMetrics().lineSpacing() * 2 <= label_date->height())
    label_date->setText(QString(dateline)+QString(timeline));
  else
    label_date->setText(timeline);
}

void kPanel::add_windowlist()
{
	if (has_windowlist_button) 
	{
		for (DesktopEntry *pE = entries.first(); pE; pE = entries.next())
		{
			if (pE->popup == windowlist) 
			{
				delete_button(pE->button);
				break;
			}
		}
	}
	else
		addButtonInternal(0, -1, -1, "windowlist");
	
	has_windowlist_button = !has_windowlist_button;
	panel_popup->setItemChecked(add_windowlist_entry, has_windowlist_button);
	writeOutConfiguration();
}
	
// --sven: kdisknav button start --
void kPanel::add_kdisknav()
{
	if (has_kdisknav_button) 
	{
		for (DesktopEntry *pE = entries.first(); pE; pE = entries.next())
		{
			if (pE->popup == m_pDiskNavMenu) 
			{
				delete_button(pE->button);
				break;
			}
		}
	}
	else
		addButtonInternal(0, -1, -1, "kdisknav");
	
	has_kdisknav_button = !has_kdisknav_button;
	panel_popup->setItemChecked(add_disknav_entry, has_kdisknav_button);
	writeOutConfiguration();
}
// --sven: kdisknav button end --

void kPanel::ask_logout(){
  KWM::logout();
}


void kPanel::call_klock(){
  execute("klock");
}

//PAL - Oct98
void kPanel::call_run(){
  KWM::sendKWMCommand("commandLine");
}

 void kPanel::call_help(){
  execute("kdehelp");
}

void kPanel::addButton(PMenuItem* pmi)
{
  QString s = pmi->getSaveName().copy();
  s.remove(0,1);
  PMenuItem* pmi2 = pmenu->searchItem(s);

  addButtonInternal(pmi2);
  writeOutConfiguration();
}

void kPanel::delete_button(QWidget* button)
{
	DesktopEntry *pE = entries.findButton(button);
	if (pE)
	{
		if (pE->drop_zone)
			delete pE->drop_zone;
		if (pE->swallowed)
			KWM::close(pE->swallowed);

		m_pButtonsArea->removeEntry(button);
		delete button;
	
		if (kde_button == button)
			kde_button = 0;

		entries.removeRef(pE);
	}
}

void kPanel::cleanup()
{
	if (m_pPagerProcess)
	{
		m_pPagerProcess->kill();
	}

	for (Window *w = kwmmapp->dock_windows.first(); w;
				w = kwmmapp->dock_windows.next())
	{
		XReparentWindow(qt_xdisplay(), *w, qt_xrootwin(), 0, 0);
		XFlush(qt_xdisplay());
	}

	for (DesktopEntry *pE = entries.first(); pE; pE = entries.next())
	{
		if (pE->swallowed)
		{
			KWM::close(pE->swallowed);
			XFlush(qt_xdisplay());
		}
	}
	QApplication::syncX();
}

void kPanel::showSystem()
{
	QPopupMenu *kmenu = pmenu->getQPopupMenu();
	if (!kmenu || kmenu->isVisible())
		return;
	
	if (panelHidden[currentDesktop])
	{
		miniButtons(1);
	}
	else 
	{
		kmenu->setActiveItem(kmenu->count()-1);
		show_popup(kmenu, kde_button);
	}
}





void kPanel::showMiniPanel ()
{
	int mh = taskbar_height + frameWidth() * 2;
	int mw = 3*mh; // mini panel width
	
	int h = QApplication::desktop()->height();
	
	if (miniPanelHidden) 
	{
		miniPanelHidden = false;
		
		
		miniPanelFrame = new QFrame(0, 0,
							WStyle_Customize|WStyle_NoBorder|WStyle_Tool);
		miniPanelFrame->setMouseTracking( true );
		miniPanelFrame->installEventFilter (this);
		
		miniPanel = new QButtonGroup(miniPanelFrame);
		miniPanel->setFrameStyle(QFrame::Panel| QFrame::Raised);
		miniPanel->setMouseTracking( true );
		miniPanel->installEventFilter (this);
		connect( miniPanel, SIGNAL( pressed( int )), this, SLOT( miniButtons(int)) );
		
		miniSystem = new myPushButton(miniPanel);
		QToolTip::add(miniSystem, i18n("Application Starter"));
		miniSystem->setFocusPolicy(NoFocus);
		
		// load KMenu icon
		KConfig *config = KApplication::getKApplication()->getConfig();
		config->setGroup("KPanel Customization");
		QString strIconName = config->readEntry("Icon", "go.xpm");
		QPixmap pixmap(kapp->getIconLoader()->loadMiniIcon(strIconName, mh, mh));
		if (pixmap.isNull())
		{
			pixmap = kapp->getIconLoader()->loadMiniIcon("go.xpm", mh, mh);
		}
		miniSystem->setPixmap(pixmap);
		
		miniSystem->setMouseTracking( true );
		miniSystem->installEventFilter (this);
		
		miniDesk = new myPushButton(miniPanel);
		QToolTip::add(miniDesk, klocale->translate("Windowlist"));
		miniDesk->setFocusPolicy(NoFocus);
		miniDesk->setPixmap(kapp->getIconLoader()
						->loadMiniIcon("window_list.xpm", mh, mh));
		miniDesk->setMouseTracking( true );
		miniDesk->installEventFilter (this);
		
		// -- pietro: kdisknav button start --
		if (bShowDiskNav)
		{
			miniDiskNav = new myPushButton(miniPanel);
			QToolTip::add(miniDiskNav, klocale->translate("KDiskNavigator"));
			miniDiskNav->setFocusPolicy(NoFocus);
			miniDiskNav->setPixmap(kapp->getIconLoader()
					->loadMiniIcon("kdisknav.xpm", mh,mh));
			miniDiskNav->setMouseTracking( true );
			miniDiskNav->installEventFilter (this);
		}
		else
		{
			miniDiskNav = 0L;
			mw -= mh; // there are only two button in panel
		}
		// -- pietro: kdisknav button end --
		
		miniPanel->setGeometry (0, 0, mw, mh);
		miniPanel->insert(miniSystem, 1);
		miniPanel->insert(miniDesk, 2);
		// -- pietro: kdisknav button start --
		if(miniDiskNav)
		{
			miniPanel->insert(miniDiskNav, 3);
		}
		// -- pietro: kdisknav button end --
		
		miniSystem->setGeometry(1, 1, mh-1, mh-2);
		miniDesk->setGeometry(mh, 1, mh-1, mh-2);
		// -- pietro: kdisknav button start --
		if(miniDiskNav)
		{
			miniDiskNav->setGeometry(mh*2, 1, mh-1, mh-2);
		}
	// -- pietro: kdisknav button end --
	}

	int sx=0; int sx1 =0;
	if (position == top_left && panelCurrentlyLeft)
		sx = x() + panel_button->x() + panel_button->width()+1;
	if (position == bottom_right && (orientation == Qt::Horizontal && panelCurrentlyLeft))
		sx1 = x() + panel_button->x() + panel_button->width()+1;
	if (position == top_left && (orientation == Qt::Vertical && !panelCurrentlyLeft))
		sx1 = x() + panel_button->x() + panel_button->width()+1;
	
	if (taskbar_position == taskbar_top_left)
		miniPanelFrame->setGeometry(sx, 0, mw, mh);
	else if (taskbar_position == hidden)
		miniPanelFrame->setGeometry(sx, 0, mw, mh);
	else if (taskbar_position == bottom)
		miniPanelFrame->setGeometry(sx1, h-mh, mw, mh);
	else if (taskbar_position == top)
		miniPanelFrame->setGeometry(sx, 0, mw, mh);
	
	miniPanelFrame->show();
	miniPanelFrame->raise();
	KWM::sendKWMCommand("moduleRaised");
}

void kPanel::hideMiniPanel()
{
   if (miniPanelHidden)
     return;              // do nothing if not there
   miniPanelHidden=true;
   miniPanelFrame->hide();
   delete miniPanelFrame;
 }

void kPanel::miniButtons(int i)
{
	switch (i)
	{
	case 1:
		{
			QPopupMenu *kmenu = pmenu->getQPopupMenu();
			if (!kmenu)
				break;
			if (taskbar_position == bottom)
			{
				kmenu->move(-1000,-1000);
				kmenu->show();
				kmenu->move(QPoint(miniPanelFrame->x() + miniSystem->x(),
													miniPanelFrame->y() - kmenu->height()));
			}
			else
				kmenu->move(QPoint(miniPanelFrame->x()+miniSystem->x(),
													miniSystem->y()+miniPanelFrame->height()));
			kmenu->exec();
		}
		break;
		
	case 2:
		generateWindowlist(windowlist);
		if (taskbar_position == bottom)
		{
			windowlist->move(-1000,-1000);
			windowlist->show();
			windowlist->move(QPoint(miniPanelFrame->x()+miniDesk->x(),
														miniPanelFrame->y() - windowlist->height()));
		}
		else
			windowlist->move(QPoint(miniPanelFrame->x()+miniDesk->x(),
														miniDesk->y() + miniPanelFrame->height()));
		
		windowlist->exec();
		break;
		// -- pietro: kdisknav button start --
	case 3:
		if (!m_pDiskNavMenu) // might be uninitialized...
			break;
		
		if (taskbar_position == bottom)
		{
			m_pDiskNavMenu->move(-1000,-1000);
			m_pDiskNavMenu->show();
			m_pDiskNavMenu->move(QPoint(miniPanelFrame->x()+miniDesk->x(),
																miniPanelFrame->y()-m_pDiskNavMenu->height()));
		}
		else
			m_pDiskNavMenu->move(QPoint(miniPanelFrame->x()+miniDesk->x(),
																miniDesk->y()+miniPanelFrame->height()));
		
		m_pDiskNavMenu->exec();
		break;
		// -- pietro: kdisknav button end --
	}
	
	QButton* button = miniPanel->find(i);
	QEvent ev(QEvent::Leave);
	QMouseEvent mev (QEvent::MouseButtonRelease,
	QCursor::pos(), LeftButton, LeftButton);
	QApplication::sendEvent(button, &ev);
	QApplication::sendEvent(button, &mev);
}



QString kPanel::findMenuEditor( const QString& folder)
{
    QString result;
    QDir d ( folder );
    if (!d.exists() )
	return result;
    
    const QFileInfoList *list = d.entryInfoList();
    QFileInfoListIterator it( *list );
    QFileInfo *fi;
    if( it.count() < 3 )
	return result;
    while ( (fi=it.current()) && result.isEmpty() )
	{
	    if( fi->fileName() == "." || fi->fileName() == ".." )
		{ ++it; continue; }
	    if( fi->isDir() && fi->isReadable() )
		{
		    result = findMenuEditor( fi->filePath() );
		}
	    else
		{
		    if (fi->fileName() == "KMenuEdit.kdelnk")
			result = fi->filePath();
		}
	    ++it;
	}
    return result;
}

//
// helper QDesktopPreviewFrame class definition and implementation
//
class QDesktopPreviewFrame : public QFrame
{
public:
	QDesktopPreviewFrame() : QFrame(0, 0, WStyle_Customize | WStyle_NoBorder | WStyle_Tool){};

protected:
	void focusOutEvent(QFocusEvent *);
};

void QDesktopPreviewFrame::focusOutEvent(QFocusEvent *)
{
	if (isVisible())
	{
		hide();
	}
}

void kPanel::desktopPreviewProcessExited(KProcess *p)
{
	if (m_pPagerProcess == p)
	{
		m_pPagerProcess = 0;
		m_pDesktopPreviewFrame->hide();
		delete m_pDesktopPreviewFrame;
		m_pDesktopPreviewFrame = 0;
	}
}

void kPanel::desktopPreview()
{
	// create desktop preview frame
	if (!m_pDesktopPreviewFrame)
	{
		m_pDesktopPreviewFrame = new QDesktopPreviewFrame();
		m_pDesktopPreviewFrame->setFrameStyle(QFrame::Panel| QFrame::Raised);
		m_pDesktopPreviewFrame->hide();
		int w = QApplication::desktop()->width();
		int h = QApplication::desktop()->height();

		int nHg = 160;
		int nNumColumns = (number_of_desktops + 1) / 2; // 2 rows always
		int nCorr = 6 - (nNumColumns - 1) * 4; // {6, 2, -2, -6}  some correction needed
		int nWd = (80 * nNumColumns * w) / h + nCorr;
		m_pDesktopPreviewFrame->setGeometry(0, 0, nWd, nHg);

		QString strAppPath(kapp->kde_bindir() + "/kpager");
		QFileInfo info(strAppPath);
		if(info.exists() && info.isFile() && info.isExecutable())
		{
			KProcess * process = new KProcess();
			if (!process)
				return;

			// set executable
			process->setExecutable(strAppPath);

			// set parameters
			*process << "-parent";
			*process << QString().setNum(m_pDesktopPreviewFrame->winId());

			QObject::connect(process, SIGNAL(processExited(KProcess *)),
											 this, SLOT(desktopPreviewProcessExited(KProcess *)));

			// start process with a link to StdIn for oneway comm.
			process->start(KProcess::NotifyOnExit);
			m_pPagerProcess = process;
		}
	}
	
	if (m_pDesktopPreviewFrame)
	{
		if (!m_pDesktopPreviewFrame->isVisible())
		{
			// calculate correct frame position
			int w = QApplication::desktop()->width();
			int h = QApplication::desktop()->height();
			int nX, nY;
			QPoint btnPos(m_pBtnDesktopPreview->mapToGlobal(m_pBtnDesktopPreview->pos()));
			if (orientation == Qt::Horizontal)
			{
				nX = w - m_pDesktopPreviewFrame->width();
				if (nX > btnPos.x())
					nX = btnPos.x();

				if (position == top_left)
				{
					nY = height();	
				}
				else
				{
					nY = h - m_pDesktopPreviewFrame->height() - height();
				}
			}
			else // orientation == vertical
			{
				nY = h - m_pDesktopPreviewFrame->height();
				if (nY > btnPos.y())
					nY = btnPos.y();

				if (position == top_left)
				{
					nX = width();
				}
				else
				{
					nX = w - width() - m_pDesktopPreviewFrame->width();
				}
			}
			
			if (m_pDesktopPreviewFrame->x() != nX || m_pDesktopPreviewFrame->y() != nY)
				m_pDesktopPreviewFrame->move(nX, nY);

			m_pDesktopPreviewFrame->show();
			m_pDesktopPreviewFrame->setActiveWindow();
			m_pDesktopPreviewFrame->setFocus();
		}
		else
		{
			m_pDesktopPreviewFrame->hide();
		}
	}
}

//
//	ToolTip implementation
//
ToolTip::ToolTip()
{
	menu_tool_tips = 1000;
	menu_tool_tips_disabled = false;

	// create info label
	info_label = new QLabel(0, 0, WStyle_Customize | WStyle_NoBorder | WStyle_Tool);
	info_label->setMouseTracking(true);
	{
		QColorGroup g(black, QColor(255,255,220),
									QColor(96,96,96), black, black,
									black, QColor(255,255,220));
		info_label->setPalette(QPalette( g, g, g ));
	}

	if ( QApplication::style() == MotifStyle )
		info_label->setFrameStyle( QFrame::Plain | QFrame::Box );
	else
		info_label->setFrameStyle( QFrame::Raised | QFrame::Panel );

	info_label->setLineWidth( 1 );
	info_label->setMargin( 1 );
	info_label->setAlignment( AlignLeft | AlignTop );
	info_label->setAutoResize( true );

	// create timer to show tips
	tipTimer = new QTimer( this );
	connect( tipTimer, SIGNAL(timeout()), this, SLOT(tipTimerDone()));
	// create timer to hide tips
	tipSleepTimer = new QTimer( this );
	connect( tipSleepTimer, SIGNAL(timeout()), this, SLOT(tipSleepTimerDone()));
}

void ToolTip::show()
{
	tipTimerDone();
}

bool ToolTip::isVisible()
{
	return (tipTimer->isActive() || info_label->isVisible());
}

void ToolTip::hide()
{
	tipTimer->stop();
	tipSleepTimer->stop();

	if (info_label->isVisible())
		info_label->hide();
}

void ToolTip::showToolTip(QString s)
{
  if (s.length() > 60)
	{
    QString s2("");
    int w = 0;
    for (unsigned int i = 0; i < s.length(); i++)
		{
      if (w > 55 && s.mid(i,1) == " ")
			{
        s2.append("\n");
        w = -1;
      }
      else 
			{
        s2.append(s.mid(i,1));
      }
      w++;
    }
    s = s2;
  }

  if (!s.isEmpty()) 
	{
    info_label->hide();
    info_label->setText(s);
    info_label->adjustSize();

		if (!menu_tool_tips_disabled && menu_tool_tips >= 0)
		{
			tipTimer->start(menu_tool_tips, true);
    }
  }
  else
	{
    hide();
    info_label->setText("");
  }
}

void ToolTip::tipTimerDone()
{
	info_label->move(QCursor::pos().x() + 6, QCursor::pos().y() + 16);
	// make sure it is visible
	if (info_label->y() + info_label->height() > QApplication::desktop()->height())
		info_label->move(info_label->x(), QCursor::pos().y()-16 - info_label->height());
	if (info_label->x() + info_label->width() > QApplication::desktop()->width())
		info_label->move(QCursor::pos().x()-6 - info_label->width(), info_label->y());
	
	if (!QString(info_label->text()).isEmpty())
	{
		info_label->raise();
		info_label->show();
		tipSleepTimer->start(4000, true);
	}
	else
		info_label->hide();
}

void ToolTip::tipSleepTimerDone()
{
	hide();
}


