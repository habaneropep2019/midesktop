/*
  events.C  - Part of the KDE project.

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

#include "mipanel.h"
#include <qapp.h>
#include <qmsgbox.h>
#include <stdio.h>
#include <unistd.h>
#include <X11/Xatom.h>
#include <qkeycode.h>
#include <kfm.h>
#include <ksimpleconfig.h>
#include <qregexp.h>
#include <qtimer.h>
#include <qlayout.h>
#include <kprocess.h>
#include <ktoolboxmgr.h>

#include <sys/stat.h>

extern kPanel *the_panel;

bool in_animation = false;
void animateMove(QWidget*w, int xn, int yn, int step )
{
    in_animation = true;
    int xi = w->x();
    int yi = w->y();
    for (; QABS(xi-xn)>QABS(step); xi+=step){
	qApp->syncX();
	qApp->processEvents();
	w->move(xi,yi);
    }
    for (; QABS(yi-yn)>QABS(step); yi+=step){
	qApp->syncX();
	qApp->processEvents();
	w->move(xi,yi);
    }
    w->move(xn, yn);
    in_animation = false;
}

myFrame::myFrame(bool _autoHide, unsigned int delay,
		 QWidget *parent, const char* name, WFlags f)
  :QFrame(parent, name, f){

  hideTimer = new QTimer(this);
  connect( hideTimer, SIGNAL(timeout()),
	   this, SLOT(hideTimerDone()) );
  autoHide = _autoHide;
  autoHidden = false;
  hide_delay = delay;//CT
  if (autoHide)
    hideTimer->start(delay, true);
}

void myFrame::enterEvent(QEvent *)
{
  hideTimer->start(hide_delay, true);
  if (!autoHidden)
    return;
  autoHidden = false;
  emit showMe();
}

void myFrame::hideTimerDone(){
  if (!autoHide)
    return;
  bool do_hide = true;
  // check for popups
  if (!QApplication::activePopupWidget() &&
      !QApplication::widgetAt(QCursor::pos()) &&
      XGrabPointer(qt_xdisplay(), qt_xrootwin(), False,
		   ButtonPressMask | ButtonReleaseMask |
		   PointerMotionMask |
		   EnterWindowMask | LeaveWindowMask,
		   GrabModeAsync, GrabModeAsync, None,
		   None , CurrentTime) == GrabSuccess){
    XUngrabPointer(qt_xdisplay(), CurrentTime);
    XSync(qt_xdisplay(), false);
  }
  else
    do_hide = false;
  do_hide = do_hide && !in_animation;
  if (!do_hide)
    hideTimer->start(hide_delay, true);
  else {
    autoHidden = true;
    emit hideMe();
  }
}


void kPanel::showTaskbar()
{
	if (in_animation)
		return;
	doGeometry(TRUE);
	animateMove(taskbar_frame,
							taskbar_frame_geometry.x(),
							taskbar_frame_geometry.y(),
							taskbar_position == bottom?-autoHideTaskbarSpeed*2:
																				autoHideTaskbarSpeed*2);
	doGeometry();
}

void kPanel::hideTaskbar()
{
  if (in_animation)
      return;
  doGeometry(TRUE);
  animateMove(taskbar_frame,
	      taskbar_frame_geometry.x(),
	      taskbar_frame_geometry.y(),
	      taskbar_position == bottom?autoHideTaskbarSpeed:
	                          -autoHideTaskbarSpeed);
  doGeometry();
}


void kPanel::kwmInit()
{
	if (!taskbar->isEmpty())
	{
		taskbar->removeAll();

		layoutTaskbar();
		doGeometry();
	}
	
	// tell kwm to keep the panel and taskbar raised. This will move into libkdecore after KDE-1.1
	{
		XEvent ev;
		long mask;
		
		memset(&ev, 0, sizeof(ev));
		ev.xclient.type = ClientMessage;
		ev.xclient.window = qt_xrootwin();
		ev.xclient.message_type = XInternAtom(qt_xdisplay(), "KWM_KEEP_ON_TOP", False);
		ev.xclient.format = 32;
		mask = SubstructureRedirectMask;
		ev.xclient.data.l[1] = CurrentTime;
		if (taskbar_frame) // first a taskbar
		{
			ev.xclient.data.l[0] = (long)taskbar_frame->winId();
			XSendEvent(qt_xdisplay(), qt_xrootwin(), False, mask, &ev);
		}
		ev.xclient.data.l[0] = (long)winId();
		XSendEvent(qt_xdisplay(), qt_xrootwin(), False, mask, &ev);

	}
}


static Window ignore_this_dialog = None;

void kPanel::dialogWindowAdd(Window w)
{
	ignore_this_dialog = w;
}

void kPanel::windowAdd(Window w)
{
	if (w == ignore_this_dialog ) 
	{
		ignore_this_dialog = None;
		return;
	}
	else 
	{
		//compatibility: ignore transient windows
		Window trans = None;
		if (XGetTransientForHint(qt_xdisplay(), w, &trans))
		{
			if (trans != None && trans != qt_xrootwin())
				return;
		}
	}
	
	myTaskButton* b = taskbar->createTaskButton();
	
	b->win = w;
	QPixmap pm = KWM::miniIcon(w, 16, 16);
	if (!pm.isNull())
		b->setPixmap(pm);
	else
		b->setPixmap(defaultPixmap);
	
	QString t = KWM::titleWithState(w);
	b->setText(t);
	
	b->virtual_desktop = KWM::desktop(w);
	b->m_bSticky = KWM::isSticky(w);

	taskbar->insert(b);
	connect(b, SIGNAL(clicked(myTaskButton*)), this, SLOT(taskbarClicked(myTaskButton*)));
	connect(b, SIGNAL(pressed(myTaskButton*)), this, SLOT(taskbarPressed(myTaskButton*)));
	
	// swallowing?
	DesktopEntry *pE;
	for (pE = entries.first(); pE; pE = entries.next())
	{
		if (!pE->swallow.isEmpty()  && !pE->swallowed)
		{
			QRegExp r;
			r = pE->swallow;
			if (r.match(t) != -1)
			{
				break;
			}
		}
	}
	
	if (pE)
	{
//printf("swallow %s\n", (const char *)t);
		DesktopEntry *pFound;
		for (pFound = entries.first(); pFound; pFound = entries.next())
		{
			if (pFound->swallowed == w)
				break;
		}

		if (!pFound)
		{
			pE->button->setText("");
			pE->swallowed = w;
			KWM::prepareForSwallowing(w);
			XSetWindowBackground(qt_xdisplay(),
				 w,
				 pE->button->backgroundColor().pixel());
			XReparentWindow(qt_xdisplay(), w,
					pE->button->winId(), 3, 3);
			XSelectInput(qt_xdisplay(), w, EnterWindowMask | LeaveWindowMask);
			XResizeWindow(qt_xdisplay(), w,
				pE->button->width()-6,
				pE->button->height()-6);
			XMapWindow(qt_xdisplay(),w);
	
	
			// install a passive grab on this button to ensure that
			// kpanel recieves the button events.
	
			// Exception: do not install a passive grab for the left mouse
			// button if there is no Exec property in the kdelnk file.
			KSimpleConfig pConfig(pE->pmi->getFullPathName(),true);
			pConfig.setGroup("KDE Desktop Entry");
			QString aString = pConfig.readEntry("Exec", "");
			if (aString.isEmpty())
			{
				printf("passive grab ohne LMB!\n");
				XGrabButton(qt_xdisplay(),
							Button2,
							AnyModifier, w, True,
							ButtonPressMask,
							GrabModeSync, GrabModeAsync, None, None);
				XGrabButton(qt_xdisplay(),
							Button3,
							AnyModifier, w, True,
							ButtonPressMask,
							GrabModeSync, GrabModeAsync, None, None);
			}
			else
				XGrabButton(qt_xdisplay(), AnyButton, AnyModifier, w, True,
							ButtonPressMask,
							GrabModeSync, GrabModeAsync, None, None);
	
			pE->button->swallowed_window = w;
		}
	}
	
	if (taskbar_frame)
	{
		int nHg = taskbar_frame->height();
		layoutTaskbar(true);
		if(nHg != taskbar_frame->height())
		{
			doGeometry();
			KWM::sendKWMCommand("deskFitToWindow");
		}
	}
	layoutTaskbar();
}

void kPanel::windowRemove(Window w)
{
  myTaskButton* b = taskButtonFromWindow(w);
  if (!b)
    return;
  taskbar->remove(b);
  delete b;

	if (taskbar_frame)
	{
		int nHg = taskbar_frame->height();
		layoutTaskbar(true);
		if (nHg != taskbar_frame->height())
		{
			doGeometry();
			KWM::sendKWMCommand("deskFitToWindow");
		}
	}
  layoutTaskbar();
  // PAL - Nov98: Hack to avoid kcmkpanel zombie processes.
  waitpid(-1, 0, WNOHANG);
}

void kPanel::windowChange(Window w)
{
  myTaskButton* b = taskButtonFromWindow(w);
  if (!b)
    return;
  b->setText(KWM::titleWithState(w));
  int d = KWM::desktop(w);
	bool bSticky = KWM::isSticky(w);
	
	if (b->m_bSticky != bSticky)
	{
		if (b->m_bSticky)
			b->virtual_desktop = d; // no need to update anything
		b->m_bSticky = bSticky;
	}

	if (d != b->virtual_desktop)
	{
		taskbar->taskChangedDesktop(b, d);
    layoutTaskbar();
  }
}

void kPanel::windowActivate(Window w)
{
  myTaskButton* b = taskButtonFromWindow(w);
  if (!b) {
      myTaskButton::setNoActive();
    return;
  }
  b->setActive();
}

void kPanel::windowIconChanged(Window w)
{
  myTaskButton* b = taskButtonFromWindow(w);
  if (!b)
    return;
  QPixmap pm = KWM::miniIcon(w, 16, 16);
  if (!pm.isNull())
    b->setPixmap(pm);
}

void kPanel::windowRaise(Window /* w */)
{
  if (panel_button_frame_standalone->isVisible())
    panel_button_frame_standalone->raise();
  if (panel_button_frame_standalone2 && panel_button_frame_standalone2->isVisible())
    panel_button_frame_standalone2->raise();
}


void kPanel::layoutDockArea()
{
	int i = 0;
	int nCnt = kwmmapp->dock_windows.count();
	int nSz = (nCnt ? nCnt * 24 + 3 : 1);

	if (orientation == Qt::Vertical)
	{
		if (nSz != dock_area->height());
		{
			dock_area->setFixedHeight(nSz);
			m_pLayout->activate();
		}
		for (Window *w = kwmmapp->dock_windows.first(); w; w = kwmmapp->dock_windows.next())
		{
			XMoveResizeWindow(qt_xdisplay(), *w, dock_area->width() / 2 - 12, 3 + i * 24, 24, 24);
			i++;
		}
	} 
	else // horizontal
	{
		if (nSz != dock_area->width())
		{
			dock_area->setFixedWidth(nSz);
			m_pLayout->activate();
		}

		int nY = dock_area->height() / 2 - 12;
		for (Window *w = kwmmapp->dock_windows.first(); w; w = kwmmapp->dock_windows.next())
		{
			XMoveResizeWindow(qt_xdisplay(), *w, 3 + i * 24, nY, 24, 24);
			i++;
		}
	}
}

void kPanel::dockWindowAdd(Window w)
{
	XReparentWindow(qt_xdisplay(), w, dock_area->winId(), 0, 0);
  XMapWindow(qt_xdisplay(), w);
  layoutDockArea();
}

void kPanel::dockWindowRemove(Window)
{
  layoutDockArea();
}

// void kPanel::playSound(QString e){
//   QDateTime d = QDateTime::currentDateTime();
//   printf("sound event: %s (%.2d:%.2d:%.2d)\n",
// 	 e.data(),
// 	 d.time().hour(),
// 	 d.time().minute(),
// 	 d.time().second()
// 	 );
// }

myTaskButton* kPanel::taskButtonFromWindow(Window w)
{
  for (int i = 0; i < taskbar->getNumEntries(); i++)
	{ 
		myTaskButton* b = taskbar->getEntryAt(i);
    if (b->win == w)
      return b;
  }
  return 0;
}

void kPanel::kwmDesktopChange(int nd)
{
  if ( edit_button != 0)
    restore_editbutton( False );

  currentDesktop = nd;
  QButton* b;
  for (int i=0; (b=(QPushButton*)desktopbar->find(i))!=0; i++)
	{
    if ((i+1==currentDesktop && !b->isOn())
				|| (i+1!=currentDesktop && b->isOn()))
			b->toggle();
  }

  taskbar->changedCurrentDesktop(currentDesktop);

	if (panelHidden[currentDesktop])
	{
      if (panelHiddenLeft[currentDesktop])
	  hidePanelLeft();
      else
	  hidePanelRight();
  }
  else
    showPanel();
}

void kPanel::kwmDesktopNameChange(int d, QString name)
{
  QButton* b = (QButton*)desktopbar->find(d-1);
  if (b)
	{
    set_button_text(b, name);
  }
}

void kPanel::kwmDesktopNumberChange(int n)
{
  if (n != number_of_desktops)
    restart();
}

void kPanel::kwmCommandReceived(QString com)
{
  if (com == "kpanel:restart")
	{
    restart();
  }

  if (com == "kpanel:hide")
    hidePanelLeft ();
  if (com == "kpanel:show")
    showPanelFromLeft ();
  if (com == "kpanel:system") {
      if (!in_animation)
	  showSystem ();
  }

  if (com.left(11) == "kpanel:icon")
	{
    if (com.mid(12, 1) != ":")
      return;
    int in = 0;
    if (com.mid(11,1) == "2") in =  1;
    if (com.mid(11,1) == "3") in =  2;
    if (com.mid(11,1) == "4") in =  3;
    // search for the identity
    QString identity = com.right(com.length() - 13);
    
		DesktopEntry *pE;
		for (pE = entries.first(); pE; pE = entries.next())
		{
      if (pE->identity == identity)
			{
				if (pE->icon[in])
					pE->button->setPixmap(*(pE->icon[in]));
      }
    }
  }

  // Command from krootwm: open GO-menu, Syntax: "kpanel:goxxxxyyyy"
  if (com.length()== 17 && com.left(9) == "kpanel:go")
	{
    QPoint pos(com.mid( 9, 4).toInt(), com.mid(13, 4).toInt());
		QPopupMenu *kmenu = pmenu->getQPopupMenu();
    if (kmenu)
		{
      kmenu->popup(pos);
      // dirty hack to work around a possible qt bug (Matthias, qt-1.33)
      XGrabPointer( qt_xdisplay(), kmenu->winId(), TRUE,
		    ButtonPressMask | ButtonReleaseMask |
		    PointerMotionMask | EnterWindowMask | LeaveWindowMask,
		    GrabModeAsync, GrabModeAsync,
		    None, None, CurrentTime );
    }

  }

  // PAL - Oct98
  // Display the busy cursor if an application is starting
  if(com == "startingApp")
    QApplication::setOverrideCursor(waitCursor);
  else if(com == "appStarted")
    QApplication::restoreOverrideCursor();
}

bool kPanel::eventFilter(QObject *ob, QEvent *ev)
{
	// --sven: kdisknav button start --
	if (ob == m_pDiskNavMenu && ev->type() == QEvent::MouseButtonRelease)
	{
		bool doIt = false;
	
		if (miniPanelHidden)
		{
			for (DesktopEntry *pE = entries.first(); pE; pE = entries.next())
			{
				// is this our button?
				if (pE->popup == m_pDiskNavMenu &&
						QApplication::widgetAt(((QMouseEvent *) ev)->globalPos(), true)
								== pE->button)
				{
					doIt = true;
					break;
				}
			}
		}
		else if (miniDiskNav != 0L && 
						QApplication::widgetAt(((QMouseEvent *) ev)->globalPos(), true) == miniDiskNav)
			doIt = true;
	
		if (doIt) 
		{
			m_pDiskNavMenu->setActiveItem(0); // set first active
			return true;  // ignore release
		}
	}
	// --sven: kdisknav button end --
	
	if (m_bPrepareMovePanel)
	{
		m_bPrepareMovePanel = false;
		if (ev->type() == QEvent::MouseMove)
		{
			doPanelReposition();
		}
	}

	// right button click in the taskbar initialises a popup menu
	if (ob == taskbar->getButtonsArea()->getButtonsArea())
	{
		if (ev->type() == QEvent::MouseButtonPress)
		{
			mousePressEvent((QWidget *)ob, (QMouseEvent*)ev);
		}
		return false;
	}

	// KeyPress is also an X11 macro. Undef it just for this section.

#ifdef KeyPress
#undef KeyPress
#endif

	switch (ev->type())
	{
		case QEvent::KeyPress:
			if (edit_button != 0)
			{ 
				if(((QKeyEvent*)ev)->key() == Key_Escape) 
				{
					restore_editbutton( false );
					return true;
				}
				else if ((((QKeyEvent*)ev)->key() == Key_Return ||
									((QKeyEvent*)ev)->key() == Key_Enter)) 
				{
					restore_editbutton( true );
					return true;
				}
			};
			break;
	
		case QEvent::MouseButtonPress:
		case QEvent::MouseButtonDblClick:
		{
			if (moving_button)
			{
				stopButtonMove();
				writeOutConfiguration();
			}

			if (ob == panel_button || ob->inherits("myTaskButton"))
				break;
			
			QMouseEvent* mev = (QMouseEvent*)ev;
			if (ob == edit_button)
			{
				if (!edit_button->rect().contains(mev->pos()))
				{
					restore_editbutton(false);
					return true;
				}
				break;
			}
	
			// PAL - Sept98: To support right-click on the buttons_area
			if(ob == m_pButtonsArea->getButtonsArea())
			{
				mousePressEvent((QWidget *)ob, mev);
				break;
			}
	
			if (mev->button() == MidButton)
			{
				if (ob->isWidgetType())
				{
					// minipanel cannot be edited!
					if (((QWidget*)ob)->parentWidget() != miniPanel)
					{
						startButtonMove((QWidget*)ob, mev);
					}
				}
			}
			break;
		}
	
		case QEvent::MouseButtonRelease:
		{
			if (moving_button)
			{
				stopButtonMove();
				writeOutConfiguration();

			}
		}
		break;
		
		case QEvent::MouseMove:
		{
			if (moving_button)
			{
				doButtonMove();
			}
		}
		break;
	};
	return false;
}

void kPanel::startButtonMove(QWidget *pWidget2Move, QMouseEvent *evt/*= 0*/)
{
	moving_button = pWidget2Move;
	moving_button_oldpos = moving_button->pos();

	if (evt)
	{
		moving_button_offset = evt->pos();
	}
	else
	{
		moving_button_offset = moving_button->mapFromGlobal(QCursor::pos());	
//		moving_button_offset = QPoint(moving_button->width()/2, moving_button->height()/2);
	}

	((myPushButton*)moving_button)->flat = False;
	moving_button->repaint();
	moving_button->raise();
	moving_button->setCursor(sizeAllCursor);

	if (!evt)
	{
		XGrabPointer(qt_xdisplay(), moving_button->winId(), false,
									ButtonPressMask | ButtonReleaseMask |
									PointerMotionMask | EnterWindowMask | LeaveWindowMask,
									GrabModeAsync, GrabModeAsync,
									None, None, CurrentTime );
		// moving_button->grabMouse();
	}
}

void kPanel::stopButtonMove(bool bCancel/*= false*/)
{
	if (moving_button)
	{
		((myPushButton*)moving_button)->flat = True;
		moving_button->repaint();
		moving_button->setCursor(arrowCursor);
		XUngrabPointer( qt_xdisplay(),0 );
		// moving_button->releaseMouse();
	
		m_pButtonsArea->moveEntry(moving_button, moving_button_oldpos);
	
		// resetting moving variables
		moving_button = 0;
	}
}

void kPanel::doButtonMove()
{
	if (!moving_button)
		return;

	int x, y;
	if (orientation == Qt::Horizontal)
	{
		x = m_pButtonsArea->getButtonsArea()->mapFromGlobal(QCursor::pos()).x()
					- moving_button_offset.x();
		y = moving_button->y(); // no vertical movement allowed
	}
	else
	{
		x = moving_button->x(); // no horizontal movement allowed
		y = m_pButtonsArea->getButtonsArea()->mapFromGlobal(QCursor::pos()).y()
					- moving_button_offset.y();
	}

#ifdef MMM
	if (orientation == Qt::Horizontal)
	{
		if (x < 0) 
			x = 0;
		else if(x < m_pButtonsArea->contentsX())
			scrollTopLeft(5);
		if (x > m_pButtonsArea->getButtonsArea()->width()) 
			x = m_pButtonsArea->getButtonsArea()->width();
		else if(x + moving_button->width() > 
						m_pButtonsArea->contentsX() + buttons_scroll_area->width())
			scrollBottomRight(5);
		y = moving_button->y();
	}
	else // vertical
	{
		if (y < 0) 
			y=0;
		else if(y < m_pButtonsArea->contentsY())
			scrollTopLeft(5);
		if (y > m_pButtonsArea->getButtonsArea()->height()) 
			y = m_pButtonsArea->getButtonsArea()->height();
		else if(y + moving_button->height() > 
						m_pButtonsArea->contentsY() + buttons_scroll_area->height())
			scrollBottomRight(5);
		x = moving_button->x();
	}
#endif
	moving_button->move(x, y);
}

void kPanel::resizeEvent( QResizeEvent * )
{
  //  setGeometry(0,0,QApplication::desktop()->width(), menu->height());

}

void kPanel::enterEvent( QEvent *)
{
  if (in_animation)
      return;
  hideTimer->start(autoHideDelay, true);

  if (taskbar_frame &&
      ((orientation == Qt::Horizontal && position == top_left &&
      taskbar_frame->autoHidden && taskbar_position == top)
      ||
      (orientation == Qt::Horizontal && position == bottom_right &&
      taskbar_frame->autoHidden && taskbar_position == bottom)))
	{
    QEvent ev(QEvent::Enter);
    QApplication::sendEvent(taskbar_frame, &ev);
  }

  if (!autoHidden)
    return;
  if (orientation == Qt::Horizontal){
    if (position == top_left)
      animateMove (this, 0,0,4*autoHideSpeed);
    else
      animateMove(this, 0, QApplication::desktop()->height()-height(), -4*autoHideSpeed);
  }
  else {
    if (position == top_left)
      animateMove (this, 0,0, 4*autoHideSpeed);
    else
      animateMove (this, QApplication::desktop()->width()-width(),0, -4*autoHideSpeed);

  }
  autoHidden = false;
  doGeometry();
  layoutTaskbar();
}

void kPanel::leaveEvent( QEvent * )
{
}


void kPanel::hideTimerDone()
{
  bool do_hide = true;
  if (!autoHide)
    return;


  // check for popups
  if (!QApplication::activePopupWidget() &&
      !QApplication::widgetAt(QCursor::pos()) &&
      XGrabPointer(qt_xdisplay(), qt_xrootwin(), False,
		   ButtonPressMask | ButtonReleaseMask |
		   PointerMotionMask |
		   EnterWindowMask | LeaveWindowMask,
		   GrabModeAsync, GrabModeAsync, None,
		   None , CurrentTime) == GrabSuccess)
		{
    XUngrabPointer(qt_xdisplay(), CurrentTime);
    XSync(qt_xdisplay(), false);
  }
  else
    do_hide = false;

  do_hide = do_hide && !in_animation;

	for (DesktopEntry *pE = entries.first(); pE; pE = entries.next())
	{
    do_hide = do_hide && (pE->button->flat && !pE->button->isDown());
  }

  do_hide = do_hide && !geometry().contains(QCursor::pos());
  if (!do_hide){
    hideTimer->start(autoHideDelay, true);
  }
  else {
    if (orientation == Qt::Horizontal){
	if (position == top_left)
	    animateMove (this, x(), y()-height()+4, -autoHideSpeed);
	else
	    animateMove (this, x(), y()+height()-4, autoHideSpeed);
    }
    else {
	if (position == top_left)
	    animateMove (this, x()-width()+4, y(), -autoHideSpeed);
	else
	    animateMove (this, x()+width()-4, y(), autoHideSpeed);
	
    }
    autoHidden = true;
    doGeometry();
    layoutTaskbar();
  }
}

void kPanel::standalonePanelButtonClicked()
{
  enterEvent(0);
  showPanelFromLeft();
}

void kPanel::standalonePanelButton2Clicked()
{
  enterEvent(0);
  showPanelFromRight();
}

void kPanel::mousePressEvent(QWidget *pWidget, QMouseEvent*  ev  )
{
  if (ev->button() == RightButton)
	{
    QPopupMenu* p = new QPopupMenu();
    if (pWidget == taskbar->getButtonsArea()->getButtonsArea())
		{
			QPopupMenu *menu = new QPopupMenu();
			menu->setCheckable(true);
			menu->insertItem(i18n("In Current Desktop"), taskbar, SLOT(taskbarModeDesktop()), 3);
			menu->setId(0, 1);
			menu->setItemChecked(menu->idAt(0), taskbar->isShowDesktopSpecificTasks());
			menu->insertItem(i18n("All Tasks"), taskbar, SLOT(taskbarModeAll()), 4);
			menu->setId(1, 2);
			menu->setItemChecked(menu->idAt(1), !taskbar->isShowDesktopSpecificTasks());
			p->insertItem(i18n("Show Tasks"), menu);
			p->insertSeparator();
		}

    p->insertItem(i18n("Configure"), this, SLOT(configurePanel()));
    p->insertItem(i18n("Restart"), this, SLOT(restart()));
		

    p->popup(pWidget->mapToGlobal(ev->pos()));  // PAL - Sept98
  }
	else if(ev->button() == LeftButton)
	{
		if (m_pMoveMgr || !m_bTaskbarDocked)
			return;
		
		m_bPrepareMovePanel = true;
	}
}

void kPanel::doPanelReposition()
{
	m_pMoveMgr = new KToolBoxManager(this, true);

	int w = QApplication::desktop()->width();
	int h = QApplication::desktop()->height();
	int nWd = box_width + frameWidth()*2;
	int nHg = box_height + frameWidth()*2;

	m_pMoveMgr->addHotSpot( 0, 0, w, nHg); // top
	m_pMoveMgr->addHotSpot( 0, h - nHg, w, nHg); // bottom
	m_pMoveMgr->addHotSpot( 0, 0, nWd, h); // left
	m_pMoveMgr->addHotSpot( w - nWd, 0, nWd, h); // right

	connect (m_pMoveMgr, SIGNAL(onHotSpot(int)), SLOT(slotHotSpot(int)));
	m_pMoveMgr->doMove(true, false, true);

	delete m_pMoveMgr;
	m_pMoveMgr = 0;

	if (m_nMovePos != -1)
	{
		int nCurPos = -1;
		if (orientation == Qt::Horizontal)
		{
			nCurPos = (position == top_left ? 0 : 1);
		}
		else if (orientation == Qt::Vertical)
		{
			nCurPos = (position == top_left ? 2 : 3);
		}

		if (nCurPos != m_nMovePos)
		{
			KConfig *config = KApplication::getKApplication()->getConfig();
			config->setGroup("kpanel");
			QString strPos;
			
			switch (m_nMovePos)
			{
				case 0:
					strPos = "top";
					break;

				case 1:
				default:
					strPos = "bottom";
					break;

				case 2:
					strPos = "left";
					break;

				case 3:
					strPos = "right";
					break;
			}
			config->writeEntry("Position", (const char *)strPos);
			config->sync();

			restart();
		}
	}
}


void kPanel::slotHotSpot (int hs)
{
	if (m_pMoveMgr == 0)
		return;
	
	switch (hs)
	{
		case 0: //top
			m_pMoveMgr->setGeometry(0);
			m_nMovePos = 0;
			break;
		
		case 1: //bottom
			m_pMoveMgr->setGeometry(1);
			m_nMovePos = 1;
			break;
		
		case 2: //left
			m_pMoveMgr->setGeometry(2);
			m_nMovePos = 2;
			break;
		
		case 3: //right
			m_pMoveMgr->setGeometry(3);
			m_nMovePos = 3;
			break;
		
	case -1: // left all
			m_pMoveMgr->setGeometry(m_pMoveMgr->mouseX(), m_pMoveMgr->mouseY(), width(), height());
			m_nMovePos = -1;
			break;
	}
}

void kPanel::slotDropEvent( KDNDDropZone *_zone )
{
	if (_zone == drop_zone)
	{
//		QString strURL = _zone->getData();
		// PAL - Oct98 (2): http or ftp kdelnk, in the form http:blabla
		// or ftp:blabla didn't worked because the ":" is replaced by
		// "%3A".  We must change it back so that the .kdelnd file can
		// be opened.
//		a.replace(QRegExp("%3A"), ":");
		KURL u ( _zone->getData() );
		// TODO kde link auf den button legen!!!
		// Which means, in English ?
		if (u.isLocalFile()) 
		{
			QString a = u.path(); // decodes the URL if necessary
			if (a.right(1) == "/")
				a.truncate(a.length()-1);
		
			QPoint p = mapFromGlobal(QPoint(drop_zone->getMouseX(), drop_zone->getMouseY()));
			if (kde_button->geometry().contains(p))
			{
				// file was dropped onto the K-Button
				if (addToMenuFromDrop(a))
				{
					return;
				}
			}
			else if (m_pButtonsArea->geometry().contains(p))
			{
				PMenuItem* pmi = pmenu->searchItem(a);
				if (pmi)
				{
					int x = margin;
					int y = margin;
					if (orientation == Qt::Vertical)
					{
						y = p.y() - m_pButtonsArea->y() - box_height/2;
						if (y<margin) y=margin;
					}
					else
					{
						x = p.x() - m_pButtonsArea->x() - box_width/2;
						if (x<margin) x=margin;
					}
					addButtonInternal(pmi, x, y);
					writeOutConfiguration();
					return;
				}
			}
		}
	
		QMessageBox::warning( 0, "KPanel",
			i18n("Cannot put this object as a button onto the panel!"),
			i18n("Oops!"));
		return;
	}
	
	for (DesktopEntry *pE = entries.first(); pE; pE = entries.next())
	{
		if (pE->drop_zone == _zone && pE->pmi)
		{
			KFM* kfm = new KFM;
			QString com = pE->pmi->getFullPathName();
			com.prepend("file:");
			kfm->exec(com.data(),_zone->getData());
			delete kfm;
			return;
		}
	}
}

bool kPanel::addToMenuFromDrop(QString & strPath)
{
	// file was dropped onto the K-Button
	return pmenu->addPersonalFromDrop(strPath);
}

void kPanel::kdisplayPaletteChanged()
{
  doGeometry();
  layoutTaskbar();

  QButton* tmp_button;
  for (int i=0; (tmp_button = desktopbar->find(i)); i++)
	{
    set_button_text(tmp_button, KWM::getDesktopName(i+1));
  }
}

QWidget* kPanel::parentOfSwallowed(Window w)
{
	for (DesktopEntry *pE = entries.first(); pE; pE = entries.next())
	{
    if(pE->swallowed == w)
		{
			return pE->button;
		}
	}
  return 0;
}

//
// KMenuEventFilter class implementation
//
KMenuEventFilter::KMenuEventFilter(ToolTip *pToolTip)
	: m_pToolTip(pToolTip)
{
	m_pCurrentPopup = m_pEditPopup = m_pNewPopup = 0;
	m_bIsCopied = m_bIsCut = false;
	m_pSavMenu = 0;
	m_nSavId = -1;
}

enum EditMenuCmds { cmdOpen = 10, cmdCut, cmdCopy, cmdPaste, cmdNewFolder, cmdNewItem, cmdDelete, cmdProperties, cmdNew};

bool KMenuEventFilter::eventFilter(QObject *ob, QEvent *ev)
{
//printf("%d\n", ev->type());	
	// when edit popup visible filter out some events
	if (m_pCurrentPopup)
	{
		if (m_pCurrentPopup->isVisible())
		{
			switch(ev->type())
			{
				case QEvent::KeyPress:
printf("%d\n", ((QKeyEvent*)ev)->key());
break;

				case QEvent::MouseButtonPress:
					m_pCurrentPopup->hide();
					// need this in order to fix tip problem when selection remains
					// but actual click going into parent item
					m_pToolTip->showToolTip("");
					return true;
				case QEvent::MouseMove:
				case QEvent::MouseButtonRelease:
				case QEvent::MouseButtonDblClick:
					return true;
			}
		}
		else
		{
			// filter out first relese event after edit menu dissapeared
			if (ev->type() == QEvent::MouseButtonRelease)
			{
				return true;
			}
			m_pCurrentPopup = 0;
		}
	}

	switch (ev->type())
	{
		case QEvent::KeyPress:
			if (m_pToolTip->isVisible())
				m_pToolTip->hide();
			break;
	
		case QEvent::MouseButtonPress:
			{
				// hide info_tip at first
				QMouseEvent* mev = (QMouseEvent*)ev;
				if (mev->button() == RightButton)
				{
					if (!m_pToolTip->isVisible())
						m_pToolTip->show();
				}
			}
			break;
	
		case QEvent::MouseButtonRelease:
		{
			if (m_pToolTip->isVisible())
				m_pToolTip->hide();
		
			QMouseEvent* mev = (QMouseEvent*)ev;
			if (mev->button() == RightButton)
			{
				// show popup "edit" menu
				myPopupMenu *menu = (myPopupMenu *)ob;
				QPoint point(mev->pos());
				if (menu->isVisible() && menu->contentsRect().contains(point, true) 
						&& menu->getParentMenu() && menu->count() > 0)
				{
					int nId = menu->idAt(mev->pos());

					PMenuItem * item;
					if (nId < 0 || !(item = menu->getParentMenu()->searchItem(nId)))
					{
						return true;
					}

					if (item->getEditPopupMenuStatus() & EPM_DONT_SHOW)
					{
						return true;
					}

					m_nCurrentId = nId;
					m_pCurrentMenu = menu->getParentMenu();

					if (item->getType() >= submenu)
					{
						bool bIsDir = (item->getType() == submenu);

						// create edit popup
						if (!m_pEditPopup)
						{
							m_pEditPopup = new QPopupMenu();
							connect(m_pEditPopup, SIGNAL(activated(int)), this, SLOT(activated(int)));
						}
						else
							m_pEditPopup->clear();

						if (bIsDir)
						{
							// create new menu for dirs
							if (!m_pNewPopup)
							{
								m_pNewPopup = new QPopupMenu();
								m_pNewPopup->insertItem(i18n("&Folder"), cmdNewFolder); 
								m_pNewPopup->insertItem(i18n("&Nickname"), cmdNewItem); 
								connect(m_pNewPopup, SIGNAL(activated(int)), this, SLOT(activated(int)));							
							}
							m_pEditPopup->insertItem(i18n("&New"), m_pNewPopup); 
						}
						else
						{
							m_pEditPopup->insertItem(i18n("&Open"), cmdOpen); 
						}
						m_pEditPopup->insertSeparator();
						m_pEditPopup->insertItem(i18n("Cu&t"), cmdCut); 
						m_pEditPopup->insertItem(i18n("&Copy"), cmdCopy);
						if (bIsDir)
							m_pEditPopup->insertItem(i18n("&Paste"), cmdPaste); 
						m_pEditPopup->insertSeparator();
						m_pEditPopup->insertItem(i18n("&Move to Trash"), cmdDelete); 
						m_pEditPopup->insertSeparator();
						m_pEditPopup->insertItem(i18n("P&roperties..."), cmdProperties); 
	
						if (bIsDir)
						{
							m_pEditPopup->setItemEnabled(cmdOpen, false);
							m_pEditPopup->setItemEnabled(cmdCopy, false);
							m_pEditPopup->setItemEnabled(cmdCut, false);
						}

						// can delete?
						bool bCanDelete  = !(item->getEditPopupMenuStatus() & EPM_CANT_DELETE);
						if (bCanDelete)
						{
							QString strName(item->getFullPathName());
							if (!strName.isEmpty())
							{
								QFileInfo fi(strName);
								bCanDelete = fi.exists() && fi.isWritable();
							}
						}
						if (!bCanDelete)
						{
							m_pEditPopup->setItemEnabled(cmdDelete, false);
							m_pEditPopup->setItemEnabled(cmdCut, false);
						}

						if (!m_bIsCopied && !m_bIsCut)
							m_pEditPopup->setItemEnabled(cmdPaste, false);
	
						m_pCurrentPopup = m_pEditPopup;
						m_pEditPopup->popup(menu->mapToGlobal(mev->pos()));
					}
				}
				return true;
			}
		}
		break;

		case QEvent::Leave:
			m_pToolTip->showToolTip("");
			break;
	}
	return false;
}

//
// helper class derived from KProcess
//
class KNewItemProcess : public KProcess
{
public:
	KNewItemProcess(PMenu *pParent) { m_pParent = pParent; };
	KNewItemProcess(const char * strPath) { m_strPath = strPath; };
	PMenu *getParentMenu() { return m_pParent; };
	const char *getSavedPath() { return m_strPath; };

private:
	PMenu *m_pParent;
	QString m_strPath;
};

//
// menu item selected - do action!
//
void KMenuEventFilter::activated(int nId)
{
	switch (nId)
	{
	case cmdOpen: // open
		{
			// at first hide kmenu
			the_panel->getKMenu()->getQPopupMenu()->hide();

			PMenuItem * item = m_pCurrentMenu->searchItem(m_nCurrentId);
			if (item)
				item->exec();
		}
		break;

	case cmdCut: // cut
		m_bIsCopied = false;
		m_bIsCut = true;
		m_pSavMenu = m_pCurrentMenu;
		m_nSavId  = m_nCurrentId;
		break;

	case cmdCopy: // copy
		m_bIsCopied = true;
		m_bIsCut = false;
		m_pSavMenu = m_pCurrentMenu;
		m_nSavId  = m_nCurrentId;
		break;

	case cmdPaste: // paste
		if (m_pSavMenu && m_nSavId >= 0)
		{
			// insert into current item (it should be a directory)
			PMenuItem * item = m_pCurrentMenu->searchItem(m_nCurrentId);
			if (item->getMenu())
				item->getMenu()->insertItem(m_pSavMenu, m_nSavId, m_bIsCut);
			if (m_bIsCut)
			{
				// can't paste any more
				m_pSavMenu = 0;
				m_nSavId  = -1;
				m_bIsCut = false;
			}
		}
		break;

	case cmdDelete: // delete
		{
			if (m_pCurrentMenu && m_nCurrentId >= 0)
			{
				m_pCurrentMenu->deleteItem(m_nCurrentId);	
				m_pCurrentMenu = 0;
				m_nCurrentId = -1;
			}
		}
		break;

	case cmdProperties:	// properties
		{
			// hide menu at first
			the_panel->getKMenu()->getQPopupMenu()->hide();
	
			PMenuItem * item = m_pCurrentMenu->searchItem(m_nCurrentId);
			if (item)
			{
				QString strName(item->getFullPathName());
				if (!strName.isEmpty())
				{
					KFM* kfm = new KFM;
					kfm->openProperties(strName.latin1());
					delete kfm;
				}
			}
		}
		break;

	case cmdNewFolder: // new folder
		{
			PMenuItem * item = m_pCurrentMenu->searchItem(m_nCurrentId);
			if (item->getMenu())
			{
				item->getMenu()->createNewFolder();
			}
		}
		break;

	case cmdNewItem: // new item
		{
			PMenuItem * item = m_pCurrentMenu->searchItem(m_nCurrentId);
			if (item->getMenu())
			{
				// can add item to this directory?
				QString strPath;
				if (item->getMenu()->checkWritablePath(strPath))
				{
					KProcess *proc = new KNewItemProcess(item->getMenu());
					QString strApp(KApplication::kde_bindir());
					strApp += "/nickname";
					*proc << (const char *)strApp;
					*proc << "-path";
					*proc << (const char *)strPath;
					connect(proc, SIGNAL(processExited(KProcess*)), this, SLOT(processExited(KProcess*)));
					
					// hide menu before start
					the_panel->getKMenu()->getQPopupMenu()->hide();
	
					proc->start(KProcess::NotifyOnExit);
				}
			}
		}
		break;

	default:
		break;
	}
}

void KMenuEventFilter::processExited(KProcess*proc)
{
	PMenu *pItem = ((KNewItemProcess *)proc)->getParentMenu();
	delete proc;

	// update menu
	pItem->update();
}

void KMenuEventFilter::processExitedRoot(KProcess*proc)
{
	QString strTmpPath(((KNewItemProcess *)proc)->getSavedPath());
	delete proc;

	QDir dir(strTmpPath);
	if (dir.exists())
	{
		const QFileInfoList *fileinfolist = dir.entryInfoList();
		QFileInfoListIterator it( *fileinfolist );

		QFileInfo *fi;
		while ((fi = it.current()))
		{
			if (fi->fileName() != "." && fi->fileName() != "..")
			{ 
				QString strName(fi->absFilePath());
				the_panel->addToMenuFromDrop(strName);
				// delte file
				dir.remove(strName);
				break;
			}
			++it;
		}
		// remove temp dir 
		dir.rmdir(dir.absPath());
	}
}

extern bool createDefaultNewFolder(QString & strPath); // in pmenu.C
extern QString personal;

void KMenuEventFilter::addNewRootMenuFolder()
{
	PMenu *menu = the_panel->getKMenu();
	if (menu)
	{
		// it's can be not so easy in root directory
		// to simplify use a hack:
		// at first creat actual directory and after grop it into root menu
		QString strPath(personal);
		if (::createDefaultNewFolder(strPath))
		{
			the_panel->addToMenuFromDrop(strPath);
		}
	}
}

void KMenuEventFilter::addNewRootMenuItem()
{
	PMenu *menu = the_panel->getKMenu();
	if (menu)
	{
		QString strTmpPath(KApplication::localkdedir() +"/share/apps/kpanel/tmp");
		QDir dir(strTmpPath);
		if (dir.exists() || !::mkdir(strTmpPath, S_IRWXU))
		{
			if (::createDefaultNewFolder(strTmpPath))
			{
				// can add item to this directory?
				KProcess *proc = new KNewItemProcess((const char *)strTmpPath);
				QString strApp(KApplication::kde_bindir());
				strApp += "/nickname";
				*proc << (const char *)strApp;
				*proc << "-path";
				*proc << (const char *)strTmpPath;
				connect(proc, SIGNAL(processExited(KProcess*)), this, SLOT(processExitedRoot(KProcess*)));
	
				// hide menu before start
				menu->getQPopupMenu()->hide();
		
				proc->start(KProcess::NotifyOnExit);
			}
		}
	}
}



