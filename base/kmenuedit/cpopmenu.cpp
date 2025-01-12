//
//  kmenuedit
//
//  Copyright (C) 1997 Christoph Neerfeld
//  email:  Christoph.Neerfeld@home.ivm.de or chris@kde.org
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//

#include <qmenubar.h>
#include <qaccel.h>
#include <qpainter.h>
#include <qdrawutl.h>
#include <qscrbar.h>				// qDrawArrow
#include <qapp.h>

#include "cpopmenu.h"

#include "cpopmenu.moc"

/*****************************************************************************
  CPopupMenu member functions
 *****************************************************************************/

CPopupMenu::CPopupMenu( QWidget *parent, const char *name )
    : QPopupMenu( parent, name )
{
    initMetaObject();
    setMouseTracking(TRUE);
}

void CPopupMenu::mousePressEvent( QMouseEvent *e )
{
  if( !rect().contains( e->pos() ) )
    {
      return;
    }
  QPopupMenu::mousePressEvent( e);
  return;
}

void CPopupMenu::hide()
{
  releaseMouse();
  QPopupMenu::hide();
}

void CPopupMenu::setActItem( int it)
{
  actItem = it;
}

