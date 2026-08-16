// -*- C++ -*-

/* This file is part of KPanel
  
  Copyright (C) 1997 Christoph Neerfeld
  email:  Christoph.Neerfeld@home.ivm.de or chris@kde.org
  
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

/*
 * Several modifications and extensions by Matthias Ettrich <ettrich@kde.org>
 */

#include <qdstream.h>
#include <qtstream.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qapp.h>
#include <qfont.h>
#include <qdir.h>
#include <qpixmap.h>

#include <kapp.h>
#include <kiconloader.h>
#include <kfm.h>
#include <kmsgbox.h>

#include <ksimpleconfig.h>

#include <sys/stat.h>

#include "pmenu.h"
#include "pmenu.moc"

extern "C" {
#include <stdlib.h>
}

#include "mipanel.h"
#include <kaccel.h>

extern kPanel *the_panel;

static int global_id = 1;

bool ready_for_event_loop = false;

QString personal;
QString kde_apps;

//
// initialize statics
//
PMenuItem* PMenu:: menu_editor_item = 0;
QStrList* PMenu::m_pMenusToClear = 0;
bool PMenu::m_bShowEmptyFolders = true;
bool RootMenu::m_bMergePersonalGlobal = false;

static void getPaths()
{
  if (!personal) 
	{
    KConfig* config = kapp->getConfig();
    config->setGroup("KDE Desktop Entries");
    QString temp = KApplication::localkdedir()+"/share/applnk";
    personal = config->readEntry("PersonalPath", temp.data() );
    temp = KApplication::kde_appsdir();
    kde_apps = config->readEntry("Path", temp.data() );
  }
}


bool isKdelnkFile(const char* filename)
{
  QFile file(filename);
  if (file.open(IO_ReadOnly))
	{
    char s[65];
    int r = file.readBlock(s, 64);
    file.close();

    if(r == -1)
      return false;

    // terminate string
    s[r] = '\0';

    return (QString(s).find("[KDE Desktop Entry]", 0, false) != -1);
  }

  return false;
}

bool createDefaultNewFolder(QString & strPath)
{
	bool bRet = false;

	strPath += '/';
	strPath += "Folder"; // "Folder %d"

	QDir dir(strPath);
	if (!dir.exists()) 
	{
		bRet = !::mkdir(dir.absPath(), S_IRWXU);
	}
	else
	{
		strPath += ' ';
		int nInd = 1;
		do
		{
			dir.setPath(strPath + QString().setNum(nInd));
			nInd++;
		}
		while (dir.exists() && nInd < 100);
		strPath = dir.absPath();
		bRet = !::mkdir(dir.absPath(), S_IRWXU);
	}
	return bRet;
}

//
// PmenuItem class implementation
//
PMenuItem::PMenuItem()
{
  initMetaObject();
  entry_type = empty;
  sub_menu = 0;
  cmenu = 0;
  recv = 0;
  memb = 0;
  id = global_id++;
	m_nEditPopupMenuStatus = 0;
	m_nAccel = -1;
}

PMenuItem::PMenuItem( EntryType e, QString t, QString c, QString n,
		      PMenu *menu, QObject *receiver, char *member,
		      QPopupMenu *cm, bool /* ro */, QString d, QString co )
{
	initMetaObject();
	entry_type = e;
	text_name = t;
	command_name = c;
	pixmap_name = n;
	if (!pixmap_name.isEmpty())
	{
		pixmap = KApplication::getKApplication()->getIconLoader()->loadApplicationMiniIcon( pixmap_name , 16, 16);
	}
	else
	{
		// pixmap = 0;
		pixmap = QPixmap();
	}
	sub_menu = menu;
	cmenu = (myPopupMenu *) cm;
	recv = receiver;
	memb = member;
	dir_path = d;
	comment = co;
	// PI: no useless comments!
	//if (comment.isEmpty())
	//  comment = text_name;
	id = global_id++;
	m_nEditPopupMenuStatus = 0;

	// find accelerator
	m_nAccel = text_name.find('&');
	if (m_nAccel >= 0)
	{
		text_name.remove(m_nAccel, 1); 
	}

}

PMenuItem::PMenuItem( PMenuItem &item )
{
  initMetaObject();
  text_name       = item.text_name;
  pixmap_name     = item.pixmap_name;
  big_pixmap_name = item.big_pixmap_name;
  pixmap          = item.pixmap;
  entry_type      = item.entry_type;
  command_name    = item.command_name;
  comment         = item.comment;
  dir_path        = item.dir_path;
  real_name = item.real_name;
	m_nAccel = item.m_nAccel;
  if( entry_type == submenu )
	{
		sub_menu = new PMenu( *(item.sub_menu) );
		cmenu    = new myPopupMenu(sub_menu);
	}
  else
	{
		sub_menu = 0;
		cmenu    = 0;
	}
  recv         = item.cmenu;
  memb         = item.memb;
  if( entry_type == prog_com )
	{
		entry_type = unix_com;
		recv = 0;
		memb = 0;
	}
  id = global_id++;
	m_nEditPopupMenuStatus = 0;
}

PMenuItem::~PMenuItem()
{
	if( cmenu )
    delete cmenu;
  if( sub_menu )
	{
		delete sub_menu;
	}
}

bool PMenuItem::isModified()
{
	QString strName(getFullPathName());
	if (!strName.isEmpty())
	{
		struct stat filestat;
		if (stat(strName, &filestat))
		{
			return true; 
		}
		else
		{
			int nModificationTime = filestat.st_mtime;
			if (getType() == submenu)
			{
				strName += "/.directory";
				if (!stat(strName, &filestat))
				{
					nModificationTime = filestat.st_mtime;
				}
			}

			if (m_nLastModificationTime != nModificationTime)
			{ 
				return true;
			}
		}
	}
	return false;
}

short PMenuItem::parse(QFileInfo *fi, PMenu *menu)
{
	if (ready_for_event_loop)
		KApplication::getKApplication()->processEvents();
	
	// save last modification time
	{
		struct stat filestat;
		if (!stat(fi->absFilePath(), &filestat))
		{
			m_nLastModificationTime = filestat.st_mtime;
		}
	}

	real_name = fi->fileName();
	int pos = fi->fileName().find(".kdelnk");
	if( pos >= 0 )
		text_name = fi->fileName().left(pos);
	else
		text_name = fi->fileName();
	
	if (menu != 0)
	{
		QString file = fi->absFilePath();
		dir_path = fi->dirPath();
		file += "/.directory";
		QFileInfo configFI(file);
		if (configFI.exists())
		{
			{
				// save last modification time of .directory file
				struct stat filestat;
				if (!stat(file, &filestat))
				{
					m_nLastModificationTime = filestat.st_mtime;
				}
			}

			KSimpleConfig kconfig(file, true);
			kconfig.setGroup("KDE Desktop Entry");
			pixmap_name = kconfig.readEntry("MiniIcon");
			big_pixmap_name = kconfig.readEntry("Icon");
			comment = kconfig.readEntry("Comment");
			text_name = kconfig.readEntry("Name", text_name);
		}
		entry_type  = submenu;
		sub_menu    = menu;
		cmenu       = new myPopupMenu(sub_menu);
	}
	else
	{
		KSimpleConfig kconfig(fi->absFilePath(), true);
		kconfig.setGroup("KDE Desktop Entry");
		command_name = kconfig.readEntry("Exec");
		entry_type = unix_com;
		pixmap_name     = kconfig.readEntry("MiniIcon");
		big_pixmap_name = kconfig.readEntry("Icon");
		comment         = kconfig.readEntry("Comment");
		text_name = kconfig.readEntry("Name", text_name);
		dir_path        = fi->dirPath(true);
	}

	// find accelerator
	m_nAccel = text_name.find('&');
	if (m_nAccel >= 0)
	{
		text_name.remove(m_nAccel, 1); 
	}

	KIconLoader *pIconLoader = KApplication::getKApplication()->getIconLoader();
	QPixmap tmppix;
	pixmap = tmppix;
	if (!pixmap_name.isEmpty())
	{
		pixmap = pIconLoader->loadApplicationMiniIcon(pixmap_name, 16, 16);
	}
	if (pixmap.isNull() && !big_pixmap_name.isEmpty())
	{
		pixmap = pIconLoader->loadApplicationMiniIcon(big_pixmap_name, 16, 16);
	}
	if (pixmap.isNull() && getType() == unix_com)
	{
		QString tmp = real_name.copy();
		int pos = tmp.find(".kdelnk");
		if( pos >= 0 )
		tmp = tmp.left(pos);
		tmp.append(".xpm");
		pixmap = pIconLoader->loadApplicationMiniIcon(tmp, 16, 16);
	}
	
	if (pixmap.isNull())
	{
		char *strPixmap = (fi->isDir() ? "folder.xpm" : "mini-default.xpm");
		pixmap = pIconLoader->loadApplicationMiniIcon(strPixmap, 16, 16);
	}
	
	// PI: no useless comments!
	//if (comment.isEmpty())
	//  comment = text_name;
	if (big_pixmap_name.isEmpty())
	{
		QString tmp = real_name.copy();
		int pos = tmp.find(".kdelnk");
		if( pos >= 0 )
		tmp = tmp.left(pos);
		tmp.append(".xpm");
		big_pixmap_name = tmp.copy();
	}
	
	return 0;
}

short PMenuItem::parse( QString abs_file_path )
{
  QFileInfo fi(abs_file_path);
  return parse(&fi);
}

void PMenuItem::addFolderToRecentList()
{
  PFileMenu::updateRecentFolders(getDirPath());
}


void PMenuItem::exec()
{
	QDir d(getDirPath());
	QString cpath(d.canonicalPath() + "/" + real_name);
	
	if (!ready_for_event_loop)
	{ 
		// hack: avoid doing this when the menu is not fully parsed.
		// otherwise PFileMenu::updateRecentFiles will segfault (Matthias)
		
		// NOTE: Update the Recent section if the entry is not already in
		// the Global or Local section;
		// However, to perform this check PMenuItem should have a pointer to
		// its container (PMenu);
		bool bChanged = PFileMenu::updateRecentFiles(cpath);
		
		// update recent part of Kmenu
		the_panel->updateRecentMenuItems(cpath, bChanged);
	}
	
	KFM* kfm = new KFM;
	kfm->exec(cpath, 0L);
	delete kfm;
}

QString PMenuItem::getSaveName()
{
	getPaths();
	QString temp = getFullPathName();
	if (temp == personal || temp == personal + "/")
	{
		temp = "$$PERSONAL";
	}
	else if (temp.left(personal.length()) ==  personal)
	{
		// kdelnk file is in $HOME/Personal
		temp = temp.right(temp.length()-personal.length());
		temp.prepend("$$PERSONAL");
	}
	else if (temp.left(kde_apps.length()) == kde_apps)
	{
		// kdelnk file is in $KDEDIR/apps
		temp = temp.right(temp.length()-kde_apps.length());
		temp.prepend("$$KDEAPPS");
	}
	
	return temp;
}

bool PMenuItem::writeConfig( QDir dir )
{
  QString file = dir.absPath();
  dir_path = file.copy();
  file += ( (QString) "/" + real_name );
  QFile config(file);
  if( !config.open(IO_ReadWrite) )
    return FALSE;
  config.close();
  KConfig kconfig(file);
  kconfig.setGroup("KDE Desktop Entry");
	if (m_nAccel < 0) 
	{
		kconfig.writeEntry("Name", text_name );
	}
	else
	{
		QString s(text_name);
		s.insert(m_nAccel, '&'); 
		kconfig.writeEntry("Name", s);
	}
  kconfig.writeEntry("Exec", command_name );
  kconfig.writeEntry("Type", "Application");
  kconfig.writeEntry("Icon", big_pixmap_name );
  kconfig.writeEntry("MiniIcon", pixmap_name );
  kconfig.sync();
  return TRUE;
}

#define INCLUDE_MENUITEM_DEF
#include <qmenudata.h>
void PMenuItem::insertIntoQMenu(QPopupMenu *menu)
{
	if (m_nAccel < 0) 
	{
		menu->insertItem(pixmap, text_name, cmenu, getId());
	}
	else
	{
		QString s(text_name);
		s.insert(m_nAccel, '&'); 
		menu->insertItem(pixmap, s, cmenu, getId());
	}
}

//------------------------------------------------------------------------------

PMenu::PMenu() : parentItem(0)
{
  initMetaObject();
  list.setAutoDelete(true);
  cmenu = 0;
  altSort = false;
  parsed = true;
}

PMenu::PMenu( PMenu &menu ) : parentItem(0)
{
  initMetaObject();
  cmenu = 0;
  list.setAutoDelete(true);
  PMenuItem *item, *new_item;
  for( item = menu.list.first(); item != 0; item = menu.list.next() )
    {
      new_item = new PMenuItem( *item );
      list.append(new_item);
    }
  altSort = false;
  parsed = menu.parsed;
}

PMenu::~PMenu()
{
  list.clear();
}

void PMenu::createMenu( QPopupMenu *menu, kPanel *panel, bool add_button/*= FALSE*/)
{
	if (cmenu != menu)
	{
		cmenu = (myPopupMenu *)menu;
		menu->installEventFilter(panel->m_pKMenuEventFilter);
		connect( menu, SIGNAL(highlighted(int)), this, SLOT(highlighted(int)) );
		if (add_button)
			connect( menu, SIGNAL(aboutToShow()), this, SLOT(aboutToShowAddMenu()) );
		else
			connect( menu, SIGNAL(aboutToShow()), this, SLOT(aboutToShow()) );
	}

	for (PMenuItem *item = list.first(); item != 0; item = list.next())
	{
		switch (item->getType()) 
		{
			case separator:
				menu->insertSeparator();
				continue;

			case submenu:
				item->sub_menu->parentItem = item;
				if( add_button )
				{
					item->cmenu->setFont(menu->font());
					item->cmenu->insertItem(item->pixmap, item->text_name, item->getId());
					item->cmenu->connectItem(item->getId(), item, SLOT(execAddButton()) );
					connect( item, SIGNAL(addButton(PMenuItem*)),
								 (QObject *) panel, SLOT(addButton(PMenuItem*)) );
					connect( item->cmenu, SIGNAL(highlighted(int)), this, SLOT(highlighted(int)) );
					item->cmenu->insertSeparator();
					// create submenu
					item->sub_menu->createMenu( item->cmenu, panel, true );
					item->insertIntoQMenu(menu);
					connect( item, SIGNAL(showToolTip(QString)), panel->m_pToolTip,
						 SLOT(showToolTip(QString)) );
				}
				else
				{
					// Hide certain sub-dirs from the menu
					if (m_pMenusToClear)
					{
						bool bFind = false;
						char *strMenu2Clear = m_pMenusToClear->first();
						while (strMenu2Clear)
						{
							if (strMenu2Clear == item->getFullPathName())
							{
								bFind = true;
								break;
							}
							strMenu2Clear = m_pMenusToClear->next();
						} 	
					
						if (bFind)
							continue;
					}
			
					item->cmenu->setFont(menu->font());
					item->sub_menu->createMenu( item->cmenu, panel );
					item->insertIntoQMenu(menu);
					connect( item, SIGNAL(showToolTip(QString)), panel->m_pToolTip,
						 SLOT(showToolTip(QString)) );
				}
				continue;

			case dirbrowser:
				item->cmenu->setFont(menu->font());
				item->sub_menu->createMenu( item->cmenu, panel );
				item->insertIntoQMenu(menu);
				((PFileMenu*)(item->sub_menu))->id = item->getId();
				
				item->cmenu->id = item->getId();
				
				item->sub_menu->parentItem = item;
				
				connect( item, SIGNAL(showToolTip(QString)), panel->m_pToolTip,
					SLOT(showToolTip(QString)) );
				continue;

			case url:
				item->insertIntoQMenu(menu);
				connect( item, SIGNAL(showToolTip(QString)), panel->m_pToolTip,
					 SLOT(showToolTip(QString)) );

				menu->connectItem(item->getId(), item, SLOT(exec()));
				// NOTE: only files accessed through DiskNavigator menus update the
				// recent folders.
				
				menu->connectItem(item->getId(), item, SLOT(addFolderToRecentList()));
				
				continue;

			case label:
				item->insertIntoQMenu(menu);
				connect( item, SIGNAL(showToolTip(QString)), panel->m_pToolTip,
					 SLOT(showToolTip(QString)) );
				continue;

      case unix_com:
				item->insertIntoQMenu(menu);
				if (add_button)
				{
					menu->connectItem( item->getId(), item, SLOT(execAddButton()) );
					connect(item, SIGNAL(addButton(PMenuItem *)), panel, SLOT(addButton(PMenuItem*)));
				}
				else
				{
					//if( !item->command_name.isEmpty() )
					menu->connectItem( item->getId(), item, SLOT(exec()) );
				}
				connect( item, SIGNAL(showToolTip(QString)), panel->m_pToolTip,
					 SLOT(showToolTip(QString)) );
				continue;

			case prog_com:
				item->insertIntoQMenu(menu);
				if( add_button )
					{
						menu->connectItem( item->getId(), item, SLOT(execAddButton()) );
						connect( item, SIGNAL(addButton(PMenuItem *)), panel,
							 SLOT(addButton(PMenuItem*)) );
					}
				else
					menu->connectItem(item->getId(), item->recv, item->memb);
				connect( item, SIGNAL(showToolTip(QString)), panel->m_pToolTip,
					 SLOT(showToolTip(QString)) );
				continue;

			case add_but:
				item->cmenu->setFont(menu->font());
				// create submenu
				item->sub_menu->createMenu( item->cmenu, panel, true );
				item->insertIntoQMenu(menu);
				item->cmenu->installEventFilter(panel->m_pKMenuEventFilter);
				connect( item, SIGNAL(showToolTip(QString)), panel->m_pToolTip,
					 SLOT(showToolTip(QString)) );
				connect( item->cmenu, SIGNAL(highlighted(int)), this,
					 SLOT(highlighted(int)) );
				continue;

			case empty:
				continue;
		};
	}
	return;
}

void PMenu::add (PMenuItem *item)
{
  list.append(item);
}

void PMenu::insert( PMenuItem *item, int index )
{
	if (index < 0 || index > (int)list.count())
		list.append(item);
	else
		list.insert(index, item);
}

short PMenu::parseLazy(const QDir & d )
{
  parsed = false;

  const QFileInfoList *list = d.entryInfoList();
  if (!m_bShowEmptyFolders && list->count() < 3)
		return -1;
  else
    return 0;
}

short PMenu::parse(QDir d, bool bParseSubdirs/*=false*/, PMenuItem *pInsertBefore/*= 0*/)
{
	if (!d.exists())
		return -1;
	
	parsed = true;
	PMenuItem *new_item;
	PMenu *new_menu;
	QDir new_dir;

	QStrList sort_order;
	sort_order.setAutoDelete(true);
	QList<PMenuItem> item_list;
	item_list.setAutoDelete(false);
	
	if (altSort == true)
		d.setSorting(QDir::DirsFirst | QDir::Name);
	
	QString file(d.path());
	file += "/.directory";
	QFileInfo config(file);
	if (config.exists())
	{
		KSimpleConfig kconfig(file, true); // kalle
		kconfig.setGroup("KDE Desktop Entry");
		kconfig.readListEntry("SortOrder", sort_order);
	}
	
	const QFileInfoList *pFileInfoList = d.entryInfoList();
	QFileInfoListIterator it( *pFileInfoList );
	if( it.count() < 3 )
	{
		return (m_bShowEmptyFolders ? 0 : -1);
	}

	QFileInfo *fi;
	while ((fi = it.current()))
	{
		if( fi->fileName() == "." || fi->fileName() == ".." )
		{ 
			++it; 
			continue; 
		}

		if (fi->isDir() && fi->isReadable())
		{
			new_menu = new PMenu;
			new_menu->setAltSort(altSort);
			new_item = new PMenuItem;
			new_dir.setPath( fi->filePath() );
			int nRet = (bParseSubdirs ? new_menu->parse(new_dir, true) : new_menu->parseLazy( new_dir ));
			if (nRet < 0 || new_item->parse( fi, new_menu ) < 0)
			{
				delete new_menu;
				delete new_item;
				new_item = 0;
				++it;
				continue;
			}
		}
		else
		{
			QString strFullPath(fi->absFilePath());
			if (!isKdelnkFile(strFullPath))
			{ 
				++it; 
				continue; 
			}

			// can this entry be copied by MiMYtype editor
			if (skipDuplicatedEntry(strFullPath, fi))
			{
				++it;
				continue;
			}

			new_item = new PMenuItem;
			if (new_item->parse(fi) < 0)
			{
				delete new_item;
				new_item = 0;
			}
		}
		
		if (new_item)
		{
			PMenuItem *tmp = item_list.first();
			
			if ( altSort == true && int(new_item->getType() ) > submenu)
				for ( ;tmp && int(tmp->getType()) <= submenu; tmp = item_list.next());
			
			for ( ;tmp && stricmp( tmp->text_name, new_item->text_name.data() ) <= 0 ;tmp = item_list.next());
			
			if ( !tmp )
				item_list.append( new_item );
			else
				item_list.insert( item_list.at(), new_item );
		}
		++it;
	}

	int nInsertPos = (pInsertBefore ? list.findRef(pInsertBefore) : -1);
	int nPos = nInsertPos;
	PMenuItem *item;
#ifdef CAN_SORT_ITEMS
	// sort items
	for (const char *item_name = sort_order.first(); item_name; item_name = sort_order.next())
	{
		for (item = item_list.first(); item != 0; item = item_list.next())
		{
			if (item->real_name == item_name)
			{
				insert(item, nPos);
				if (nPos >= 0)
					nPos++;
				item_list.removeRef(item);
				break;
			}
		}
	}
#endif
	if (item_list.count() > 0)
	{
		for (item = item_list.first(); item != 0; item = item_list.next())
		{ 
			insert(item, nPos);
			if (nPos >= 0)
				nPos++;
		}
	}
	item_list.clear();

#if CAN_INSERT_SEPARATORS
	// insert separators
	sort_order.first();
	while ((nPos = sort_order.findNext((QString)"SEPARATOR")) >= 0)
	{
		sort_order.next();
		new_item = new PMenuItem;
		new_item->entry_type = separator;
		insert(new_item, nInsertPos + nPos);
	}
#endif
	return 0;
}

//
// merges directory contents into menu between items
//
bool PMenu::mergeInto(QDir dir, PMenuItem *pHeadBnd/*= 0*/, PMenuItem *pTailBnd/*= 0*/)
{
	if (!dir.exists())
		return false;
	
	if (m_strMergedDir.isEmpty())
		m_strMergedDir = dir.absPath();

	if (altSort == true)
		dir.setSorting(QDir::DirsFirst | QDir::Name);

	const QFileInfoList *fileinfolist = dir.entryInfoList();
	QFileInfoListIterator it( *fileinfolist );
	if (it.count() < 3)
		return true;

	int nInd = (pHeadBnd ? list.findRef(pHeadBnd) + 1 : 0); //set once!

	QFileInfo *fi;
	while ((fi = it.current()))
	{
		if (fi->fileName() == "." || fi->fileName() == "..")
		{ 
			// do nothing
			++it;
			continue;
		}

		PMenuItem *new_item = 0;
		if (fi->isDir() && fi->isReadable())
		{
			PMenu *new_menu = new PMenu;
			new_menu->setAltSort(altSort);
			new_item = new PMenuItem;
			QDir subd(fi->filePath());
			if (new_menu->parseLazy(subd) < 0 || new_item->parse( fi, new_menu ) < 0)
			{
				if (new_item->getMenu() != new_menu)
					delete new_menu;
				delete new_item;
				new_item = 0;
			}
			else
			{
				// can already have same directory !
				for (; nInd < numElements(); nInd++)
				{
					PMenuItem *item = at(nInd);
					if (item == pTailBnd)
					{
						break;
					}
					else if (item->getType() > submenu || item->text_name > new_item->text_name)
					{
//printf("skipped %d %s for %s\n", nInd, (const char *)item->text_name, (const char *)new_item->text_name);
						break;
					}
					else if (item->text_name == new_item->text_name)
					{
						// found submenu with same name - no need to add one more
						delete new_item;
						new_item = 0;

						if ((new_menu = item->getMenu()))
							new_menu->mergeInto(subd);

						break;
					}
				}
			}
		}
		else
		{
			if (isKdelnkFile(fi->absFilePath()))
			{ 
				new_item = new PMenuItem;
				if (new_item->parse(fi) < 0)
				{
					delete new_item;
					new_item = 0;
				}
			}
		}
		
		if (new_item)
		{
			int i;
			for (i = nInd; i < numElements(); i++)
			{
				PMenuItem *item = at(i);
				if (item == pTailBnd)
				{
//printf("end %s\n", (const char *)new_item->text_name);
					break;
				}
				else if (new_item->getType() == submenu)
				{
					if (item->getType() > submenu || (const char *)new_item->text_name < item->text_name)
					{
						// can insert
						break;
					}
				}
				else // not a submenu
				{
					if (item->getType() > submenu && (const char *)new_item->text_name < item->text_name)
					{
						// can insert
						break;
					}
				}
			}
//printf("insert at %d %s\n", nInd, (const char *)new_item->text_name);
			insert(new_item, i);
		}
		++it;
	}

	return true;
}

void PMenu::move(short item_id, short new_pos)
{
  PMenuItem *item;
  if( item_id > new_pos )
    {
      item = list.take(item_id);
      list.insert( new_pos, item);
    }
  else
    {
      item = list.take(item_id);
      list.insert( new_pos - 1, item);
    }
}

void PMenu::remove(PMenuItem *item)
{
  if (item)
		list.removeRef(item);
}

void PMenu::create_pixmap( QPixmap &buf, PMenuItem *item, QPopupMenu *menu)
{
  int w, h;
  QPainter p;
  QFontMetrics fm = menu->fontMetrics();   // size of font set for this widget

  w  = 2 + item->pixmap.width() + 4 + fm.width( item->text_name ) + 2;
  h = ( item->pixmap.height() > fm.height() ? item->pixmap.height() : fm.height() ) + 4;

  buf.resize( w, h );                    // resize pixmap
  buf.fill( menu->backgroundColor() );   // clear it

  p.begin( &buf );
  p.drawPixmap( 2, 2, item->pixmap );              // use 2x2 border
  p.setFont( menu->font() );
  /*
  p.drawText( 2 + item->pixmap.width() + 4,        // center text in item
	      (h + fm.height()) / 2 - fm.descent(),
	      item->text_name );
	      */
  p.drawText( 2 + item->pixmap.width() + 4,        // center text in item
	      0, w, h,
	      AlignVCenter | ShowPrefix | DontClip | SingleLine,
	      item->text_name );
  p.end();
}

// search for kdelnk-file as a PMenuItem inside this PMenu hierarchy
// if it can't find the file it will return a new created PMenuItem
//
PMenuItem * PMenu::searchItem(QString name)
{
  getPaths();
  PMenuItem *found_item = 0L;
  PMenuItem *item;
  QString path;
  static PMenu* hack = 0;
  if (!hack)
    hack = this;
  //debug("searchName = %s", (const char *) name );
  name = name.stripWhiteSpace();
  if( name.left(9) == "$PERSONAL" )
	{
		// in $HOME/Personal
		name.remove(0, 9);
		name.prepend(personal);
	}
  else if( name.left(8) == "$KDEAPPS" )
	{
		// in $KDEDIR/apps
		name.remove(0, 8);
		name.prepend(kde_apps);
	}
  path = name.left( name.findRev('/') );

  if( list.first() && list.first()->dir_path != path )
	{
		// can't be inside of this menu, so search only for submenus
		for( item = list.first(); item != 0L; item = list.next() )
	{
	  if( item->getFullPathName() == name)
	    return item;
	  if( item->cmenu != 0L )
	    {
	     if( ( found_item = item->sub_menu->searchItem(name) ) )
	       return found_item;
	    }
	}
    }
  else
    {
      // should be inside of this menu
      for( item = list.first(); item != 0L; item = list.next() )
	{
	  if( item->getFullPathName() == name )
	    return item;
	}
    }

  if (hack == this)
    hack = 0;

  PMenuItem *pmi = 0;

  if (!hack){
    if (isKdelnkFile(name)){
      // generate a free entry
      pmi = new PMenuItem(unix_com);
      pmi->parse(name);
    } else {
      QFileInfo i(name);
      if (i.isDir()) {
	PMenu *dir = new PMenu();
	dir->parse(QDir(name));
	pmi = new PMenuItem(submenu);
	pmi->parse(&i, dir);
      } else {
	// create a new kdelnk-file on the fly
	if( i.isExecutable() ) {
	  pmi = new PMenuItem(unix_com,        // type
			      i.fileName(),    // name
			      i.absFilePath()  // command
			      );
	  QPixmap tmp = KApplication::getKApplication()->getIconLoader()->
	    loadApplicationIcon(i.baseName() + ".xpm");
	  if( tmp.isNull() )
	    pmi->big_pixmap_name = "exec.xpm";
	  else
	    pmi->big_pixmap_name = i.baseName() + ".xpm";
	  pmi->pixmap_name = "";
	  QString dir_name = KApplication::localkdedir() + "/share/apps/kpanel/applnk";
	  pmi->real_name = uniqueFileName(i.fileName(), dir_name );
	  if( pmi->real_name != 0 ) {
	    if( !pmi->writeConfig(QDir(dir_name))) {
	      delete pmi;
	      pmi = 0;
	    }
	  } else {
	    delete pmi;
	    pmi = 0;
	  }
	}
      }
    }
  }
  return pmi;
}

QString PMenu::uniqueFileName(QString name, QString dir_name)
{
	QDir dir(dir_name);
	if( !dir.exists() ) 
	{
		return 0;
	}
	
	QString file_name = name.simplifyWhiteSpace();
	while( file_name.contains('/') )
	{
		file_name.replace(file_name.find('/'), 1, "_");
	}
	QString suffix;
	int i = 2;
	file_name = name + ".kdelnk";
	while( dir.exists(file_name) )
	{
		file_name = name + suffix.setNum(i) + ".kdelnk";
		i++;
	}
	return file_name;
}

bool PMenu::addFromDrop(QString name, PMenuItem *pHeadBnd/*= 0*/, PMenuItem *pTailBnd/*= 0*/)
{
	QString dir_name(personal);
	QFileInfo fi(name);
	
	PMenuItem *pmi = 0;
	bool bSuccess = false;
	if (fi.isDir()) 
	{
		QString strPath(dir_name + '/' + fi.fileName());

		PMenu *dirMenu = new PMenu();
		if (dirMenu->checkWritablePath(strPath))
		{
			// copy items from name into strPath
			// TODO!!!!

			dirMenu->parse(QDir(strPath));
			pmi = new PMenuItem(submenu);
			fi.setFile(strPath);
			pmi->parse(&fi, dirMenu);
			bSuccess = true;
		}
		else
		{
			delete dirMenu;
		}
	}
	else
	{
		QString real_name = uniqueFileName(fi.baseName(), dir_name);
		if (isKdelnkFile(name))
		{
			// make a copy of the kdelnk-file
			pmi = new PMenuItem(unix_com);
			pmi->parse(&fi);
			bSuccess = copyFiles(pmi->dir_path + '/' + pmi->real_name, dir_name + '/' + real_name);
		}
		else if (fi.isExecutable())
		{
			// create a new kdelnk-file on the fly
			PMenuItem *pmi = new PMenuItem(unix_com, fi.fileName(), fi.absFilePath()); // type, name, command
			QString strPixmapName(fi.baseName() + ".xpm");
			KIconLoader *pLoader = KApplication::getKApplication()->getIconLoader();
			pmi->pixmap = pLoader->loadApplicationMiniIcon(strPixmapName, 16, 16);
			if (pmi->pixmap.isNull())
			{
				pmi->pixmap = pLoader->loadApplicationMiniIcon("mini-default.xpm", 16, 16);
				pmi->pixmap_name = "";
			}
			else
			{
				pmi->pixmap_name = strPixmapName;
			}
			QPixmap tmp = pLoader->loadApplicationIcon(strPixmapName);
			if( tmp.isNull() )
				pmi->big_pixmap_name = "exec.xpm";
			else
				pmi->big_pixmap_name = strPixmapName;
			pmi->real_name = real_name;

			if (pmi->real_name && pmi->writeConfig(QDir(dir_name)))
			{
				bSuccess = true;
			}
		}
	}

	if (!bSuccess)
	{
		delete pmi;
		pmi = 0;
	}

	if (pmi)
	{
		if (!pTailBnd)
		{
			// no user items before!
			// add tail separator
			int nInd = (pHeadBnd ? list.findRef(pHeadBnd) : -1);
			insert(pmi, nInd + 1);
			insert(new PMenuItem(separator), nInd + 2);
		}
		else // insert somwhere between
		{
			int nIndLast = list.findRef(pTailBnd);
			if (getAltSort())
			{
				int nInd = (pHeadBnd ? list.findRef(pHeadBnd) + 1 : 0);
				const char *szName = (const char *)pmi->text_name;
				for (; nInd < numElements(); nInd++)
				{
					PMenuItem *item = at(nInd);
					if (item == pTailBnd)
						break;

					if (pmi->getType() == submenu)
					{
						if (item->getType() > submenu || szName < item->text_name)
						{
							// can insert
							break;
						}
					}
					else // not a submenu
					{
						if (item->getType() > submenu && szName < item->text_name)
						{
							// can insert
							break;
						}
					}
				}
				
				// insert item
				nIndLast = nInd;
			}


			if (nIndLast >= 0)
				insert(pmi, nIndLast); // before
		}
		
		clearSubmenus();
		createMenu(cmenu, the_panel, FALSE);
	}
	return bSuccess;
}

bool PMenu::copyFiles( QString source, QString dest )
{
  char data[1024];
  QFile in(source);
  QFile out(dest);
  if( !in.open(IO_ReadOnly) )
    return FALSE;
  if( !out.open(IO_WriteOnly) )
    return FALSE;
  int len;
  while( (len = in.readBlock(data, 1024)) > 0 )
    {
      if( out.writeBlock(data,len) < len )
        {
          len = -1;
          break;
        }
    }
  out.close();
  in.close();
  if( len == -1 )
    { // abort and remove destination file
      QFileInfo fi(dest);
      fi.dir().remove(dest);
      return FALSE;
    }
  return TRUE;
}

void PMenu::clearSubmenus()
{
	if (cmenu)
		cmenu->clear();

	for (PMenuItem *item = list.first(); item != 0; item = list.next())
	{
		if (item->cmenu)
			item->sub_menu->clearSubmenus();
	}
}

PMenuItem * PMenu::searchItem(int id)
{
	PMenuItem *item;
	if (id >= 0)
	{
		for (item = list.first(); item && item->getId() != id; item = list.next());
	}
	else
	{
		item = 0;
	}
  return item;
}

#ifdef DISKNAV_DEBUG

PMenuItem * PMenu::searchItem(PMenu* _item)
{
  PMenuItem *item;
  for( item = list.first(); item != 0 && item->sub_menu != _item;
       item = list.next() );

  return item;
}
#endif

void PMenu::highlighted( int id )
{
	for (PMenuItem *item = list.first(); item; item = list.next())
	{
		if (item->getId() == id)
		{
			item->highlighted();
			break;
		}
	}
}

void PMenu::aboutToShow()
{
	parseBeforeShowing(false);
}

void PMenu::aboutToShowAddMenu()
{
  parseBeforeShowing(true);
}

void PMenu::parseBeforeShowing(bool is_add_menu)
{
	if (parsed)
	{
		// check contents
		bool bChanged = false;

		if (parentItem)
		{
			for (int i = 0; i < count(); i++)
			{
				PMenuItem *item = at(i);
				// this is enough - inserts is being processed in other place
				if (item->isModified())
				{
					bChanged = true;
					break;
				}
			}
		}

		if (bChanged)
		{
			update();
		}
		return;
	}

	update();
}

bool PMenu::deleteItem(int nId)
{
	return removeItem(nId, "trash:/");
}

bool PMenu::removeItem(int nId, const char *szMoveToDir)
{
//printf("remove item!!!\n");
	// remove item from menu 
	PMenuItem * item = searchItem(nId);
	if (item)
	{
		QString strName(item->getFullPathName());
		remove(item); // can't use item after this
		item = 0;

		// remove item from a popup menu
		QPopupMenu *popup = getQPopupMenu();
		if (popup)
		{
//		int n = popup->indexOf(nId);
			bool bVis;
			if ((bVis = popup->isVisible()))
				popup->hide();
			popup->setActiveItem(0);
//			QMenuItem *data = popup->findItem(nId);
//				data1->popup();
//				printf("data %d %d\n", (int)data1, (int)data1->pixmap());
			popup->removeItem(nId);
			if (bVis)
				popup->show();
		}
		
		if (szMoveToDir)
		{
			// move link file or directory into dest directory (can be trash)
			KFM *kfm = new KFM();
			kfm->moveClient(strName, szMoveToDir);
			delete kfm;
		}
		else
		{
			unlink(strName);
		}
		return true;
	}
	return false;
}

bool PMenu::insertItem(PMenu *pMenu, int nId, bool bMove)
{
	if (!pMenu || pMenu == this)
		return false;

	PMenuItem * item = pMenu->searchItem(nId);
	if (item)
	{
		QString strPath;
		if (!checkWritablePath(strPath))
		{
			// can't create writable path
			return false;
		}

		// insert into the menu 
		QString strItemSrcPath(item->getFullPathName());
		item = new PMenuItem(*item);
		if (item)
		{
			item->dir_path = strPath;
			int nPos;
			for (nPos = 0; nPos < numElements(); nPos++)
			{
				PMenuItem *pMenuItem = at(nPos);
				if (item->getType() == submenu)
				{
					if (pMenuItem->getType() > submenu || (const char *)item->text_name < pMenuItem->text_name)
					{
						// can insert
						break;
					}
				}
				else // not a submenu
				{
					if (pMenuItem->getType() > submenu && (const char *)item->text_name < pMenuItem->text_name)
					{
						// can insert
						break;
					}
				}
			}
			insert(item, nPos);

			// insert into popup
			QPopupMenu *popup = getQPopupMenu();
			popup->insertItem(*item->getPixmap(), item->getText(), item->getId(), nPos);
			connect(item, SIGNAL(showToolTip(QString)), the_panel->m_pToolTip, SLOT(showToolTip(QString)));
			popup->connectItem(item->getId(), item, SLOT(exec()));
	
			// check that name is unigue otherwise create a new one
			QString strDestPath(item->getDirPath());
			strDestPath += '/';
			QFileInfo fi(strDestPath + item->real_name);
			if (fi.exists())
			{
				QString strFmt(item->real_name);
				int nExt = strFmt.findRev(".kdelnk");
				if (nExt > 0)
					strFmt.truncate(nExt);
				strFmt += " (%d)";
				if (nExt)
					strFmt += ".kdelnk";

				int nInd = 1;
				QString strNewName;
				do
				{
					strNewName.sprintf((const char  *)strFmt, nInd);
					fi.setFile(strDestPath + strNewName);
					nInd++;
				}
				while(fi.exists() && nInd < 100);

				item->real_name = strNewName;
				strDestPath += strNewName;
			}

			// copy or move actual source file
			if (bMove)
			{
				// move item 
				pMenu->removeItem(nId, strDestPath);	
			}
			else // copy
			{
				KFM *kfm = new KFM();
				kfm->copyClient(strItemSrcPath, strDestPath);
				delete kfm;
			}
			return true;
		}
	}
	return false;
}

//
// the current implementation is simple but not the most efficient
// - just reloads everything
//
void PMenu::update()
{
	bool bContentsChanged = true;
	QDir dir;

	clearSubmenus();
	list.clear();

	for (int nStep = 0; nStep < 2; nStep++)
	{
		if (!nStep && parentItem)
		{
			dir.setPath(parentItem->getFullPathName());
		}
		else if(nStep && !m_strMergedDir.isEmpty()) 
		{
			dir.setPath(m_strMergedDir);
		}
		else
		{
			continue;
		}

		if (!dir.exists())
		{
			continue;
		}
	
		if (altSort)
			dir.setSorting(QDir::DirsFirst | QDir::Name);

		const QFileInfoList *fileinfolist = dir.entryInfoList();
		QFileInfoListIterator it( *fileinfolist );
		if (it.count() < 3)
			continue;

		QFileInfo *fi;
		while ((fi = it.current()))
		{
			if (fi->fileName() == "." || fi->fileName() == "..")
			{ 
				// do nothing
				++it;
				continue;
			}

			int nInd = 0;
			PMenuItem *new_item = 0;
			if (fi->isDir() && fi->isReadable())
			{
				PMenu *new_menu = new PMenu;
				new_menu->setAltSort(altSort);
				new_item = new PMenuItem;
				QDir subd(fi->filePath());
				if (new_menu->parseLazy(subd) < 0 || new_item->parse( fi, new_menu ) < 0)
				{
					if (new_item->getMenu() != new_menu)
						delete new_menu;
					delete new_item;
					new_item = 0;
				}
				else
				{
					// can already have directory with same name !
					for (; nInd < numElements(); nInd++)
					{
						PMenuItem *item = at(nInd);
						if (item->getType() > submenu || item->text_name > new_item->text_name)
						{
	//printf("skipped %d %s for %s\n", nInd, (const char *)item->text_name, (const char *)new_item->text_name);
							break;
						}
						else if (item->text_name == new_item->text_name)
						{
							// found submenu with same name - no need to add one more
							delete new_item;
							new_item = 0;
	
							if ((new_menu = item->getMenu()))
								new_menu->mergeInto(subd);
	
							break;
						}
					}
				}
			}
			else
			{
				QString strFullPath(fi->absFilePath());
				if (isKdelnkFile(strFullPath))
				{ 
					// can this entry be copied by MiMYtype editor
					if (skipDuplicatedEntry(strFullPath, fi, true)) // fi can change its file
					{
						++it;
						continue;
					}

					new_item = new PMenuItem;
					if (new_item->parse(fi) < 0)
					{
						delete new_item;
						new_item = 0;
					}
				}
			}
			
			if (new_item)
			{
				int i;
				for (i = nInd; i < numElements(); i++)
				{
					PMenuItem *item = at(i);
					if (new_item->getType() == submenu)
					{
						if (item->getType() > submenu || (const char *)new_item->text_name < item->text_name)
						{
							// can insert
							break;
						}
					}
					else // not a submenu
					{
						if (item->getType() > submenu && (const char *)new_item->text_name < item->text_name)
						{
							// can insert
							break;
						}
					}
				}
	//printf("insert at %d %s\n", nInd, (const char *)new_item->text_name);
				insert(new_item, i);
			}
			++it;
		}
	}

	if (bContentsChanged)
	{
		createMenu(cmenu, the_panel, FALSE);
	}

	return;
}

bool PMenu::checkWritablePath(QString & strPath)
{
	bool bRet = true;
	
	if (m_strMergedDir.isEmpty())
	{
		if (parentItem)
			strPath = parentItem->getFullPathName();
		else if (strPath.isEmpty())
			return false;
	}
	else
		strPath = m_strMergedDir;

	QFileInfo fi(strPath);
	if (!fi.exists())
	{
		// path does not exist - create it
		bRet = !::mkdir(strPath, S_IRWXU);
	}
	else if (!fi.isWritable())
	{
		// if this is global path then can creatre local path and merge it with global
		bRet = false;
		if (RootMenu::isPersonalGlobalMerged())
		{
			if (strPath.left(kde_apps.length()) == kde_apps)
			{
				strPath = personal + strPath.right(strPath.length() - kde_apps.length());
				if (!::mkdir(strPath, S_IRWXU))
				{
					m_strMergedDir = strPath;
					bRet = true; 
				}
			}
		}
	}
	return bRet;
}

void PMenu::createNewFolder()
{
	QString strPath;
	if (checkWritablePath(strPath))
	{
		if (::createDefaultNewFolder(strPath))
		{
			QPopupMenu *popup = getQPopupMenu();		
			bool bVis = (popup && popup->isVisible());
			if (bVis)
			{
				popup->hide();
			}
			update();
			if (bVis)
			{
				popup->show();
			}
		}
	}
}

bool PMenu::isSameAppKdelnk(const char *szGlobalFile, const char *szLocalFile)
{
	bool bRet = false;

	KSimpleConfig globalCtg(szGlobalFile, true);
	globalCtg.setGroup("KDE Desktop Entry");
	if (globalCtg.readEntry("Type") == "Application")
	{
		KSimpleConfig localCtg(szLocalFile, true);
		localCtg.setGroup("KDE Desktop Entry");
		if (localCtg.readEntry("Type") == "Application")
		{
			QString str(globalCtg.readEntry("Name"));
			// compare names
			if (!str.isEmpty() && str == localCtg.readEntry("Name"))
			{
				str = globalCtg.readEntry("Exec");
				// compare executables
				if (!str.isEmpty() && str == localCtg.readEntry("Exec"))
				{
					str = globalCtg.readEntry("Terminal");
					// compare executables
					if (str == localCtg.readEntry("Terminal"))
					{
						bRet = true;
					}
				}
			}
		}
	}
	return bRet;
}

bool PMenu::skipDuplicatedEntry(QString &strFullPath, QFileInfo *fi, bool bNoLocalCheck/*= false*/)
{
	bool bSkip = false;

	// because MIME type editor can copy entry from Global into Local directory 
	// there are following cases:
	// - if there is a local entry equal to global then show local in global
	// and don't show global at all
	if (!strncmp(strFullPath, kde_apps, kde_apps.length()))
	{
		QString strLocalFullPath(personal);
		strLocalFullPath += strFullPath.right(strFullPath.length() - kde_apps.length());
		QFileInfo fInfo(strLocalFullPath);
		if (fInfo.exists() && fInfo.isFile())
		{
			// now it's time to compare these two files
			if (isSameAppKdelnk(strFullPath, strLocalFullPath))
			{
				if (RootMenu::isPersonalGlobalMerged())
				{ 
					// no need to add local - it will be merged anyway
					bSkip = true;
				}
				else
				{
					// add local file as global
					fi->setFile(strLocalFullPath);
				}
			}
		}
	}
	else if (!bNoLocalCheck && !strncmp(strFullPath, personal, personal.length()))
	{
		QString strGlobalFullPath(kde_apps);
		strGlobalFullPath += strFullPath.right(strFullPath.length() - personal.length());
		QFileInfo fInfo(strGlobalFullPath);
		if (fInfo.exists() && fInfo.isFile())
		{
			// now it's time to compare these two files
			if (isSameAppKdelnk(strGlobalFullPath, strFullPath))
			{
				// don't add local file at all
				bSkip = true;
			}
		}
	}
	return bSkip;
}

// 
// class RootMenu implementation
//
RootMenu::RootMenu() :
	PMenu(), m_pPersonalHeadBnd(0), m_pPersonalTailBnd(0), m_pGlobalTailBnd(0)
{
}

bool RootMenu::mergePersonalEntries(QDir dir)
{
	return mergeInto(dir, m_pPersonalHeadBnd, m_pPersonalTailBnd);
}

bool RootMenu::addPersonalFromDrop(QString a)
{
	bool bRet = addFromDrop(a, m_pPersonalHeadBnd, m_pPersonalTailBnd);
	if (!m_pPersonalTailBnd)
	{
		// find added separator
		bool bNextSeparator = (m_pPersonalHeadBnd ? false : true);
		for (int i = 0; i < numElements(); i++)
		{
			PMenuItem *item = at(i);
			if (bNextSeparator)
			{
				if (item->getType() == separator)
				{
					m_pPersonalTailBnd = item;
					break;
				}
			}
			else if (item == m_pPersonalHeadBnd)
			{
				bNextSeparator = true;
			}
		}
	}
	return bRet;
}

void RootMenu::parseBeforeShowing(bool is_add_menu)
{
//printf("parseBeforeShowing\n\n");
	// is root menu changed ???
	bool bChanged = false;
	if (m_pPersonalTailBnd)
	{
		int nInd = (m_pPersonalHeadBnd ? list.findRef(m_pPersonalHeadBnd) + 1 : 0);
		for (PMenuItem *item = at(nInd); item && item != m_pPersonalTailBnd; item = list.next())
		{
			if (item->isModified())
			{
				bChanged = true;
				break;
			}
		}

		if (bChanged)
		{
			// update personal and global(in case of merge)
			clearSubmenus();
			PMenuItem *item;
			for (item = at(nInd); item && item != m_pPersonalTailBnd; item = list.current())
			{
				list.remove();
			}

			item = (nInd > 0 ? at(nInd-1) : 0);
			if (isPersonalGlobalMerged())
			{
				parse(QDir(kde_apps), false, m_pPersonalTailBnd);
			}

			mergeInto(QDir(personal), item, m_pPersonalTailBnd);

			if (!isPersonalGlobalMerged())
				m_strMergedDir = ""; // no merge
		}
	}

	bool bChangedGlobal = false;
	if (!isPersonalGlobalMerged() && m_pGlobalTailBnd)
	{
		// go throurgh separate global section
		int nIndLast = list.findRef(m_pGlobalTailBnd) - 1;
		for (PMenuItem *item = at(nIndLast); item && item->getType() != separator; item = list.prev())
		{
			if (item->isModified())
			{
				bChangedGlobal = true;
				break;
			}
		}

		if (bChangedGlobal)
		{
			// update global only
			clearSubmenus();
			PMenuItem *item;
			for (item = at(nIndLast); item && item->getType() != separator; item = list.prev())
			{
				list.remove();
			}
			parse(QDir(kde_apps), false, m_pGlobalTailBnd);
		}
	}

	if (bChanged || bChangedGlobal)
	{
		createMenu(cmenu, the_panel);
	}
}

