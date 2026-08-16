/*
   layout.C Part of the KDE project.
  
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
#include <qtooltip.h>
#include <qscrollview.h>
#include <qapp.h>
#include <qmsgbox.h>
#include <qbitmap.h>
#include <qcursor.h>
#include <qlayout.h>
#include <stdio.h>
#include <unistd.h>

const int margin = 1;
const int knDefaultScrolStep = 10;
const int knScrollBtnSize = 12;

void kPanel::resizeBarMoved(int nStep)
{
	if (orientation == Qt::Horizontal)
	{
		int nWd = m_pButtonsArea->width();
		if (nWd + nStep <= box_width)
			nStep = box_width - nWd;
		else if (nStep >= taskbar->getButtonsArea()->width() - box_width) 
			nStep = taskbar->getButtonsArea()->width() - box_width;

		if (nStep)
		{
			m_pButtonsArea->setFixedWidth(nWd + nStep);
			m_pLayout->activate();
		}
	}
	else
	{
		int nHg = m_pButtonsArea->height();
		if (nHg + nStep <= box_height)
			nStep = box_height - nHg;
		else if (nStep >= taskbar->getButtonsArea()->height() - box_height) 
			nStep = taskbar->getButtonsArea()->height() - box_height;
		if (nStep)
		{
			m_pButtonsArea->setFixedHeight(nHg + nStep);
			m_pLayout->activate();
		}
	}
}

void kPanel::layoutDesktopBar()
{
	int nDesktopBarWd, nDesktopBarHg;
	int nDesktopButtonWd, nDesktopButtonHg;
	int nX = 0, nY = 0;
	int nOffset = 0;

	nDesktopButtonHg = (box_height-1)/2;
	if (DesktopSwitchButton::getMinimumHeight() > nDesktopButtonHg)
	{
		nDesktopButtonHg = DesktopSwitchButton::getMinimumHeight();
	}

	if (orientation == Qt::Horizontal)
	{
		if (m_bDesktopSwitchButtonWithText)
		{
			nDesktopButtonWd = ((box_width-1)/2) * dbhs;
		}
		else
		{
			nDesktopButtonWd = DesktopSwitchButton::getPrefferedWidth();
		}
		
		nDesktopBarWd = nDesktopButtonWd + margin; // it can be recalculated later
		nDesktopBarHg = box_height;
		// in case of one row only can layout buttons in the middle
		if (nDesktopButtonHg*2 > nDesktopBarHg)
		{
			nY = nOffset = (nDesktopBarHg - nDesktopButtonHg) / 2;
		}
	}
	else // vertical
	{
		if (m_bDesktopSwitchButtonWithText)
		{
			nDesktopButtonWd = box_width - 2;
		}
		else
		{
			nDesktopButtonWd = DesktopSwitchButton::getPrefferedWidth();
		}
		
		nDesktopBarWd = box_width;
		nDesktopBarHg = nDesktopButtonHg + margin;	// can be recalculated later
		// in case of one row only can layout buttons in the middle
		if (nDesktopButtonWd*2 > nDesktopBarWd)
		{
			nX = nOffset = (nDesktopBarWd - nDesktopButtonWd) / 2;
		}
	}

	QButton* tmp_button;

	for (int i = 0; (tmp_button = desktopbar->find(i)); i++)
	{
		if (i)
		{
			if (orientation == Qt::Horizontal)
			{
				nY += nDesktopButtonHg + margin;
				if (nY + nDesktopButtonHg > nDesktopBarHg)
				{
					nY = nOffset;
					nX += nDesktopButtonWd + margin;
					nDesktopBarWd = nX + nDesktopButtonWd + margin;
				}
			}
			else // vertical
			{
				nX += nDesktopButtonWd;// + margin;
				if (nX + nDesktopButtonWd > nDesktopBarWd)
				{
					nX = nOffset;
					nY += nDesktopButtonHg + margin;
					nDesktopBarHg = nY + nDesktopButtonHg + margin;
				}
			}
		}
		tmp_button->setGeometry(nX, nY, nDesktopButtonWd, nDesktopButtonHg);
	}

	desktopbar->resize(nDesktopBarWd, nDesktopBarHg);
}

void kPanel::layoutTaskbar(bool bSizeChangeOnly/*= false*/)
{
	if (bSizeChangeOnly && !taskbar_frame)
			return;

	if (taskbar_frame && taskbar_position == taskbar_top_left)
	{
		int nWd = taskbar->getButtonWidth() + taskbar_frame->frameWidth() * 2;
		int nHg = taskbar->getMaxLength() + taskbar_frame->frameWidth() * 2;
		if (taskbar_frame->height() != nHg || taskbar_frame->width() != nWd)
		{
			taskbar_frame->resize(nWd , nHg);
			if (bSizeChangeOnly)
				return;
		}
	}

	int n = taskbar->getNumEntries();
	for (int i = 0; i < n; i++)
	{
		myTaskButton* button = taskbar->getEntryAt(i);
		if (button)
		{
			QRect geom(button->geometry());
			geom.moveTopLeft(button->mapToGlobal(QPoint(0,0)));
			if (taskbar_frame && taskbar_position == hidden)
				KWM::setIconGeometry(button->win, QRect());
			else
				KWM::setIconGeometry(button->win, geom);
		}
	}

}

int kPanel::numberOfTaskbarRows()
{
	// AR: taskbar can have only one row
	return 1;
}

//
// TaskBar class implementation
//
Taskbar::Taskbar(QWidget *parent, bool bDocked, int  nCurrentDesktop)
{
	m_nButtonWidth = 0;
	m_nHeight = 0;
	m_bShowDesktopSpecificTasks = true;

	m_bDocked = bDocked;
	m_nCurrentDesktop = nCurrentDesktop;
	m_pButtonsArea = new ScrollableButtonsArea(parent);
}

myTaskButton *Taskbar::createTaskButton()
{
	myTaskButton *b = new myTaskButton(m_pButtonsArea->getButtonsArea());
	return b;
}

void Taskbar::setButtonSize(int nWidth, int nHeight)
{
	m_nButtonWidth = nWidth; 
	m_nHeight = nHeight; 
	getButtonsArea()->setBoxSize(nWidth, nHeight);
};

void Taskbar::setButtonWidthLimits(int nMin, int nMax)
{ 
	getButtonsArea()->setFlexibleElementSize(nMin, nMax);
}

void Taskbar::remove(myTaskButton *pEntry)
{
	if (!m_bShowDesktopSpecificTasks || pEntry->virtual_desktop == m_nCurrentDesktop || pEntry->m_bSticky)
		m_pButtonsArea->removeEntry(pEntry);
	m_Buttons.removeRef(pEntry);
}

void Taskbar::taskbarModeAll()
{
	setShowsDesktopSpecificTasks(false);
}

void Taskbar::taskbarModeDesktop()
{
	setShowsDesktopSpecificTasks(true);
}

void Taskbar::setShowsDesktopSpecificTasks(bool b) 
{ 
	if (m_bShowDesktopSpecificTasks != b)
	{
		m_bShowDesktopSpecificTasks = b; 
		if (m_bShowDesktopSpecificTasks || ((int)m_Buttons.count() != m_pButtonsArea->getNumEntries()))
		{
			updateButtonArea();
		}
	}
}

void Taskbar::changedCurrentDesktop(int n)
{
	if (m_nCurrentDesktop != n)
	{
		m_nCurrentDesktop = n;

		if (!m_bShowDesktopSpecificTasks)
		{
			return; // nothing to do
		}

		updateButtonArea();
	}
}
		
void Taskbar::updateButtonArea()
{
	// remove entries from buttons area
	{
		QWidget *pEntry;
		while ((pEntry = m_pButtonsArea->getEntries().first()))
		{
			m_pButtonsArea->removeEntry(pEntry);
		}
	}

	// add entries into buttons area
	for (myTaskButton *pEntry = m_Buttons.first(); pEntry; pEntry = m_Buttons.next())
	{
		if (pEntry->virtual_desktop == m_nCurrentDesktop || !m_bShowDesktopSpecificTasks || pEntry->m_bSticky)
		{
			insertIntoButtonArea(pEntry);
		}
	}
}

void Taskbar::taskChangedDesktop(myTaskButton *pEntry, int nNewDesktop)
{
	int nPrevDesktop = pEntry->virtual_desktop;
	pEntry->virtual_desktop = nNewDesktop;

	if (m_bShowDesktopSpecificTasks)
	{
		if (nPrevDesktop == m_nCurrentDesktop)
		{
			// remove it from button area
			m_pButtonsArea->removeEntry(pEntry);
		}
	
		pEntry->virtual_desktop = nNewDesktop;
	
		if (nNewDesktop == m_nCurrentDesktop || pEntry->m_bSticky)
		{
			insertIntoButtonArea(pEntry);
		}
	}
}

void Taskbar::insert(myTaskButton *pEntry)
{	
	if (pEntry)
	{
		m_Buttons.append(pEntry);
		if (m_bShowDesktopSpecificTasks && m_nCurrentDesktop != pEntry->virtual_desktop && !pEntry->m_bSticky)
		{
			return;
		}

		insertIntoButtonArea(pEntry);
	}
}

void Taskbar::insertIntoButtonArea(myTaskButton *pEntry)
{
	if (m_pButtonsArea->getBoxWidth() != m_nButtonWidth)
	{
		m_nButtonWidth = m_pButtonsArea->getBoxWidth();
	}
	pEntry->resize(getButtonWidth(), getHeight());

	m_pButtonsArea->addEntry(pEntry);

	if (!pEntry->isVisible())
		pEntry->show();
}

void Taskbar::removeAll()
{
	for (myTaskButton *pEntry = m_Buttons.first(); pEntry; pEntry = m_Buttons.next())
	{
		if (pEntry->virtual_desktop == m_nCurrentDesktop)
			m_pButtonsArea->removeEntry(pEntry);
		m_Buttons.removeRef(pEntry);
		delete pEntry;
	}
}

//
// constructor
//
ScrollableButtonsArea::ScrollableButtonsArea(QWidget *parent)
	: QWidget(parent)
{
	scroll_buttons_displayed = false;
	m_bArrowPixmapsCreated = false;
	m_nButtonsAreaSize = 0;
	scroll_area_min_size = scroll_area_max_size = 0;
	m_Orientation = Qt::Horizontal;
	m_nBoxWidth = m_nBoxHeight = 0;
	m_bAllowMultipleRows = false;
	m_nNumColumns = 0;
	m_nTrailingFreeSpace = 0;
	m_nMinSize = m_nMaxSize = 0;

	m_pScroller = new QScrollView(this);
	m_pScroller->setVScrollBarMode(QScrollView::AlwaysOff);
	m_pScroller->setHScrollBarMode(QScrollView::AlwaysOff);
	m_pScroller->setFrameStyle(QFrame::NoFrame);
//	m_pScroller->setFrameStyle(QFrame::Panel| QFrame::Raised);
	m_pScroller->resize(0,0); //   initialize with nulls

	m_pButtonsArea = new QWidget(m_pScroller);

	m_pScroller->addChild(m_pButtonsArea);
	m_pButtonsArea->move(0,0);

	top_left_scroll_button = new QPushButton("Scroll Left", this);
	top_left_scroll_button->setAutoRepeat(true);
	top_left_scroll_button->setFocusPolicy(NoFocus);
	top_left_scroll_button->setMouseTracking(true);
	QToolTip::add(top_left_scroll_button, klocale->translate("Scroll the panel"));
	connect( top_left_scroll_button, SIGNAL(pressed()),
		 SLOT(scrollTopLeft()) );

	bottom_right_scroll_button = new QPushButton("Scroll Right", this);
	bottom_right_scroll_button->setAutoRepeat(true);
	bottom_right_scroll_button->setFocusPolicy(NoFocus);
	bottom_right_scroll_button->setMouseTracking(true);
	QToolTip::add(bottom_right_scroll_button, klocale->translate("Scroll the panel"));
	connect( bottom_right_scroll_button, SIGNAL(pressed()),
		 SLOT(scrollBottomRight()) );
}

void arrow_on_pixmap(QWidget *w, QPixmap* pm, Qt::ArrowType rt);

void ScrollableButtonsArea::createArrowPixmaps()    
{
	{
		// create the arrow pixmap for the scroll left button
		QPixmap pm;
		QBitmap bm;
		pm.resize(9, (m_Orientation ==
			Qt::Horizontal)?top_left_scroll_button->height():top_left_scroll_button->width());
		bm.resize(pm.width(), pm.height());
		bm.fill(color0);
		pm.setMask(bm);
		QPixmap pm2 = pm;
		arrow_on_pixmap(this, &pm, LeftArrow);

		QWMatrix m;
		m.rotate((m_Orientation == Qt::Horizontal)?0:90);
		top_left_scroll_button->setPixmap(pm.xForm(m));
	}

	{
		// create the arrow pixmap for the scroll right button
		QPixmap pm;
		QBitmap bm;
		pm.resize(9, (m_Orientation == Qt::Horizontal)?bottom_right_scroll_button->height():bottom_right_scroll_button->width());
		bm.resize(pm.width(), pm.height());
		bm.fill(color0);
		pm.setMask(bm);
		arrow_on_pixmap(this, &pm, RightArrow);

		QWMatrix m;
		m.rotate((m_Orientation == Qt::Horizontal)?0:90);
		bottom_right_scroll_button->setPixmap(pm.xForm(m));
	}
}

// PAL - Sept98: The following methods were added to support scrolling
// in the kPanel
void ScrollableButtonsArea::scrollTopLeft()
{
	scrollTopLeft(m_Orientation == Qt::Horizontal ? m_nBoxWidth : m_nBoxHeight);
}

void ScrollableButtonsArea::scrollTopLeft(int step)
{
	if (step <= 0)
	{
		step = knDefaultScrolStep;
	}

	if(m_Orientation == Qt::Vertical)
		m_pScroller->scrollBy(0, -step);
	else
		m_pScroller->scrollBy(-step, 0);
	enableDisableScrollButtons();
}

void ScrollableButtonsArea::scrollBottomRight()
{
	scrollBottomRight(m_Orientation == Qt::Horizontal ? m_nBoxWidth : m_nBoxHeight);
}

void ScrollableButtonsArea::scrollBottomRight(int step)
{
	if (step <= 0)
	{
		step = knDefaultScrolStep;
	}

	if (m_Orientation == Qt::Vertical)
		m_pScroller->scrollBy(0, step);
	else
	{
		m_pScroller->scrollBy(step, 0);
	}
	enableDisableScrollButtons();
}

void ScrollableButtonsArea::resizeEvent(QResizeEvent *)
{
	layoutComponents();
}

void ScrollableButtonsArea::layoutComponents()
{
//printf("layoutComponents = %d %d\n", width(), height());
	if (!scroll_buttons_displayed)
	{
		top_left_scroll_button->hide();
		bottom_right_scroll_button->hide();
	}
	
	if(m_Orientation == Qt::Vertical)
  {
		top_left_scroll_button->setGeometry(0, height() - knScrollBtnSize*2, width(), knScrollBtnSize);
		bottom_right_scroll_button->setGeometry(0, height() - knScrollBtnSize, width(), knScrollBtnSize);

		scroll_area_max_size = height();
		scroll_area_min_size = scroll_area_max_size - knScrollBtnSize * 2;
		
		checkButtonsAreaSize();

		if (m_nButtonsAreaSize <= scroll_area_max_size)
			m_pScroller->setGeometry(0, 0, width(), scroll_area_max_size);
		else
			m_pScroller->setGeometry(0, 0, width(), scroll_area_min_size);
	}
	else // horizontal || m_bAllowMultipleRows
	{
		top_left_scroll_button->setGeometry(width() - knScrollBtnSize*2, 0, knScrollBtnSize, height());
		bottom_right_scroll_button->setGeometry(width() - knScrollBtnSize, 0, knScrollBtnSize, height());
		
		scroll_area_max_size = width();
		scroll_area_min_size = scroll_area_max_size - knScrollBtnSize * 2;
	
		checkButtonsAreaSize();

		if (m_nButtonsAreaSize <= scroll_area_max_size)
			m_pScroller->setGeometry(0, 0, scroll_area_max_size, height());
		else
			m_pScroller->setGeometry(0, 0, scroll_area_min_size, height());
	}
	
	buttonsAreaResized();
}

// This function shows or hides the scrolling buttons
// depending on the need for them.
void ScrollableButtonsArea::showHideScrollButtons()
{
	if (m_Orientation == Qt::Vertical && !m_bAllowMultipleRows)
  {
    if (scroll_buttons_displayed)
    {
      // Hide scrolling buttons if the buttons_area fits
      // entirely in the buttons_scroll_area
      if (m_nButtonsAreaSize <= scroll_area_max_size)
      {
        if (m_pScroller->height() != scroll_area_max_size)
				{
					m_pScroller->resize(m_pScroller->width(), scroll_area_max_size);
				}
        top_left_scroll_button->hide();
        bottom_right_scroll_button->hide();
        scroll_buttons_displayed = false;
      }
    }
    else
    {
      // Display scrolling buttons if the buttons_area doesn't fit
      // entirely in the buttons_scroll_area
      if (m_nButtonsAreaSize > scroll_area_max_size)
      {
        if (m_pScroller->height() != scroll_area_min_size)
				{
					m_pScroller->resize(m_pScroller->width(), scroll_area_min_size);
				}
        top_left_scroll_button->show();
        bottom_right_scroll_button->show();
        scroll_buttons_displayed = true;
      }
    }
  }
  else // orientation == Qt::Horizontal
  {
    if (scroll_buttons_displayed)
    {
      // Hide scrolling buttons if the buttons_area fits
      // entirely in the buttons_scroll_area
      if (m_nButtonsAreaSize <= scroll_area_max_size)
      {
        if (m_pScroller->width() != scroll_area_max_size)
				{
					m_pScroller->resize(scroll_area_max_size, m_pScroller->height());
				}
        top_left_scroll_button->hide();
        bottom_right_scroll_button->hide();
        scroll_buttons_displayed = false;
      }
    }
    else
    {
      // Display scrolling buttons if the buttons_area doesn't fit
      // entirely in the buttons_scroll_area
      if (m_nButtonsAreaSize > scroll_area_max_size)
      {
        if (m_pScroller->width() != scroll_area_min_size)
				{
					m_pScroller->resize(scroll_area_min_size, m_pScroller->height());
				}
        top_left_scroll_button->show();
        bottom_right_scroll_button->show();
        scroll_buttons_displayed = true;
      }
    }
  }


	if (!m_bArrowPixmapsCreated)
	{
		m_bArrowPixmapsCreated = true;
		createArrowPixmaps();
	}
}

// This function enables/disables the scrolling buttons depending on the
// buttons area location in the scrolling area.
void ScrollableButtonsArea::enableDisableScrollButtons()
{
  if(scroll_buttons_displayed)
  {
    if(m_Orientation == Qt::Vertical)
    {
      if(m_pScroller->contentsY() > 0)
        top_left_scroll_button->setEnabled(true);
      else
        top_left_scroll_button->setEnabled(false);

      if(m_pButtonsArea->height() > m_pScroller->contentsY() + m_pScroller->height())
        bottom_right_scroll_button->setEnabled(true);
      else
        bottom_right_scroll_button->setEnabled(false);  
    }
    else
    {
      if(m_pScroller->contentsX() > 0)
        top_left_scroll_button->setEnabled(true);
      else
        top_left_scroll_button->setEnabled(false);

      if(m_pButtonsArea->width() >      
            m_pScroller->contentsX() + m_pScroller->width())
        bottom_right_scroll_button->setEnabled(true);
      else
        bottom_right_scroll_button->setEnabled(false);  
    }
  }
}

// PAL - Sept98
// This function is called when the button area has been resized.
// It shows or hides the scrolling buttons and resizes the scrolling
// area to show the button area the best way.
void ScrollableButtonsArea::buttonsAreaResized()
{
  showHideScrollButtons();
  enableDisableScrollButtons();
}

void ScrollableButtonsArea::insertEntry(QWidget *pEntry, int x, int y)
{
//printf("insert entry %d %d\n", x, y);
	QWidget *pLast = m_Entries.last();
	// the easiest case - put last
	if (!pLast || (m_Orientation == Qt::Horizontal && x >= pLast->x() + pLast->width())
			|| (m_Orientation == Qt::Vertical && y >= pLast->y() + pLast->height()))
	{
		pEntry->move(x, y);

		m_Entries.append(pEntry);
	}
	else if (pLast && ((m_Orientation == Qt::Horizontal && x >= pLast->x())
										|| (m_Orientation == Qt::Vertical && y >= pLast->y())))
	{
		addEntry(pEntry);
		return;
	}
	else
	{
		int nShift = 0;
		int nPrevBoundary = 0;
		if (m_Orientation == Qt::Horizontal)
		{
			int nWd = pEntry->width();
			for (QWidget *pE = m_Entries.first(); pE; pE = m_Entries.next())
			{
				int nX = pE->x();
				int nXE = nX + pE->width();
				if (x < nXE && !nShift)
				{
					if (nX - nWd - margin >= nPrevBoundary)
					{
						// can put it inside empty space before item
						if (x + nWd + margin > nX)
						{
							x = nX - nWd - margin;
						}
						pEntry->move(x, y);
						m_Entries.insert(m_Entries.at(), pEntry);
						// no need to shift remaining items
						break;
					}
					else // have to shift
					{ 
						x = nPrevBoundary;
						nShift = x + nWd + margin - nX;
//printf("shift  = %d\n",  nShift);
						pEntry->move(x, y);
						m_Entries.insert(m_Entries.at(), pEntry);
						m_Entries.next();
					}
				}

				if (nShift)
				{
//printf("shift %d->%d\n", nX, nX + nShift);
					pE->move(nX + nShift, pE->y());
				}
				nPrevBoundary = nXE + margin;
			}
		}
		else //vertical
		{
			int nHg = pEntry->height();
			for (QWidget *pE = m_Entries.first(); pE; pE = m_Entries.next())
			{
				int nY = pE->y();
				int nYE = nY + pE->height();
				if (y < nYE && !nShift)
				{
					if (nY - nHg - margin >= nPrevBoundary)
					{
						// can put it inside empty space before item
						if (y + nHg + margin > nY)
						{
							y = nY - nHg - margin;
						}
						pEntry->move(x, y);
						m_Entries.insert(m_Entries.at(), pEntry);
						// no need to shift remaining items
						break;
					}
					else // have to shift
					{ 
						y = nPrevBoundary;
						nShift = y + nHg + margin - nY;
//printf("shift  = %d\n",  nShift);
						pEntry->move(x, y);
						m_Entries.insert(m_Entries.at(), pEntry);
						m_Entries.next();
					}
				}

				if (nShift)
				{
//printf("shift %d->%d\n", nY, nY + nShift);
					pE->move(pE->x(), nY + nShift);
				}
				nPrevBoundary = nYE + margin;
			}
		}
	}

	checkButtonsAreaSize();
	buttonsAreaResized();
}

int ScrollableButtonsArea::getMaxLength()
{
	int nLn;

	QWidget *pLast = m_Entries.last();
	if (!pLast)
		nLn = m_nTrailingFreeSpace;
	else if (getOrientation() == Qt::Horizontal || m_bAllowMultipleRows)
	{ 
		nLn = pLast->x() + pLast->width() + margin + m_nTrailingFreeSpace;
	}
	else // vertical
	{
		nLn = pLast->y() + pLast->height() + margin + m_nTrailingFreeSpace;
	}
	return nLn;
}

void ScrollableButtonsArea::checkButtonsAreaSize()
{
	// leave space at the end (half button) to drop new objects
	bool bInit = (height() > 8 && width() > 8); // not the best way
	
	int nSz = 0;
	QWidget *pLast = m_Entries.last();
	if (m_Orientation == Qt::Horizontal || m_bAllowMultipleRows)
	{ 
		if (!pLast)
			nSz = m_nTrailingFreeSpace;
		else
			nSz = pLast->x() + pLast->width() + margin + m_nTrailingFreeSpace;

		//can fit button into it ????
		if (m_nMinSize != m_nMaxSize && pLast)
		{
			int nItemWd = pLast->width();
			int nCnt = m_Entries.count();
			int nRows = (height() + margin) / (pLast->height() + margin);
			int nItemsInRow = (nRows > 0 ? (nCnt + nRows - 1) / nRows : nCnt);
			int nBestSz = (scroll_area_max_size - margin - m_nTrailingFreeSpace) / nItemsInRow - margin;
			if (nBestSz != nItemWd)
			{
				if (nSz < scroll_area_max_size && nItemWd < m_nMaxSize)
				{
					// try to increase item size
					if (nBestSz > m_nMaxSize)
						nBestSz = m_nMaxSize;
					changeItemsSize(nBestSz, pLast->height());
					return;
				}
				else if (nSz > scroll_area_max_size && nItemWd > m_nMinSize)
				{
					// try to decrease item size
					if (nBestSz < m_nMinSize)
						nBestSz = m_nMinSize;
					changeItemsSize(nBestSz, pLast->height());
					return;
				}
			}
		}
		if (nSz != m_nButtonsAreaSize)
		{
			m_nButtonsAreaSize = nSz;
		}
		
		if (bInit)
		{
			m_pButtonsArea->resize(nSz <= scroll_area_max_size ? scroll_area_max_size : nSz, height());
		}
	}
	else // vertical)
	{
		if (!pLast)
			nSz = m_nTrailingFreeSpace;
		else
			nSz = pLast->y() + pLast->height() + margin + m_nTrailingFreeSpace;

		if (nSz != m_nButtonsAreaSize)
		{
			m_nButtonsAreaSize = nSz;
		}

		if (bInit)
		{
			m_pButtonsArea->resize(width(), nSz <= scroll_area_max_size ? scroll_area_max_size : nSz);
		}
	}
//printf("buttons area = %d %d\n", m_pButtonsArea->width(), m_pButtonsArea->height());
}

void ScrollableButtonsArea::addEntry(QWidget *pEntry)
{
	QWidget *pLast = m_Entries.last();
	m_Entries.append(pEntry);
	if (pLast)
	{
		int nX, nY;
		if (m_bAllowMultipleRows)
		{
			nY = pLast->y() + pLast->height() + margin;
			if (nY + pEntry->height() > m_pButtonsArea->height())
			{
				m_nNumColumns++;
				nY = 0;
			}
			
			nX = m_nNumColumns * (m_nBoxWidth + margin);
		}
		else if (m_Orientation == Qt::Horizontal)
		{
			nX = pLast->x() + pLast->width() + margin;
			nY = pLast->y();
		}
		else // vertical
		{
			nX = pLast->x();
			nY = pLast->y() + pLast->height() + margin;
		}
		pEntry->move(nX, nY);
	}
	else
	{
		pEntry->move(0, 0);
	}

	checkButtonsAreaSize();
	buttonsAreaResized();
}

void ScrollableButtonsArea::removeEntry(QWidget *pEntry, bool bShiftRemaining/* = true*/)
{
	int nInd;
	if ((nInd = m_Entries.findRef(pEntry)) >= 0)
	{
		m_Entries.remove();
		if (bShiftRemaining)
		{
			fillEmptyPosition(pEntry->x(), pEntry->y());
			pEntry->hide();
		}
	}

	checkButtonsAreaSize();
	buttonsAreaResized();
}

void ScrollableButtonsArea::changeItemsSize(int nNewWd, int nNewHg)
{
	if (nNewWd == m_nBoxWidth && nNewHg == m_nBoxHeight)
	{
		return;
	}

	int nX0 = 0, nY0 = 0;
	m_nBoxWidth = nNewWd;
	m_nBoxHeight = nNewHg;

	for (QWidget *pE = m_Entries.first(); pE; pE = m_Entries.next())
	{
		if (m_bAllowMultipleRows)
		{
			if (nX0 || nY0)
			{
				if (nY0 + m_nBoxHeight > m_pButtonsArea->height())
				{
					m_nNumColumns++;
					nY0 = 0;
				}
				nX0 = m_nNumColumns * (m_nBoxWidth + margin);
			}
			else
			{
				m_nNumColumns = 0;
			}

			pE->setGeometry(nX0, nY0, m_nBoxWidth, m_nBoxHeight);
			// calculate possible pos for next item
			nY0 += m_nBoxHeight + margin;
		}
		else if (m_Orientation == Qt::Horizontal)
		{
			pE->setGeometry(nX0, pE->y(), m_nBoxWidth, m_nBoxHeight);
			nX0 += m_nBoxWidth + margin;
		}
		else
		{
			pE->setGeometry(pE->x(), nY0, m_nBoxWidth, m_nBoxHeight);
			nY0 += m_nBoxHeight + margin;
		}
	}
	checkButtonsAreaSize();
}

void ScrollableButtonsArea::fillEmptyPosition(int nX, int nY)
{
	int nShift = 0;
	int nX0 = 0, nY0 = 0; // for multiple rows mode
	for( QWidget *pE = m_Entries.first(); pE; pE = m_Entries.next())
	{
		if (m_bAllowMultipleRows)
		{
			if (nX0 || nY0)
			{
				if (nY0 + pE->height() > m_pButtonsArea->height())
				{
					m_nNumColumns++;
					nY0 = 0;
				}
				nX0 = m_nNumColumns * (m_nBoxWidth + margin);
			}
			else
			{
				m_nNumColumns = 0;
			}

			pE->move(nX0, nY0);
			// calculate possible pos for next item
			nY0 += pE->height() + margin;
		}
		else if (m_Orientation == Qt::Horizontal)
		{
			int nX0 = pE->x();
			if (nX0 > nX && !nShift)
			{
				nShift = nX - nX0;
			}

			if (nShift)
			{
				pE->move(nX0 + nShift, pE->y());
			}
		}
		else
		{
			int nY0 = pE->y();
			if (nY0 > nY && !nShift)
			{
				nShift = nY - nY0;
			}

			if (nShift)
			{
				pE->move(pE->x(), nY0 + nShift);
			}
		}
	}
}

void ScrollableButtonsArea::moveEntry(QWidget *pEntry, const QPoint & OldPos)
{
	moveEntry(pEntry, OldPos.x(), OldPos.y());
}

void ScrollableButtonsArea::moveEntry(QWidget *pEntry, int nXOld, int nYOld)
{
	int nInd;
	if (pEntry && (nInd = m_Entries.findRef(pEntry)) >= 0)
	{
		bool bInserted = false;
		int nX = pEntry->x(), nY = pEntry->y();
		m_Entries.remove();

		int nPrevBoundary = 0;
		int nShift = 0;
		if (m_Orientation == Qt::Horizontal)
		{
			nY = nYOld; // no move on Y coord
			int nXE = nX + pEntry->width();
			
			for( QWidget *pE = m_Entries.first(); pE; pE = m_Entries.next())
			{
				int nX0 = pE->x();
				int nX1 = nX0 + pE->width();
				if (nXE < nX1 && nShift >= 0)
				{
					// put it here
					if (nX < nX0 && nX0 - nPrevBoundary >= pEntry->width() + margin)
					{
						// can put it into empty space before item
						m_Entries.insert(m_Entries.at(), pEntry);
						m_Entries.next();
						bInserted = true;
						if (nXE > nX0)
						{
							nX = nX0 - pEntry->width() - margin;
						}
						else if (nX < nPrevBoundary)
						{
							nX = nPrevBoundary + margin;
						}
						pEntry->move(nX, nY);
						return;
					}
					else
					{
						// insert item and shift others
						m_Entries.insert(m_Entries.at(), pEntry);
						m_Entries.next();
						bInserted = true;
						nX = nPrevBoundary + margin;
						pEntry->move(nX, nY);
						nShift = nX0 - nX - pEntry->width() - margin; // negative
					}
				}
				else if (nX0 > nXOld)
				{
					if(!nShift)
					{
						// begin shift if it's needed
						nShift = pEntry->width() + margin;
					}
					else if (nShift < 0)
					{
						// end of shifting
						return;
					}
				}

				if (nShift)
				{
					pE->move(nX0 - nShift, pE->y());
				}
				nPrevBoundary = nX1 - nShift;
			}
		}
		else // vertical
		{
			nX = nXOld; // no move on Y coord
			int nYE = nY + pEntry->height();
			
			for( QWidget *pE = m_Entries.first(); pE; pE = m_Entries.next())
			{
				int nY0 = pE->y();
				int nY1 = nY0 + pE->height();
				if (nYE < nY1 && nShift >= 0)
				{
					// put it here
					if (nY < nY0 && nY0 - nPrevBoundary >= pEntry->height() + margin)
					{
						// can put it into empty space before item
						m_Entries.insert(m_Entries.at(), pEntry);
						m_Entries.next();
						bInserted = true;
						if (nYE > nY0)
						{
							nY = nY0 - pEntry->height() - margin;
						}
						else if (nY < nPrevBoundary)
						{
							nY = nPrevBoundary + margin;
						}
						pEntry->move(nX, nY);
						return;
					}
					else
					{
						// insert item and shift others
						m_Entries.insert(m_Entries.at(), pEntry);
						m_Entries.next();
						bInserted = true;
						nY = nPrevBoundary + margin;
						pEntry->move(nX, nY);
						nShift = nY0 - nY - pEntry->height() - margin; // negative
					}
				}
				else if (nY0 > nYOld)
				{
					if(!nShift)
					{
						// begin shift if it's needed
						nShift = pEntry->width() + margin;
					}
					else if (nShift < 0)
					{
						// end of shifting
						return;
					}
				}

				if (nShift)
				{
					pE->move(pE->x(), nY0 - nShift);
				}
				nPrevBoundary = nY1 - nShift;
			}
		}
		if (bInserted)
		{
			checkButtonsAreaSize();
			buttonsAreaResized();
		}
		else
		{
			// just add it to the end
			addEntry(pEntry);  //insertEntry(pEntry, nX, nY); 
		}
	}
}


