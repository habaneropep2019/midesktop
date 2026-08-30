/*
  kpanel.C - Part of the KDE project.

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
#include <qtooltip.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <math.h>
#include <kfm.h>
#include <qbitmap.h>
#include <ksimpleconfig.h>
#include <qlayout.h>
#include <qscrollbar.h>

const int DEFAULT_BOX_WIDTH = 45;
const int DEFAULT_BOX_HEIGHT = 45;
const int knTrailingFreeSpace = 20;
const int knDesktopPreviewSize = 12;

/* 1 is the initial speed, hide_show_animation is the top speed. */
#define PANEL_SPEED(x, c) (int)((1.0-2.0*fabs((x)-(c)/2.0)/c)*hide_show_animation+1.0)

extern void execute(const char*);

class RootWidgetDropSite;
RootWidgetDropSite *m_pDropSite;

//
// RootWidgetDropSite class definition and implementation
//
class RootWidgetDropSite : public QWidget
{
public:
	RootWidgetDropSite(QWidget *parent = 0, const char *name = 0);

protected:
	void dragEnterEvent(QDragEnterEvent *event);
	void dragMoveEvent(QDragMoveEvent *event);
	void dropEvent(QDropEvent *event);
};

RootWidgetDropSite::RootWidgetDropSite(QWidget *parent, const char *name)
	: QWidget(parent, name)
{
	setAcceptDrops(TRUE);
}

void RootWidgetDropSite::dragEnterEvent(QDragEnterEvent *event)
{
//	printf("dragEnterEvent\n");
	QWidget::dragEnterEvent(event);
}

void RootWidgetDropSite::dragMoveEvent(QDragMoveEvent *event)
{
//	printf("dragEnterEvent\n");
	QWidget::dragMoveEvent(event);
}

void RootWidgetDropSite::dropEvent(QDropEvent *event)
{
//	printf("dropEvent\n");
	QWidget::dropEvent(event);
}

//
// DesktopEntry class implementation
//
DesktopEntry::DesktopEntry()
{
	init();
}

DesktopEntry::DesktopEntry(myPushButton* pBtn)
{
	init();
	button = pBtn;
}

void DesktopEntry::init()
{
  button = 0;
  popup = 0;
  pmi = 0;
  drop_zone = 0;
  app_id = 0;
  swallow = "";
  swallowed = 0;
  identity = "";
  icon[0] = 0;
  icon[1] = 0;
  icon[2] = 0;
  icon[3] = 0;
};

//
// DesktopEntriesList class implementation
//

DesktopEntriesList::DesktopEntriesList()
{
	setAutoDelete(true);
}

DesktopEntry * DesktopEntriesList::findButton(QWidget* pBtn)
{
	if ((pBtn))
	{
		for (DesktopEntry * pEntry = first(); pEntry; pEntry = next())
		{
			if (pEntry->button == pBtn)
			{
				return pEntry;
			}
		}
	}
	return 0;
}

//
// class CResizeBar
//
CResizeBar::CResizeBar(QWidget *parent, Orientation orientation, bool bFixedPos/*= false*/)
	: QFrame(parent), m_bMove(false), m_bFixedPos(bFixedPos)
{
	setLineWidth(1);
	setMidLineWidth(1);

	m_Orientation = orientation;
	if (orientation == Qt::Horizontal)
	{
		if (!bFixedPos)
			setCursor(sizeHorCursor);
		setFrameStyle( QFrame::VLine | QFrame::Raised);
		setFixedWidth(5);
	}
	else
	{
		if (!bFixedPos)
			setCursor(sizeVerCursor);
		setFrameStyle( QFrame::HLine | QFrame::Raised);
		setFixedHeight(5);
	}
}

void CResizeBar::mousePressEvent(QMouseEvent *e)
{
	if (!m_bFixedPos && e->button() == LeftButton)
	{
		m_bMove = true;
		QPoint p = QCursor::pos();
		m_nPrevPos = (m_Orientation == Qt::Horizontal ? p.x() : p.y());
	}
}

void CResizeBar::mouseMoveEvent(QMouseEvent *e)
{
	if (m_bMove)
	{
		QPoint p = QCursor::pos();
		int nPos = (m_Orientation == Qt::Horizontal ? p.x() : p.y());
		int nStep = nPos - m_nPrevPos;
		if (nStep)
		{
			m_nPrevPos = nPos; // save current pos
			emit barMoved(nStep);
		}
	}
}

void CResizeBar::mouseReleaseEvent(QMouseEvent *e)
{
	if (m_bMove)
	{
		m_bMove = false;
	}
}

//
// class DesktopSwitchButton 	implementation
//
DesktopSwitchButton::DesktopSwitchButton(QWidget *parent, int nID)
	: QToolButton(parent)
{
	QFont f(font());
	f.setPointSize(8);
	setFont(f);
	setToggleButton(true);

	createPixmap(nID);
}

void DesktopSwitchButton::createPixmap(int nID)
{
	QPainter painter;
	m_Pixmap.resize(16, 14);
	painter.begin(&m_Pixmap);

	int nX = 0, nY = 0;
	painter.setPen(QPen(white, 1));
	painter.drawRect(nX, nY, 16, 14);

	painter.setPen(QPen(lightGray, 1));
	painter.moveTo(nX + 1, nY + 13);
	painter.lineTo(nX + 1, nY + 1);
	painter.lineTo(nX + 15, nY + 1);

	painter.setPen(QPen(darkGray, 1));
	painter.moveTo(nX + 2, nY + 2);
	painter.lineTo(nX + 14, nY + 2);
	painter.lineTo(nX + 14, nY + 12);
	painter.lineTo(nX + 2, nY + 12);
	painter.drawPoint(nX + 14, nY + 12); // to fix bug in QT ?!

	painter.setPen(QPen(black, 1));
	painter.moveTo(nX + 2, nY + 12);
	painter.lineTo(nX + 2, nY + 3);
	painter.lineTo(nX + 14, nY + 3);

	painter.fillRect(nX + 3, nY + 4, 11, 8, QBrush(QColor(66,132,181)));

	painter.setPen(QPen(white, 1));
	painter.setFont(font());

	QString strID;
	strID.sprintf("%d", nID);
	painter.drawText(nX + 3, nY + 3, 11, 8, AlignCenter, strID, strID.length());

	painter.end();
}

void DesktopSwitchButton::drawButtonLabel(QPainter * pPainter)
{
	int nX = (width() - 16) / 2;
	int nY = (height() - 14) / 2;

	if ((isDown() || isOn()) && style() == WindowsStyle)
	{
		nX++; nY++;
	}
	pPainter->drawPixmap(nX, nY, m_Pixmap);
}


//
// DesktopPreviewButton class constructor
//
class DesktopPreviewButton : public QToolButton
{
public:
	DesktopPreviewButton(QWidget *pParent, Orientation o, Position p);

protected:
	void drawButtonLabel(QPainter * pPainter);

	ArrowType m_ArrowType;
};

DesktopPreviewButton::DesktopPreviewButton(QWidget *pParent, Orientation o, Position p)
	: QToolButton(pParent)
{
	if (o == Qt::Horizontal)
	{
		m_ArrowType = (p == top_left ? DownArrow : UpArrow);
  }
	else // p == vertical
	{
		m_ArrowType = (p == top_left ? RightArrow : LeftArrow);
	}
}

void DesktopPreviewButton::drawButtonLabel(QPainter * pPainter)
{
	int nX = 0, nY = 0;
	if (m_ArrowType == DownArrow)
	{
		nY = height() - 6;
		nX = (width() - 4) / 2 + 1;
  }
	else if (m_ArrowType == UpArrow)
	{
		nY = 6;
		nX = (width() - 4) / 2 + 1;
  }
	else if (m_ArrowType == RightArrow)
	{
		nX = width() - 6;
		nY = (height() - 4) / 2 + 1;
  }
	else // left arrow
	{
		nX = 6;
		nY = (height() - 4) / 2 + 1;
  }

	if ((isDown() || isOn()) && style() == WindowsStyle)
	{
		nX++; nY++;
	}

	QColorGroup colgrp = QColorGroup( colorGroup().text(),
						backgroundColor(),
						white, black, black,
						black, white );
	qDrawArrow( pPainter, m_ArrowType, WindowsStyle, false,
				nX, nY,
				0, 0,
				colgrp, true);
}

//
// kPanel class constructor
//
kPanel::kPanel( KWMModuleApplication* kwmapp_arg,
								QWidget *parent, const char *name )
	:	QFrame(parent, name, WStyle_Customize | WStyle_NoBorder | WStyle_Tool),
		moving_button(0),
		m_bPrepareMovePanel(false), m_pMoveMgr(0), m_nMovePos(-1)

{
	initing = true;
	kwmmapp = kwmapp_arg;
	int i;

	edit_button = 0;
	panel_button_frame_standalone2 = 0;
	m_pDesktopPreviewFrame = 0;
	m_pPagerProcess = 0;

	setFrameStyle(QFrame::Panel| QFrame::Raised);

	setMouseTracking(true);

	orientation = Qt::Horizontal;
	position = bottom_right;

	currentDesktop = KWM::currentDesktop();

	defaultPixmap = KApplication::getKApplication()->getIconLoader()->loadApplicationMiniIcon("mini-default.xpm", 16, 16);

	box_width = DEFAULT_BOX_WIDTH;
	box_height = DEFAULT_BOX_HEIGHT;
	margin = 1;
	dbhs = 4;
	dbrows = 2;

	has_kdisknav_button = false;
	has_windowlist_button = false;

	tbhs = 4;
	tbmhs = 2;
	number_of_desktops = KWM::numberOfDesktops();
	if (number_of_desktops == 1)
		 number_of_desktops = 0;

	hideTimer = new QTimer( this );
	connect( hideTimer, SIGNAL(timeout()),
		 this, SLOT(hideTimerDone()) );

	// parse the configuration
	KConfig *config = KApplication::getKApplication()->getConfig();
	// Don't write dirty entries at destruction time.
	config->rollback();

	// show disk navigator or not ????
	config->setGroup("kmenu");
	bShowDiskNav = (config->readEntry("ShowDiskNavigator", "off") == "on");

	config->setGroup("kpanel");
	if (!config->hasKey("Style"))
	{
		config->writeEntry("Style", "normal");
		config->writeEntry("BoxWidth", DEFAULT_BOX_WIDTH);
		config->writeEntry("BoxHeight", DEFAULT_BOX_HEIGHT);
		config->writeEntry("Margin",0);

		config->writeEntry("TaskbarButtonHorizontalSize",4);
		config->writeEntry("TaskbarButtonMinimumHorizontalSize",3);
		config->writeEntry("DesktopButtonFont","*-helvetica-medium-r-normal--12-*");
		config->writeEntry("DesktopButtonRows",2);
		config->writeEntry("DateFont","*-times-medium-i-normal--12-*");
	}

	QString a = config->readEntry("Position");

	if ( a == "left")
	{
		orientation = Qt::Vertical;
		position = top_left;
	}
	else if ( a == "right")
	{
		// no longer valid! Do left instead
		orientation = Qt::Vertical;
		//position = top_left;
		position = bottom_right;

	}
	else if ( a == "top"){
		orientation = Qt::Horizontal;
		position = top_left;
	}
	else if ( a == "bottom"){
		orientation = Qt::Horizontal;
		position = bottom_right;
	}


	// Read in our background pixmap (if any )
	if (config->hasKey("BackgroundTexture"))
	{
		mBackTexture = kapp->getIconLoader()
			->loadIcon(config->readEntry("BackgroundTexture" ));
		if (!mBackTexture.isNull())
		{
			//CT 15Jan1998 - little trick (rotate pixmap) to make vertical kpanel
			// beautier -  thanks to Stefan Taferner for the idea
			if (orientation == Qt::Vertical)
			{
				QWMatrix tmp_mtx;
				tmp_mtx.rotate(90);
				mBackTexture = mBackTexture.xForm(tmp_mtx);
			}
			//CT
			setBackgroundPixmap( mBackTexture );
		}
	}

	if (config->hasKey("BoxWidth"))
		box_width = config->readNumEntry("BoxWidth");
	if (config->hasKey("BoxHeight"))
		box_height = config->readNumEntry("BoxHeight");
	if (config->hasKey("Margin"))
		margin = config->readNumEntry("Margin");

	m_bDesktopSwitchButtonWithText = false;
	if (config->hasKey("DesktopButtonsWithText"))
		m_bDesktopSwitchButtonWithText = (config->readEntry("DesktopButtonsWithText") == "on");
	else
		config->writeEntry("DesktopButtonsWithText", "off");

	if (config->hasKey("DesktopButtonHorizontalSize"))
		dbhs = config->readNumEntry("DesktopButtonHorizontalSize");

	if (dbhs<1) dbhs = 1;
	if (dbhs>6) dbhs = 6;

	if (config->hasKey("DesktopButtonRows"))
		dbrows = config->readNumEntry("DesktopButtonRows");

	if (dbrows<1) dbrows = 1;
	else if (dbrows>2) dbrows = 2;

	// create "tool tip" manager
	m_pToolTip = new ToolTip();

	if (config->hasKey("MenuToolTips"))
		m_pToolTip->setToolTipsDelay(config->readNumEntry("MenuToolTips"));
	else
		config->writeEntry("MenuToolTips", m_pToolTip->getToolTipsDelay());

	m_pKMenuEventFilter = new KMenuEventFilter(m_pToolTip);

	foldersFirst = true;
	if (config->hasKey("FoldersFirst"))
		foldersFirst = (config->readEntry("FoldersFirst") == "on");
	else
		config->writeEntry("FoldersFirst", "on");

	//panel: autoHide, speed, delay
	autoHide = false;
	if (config->hasKey("AutoHide"))
		autoHide = (config->readEntry("AutoHide") == "on");
	else
		config->writeEntry("AutoHide", "off");

	//CT 16Oct1998 delay
	if (config->hasKey("AutoHideDelay"))
	{
		autoHideDelay = config->readNumEntry("AutoHideDelay");
		if (autoHideDelay < 10) autoHideDelay =  10;
		if (autoHideDelay > 10000) autoHideDelay = 10000;
	}
	else
		config->writeEntry ("AutoHideDelay", autoHideDelay=6000);
	//CT

	autoHidden = False;
	if (autoHide)
		hideTimer->start(autoHideDelay+2000, true);

	if (config->hasKey("AutoHideSpeed")) {
		autoHideSpeed = config->readNumEntry("AutoHideSpeed");
		if (autoHideSpeed < 1) autoHideSpeed = 1;//CT
	}
	else
		config->writeEntry("AutoHideSpeed", autoHideSpeed=4);

	//taskbar: autoHide, speed, delay
	autoHideTaskbar = false;
	if (config->hasKey("AutoHideTaskbar"))
		autoHideTaskbar = (config->readEntry("AutoHideTaskbar") == "on");
	else
		config->writeEntry("AutoHideTaskbar", "off");

	//CT 16Oct1998 delay
	if (config->hasKey("AutoHideTaskbarDelay")) {
		autoHideTaskbarDelay = config->readNumEntry("AutoHideTaskbarDelay");
		if (autoHideTaskbarDelay < 10) autoHideTaskbarDelay =  10;
		if (autoHideTaskbarDelay >10000) autoHideTaskbarDelay = 10000;
	}
	else
		config->writeEntry ("AutoHideTaskbarDelay", autoHideTaskbarDelay=6000);

	if (config->hasKey("AutoHideTaskbarSpeed")) {
		autoHideTaskbarSpeed = config->readNumEntry("AutoHideTaskbarSpeed");
		if (autoHideTaskbarSpeed < 1) autoHideTaskbarSpeed = 1;//CT
	}
	else
		config->writeEntry("AutoHideTaskbarSpeed", autoHideTaskbarSpeed=4);


	clockAmPm = true;
	if (config->hasKey("ClockAmPm"))
		clockAmPm = (config->readEntry("ClockAmPm") == "on");
	else
		config->writeEntry("ClockAmPm", "on");

	clockBeats = false;
	if( config->hasKey("ClockBeats"))
		clockBeats = (config->readEntry("ClockBeats") == "on");
	else
		config->writeEntry("ClockBeats", "off" );

	QString panelHiddenString = "00000000";
	panelHiddenString = config->readEntry("PanelHidden",
					panelHiddenString);

	for (i=1;i<=8;i++)
		panelHidden[i] = (panelHiddenString.mid(i-1,1)=="1");

	QString panelHiddenLeftString = "11111111";
	panelHiddenLeftString = config->readEntry("PanelHiddenLeft",
					panelHiddenLeftString);

	for (i=1;i<=8;i++)
		panelHiddenLeft[i] = (panelHiddenLeftString.mid(i-1,1)=="1");
	panelCurrentlyHidden = panelHidden[currentDesktop];
	panelCurrentlyLeft = panelHiddenLeft[currentDesktop];
	miniPanelHidden = True;

	/* Let's read the top speed here:  */
	/*CT verify a bit ( negative values were allowed rendering edge
		buttons unuseful */
	if (config->hasKey("HideShowAnimation")) {
		hide_show_animation = config->readNumEntry("HideShowAnimation");
		if (hide_show_animation <  0) hide_show_animation =   0;
		if (hide_show_animation >200) hide_show_animation = 200;
	}
	else
		config->writeEntry("HideShowAnimation",hide_show_animation=50);
	//CT

	wait_cursor_button = 0;

	popup_item = 0;

	windowlist = new myPopupMenu;
	CHECK_PTR( windowlist );
	windowlist->setCheckable(true);
	connect( windowlist, SIGNAL(activated( int )),
		 SLOT(windowlistActivated(int)) );
	init_popup(windowlist);

	// --sven: kdisknav button start --
	m_pDiskNavMenu = new myPopupMenu;
	CHECK_PTR( m_pDiskNavMenu );
	// --sven: kdisknav button end --

	int w = QApplication::desktop()->width();
	int h = QApplication::desktop()->height();

	box_width -= frameWidth() * 2;
	box_height -= frameWidth() * 2;

	// create kpanel elements

	// create a container for dock frame
	QWidget *pDockAreaFrameContainer = new QWidget(this);

	// create "dock" frame that contains "dock" area and clock
	QFrame *dock_area_frame = new QFrame(pDockAreaFrameContainer);
	dock_area_frame->setFrameStyle(QFrame::Panel| QFrame::Sunken);

	// create "dock" area"
	dock_area = new QWidget(dock_area_frame);

	// create "clock" area
	label_date = new QLabel(dock_area_frame);
	bool label_date_visible = true;
	if (config->hasKey("DateVisible"))
	{
		if (config->readEntry("DateVisible") == "false")
		{
			label_date_visible = false;
		}
	}

	// try to improve a layout of desktop buttons
	if (orientation == Qt::Vertical)
	{
		if (!m_bDesktopSwitchButtonWithText)
		{
			int n1 = DesktopSwitchButton::getPrefferedWidth();
			int b = ((box_width + n1 - 1) / n1) * n1 - box_width;
			if (b > 0 && b < 3)
			{
				box_width += b;
			}
		}
	}

	// create "buttons" area
	m_pButtonsArea = new ScrollableButtonsArea(this);
	m_pButtonsArea->getButtonsArea()->installEventFilter(this);
	m_pButtonsArea->setOrientation(orientation);
	m_pButtonsArea->setTrailingEmptySpace(knTrailingFreeSpace);
	if (!mBackTexture.isNull())
	{
		m_pButtonsArea->getButtonsArea()->setBackgroundPixmap( mBackTexture );
	}
	m_pButtonsArea->setBoxSize(box_width, box_height);

	// creta kde button
	kde_button = new myPushButton(this, klocale->translate("Go!"));
	kde_button->setCursor(waitCursor);
	QToolTip::add(kde_button, i18n("Application Starter"));

	connect(kde_button, SIGNAL(clicked()), SLOT(button_clicked()));
	connect(kde_button, SIGNAL(pressed()), SLOT(button_pressed()));

	entries.append(new DesktopEntry(kde_button));

	 if (!mBackTexture.isNull())
		 kde_button->setBackgroundPixmap( mBackTexture );

	// The control group
	control_group = new QFrame(this);
	control_group->setBackgroundColor(backgroundColor());
	if ( !mBackTexture.isNull() )
		control_group->setBackgroundPixmap( mBackTexture );


	bool bShowDesktopPreviewButton = true;
	if (bShowDesktopPreviewButton)
	{
		m_pBtnDesktopPreview = new DesktopPreviewButton(control_group, orientation, position);
		m_pBtnDesktopPreview->setFocusPolicy(NoFocus);
		QToolTip::add(m_pBtnDesktopPreview, i18n("Desktop Manager"));

		connect(m_pBtnDesktopPreview, SIGNAL(clicked()), SLOT(desktopPreview()) );
	}

	// Desktop bar
	desktopbar = new QButtonGroup(control_group);
	CHECK_PTR(desktopbar);
	desktopbar->setFrameStyle(QFrame::NoFrame);
	desktopbar->setExclusive(true);
	desktopbar->setBackgroundColor(backgroundColor());
	if ( !mBackTexture.isNull() )
		desktopbar->setBackgroundPixmap( mBackTexture );
	desktopbar->installEventFilter( this );

	for (i = 0; i < number_of_desktops; i++)
	{
		QButton* tmp_button;
		if (m_bDesktopSwitchButtonWithText)
		{
			QPushButton *tmp_push_button = new QPushButton("Desktop", desktopbar);
			tmp_push_button->setToggleButton(true);
			tmp_button = tmp_push_button;
		}
		else // small button with number in it
		{
			tmp_button = new DesktopSwitchButton(desktopbar, i+1);
		}

		tmp_button->installEventFilter(this);
		tmp_button->setFocusPolicy(NoFocus);

		if (currentDesktop - 1 == i)
		{
			tmp_button->toggle();
		}
	}

	connect(desktopbar, SIGNAL(clicked(int)), SLOT(desktop_change(int)));

	// is Taskbar docked
	m_bTaskbarDocked = true;
	if (config->hasKey("TaskbarDocked"))
		m_bTaskbarDocked = (config->readEntry("TaskbarDocked") == "true");
	else
		config->writeEntry("TaskbarDocked", m_bTaskbarDocked);

	// create taskbar
	bool bTaskbarButtonCanChangeSize = false;
	if (!m_bTaskbarDocked)
	{
		taskbar_frame = new myFrame(autoHideTaskbar, autoHideTaskbarDelay, 0, 0,
			 WStyle_Customize | WStyle_NoBorder | WStyle_Tool);
		taskbar_frame->setFrameStyle(QFrame::Panel| QFrame::Raised);
		taskbar_frame->hide();

		connect(taskbar_frame, SIGNAL(showMe()), SLOT(showTaskbar()));
		connect(taskbar_frame, SIGNAL(hideMe()), SLOT(hideTaskbar()));

		a = config->readEntry("TaskbarPosition");
		if (a == "top")
		{
			taskbar_position = top;
			bTaskbarButtonCanChangeSize = true;
		}
		else if (a == "bottom")
		{
			taskbar_position = bottom;
			bTaskbarButtonCanChangeSize = true;
		}
		else if (a == "hidden")
		{
			taskbar_position = hidden;
		}
		else if (a == "top_left")
		{
			taskbar_position = taskbar_top_left;
		}
		else // default position
		{
			taskbar_position = taskbar_top_left;
		}

		taskbar = new Taskbar(taskbar_frame, m_bTaskbarDocked, currentDesktop);
		if (taskbar_position == taskbar_top_left)
			taskbar->setOrientation(Qt::Vertical);
		else
			taskbar->setOrientation(Qt::Horizontal);
	}
	else
	{
		taskbar_frame = 0;
		taskbar_position = hidden;
		taskbar = new Taskbar(this, m_bTaskbarDocked, currentDesktop);
		if (orientation == Qt::Horizontal)
		{
			taskbar->getButtonsArea()->setAllowMultipleRows(true);
			bTaskbarButtonCanChangeSize = true;
		}
		else
			taskbar->setOrientation(Qt::Vertical);

		if (!mBackTexture.isNull())
		{
			taskbar->getButtonsArea()->getButtonsArea()->setBackgroundPixmap(mBackTexture);
		}
	}
	taskbar->getButtonsArea()->getButtonsArea()->installEventFilter(this);

	m_pDropSite = new RootWidgetDropSite(taskbar->getButtonsArea()->getButtonsArea());

	if (config->hasKey("TaskbarButtonHorizontalSize"))
		tbhs = config->readNumEntry("TaskbarButtonHorizontalSize");

	if (config->hasKey("TaskbarButtonMinimumHorizontalSize"))
		tbmhs = config->readNumEntry("TaskbarButtonMinimumHorizontalSize");

	load_and_set_some_fonts();

	if (bTaskbarButtonCanChangeSize)
	{
		taskbar->setButtonWidthLimits(tbmhs*taskbar_height, (tbhs*taskbar_height*3)/2);
		taskbar->setButtonSize((tbhs*taskbar_height*3)/2, taskbar_height);
	}
	else if (!m_bTaskbarDocked)
	{
		taskbar->setButtonSize(tbhs*taskbar_height, taskbar_height);
	}
	else // vertical and docked
	{
		taskbar->setButtonSize(box_width, taskbar_height);
	}

//     exit_button = new QPushButton("Exit", control_group);
//     exit_button->setFocusPolicy(NoFocus);
//     exit_button->setMouseTracking(true);
//     QToolTip::add(exit_button, klocale->translate("Logout"));
//     connect(exit_button, SIGNAL(clicked()), SLOT(ask_logout()));
//     exit_button->installEventFilter( this );
	//     exit_button->setPalette(pal);

//      = new QPushButton("lock", control_group);
//     lock_button->setFocusPolicy(NoFocus);
//     lock_button->setMouseTracking(true);
//     QToolTip::add(lock_button, klocale->translate("Lock screen"));
//     connect(lock_button, SIGNAL(clicked()), SLOT(call_klock()));
//     lock_button->installEventFilter( this );
	//     lock_button->setPalette(pal);


	panel_button = new QPushButton("panel", this);
	panel_button->setFocusPolicy(NoFocus);
	QToolTip::add(panel_button, klocale->translate("Hide the panel"));

	connect(panel_button, SIGNAL(clicked()), SLOT(hidePanelLeft()));

	bool bShowSecondPanelButton = false;
	if (config->hasKey("ShowSecondPanelButton"))
		bShowSecondPanelButton = (config->readEntry("ShowSecondPanelButton") == "true");

	if (bShowSecondPanelButton)
	{
		panel_button2 = new QPushButton("panel2", this );
		panel_button2->setFocusPolicy( NoFocus );
		QToolTip::add( panel_button2, klocale->translate( "Hide the panel" ) );

		connect(panel_button2, SIGNAL(clicked()), SLOT(hidePanelRight()));
	}

	panel_button_frame_standalone  = new QFrame(0, 0, WStyle_Customize | WStyle_NoBorder | WStyle_Tool );
	panel_button_standalone  = new QPushButton("panel", panel_button_frame_standalone);
	QToolTip::add(panel_button_standalone, klocale->translate("Show the panel"));
	panel_button_standalone->setFocusPolicy(NoFocus);

	connect(panel_button_standalone, SIGNAL(clicked()),
					SLOT(standalonePanelButtonClicked()));

	if (bShowSecondPanelButton)
	{
		panel_button_frame_standalone2  = new QFrame(0, 0, WStyle_Customize | WStyle_NoBorder | WStyle_Tool );
		panel_button_standalone2  = new QPushButton("panel", panel_button_frame_standalone2);
		QToolTip::add(panel_button_standalone2, klocale->translate("Show the panel"));
		panel_button_standalone2->setFocusPolicy(NoFocus);

		connect( panel_button_standalone2, SIGNAL(clicked()),
						 SLOT(standalonePanelButton2Clicked()) );
	}

	kde_button->setFixedSize(box_width, box_height);

	// create resize bars
	CResizeBar * m_pRB0 = new CResizeBar(this, orientation, true);
	CResizeBar * m_pRB1 = new CResizeBar(this, orientation);
	CResizeBar * m_pRB2 = new CResizeBar(this, orientation, true);

	if (margin == 0)
	{
		margin++;
	}

	int nDesktopBarPos = 0;
	QBoxLayout *pDockLayout;
	const QString strTimeTest(clockAmPm ? "10:00AM" : "00:00");

	if (orientation == Qt::Horizontal)
	{
		m_pRB0->setFixedHeight(box_height - 4);
		m_pRB1->setFixedHeight(box_height - 4);
		m_pRB2->setFixedHeight(box_height - 4);

		panel_button->setFixedSize(12, box_height);
		if (bShowSecondPanelButton)
			panel_button2->setFixedSize(12, box_height);

		// fix label_date font size (PGB)
		label_date->setFixedWidth(label_date_visible ? label_date->fontMetrics().width(strTimeTest) : 1);
		label_date->setAlignment( AlignHCenter|AlignVCenter );
		dock_area->setFixedWidth(1); // initial width

		if (bShowDesktopPreviewButton)
		{
			m_pBtnDesktopPreview->setGeometry(0, 0, knDesktopPreviewSize, box_height);
			nDesktopBarPos = knDesktopPreviewSize + margin;
		}
		desktopbar->move(nDesktopBarPos, 0);

		int nHg = box_height + frameWidth()*2;
		setGeometry(0, position == top_left ? 0 : h - nHg, w, nHg);

		layoutDesktopBar();

		control_group->setFixedSize(nDesktopBarPos + desktopbar->width(), desktopbar->height());

		m_pLayout = new QHBoxLayout(this, frameWidth());
		pDockLayout = new QHBoxLayout(dock_area_frame, dock_area_frame->frameWidth());
	}
	else // orientation == vertical
	{
		m_pRB0->setFixedWidth(box_width - 4);
		m_pRB1->setFixedWidth(box_width - 4);
		m_pRB2->setFixedWidth(box_width - 4);

		panel_button->setFixedSize(box_width, 12);
		if (bShowSecondPanelButton)
			panel_button2->setFixedSize(box_width, 12);

		label_date->setFixedHeight(label_date_visible ? label_date->fontMetrics().lineSpacing() * 3 : 1);
		label_date->setAlignment(AlignHCenter|AlignBottom);
		dock_area->setFixedHeight(1); // initial height

		if (bShowDesktopPreviewButton)
		{
			m_pBtnDesktopPreview->setGeometry(0, 0, box_width, knDesktopPreviewSize);
			nDesktopBarPos = knDesktopPreviewSize + margin;
		}
		desktopbar->move(0, nDesktopBarPos);

		int nWd = box_width + frameWidth()*2;
		setGeometry(position == top_left ? 0 : w - nWd, 0, nWd, h);

		layoutDesktopBar();

		control_group->setFixedSize(desktopbar->width(), nDesktopBarPos + desktopbar->height());

		m_pLayout = new QVBoxLayout(this, frameWidth());
		pDockLayout = new QVBoxLayout(dock_area_frame, dock_area_frame->frameWidth());
	}

	pDockLayout->addWidget(dock_area);
	pDockLayout->addWidget(label_date);

	QGridLayout *pDockContainerLayout = new QGridLayout(pDockAreaFrameContainer, 1, 1, 1);
	pDockContainerLayout->addWidget(dock_area_frame, 0, 0);

	m_pLayout->addWidget(panel_button);
	m_pLayout->addWidget(kde_button);
	m_pLayout->addWidget(m_pRB0);
	m_pLayout->addWidget(m_pButtonsArea);
	if (m_bTaskbarDocked)
	{
		m_pLayout->addWidget(m_pRB1);
		m_pLayout->addWidget(taskbar->getButtonsArea());
		connect(m_pRB1, SIGNAL(barMoved(int)), this, SLOT(resizeBarMoved(int)));
	}
	else
	{
		delete m_pRB1;
		QGridLayout *taskbar_layout = new QGridLayout(taskbar_frame, 1, 1, taskbar_frame->frameWidth());
		taskbar_layout->addWidget(taskbar->getButtonsArea(), 0, 0);
		taskbar_layout->activate();
	}
	m_pLayout->addWidget(m_pRB2);
	m_pLayout->addWidget(control_group);

	m_pLayout->addWidget(pDockAreaFrameContainer);

	if (bShowSecondPanelButton)
		m_pLayout->addWidget(panel_button2);

	m_pLayout->activate();
	// activate theese layouts later to get correct size
	pDockContainerLayout->activate();
	pDockLayout->activate();

	{
		// create the stipple pixmap for the panel button
		QPixmap pm;
		QBitmap bm;
		QPainter paint;
		QPainter paint2;
		pm.resize(9, (orientation == Qt::Horizontal)?panel_button->height():panel_button->width());
		bm.resize(pm.width(), pm.height());
		bm.fill(color0);
		pm.setMask(bm);
		paint.begin(&pm);
		paint2.begin(pm.mask());
		int stipple_height = 3;
		paint.setPen( colorGroup().light() );
		paint2.setPen( color1 );
		while ( stipple_height < pm.height()-4 )
		{
			paint.drawPoint( 1, stipple_height+1);
			paint.drawPoint( 4, stipple_height);
			paint2.drawPoint( 1, stipple_height+1);
			paint2.drawPoint( 4, stipple_height);
			stipple_height+=3;
		}
		paint.setPen( colorGroup().dark() );
		stipple_height = 4;
		while ( stipple_height < pm.height()-4 )
		{
			paint.drawPoint( 2, stipple_height+1);
			paint.drawPoint( 5, stipple_height);
			paint2.drawPoint( 2, stipple_height+1);
			paint2.drawPoint( 5, stipple_height);
			stipple_height+=3;
		}

		paint.end();
		paint2.end();
		QPixmap pm2 = pm;
		arrow_on_pixmap(&pm, LeftArrow);
		arrow_on_pixmap(&pm2, RightArrow);

		QWMatrix m;
		m.rotate((orientation == Qt::Horizontal)?0:90);
		panel_button->setPixmap(pm.xForm(m));
		panel_button_standalone->setPixmap(pm2.xForm(m));
		if (bShowSecondPanelButton)
		{
			panel_button2->setPixmap(pm2.xForm(m));
			panel_button_standalone2->setPixmap(pm.xForm(m));
		}
	}

	// load KMenu icon (custom or predefined)
	config->setGroup("KPanel Customization");
	QString strIconName = config->readEntry("Icon", "go.xpm");

	QPixmap pm;
	if (box_width-10 > 16 || box_height-10 > 16)
	{
		pm =  kapp->getIconLoader()->loadApplicationIcon(strIconName, box_width-4, box_height-4);
	}
	else
	{
		pm =  kapp->getIconLoader()->loadApplicationMiniIcon(strIconName, box_width-4, box_height-4);
	}

	if (pm.isNull())
		pm = load_pixmap("go.xpm");

	kde_button->setPixmap(create_arrow_pixmap(pm));
	//

	panel_button_frame_standalone->setGeometry(x() + panel_button->x(),
							 y() + panel_button->y(),
							 panel_button->width(),
							 panel_button->height());
	panel_button_standalone->setGeometry(0,0,panel_button->width(), panel_button->height());

	if (bShowSecondPanelButton)
	{
		panel_button_frame_standalone2->setGeometry(x() + width() - panel_button2->width(),
					y() + panel_button2->y(),
					panel_button2->width(),
					panel_button2->height() );
		panel_button_standalone2->setGeometry(0, 0, panel_button2->width(), panel_button2->height());
	}

	drop_zone = new KDNDDropZone(this, DndURL);
	connect(drop_zone,
		SIGNAL( dropAction( KDNDDropZone *) ),
		this,
		SLOT( slotDropEvent( KDNDDropZone *) ) );

	// fix label_date font size (PGB)
	QFont savfont(label_date->font());
	QFont tmpfont(savfont);
	QFontMetrics fm(label_date->fontMetrics());
	while(fm.width(strTimeTest) > label_date->width() ||
				fm.lineSpacing() * 3 > label_date->height())
	{
		int nPtSz = tmpfont.pointSize();
		if (nPtSz < 12 && orientation != Qt::Vertical)
		{
			// reset font - can't fit it anyway
			label_date->setFont(savfont);
			break;
		}
		tmpfont.setPointSize(nPtSz-1);
		label_date->setFont(tmpfont);
		fm = label_date->fontMetrics();
	}

	// update label_date size
	if (orientation == Qt::Horizontal)
	{
		if (fm.width(strTimeTest) < label_date->width() - 4)
		{
			label_date->setFixedWidth(fm.width(strTimeTest));
			m_pLayout->activate();
		}
	}
	else
	{
		if (fm.lineSpacing() * 3 < label_date->height() - 4)
		{
			label_date->setFixedHeight(fm.lineSpacing() * 3);
			m_pLayout->activate();
		}
	}

	// to make the clock more precise, start the clock
	// on the next minute break
	set_label_date();

	QTime cur_time = QTime::currentTime();
	clock_timer_id = 0;
	int next_minute_break = (60 - cur_time.second()) * 1000 - cur_time.msec();
	QTimer::singleShot(next_minute_break, this, SLOT(slotUpdateClock()));

	// create KMenu
	pmenu = new RootMenu();
	pmenu->setAltSort(foldersFirst);

	readInConfiguration();
	KApplication::getKApplication()->getConfig()->sync();
	doGeometry();
	KWM::sendKWMCommand("deskFitToWindow");

	if (panelHiddenString=="00000000")
	{
		for (i=1;i<=8;i++)
			KWM::setWindowRegion(i, KWM::getWindowRegion(currentDesktop));
	}

	layoutTaskbar();

	for (i=0; i < number_of_desktops; i++)
	{
		set_button_text(desktopbar->find(i), KWM::getDesktopName(i+1));
	}

	initing = false;
}

void kPanel::restart()
{
  cleanup();
  restart_the_panel();
}

void kPanel::desktop_change(int new_desk)
{
  new_desk++;

  if (new_desk == currentDesktop && m_bDesktopSwitchButtonWithText)
	{
    // create in-place edit for desktop name edition
		QPushButton *tmp_button = (QPushButton*)desktopbar->find(new_desk-1);
    edit_button = new QLineEdit(desktopbar,"edit_button");
    edit_button->setGeometry(tmp_button->geometry());
    edit_button->setText(KWM::getDesktopName(new_desk));
    edit_button->raise();
    edit_button->show();
    edit_button->grabKeyboard();
    edit_button->grabMouse();
    edit_button->setFocus();
    edit_button->installEventFilter(this);
    QColorGroup g = tmp_button->palette().active();
    g = QColorGroup( g.foreground(),
		     g.background(),
		     g.light(),
		     g.dark(),
		     g.mid(),
		     g.text(),
		     g.background());
    edit_button->setPalette( QPalette( g, g, g) );
  }
	else
	{
    if ( edit_button != 0)
      restore_editbutton( False );
    currentDesktop = new_desk;
    KWM::switchToDesktop(currentDesktop);
  }
};

void kPanel::restore_editbutton( bool takeit )
{
  if ( takeit )
	{
    KWM::setDesktopName(currentDesktop, edit_button->text());
  }

	if (edit_button != 0)
	{
    edit_button->releaseKeyboard();
    edit_button->releaseMouse();
    edit_button->hide();
    delete edit_button;
    edit_button = 0;
  };
};



void kPanel::button_clicked()
{
	DesktopEntry * pEntry = entries.findButton(myPushButton::most_recent_pressed);
	if (pEntry)
	{
		if (pEntry->button->last_button == LeftButton &&
			pEntry->pmi && pEntry->pmi->getType() == unix_com)
		{
			pEntry->pmi->exec();
		}
	}
}


void kPanel::button_pressed()
{
	DesktopEntry * pEntry = entries.findButton(myPushButton::most_recent_pressed);
	if (pEntry)
	{
		if (pEntry->button->last_button == LeftButton && pEntry->popup)
		{
			show_popup(pEntry->popup, pEntry->button);
		}
		else if (pEntry->button->last_button == LeftButton && pEntry->pmi && pEntry->pmi->getQPopupMenu())
		{
			show_popup(pEntry->pmi->getQPopupMenu(), pEntry->button);
		}
		else if (pEntry->button->last_button == RightButton)
		{
			enum { cmdMove, cmdRemove, cmdProperties};

			int nId = -1;
			if (pEntry->button == kde_button)
			{
				// popup menu for kde_button
				myPopupMenu popup;
				myPopupMenu popupNew;
				popupNew.insertItem(i18n("&Folder"),
														m_pKMenuEventFilter,
														SLOT(addNewRootMenuFolder()));
				popupNew.insertItem(i18n("&Nickname"),
														m_pKMenuEventFilter,
														SLOT(addNewRootMenuItem()));

				popup.insertItem(i18n("&New"), &popupNew, 10);

				popup.insertSeparator();

				popup.insertItem(i18n("Configure"), this, SLOT(configurePanel()));
				popup.insertItem(i18n("Restart"), this, SLOT(restart()));

				nId = show_popup(&popup, kde_button);
			}
			else
			{
				if (!popup_item)
				{
					popup_item = new myPopupMenu;
					CHECK_PTR(popup_item);
					popup_item->insertItem(i18n("&Move"), cmdMove);
					popup_item->insertItem(i18n("&Remove"), cmdRemove);
					popup_item->insertSeparator();
					popup_item->insertItem(i18n("&Properties"), cmdProperties);
					init_popup(popup_item);
				}

				if (pEntry->popup == windowlist)
				{
					popup_item->setItemEnabled(cmdMove, true);
					popup_item->setItemEnabled(cmdRemove, true);
					popup_item->setItemEnabled(cmdProperties, false);
				}
				else
				{
					popup_item->setItemEnabled(cmdMove, true);
					popup_item->setItemEnabled(cmdRemove, true);
					popup_item->setItemEnabled(cmdProperties, true);
				}
				nId = show_popup(popup_item, pEntry->button);
			}

			switch (nId)
			{
				case cmdProperties:
					if (pEntry->pmi)
					{
						QString a(pEntry->pmi->getFullPathName());

						KFM* kfm = new KFM;
						kfm->openProperties(a.latin1()); // Single argument, converted to const char*
						delete kfm;
					}
					break;

				case cmdMove: // move
					{
						startButtonMove(pEntry->button);
					}
					break;

				case cmdRemove: // remove
					{
						if (pEntry->swallowed)
							KWM::close(pEntry->swallowed);
						delete_button(pEntry->button);
						writeOutConfiguration();
					}
					break;

				default:
					break;
			}
		}
	}
}


void kPanel::taskbarClicked(myTaskButton* b)
{
  switch (b->last_button)
	{
  case MidButton:
    KWM::setIconify(b->win, !KWM::isIconified(b->win));
    break;
  case RightButton:

    break;
  default:
      if (b->isActive() && KWM::desktop(b->win) == KWM::currentDesktop())
	  KWM::setIconify(b->win, TRUE );
      else
	  KWM::activate(b->win);
    break;
  }
}


// PAL - Oct98: I added the "to desktop" option in the taskbar
// right-click menu.
void kPanel::taskbarPressed(myTaskButton *b)
{

  // We need to keep this in mind for the to_desktop_menu_activated
  // slot.  This is due to a problem with QPopupMenu:  When calling
  // exec, -1 is returned instead of the menu ID if the menu is a
  // multi-level one.
  last_taskbar_button_pressed = b;

  if (b->last_button != RightButton)
    return;

  QPopupMenu pop;
  QPopupMenu dm;
  pop.setMouseTracking(true);
  dm.setMouseTracking(true);

  pop.insertItem(KWM::getUnMaximizeString(), OP_RESTORE);
  pop.insertItem(KWM::getIconifyString(), OP_ICONIFY);
  pop.insertItem(KWM::getMaximizeString(), OP_MAXIMIZE);
  pop.insertItem(KWM::getStickyString(), OP_STICKY);
  pop.insertItem(KWM::getOntoCurrentDesktopString(),
                 OP_ONTO_CURRENT_DESKTOP);

  // We create the "to desktop" menu
  for (int i=1; i <= KWM::numberOfDesktops(); i++)
  {
    QString b = "&";
    b.append(KWM::getDesktopName(i));
    dm.insertItem(b);
  }
  pop.insertItem(KWM::getToDesktopString(), &dm);
  dm.setItemChecked(KWM::desktop(b->win) - 1, true);
  connect(&dm, SIGNAL(activated(int)), SLOT(to_desktop_menu_activated(int)));

  // Now disable items that need to be disabled

  if (KWM::fixedSize(b->win))
  {
    pop.setItemEnabled(OP_RESTORE, false);
    pop.setItemEnabled(OP_RESIZE, false);
    pop.setItemEnabled(OP_ICONIFY, false);
    pop.setItemEnabled(OP_MAXIMIZE, false);
  }

  if (KWM::isMaximized(b->win))
  {
    pop.setItemEnabled(OP_MAXIMIZE, false);
  }
  else
  {
    pop.setItemEnabled(OP_RESTORE, false);
  }

  if (KWM::isIconified(b->win))
  {
    pop.setItemEnabled(OP_ICONIFY, false);
    pop.setItemEnabled(OP_RESTORE, true);
  }

  // Check the sticky menu item if the client is sticky
  pop.setItemChecked(OP_STICKY, KWM::isSticky(b->win));

  if (b->virtual_desktop == KWM::currentDesktop())
    pop.setItemEnabled(OP_ONTO_CURRENT_DESKTOP, false);

  pop.insertSeparator();
  pop.insertItem(i18n("Iconify other &windows"), OP_ICONIFY_OTHER_WINDOWS);
  if (b->virtual_desktop != KWM::currentDesktop())
    pop.setItemEnabled(OP_ICONIFY_OTHER_WINDOWS, false);

	pop.insertSeparator();
  pop.insertItem(KWM::getCloseString(), OP_CLOSE);


  switch (show_popup(&pop, b, true))
	{
// KARTIKS
//  case OP_MAXIMIZE:
//    KWM::setMaximize(b->win, true);
//    break;
  case OP_RESTORE:
	if (KWM::isIconified(b->win))
	{
		KWM::setIconify(b->win, false);
	}
	else
	{
		KWM::setMaximize(b->win, false);
	}
    break;
  case OP_ICONIFY:
    KWM::setIconify(b->win, true);
    break;
  case OP_MAXIMIZE:
	if (KWM::isIconified(b->win))
	{
		KWM::setIconify(b->win, false);
	}
    KWM::setMaximize(b->win, true);
    break;
  case OP_STICKY:
    KWM::setSticky(b->win, !KWM::isSticky(b->win));
    break;
  case OP_CLOSE:
    KWM::close(b->win);
  case OP_ONTO_CURRENT_DESKTOP:
    KWM::moveToDesktop(b->win, KWM::currentDesktop());
    break;
  case OP_ICONIFY_OTHER_WINDOWS:
    {
			int cd = KWM::currentDesktop();
			for (int i = 0; i < taskbar->getNumEntries(); i++)
			{
				myTaskButton* bi = taskbar->getEntryAt(i);
				if (b != bi && KWM::desktop(bi->win) == cd)
					KWM::setIconify(bi->win, true);
			}
	  }
    break;
  default:
    break;
  }
}

// PAL - Oct98
// This function is called when a desktop is selected in the
// "To desktop" popup menu.
void kPanel::to_desktop_menu_activated(int item)
{
  if (last_taskbar_button_pressed)
		KWM::moveToDesktop(last_taskbar_button_pressed->win, item + 1);
}


void kPanel::addButtonInternal(PMenuItem* pmi, int x, int y, QString name)
{
//printf("add Button Internal %d %d\n", x, y);
	QPixmap pm;
	myPushButton *btn = new myPushButton(m_pButtonsArea->getButtonsArea());
	DesktopEntry *pEntry = new DesktopEntry(btn);
	entries.append(pEntry);
	if (!mBackTexture.isNull())
		btn->setBackgroundPixmap( mBackTexture );

	btn->installEventFilter( this );

	pEntry->popup = 0;
	pEntry->pmi = pmi;
	pEntry->swallow = "";
	pEntry->swallowed = 0;
	pEntry->identity = "";
	pEntry->icon[0] = 0;
	pEntry->icon[1] = 0;
	pEntry->icon[2] = 0;
	pEntry->icon[3] = 0;
	pEntry->drop_zone = 0;

	connect(btn, SIGNAL(clicked()), SLOT(button_clicked()));
	connect(btn, SIGNAL(pressed()), SLOT(button_pressed()));

	if (pmi)
	{
		if (pmi->getType() == submenu)
		{
			PMenu* pm = new PMenu;
			pm->setAltSort(foldersFirst);
			pm->parse(QDir(pmi->getFullPathName()));
			PMenuItem* pmi2 = new PMenuItem;
			QFileInfo fi(pmi->getFullPathName());
			pmi2->parse(&fi, pm);
			pmi = pmi2;
			pm->createMenu(pmi->getQPopupMenu(), this);
			pEntry->pmi = pmi;

			btn->setPixmap(create_arrow_pixmap( load_pixmap(pmi->getBigIconName(), True)));
		}
		else
		{
			btn->setPixmap(load_pixmap(pmi->getBigIconName()));

			QFileInfo fi(pmi->getFullPathName());
			if (fi.exists() && fi.isReadable())
			{
				KSimpleConfig pConfig(pmi->getFullPathName(),true);
				pConfig.setGroup("KDE Desktop Entry");
				if (pConfig.hasKey("SwallowTitle"))
				{
					pEntry->swallow = QString(pConfig.readEntry("SwallowTitle")).copy();
					if (!pEntry->swallow.isEmpty() &&
							pConfig.hasKey("SwallowExec"))
					{
						KWM::doNotManage(pEntry->swallow);
						QString aString(pConfig.readEntry("SwallowExec"));
						if (!initing)
							execute(aString.data());
						else
							swallowed_applications.append(aString);
					}
				}
				if (pConfig.hasKey("PanelIdentity"))
				{
					pEntry->icon[0] = new QPixmap();
					*(pEntry->icon[0]) = *pEntry->button->pixmap();
					pEntry->identity = QString(pConfig.readEntry("PanelIdentity")).copy();
					if (pConfig.hasKey("Icon2"))
					{
						pEntry->icon[1] = new QPixmap();
						*(pEntry->icon[1]) = load_pixmap(pConfig.readEntry("Icon2"));
					}
					if (pConfig.hasKey("Icon3"))
					{
						pEntry->icon[2] = new QPixmap();
						*(pEntry->icon[2]) = load_pixmap(pConfig.readEntry("Icon3"));
					}
					if (pConfig.hasKey("Icon4"))
					{
						pEntry->icon[3] = new QPixmap();
						*(pEntry->icon[3]) = load_pixmap(pConfig.readEntry("Icon4"));
					}
				}
			}

			pEntry->drop_zone = new KDNDDropZone(btn, DndURL);
			connect(pEntry->drop_zone,
							SIGNAL( dropAction( KDNDDropZone *) ),
							this,
							SLOT( slotDropEvent( KDNDDropZone *) ) );

		}

		QString getText(pmi->getText());
		if (getText.isEmpty())
			getText = i18n("No name available");
		QToolTip::add(btn, getText);
	}
	else if (name == "windowlist")
	{
		pEntry->popup = windowlist;
		pEntry->button->setPixmap(create_arrow_pixmap(load_pixmap("window_list.xpm")));
		QToolTip::add(btn, i18n("Windowlist"));
	}
		// --sven: kdisknav button start --
	else if (name == "kdisknav")
	{
		pEntry->popup = m_pDiskNavMenu;
		// store this buttonand check if release is over this button!!!
		btn->setPixmap(create_arrow_pixmap(load_pixmap("kdisknav.xpm")));
		QToolTip::add(btn, klocale->translate("Quickly navigate through the filesystem"));
	}
	 // --sven: kdisknav button end --

	btn->setFixedSize(box_width, box_height);
	btn->adjustSize();

	if (x != -1 || y != -1)
	{
		m_pButtonsArea->insertEntry(btn, x, y);
	}
	else
	{
		m_pButtonsArea->addEntry(btn);
	}
	btn->show();
}


void kPanel::launchSwallowedApplications()
{
	char* s;
	for (s = swallowed_applications.first(); s;
				s = swallowed_applications.next())
		execute(s);
}


void kPanel::show()
{
	if (!panelCurrentlyHidden)
	{
		QFrame::show();
	}
	else
	{
		if (panelCurrentlyLeft)
		{
			panel_button_frame_standalone->show();
			panel_button_frame_standalone->raise();
			if (panel_button_frame_standalone2)
				panel_button_frame_standalone2->hide();
		}
		else
		{
			if (panel_button_frame_standalone2)
			{
				panel_button_frame_standalone2->show();
				panel_button_frame_standalone2->raise();
			}
			panel_button_frame_standalone->hide();
		}
		showMiniPanel();
	}

	if (taskbar_frame && taskbar_position != hidden)
	{
		taskbar_frame->show();
	}
	KWM::sendKWMCommand("moduleRaised");
}


void kPanel::showPanel()
{
	if (!panelCurrentlyHidden)
		return;
	if (panelCurrentlyLeft)
		showPanelFromLeft();
	else
		showPanelFromRight();
}

extern bool in_animation;

void kPanel::hidePanelLeft()
{
  Bool old = panelHidden[currentDesktop];

  panelHidden[currentDesktop] = True;
  panelHiddenLeft[currentDesktop] = True;

  if (in_animation)
      return;

  if (panelCurrentlyHidden && !panelCurrentlyLeft) {
      showPanelFromRight(False);
      panelHidden[currentDesktop] = True;
      panelHiddenLeft[currentDesktop] = True;
  }

  if (!panelCurrentlyHidden){
    QPoint p = pos();
    if (autoHidden){
      if (orientation == Qt::Horizontal){
	if (position == top_left)
	  p.setY(y()+height()-4);
	else
	  p.setY(y()-height()+4);
      }
      else {
	if (position == top_left)
	  p.setX(x()+width()-4);
	else
	  p.setX(x()-width()+4);
      }
    }
    panel_button_frame_standalone->setGeometry(p.x() + panel_button->x(),
					       p.y() + panel_button->y(),
					       panel_button->width(),
					       panel_button->height());
    panelCurrentlyHidden = True;
    panelCurrentlyLeft = True;

    QRect geom = geometry();

    if(hide_show_animation) {
      in_animation = true;
      if (orientation == Qt::Vertical) {
	for (int i = 0; i < geom.height() - panel_button_frame_standalone->height();
	     i+=PANEL_SPEED(i,geom.height())){
	  move(geom.x(), geom.y()-i);
	  qApp->syncX();
	  qApp->processEvents();
	}
      } else {
	for (int i = 0; i < geom.width() - panel_button_frame_standalone->width();
	     i += PANEL_SPEED(i,geom.width())){
	  move(geom.x()-i, geom.y());
	  qApp->syncX();
	  qApp->processEvents();
	}
      }
    }
    panel_button_frame_standalone->show();
    panel_button_frame_standalone->raise();
    QFrame::hide();
    move(geom.x(), geom.y());
    in_animation = false;


    showMiniPanel();
    //panel_button_frame_standalone->show();
    //panel_button_frame_standalone->raise();


    doGeometry();
    layoutTaskbar (); //geometry changed
   }

  if (!panelHidden[currentDesktop]){
     showPanelFromLeft();
     return;
  }

  if (old != panelHidden[currentDesktop]){
    KConfig *config = KApplication::getKApplication()->getConfig();
    config->setGroup("kpanel");
    QString a;
    int i;
    for (i=1;i<=8;i++)
      a.append(panelHidden[i]?"1":"0");
    config->writeEntry("PanelHidden", a);
    a = "";
    for (i=1;i<=8;i++)
      a.append(panelHiddenLeft[i]?"1":"0");
    config->writeEntry("PanelHiddenLeft", a);
    config->sync();
  }
}

void kPanel::hidePanelRight()
{
  Bool old = panelHidden[currentDesktop];

  panelHidden[currentDesktop] = True;
  panelHiddenLeft[currentDesktop] = False;

  if (in_animation)
      return;

  if (panelCurrentlyHidden && panelCurrentlyLeft){
      showPanelFromLeft(False);
      panelHidden[currentDesktop] = True;
      panelHiddenLeft[currentDesktop] = False;
  }

  if (!panelCurrentlyHidden){
    QPoint p = pos();
    if (autoHidden){
      if (orientation == Qt::Horizontal){
	if (position == top_left)
	  p.setY(y()+height()-4);
	else
	  p.setY(y()-height()+4);
      }
      else {
	if (position == top_left)
	  p.setX(x()+width()-4);
	else
	  p.setX(x()-width()+4);
      }
    }
    panel_button_frame_standalone2->setGeometry(x() + width() - panel_button2->width(),
						p.y() + panel_button2->y(),
						panel_button2->width(),
						panel_button2->height());
    panelCurrentlyHidden = True;
    panelCurrentlyLeft = False;

    QRect geom = geometry();

    if(hide_show_animation) {
      in_animation = true;

      if (orientation == Qt::Vertical) {
	for (int i = 0; i < geom.height() - panel_button_frame_standalone2->height();
	     i += PANEL_SPEED(i,geom.height())){
	  move(geom.x(), geom.y()+i);
	  qApp->syncX();
	  qApp->processEvents();
	}
      } else {
	for (int i = 0; i < geom.width() - panel_button_frame_standalone2->width();
	     i += PANEL_SPEED(i,geom.width())){
	  move(geom.x()+i, geom.y());
	  qApp->syncX();
	  qApp->processEvents();
	}
      }
    }
    panel_button_frame_standalone2->show();
    panel_button_frame_standalone2->raise();
    QFrame::hide();
    move(geom.x(), geom.y());
    in_animation = false;


    showMiniPanel();
    //panel_button_frame_standalone2->show();
    //panel_button_frame_standalone2->raise();


    doGeometry();
    layoutTaskbar (); //geometry changed
   }

  if (!panelHidden[currentDesktop]){
     showPanelFromRight();
     return;
  }

  if (old != panelHidden[currentDesktop]){
    KConfig *config = KApplication::getKApplication()->getConfig();
    config->setGroup("kpanel");
    QString a;
    int i;
    for (i=1;i<=8;i++)
      a.append(panelHidden[i]?"1":"0");
    config->writeEntry("PanelHidden", a);
    a = "";
    for (i=1;i<=8;i++)
      a.append(panelHiddenLeft[i]?"1":"0");
    config->writeEntry("PanelHiddenLeft", a);
    config->sync();
  }
}

void kPanel::showPanelFromLeft( bool smooth)
{
  Bool old = panelHidden[currentDesktop];

  panelHidden[currentDesktop] = False;
  if (in_animation)
      return;

  panel_button_frame_standalone->hide();


  if (panelCurrentlyHidden){
    panelCurrentlyHidden = False;

    QRect geom = geometry();
    in_animation = true;
    move(-10000, -10000);
    QFrame::show();
    if(hide_show_animation) {
      if (orientation == Qt::Vertical) {
	for (int i = geom.height(); i>12;i-=smooth?(PANEL_SPEED(i,geom.height())):hide_show_animation){
	  move(geom.x(), geom.y()-i);
	  qApp->syncX();
	  qApp->processEvents();
	}
      } else {
	for (int i = geom.width(); i>12;i-=smooth?(PANEL_SPEED(i,geom.width())):hide_show_animation){
	  move(geom.x()-i, geom.y());
	  qApp->syncX();
	  qApp->processEvents();
	}
      }
    }
    move(geom.x(), geom.y());
    in_animation = false;

  }
  if (!smooth)
      return;
  hideMiniPanel();
  doGeometry();
  layoutTaskbar();

  if (panelHidden[currentDesktop]){
     hidePanelLeft();
     return;
  }

  if (old != panelHidden[currentDesktop]){
    KConfig *config = KApplication::getKApplication()->getConfig();
    config->setGroup("kpanel");
    QString a;
    int i;
    for (i=1;i<=8;i++)
      a.append(panelHidden[i]?"1":"0");
    config->writeEntry("PanelHidden", a);
    a = "";
    for (i=1;i<=8;i++)
      a.append(panelHiddenLeft[i]?"1":"0");
    config->writeEntry("PanelHiddenLeft", a);
    config->sync();
  }
}

void kPanel::showPanelFromRight(bool smooth)
{
  Bool old = panelHidden[currentDesktop];

  panelHidden[currentDesktop] = False;
  if (in_animation)
      return;

  panel_button_frame_standalone2->hide();


  if (panelCurrentlyHidden){
    panelCurrentlyHidden = False;

    QRect geom = geometry();
    in_animation = true;
    move(-10000, -10000);
    QFrame::show();
    if(hide_show_animation) {
      if (orientation == Qt::Vertical) {
	for (int i = geom.height(); i>12;i-=smooth?(PANEL_SPEED(i,geom.height())):hide_show_animation){
	  move(geom.x(), geom.y()+i);
	  qApp->syncX();
	  qApp->processEvents();
	}
      } else {
	for (int i = geom.width(); i>12;i-=smooth?(PANEL_SPEED(i,geom.width())):hide_show_animation){
	  move(geom.x()+i, geom.y());
	  qApp->syncX();
	  qApp->processEvents();
	}
      }
    }
    move(geom.x(), geom.y());
    in_animation = false;

  }
  if (!smooth)
      return;
  hideMiniPanel();
  doGeometry();
  layoutTaskbar();

  if (panelHidden[currentDesktop]){
     hidePanelRight();
     return;
  }

  if (old != panelHidden[currentDesktop]){
    KConfig *config = KApplication::getKApplication()->getConfig();
    config->setGroup("kpanel");
    QString a;
    int i;
    for (i=1;i<=8;i++)
      a.append(panelHidden[i]?"1":"0");
    config->writeEntry("PanelHidden", a);
    a = "";
    for (i=1;i<=8;i++)
      a.append(panelHiddenLeft[i]?"1":"0");
    config->writeEntry("PanelHiddenLeft", a);
    config->sync();
  }
}



void kPanel::doGeometry (bool do_not_change_taskbar)
{
	int space = 0;

	int w = QApplication::desktop()->width();
	int h = QApplication::desktop()->height();
	int pw = width()+space; // panel...
	int ph = height()+space;
	int px = x();
	int py = y();
	int pxR = x(); // for the window region
	int pyR = y(); // for the window region
	int pwR = width()+space; // for the window region
	int phR = height()+space;// for the window region
	int mw = 0;       // mini panel...
	int mh = 0;
	int sw = 0;      // standalone...
	int sh = 0;
	int sx = 0;

	int tfx = 0; //correction values for the taskbar autoHide feature
	int tfy = 0;
	int tfxR = 0; //correction values for the taskbar autoHide feature used
	int tfyR = 0; // to set the window region

	int tbw, tbh; // keep previous taskbar size
	int th = 0;

	if (taskbar_frame)
	{
		tbw = taskbar_frame->width();
		tbh = taskbar_frame->height();
		taskbar_frame_geometry = taskbar_frame->geometry();
		// calculate taskbar height
		th = (taskbar_position == hidden ? 0 : taskbar_height * numberOfTaskbarRows() + taskbar_frame->frameWidth() * 2);
	}
	else
	{
		tbw = tbh = 0;
		taskbar_frame_geometry.setRect(0,0,0,0);
	}

	if (autoHideTaskbar)
	{
		if (taskbar_position == top)
		{
			tfyR = -th+4;
		}
		else if (taskbar_position == bottom)
		{
			tfyR = th-4;
		}
		else // taskbar_top_left
		{
			tfxR = -tbhs*taskbar_height+4;
		}
	}

	if (taskbar_frame && taskbar_frame->autoHidden)
	{
		tfy = tfyR;
		tfx = tfxR;
	}

	if (panelCurrentlyHidden) // i.e. standalone shown
	{
		pw = pwR = 0;
		ph = phR = 0;
		px = pxR = 0;
		py = pyR = 0;
		sw = panel_button_frame_standalone->width()+space;
		sh = panel_button_frame_standalone->height()+space;

		mh = taskbar_height + frameWidth() * 2; // miniPanelFrame
		mw = (bShowDiskNav ? 3 : 2)*mh; // miniPanelFrame

		sx = sw;
		if (!panelCurrentlyLeft)
		{
			//right panel button, grrrrr it is becoming hacky......
			if (orientation == Qt::Horizontal)
			{
				if (position == top_left)
				{
					if (taskbar_position != bottom)
						sx = 0;
				}
				else
				{ // position == bottom_right
					if (taskbar_position != top)
						sx = 0;
				}
			}
			else // orientation == vertical
			{
				if (position == top_left)
				{
					if (taskbar_position != bottom)
						sx = 0;
				}
				else // position == bottom_right
				{
					if (taskbar_position == top)
					{
						sw = 0;
						sx = 0;
					}
					else if (taskbar_position != bottom)
						sx = 0;
				}
			}
		}
		else
		{
			// some corrections....
			if (orientation == Qt::Vertical)
			{
				if (taskbar_position == bottom)
				{
					sx = 0;
					sw = 0;
				}
			}
		}
	}
	else if (autoHide)
	{
		// panel is in autohide mode and not hidden. Adjust the window region to the hidden state.
		if (!autoHidden)
		{
			if (orientation == Qt::Horizontal)
			{
				if (position == top_left)
					pyR = pyR -  height() + 4;
				else
					pyR = pyR +  height() - 4;
			}
			else
			{
				if (position == top_left)
					pxR = pxR - width() + 4;
				else
					pxR = pxR + width() - 4;
			}
		}

		if (pxR < 0)
		{
			pwR += pxR;
			pxR = 0;
		}
		if (pyR < 0)
		{
			phR += pyR;
			pyR = 0;
		}
		if (pxR + pwR >w)
		{
			pwR = w-pxR+space;
		}
		if (pyR + phR >h)
		{
			phR = h-pyR+space;
		}


		if (autoHidden)
		{
			pw = pwR;
			ph = phR;
			px = pxR;
			py = pyR;
		}
	}

	if (orientation == Qt::Horizontal)
	{
		if (position == top_left)
		{
			if (taskbar_position == top)
			{
				taskbar_frame_geometry.setRect(tfx+px+sx+mw, tfy+py+ph, w-mw-sw, th);
				KWM::setWindowRegion(currentDesktop,
							QRect(0, tfyR+pyR+phR+th+space,
										w, -tfyR+h-phR-th-space));
			}
			else if (taskbar_position == bottom)
			{
				taskbar_frame_geometry.setRect(tfx+px+mw, tfy+h-th, w-mw, th);
				KWM::setWindowRegion(currentDesktop, QRect(0, phR, w, tfyR+h-phR-th-space));
			}
			else
			{
				taskbar_frame_geometry.moveTopLeft(QPoint(tfx+px+sx, tfy+py+ph+mh));
				if (taskbar_position == taskbar_top_left)
					KWM::setWindowRegion(currentDesktop,
								QRect(tfxR+tbhs*th+space+sx, phR,
											-tfxR+w-tbhs*th-sw-space, h-phR));
				else // if (taskbar_position=hidden)
					KWM::setWindowRegion(currentDesktop, QRect(0, phR, w, h-phR));
			}
		}
		else // (if position == bottom_right)
		{
		if (taskbar_position == bottom)
		{
			taskbar_frame_geometry.setRect(tfx+px+mw+sx, tfy+h-ph-th, w-mw-sw, th);
			KWM::setWindowRegion(currentDesktop, QRect(0, 0, w, tfyR+h-phR-th-space));
		}
		else if (taskbar_position == top)
		{
			taskbar_frame_geometry.setRect(tfx+px+mw, tfy+0, w-mw, th);
			KWM::setWindowRegion(currentDesktop, QRect(0, tfyR+th+space, w, -tfyR+h-phR-th-space));
		}
		else
		{
			taskbar_frame_geometry.moveTopLeft(QPoint(tfx+0, tfy+mh));
			if (taskbar_position == taskbar_top_left)
				KWM::setWindowRegion(currentDesktop,
								QRect(tfxR+tbhs*taskbar_height+space, 0,
											-tfxR+w-tbhs*taskbar_height-space, h-phR));
			else
				KWM::setWindowRegion(currentDesktop, QRect(0, 0, w, h-phR));
		}
	}
	}
	else // if (orientation == vertical)
	{
		if (position == top_left)
		{
			if (taskbar_position == top)
			{
				taskbar_frame_geometry.setRect(tfx+px+pw+mw+sx, tfy+py, w-pw-mw-sw, th);
				KWM::setWindowRegion(currentDesktop,
								QRect(pwR, tfyR+taskbar_frame->height()+space,
											w-pwR, -tfyR+h-taskbar_frame->height()-space));
			}
			else if (taskbar_position == bottom)
			{
				taskbar_frame_geometry.setRect(tfx+px+pw+mw+sx, tfy+h-th, w-pw-mw-sw, th);
				KWM::setWindowRegion(currentDesktop,
								QRect(pwR, 0, w-pwR, tfyR+h - taskbar_frame->height()-space));
			}
			else
			{
				taskbar_frame_geometry.moveTopLeft(QPoint(tfx+px+pw+sx, tfy+py+mh));
				if (taskbar_position == taskbar_top_left)
					KWM::setWindowRegion(currentDesktop,
								QRect(tfxR+pwR+tbhs*taskbar_height+space, 0,
											-tfxR+w-pwR-tbhs*taskbar_height-space, h));
				else
					KWM::setWindowRegion(currentDesktop, QRect(pwR, 0, w-pwR, h));
			}
		}
		else // if (position == bottom_right)
		{
			if (taskbar_position == top)
			{
				taskbar_frame_geometry.setRect(tfx+mw, tfy+0, w-pw-mw-sw, th);
				KWM::setWindowRegion(currentDesktop,
						QRect(0, tfyR+taskbar_frame->height()+space,
									w-pwR, -tfyR+h-taskbar_frame->height()-space));
			}
			else if (taskbar_position == bottom)
			{
				taskbar_frame_geometry.setRect(tfx+mw, tfy+h-th, w-pw-mw-sw, th);
				KWM::setWindowRegion(currentDesktop, QRect(0, 0, w-pwR, tfyR+h-taskbar_frame->height()-space));
			}
			else
			{
				taskbar_frame_geometry.moveTopLeft(QPoint(tfx+0, tfy+mh));
				if (taskbar_position == taskbar_top_left)
					KWM::setWindowRegion(currentDesktop,
								QRect(tfxR+tbhs*taskbar_height+space, 0,
											-tfxR+w-tbhs*taskbar_height-space-pwR, h));
				else
					KWM::setWindowRegion(currentDesktop, QRect(0, 0, w-pwR, h));
			}
		}
	}

	if (do_not_change_taskbar)
		return;

	if (taskbar_frame)
	{
		taskbar_frame->setGeometry(taskbar_frame_geometry);
		if (taskbar_frame->width() != tbw || taskbar_frame->height() != tbh)
		{
			doGeometry();
		}
	}
}



void kPanel::load_and_set_some_fonts()
{
  KConfig *config = KApplication::getKApplication()->getConfig();

  config->setGroup("kpanel");

//   if (config->hasKey("MenuFont")){
//     QFont tmpfont;
//     tmpfont.setRawMode(true);
//     tmpfont.setFamily(config->readEntry("MenuFont"));
//     KApplication::setFont(tmpfont, True);
//   }

  if (config->hasKey("DateFont"))
	{
    QFont tmpfont = config->readFontEntry( "DateFont" );
    label_date->setFont(tmpfont);
    label_date->adjustSize();
  }

  if (config->hasKey("DesktopButtonFont"))
	{
		QFont tmpfont = config->readFontEntry( "DesktopButtonFont" );
		QButton* tmp_button;
		for (int i=0; (tmp_button = desktopbar->find(i)); i++)
		{
			tmp_button->setFont(tmpfont);
		}
  }

	if (m_bTaskbarDocked && orientation != Qt::Vertical)
	{
		int nMinHg = taskbar->getButtonsArea()->fontMetrics().height() + 6;
		int nRows = (box_height - margin) / (nMinHg + margin);
		taskbar_height = (nRows > 1 ? (box_height-1) / nRows : box_height - margin);
	}
  else
		taskbar_height = taskbar->getButtonsArea()->fontMetrics().height() + 8;
}

void kPanel::slotUpdateClock()
{
  set_label_date();

  // if this is the first time this function is called, install a timer
  if(clock_timer_id == 0)
	{
    QTimer *t = new QTimer(this);
    connect(t, SIGNAL(timeout()),
	    this, SLOT(slotUpdateClock()));

    if( clockBeats == true )
      clock_timer_id = t->start(86000);
    else
      clock_timer_id = t->start(60000);
  }
}

#include "mipanel.moc"
