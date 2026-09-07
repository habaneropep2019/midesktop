/*
  props.C - Part of the KDE project.
  
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



#include "pmenu.h"
#include "mipanel.h"
#include <klocale.h>
#include <qmsgbox.h>
#include <kfm.h>
#include <time.h>
extern void execute(const char*);

//
// helper classes definition
//
class RecentlyLaunchedApps;
class RecentlyLaunchedAppInfo
{
	friend RecentlyLaunchedApps;

protected:
	RecentlyLaunchedAppInfo(int nLaunchCount, time_t lastLaunchTime)
	{
		m_nLaunchCount = nLaunchCount; m_LastLaunchTime = lastLaunchTime;
	}
	
	int getLaunchCount() { return m_nLaunchCount; };
	time_t getLastLaunchTime() { return m_LastLaunchTime; };
	void increaseLaunchCount() { m_nLaunchCount++; };
	void setLaunchCount(int nLaunchCount) { m_nLaunchCount = nLaunchCount; };
	void setLastLaunchTime(time_t lastLaunch) { m_LastLaunchTime = lastLaunch; };

private:
	int m_nLaunchCount;
	time_t m_LastLaunchTime;
}; // class RecentlyLaunchedAppInfo

class RecentlyLaunchedApps
{
public:
	RecentlyLaunchedApps();
	void init();
	void save();
	void appLaunched(const char * szApp);
	void getRecentApps(QStrList & RecentApps);
	void setMaxEntries(int nMax) { m_nMaxEntries = nMax; };

private:
	void checkOverlimit();

	int m_nMaxEntries;
	QDict<RecentlyLaunchedAppInfo> m_Info;
	bool m_bRecentVsOften;
	int m_nNumVisible;
};

//
// helper classes implementation
//
RecentlyLaunchedApps::RecentlyLaunchedApps()
{
	m_bRecentVsOften = false;
	m_nNumVisible = 5; // max entries to display 
	m_nMaxEntries = 25; // max entries to store
	m_Info.resize(m_nMaxEntries * 2);
	m_Info.setAutoDelete(TRUE);
}

void RecentlyLaunchedApps::init()
{
	KConfig *config = KApplication::getKApplication()->getConfig();

	config->setGroup("kmenu");

	if (config->hasKey("NumVisibleEntries"))
	{
		m_nNumVisible = config->readEntry("NumVisibleEntries").toUInt();
	}

	if (config->hasKey("MaxRecentAppsEntries"))
	{
		int nMax = config->readEntry("MaxRecentAppsEntries").toUInt();
		setMaxEntries(nMax);
	}
	
	if (config->hasKey("RecentVsOften"))
	{
		m_bRecentVsOften = (config->readEntry("RecentVsOften").toUInt() != 0);
	}
	
	if (config->hasKey("RecentAppsStat"))
	{
		QStrList RecentApps;
		config->readListEntry("RecentAppsStat", RecentApps);
		for (const char *strInfo = RecentApps.first(); strInfo; strInfo = RecentApps.next())
		{
			int nCount;
			long lTime;
			char szPath[strlen(strInfo)];
			sscanf(strInfo, "%d %ld %s", &nCount, &lTime, (char *)&szPath);
			QFileInfo fi(szPath);
			if (fi.exists() && fi.isFile())
			{
				m_Info.insert(szPath, new RecentlyLaunchedAppInfo(nCount, (time_t)lTime));
			}
		}
	}
}

void RecentlyLaunchedApps::save()
{
	KConfig *config = KApplication::getKApplication()->getConfig();

	config->setGroup("kmenu");

	if (!m_Info.isEmpty())
	{
		QStrList RecentApps;
		QDictIterator<RecentlyLaunchedAppInfo> it(m_Info);
		for (RecentlyLaunchedAppInfo *pInfo = it.toFirst(); pInfo; ++it, pInfo = it.current())
		{
			QString strPath(it.currentKey());
			QString strInfo;
			strInfo.sprintf("%d %ld %s", pInfo->getLaunchCount(), (long)pInfo->getLastLaunchTime(),
											it.currentKey());
			RecentApps.append(strInfo);
		}
		config->writeEntry("RecentAppsStat", RecentApps);
		config->sync();
	}
}

void RecentlyLaunchedApps::appLaunched(const char * szApp)
{
	if (szApp)
	{
		RecentlyLaunchedAppInfo *pInfo = m_Info.find(szApp);
		if (pInfo)
		{
			pInfo->increaseLaunchCount();
			pInfo->setLastLaunchTime(time(0));
		}
		else
		{
			m_Info.insert(szApp, new RecentlyLaunchedAppInfo( 1, time(0)));
			checkOverlimit();
		}
	}
}

void RecentlyLaunchedApps::checkOverlimit()
{
	if ((int)m_Info.count() >= m_nMaxEntries)
	{
		QStrList strsToClear;
		QDictIterator<RecentlyLaunchedAppInfo> it(m_Info);

		for( int i = m_Info.count() - m_nMaxEntries; i > 0; i--)
		{
			// not most effective but easiest
			QString strFoundPath;
			RecentlyLaunchedAppInfo *pFoundInfo = 0;
			for (RecentlyLaunchedAppInfo *pInfo = it.toFirst(); pInfo; ++it, pInfo = it.current())
			{
				QString strPath(it.currentKey());
				if (strFoundPath != strPath && strsToClear.find(strPath) < 0)
				{
					if (!pFoundInfo)
					{
						pFoundInfo = pInfo;
						strFoundPath = strPath;
					}
					else if (m_bRecentVsOften)
					{
						if (pFoundInfo->getLastLaunchTime() >= pInfo->getLastLaunchTime())
						{
							pFoundInfo = pInfo;
							strFoundPath = strPath;
						}
					}
					else
					{
						if (pFoundInfo->getLaunchCount() > pInfo->getLaunchCount() ||
									(pFoundInfo->getLaunchCount() == pInfo->getLaunchCount() &&
									pFoundInfo->getLastLaunchTime() >= pInfo->getLastLaunchTime()))
						{
							pFoundInfo = pInfo;
							strFoundPath = strPath;
						}
					}
				}
			}
			strsToClear.append(strFoundPath);
		}
		if (!strsToClear.isEmpty())
		{
			for (const char * str = strsToClear.first(); str; str = strsToClear.next())
			{
				m_Info.remove(str);
			}
		}
	}
}

void RecentlyLaunchedApps::getRecentApps(QStrList & RecentApps)
{
	// not most effective but easiest
	QDictIterator<RecentlyLaunchedAppInfo> it(m_Info);
	
	RecentApps.clear();
	
	RecentlyLaunchedAppInfo MinInfo(0, 0);
	QString strFoundPath;
	for (int i = 0; i < m_nNumVisible && i < (int)it.count(); i++)
	{
		RecentlyLaunchedAppInfo *pFoundInfo = &MinInfo;
		for (RecentlyLaunchedAppInfo *pInfo = it.toFirst(); pInfo; ++it, pInfo = it.current())
		{
			QString strPath(it.currentKey());
			if (strFoundPath != strPath && RecentApps.find(strPath) < 0)
			{
				if (m_bRecentVsOften)
				{
					if (pFoundInfo->getLastLaunchTime() <= pInfo->getLastLaunchTime())
					{
						pFoundInfo = pInfo;
						strFoundPath = strPath;
					}
				}
				else
				{
					if (pFoundInfo->getLaunchCount() < pInfo->getLaunchCount() ||
								(pFoundInfo->getLaunchCount() == pInfo->getLaunchCount() &&
								pFoundInfo->getLastLaunchTime() <= pInfo->getLastLaunchTime()))
					{
						pFoundInfo = pInfo;
						strFoundPath = strPath;
					}
				}
			}
		}
		if (pFoundInfo != &MinInfo) // should always go through
		{
			RecentApps.append(strFoundPath);
		}
	}
}
//
// end of helper classes implementation 
//

void kPanel::writeOutConfiguration()
{
	KConfig *config = KApplication::getKApplication()->getConfig();
	config->setGroup("kpanelButtons");
	QString s;
	QString b;

	float delta;
	float real_space;
	
	QStrList button;
	QStrList buttondelta;
	
	if (orientation == Qt::Vertical)
	 real_space = height();
	else
	 real_space = width();

	QWidget *pPrevEntry = 0;
	for (int i = 0; i < m_pButtonsArea->getNumEntries(); i++)
	{
		QWidget *pEntry = m_pButtonsArea->getEntryAt(i);
		DesktopEntry *pDE;
		if ((pDE = entries.findButton(pEntry)))
		{
			if (pDE->popup == windowlist)
				s = "windowlist";
		 // --sven: kdisknav button start --
			else if (m_pDiskNavMenu && pDE->popup == m_pDiskNavMenu)
				s = "kdisknav";
		 // --sven: kdisknav button end --
			else if (pDE->pmi)
			{
				s = pDE->pmi->getSaveName();
			}
			else
				s = "system";
		}
		else
		{
			s = "widget:";
			s += pEntry->name();
		}

		button.append(s);

		if (pPrevEntry)
		{
			if (orientation == Qt::Vertical)
			{
				delta = (pEntry->y() - pPrevEntry->y()
											- pPrevEntry->height()); 
			}
			else
			{
				delta = (pEntry->x() - pPrevEntry->x()
											- pPrevEntry->width()); 
			}
		
			delta = (delta*10000)/real_space;
			b.setNum(delta);
			buttondelta.append(b);
		}
		pPrevEntry = pEntry;
	}
	
	config->writeEntry("Buttons", button);
	config->writeEntry("ButtonDelta", buttondelta);
	
	config->setGroup("kdisknav");
	
	config->writeEntry("RecentFolders", recent_folders);
	config->writeEntry("RecentFiles", recent_files);
	
	config->sync();
};

//
// creates item for "recent" section of KMenu
//
PMenuItem * createNewRecentMenuItem(const char * szAdd)
{
	PMenuItem * item;
	QFileInfo fi;
	fi.setFile(szAdd);
	if (QString(szAdd).right(7) == ".kdelnk")
	{
		item = new PMenuItem();
		item->parse(&fi);
	}
	else
	{
		PFileMenu pfilemenu("");
		item = pfilemenu.newFileItem(&fi, false);
	}
	return item;
}

RecentlyLaunchedApps m_RecentApps; // stores "recent" history

//
// updates "recent" section of KMenu
//
void kPanel::updateRecentMenuItems(const char *szItem, bool bRecentFilesChanged)
{
	if (!szItem)
	{
		return;
	}

	// in some cases don't add item into menu
	QString strItemToAdd(szItem);
	if (strItemToAdd.right(7) == ".kdelnk")
	{
		int nFind = strItemToAdd.findRev('/');
		if (nFind > 0 && strItemToAdd.left(nFind) == KApplication::kde_appsdir())
		{
			// don't add items alreary in KMenu root
			return;
		}
	}

	m_RecentApps.appLaunched(szItem);
	m_RecentApps.save();
	
	QStrList RecentApps;
	m_RecentApps.getRecentApps(RecentApps);

	QPopupMenu *menu = pmenu->getQPopupMenu();
	// remove previous items
	if (m_RecentMenuItems.count() > 0)
	{
		for (PMenuItem* item = m_RecentMenuItems.first(); item; item = m_RecentMenuItems.next())
		{
			QString strPath(item->getFullPathName());

			// remove
			menu->removeItem(item->getId());
			pmenu->remove(item); // can't use item after this
		}
		m_RecentMenuItems.clear();
	}
	else if (RecentApps.count() > 0)
	{
		PMenuItem * item;
		pmenu->insert(item = new PMenuItem(separator), 0);
		pmenu->setPersonalSectionBounds(item);
		menu->insertSeparator(0);
	}

	int i = 0;
	// insert new items
	for(const char * szAdd = RecentApps.first(); szAdd; szAdd = RecentApps.next())
	{
		PMenuItem * item = createNewRecentMenuItem(szAdd);
		item->setEditPopupMenuStatus(EPM_CANT_DELETE);
				
		if (item != NULL)
		{
			pmenu->insert(item, i);
			menu->insertItem(*item->getPixmap(), item->getText(), item->getId(), i);
			connect(item, SIGNAL(showToolTip(QString)), m_pToolTip, SLOT(showToolTip(QString)));
			menu->connectItem(item->getId(), item, SLOT(exec()));
			m_RecentMenuItems.append(item);
			i++; // ??? not increase in "recent order" case
		}
	}
}

void kPanel::parseMenus()
{
	ready_for_event_loop = true;
	KConfig *config = KApplication::getKApplication()->getConfig();
	
	config->setGroup("KDE Desktop Entries");
	QString temp = KApplication::localkdedir() +"/share/applnk";
	QString personal = config->readEntry("PersonalPath", temp.data() );
	temp = KApplication::kde_appsdir();
	QString kde_apps = config->readEntry("Path", temp.data() );
	
	// what to show predefined menus
	config->setGroup("kmenu");
	bool bShowPanelMenu = (config->readEntry("ShowPanelMenu", "off") == "on");
	bool bShowRunMenu = (config->readEntry("ShowRunMenu", "off") == "on");
	bool bShowLockScreenMenu = (config->readEntry("ShowLockScreenMenu", "off") == "on");
	
	config->setGroup("kpanel");
	bool personalFirst = false;
	if (config->hasKey("PersonalFirst"))
		personalFirst = (config->readEntry("PersonalFirst") == "on");
	else
		config->writeEntry("PersonalFirst", "off");

	bool bMergePersonalGlobal = true;
	if (config->hasKey("MergePersonalGlobal"))
		bMergePersonalGlobal = (config->readEntry("MergePersonalGlobal") == "on");
	else
		config->writeEntry("MergePersonalGlobal", "on");

	RootMenu::setMergePersonalGlobal(bMergePersonalGlobal);

	config->setGroup("KDE Panel Modifications");
	if (config->hasKey("MenuOmissions"))
	{
		QStrList list2clear;
		config->readListEntry("MenuOmissions", list2clear );
		if (!list2clear.isEmpty())
		{
			PMenu::m_pMenusToClear = new QStrList();
			for (char *s = list2clear.first(); s; s = list2clear.next())
			{
				PMenu::m_pMenusToClear->append(kde_apps + s);
			}
		}
	}

	// create and fill "recent" section at first
	m_RecentApps.init();
	QStrList RecentApps;
	m_RecentApps.getRecentApps(RecentApps);
	
	if (RecentApps.count() > 0)
	{
		for (const char * str = RecentApps.first(); str; str = RecentApps.next())
		{
			PMenuItem * item = createNewRecentMenuItem(str);
			item->setEditPopupMenuStatus(EPM_CANT_DELETE);
			if (item != NULL)
			{
				pmenu->add(item);
				m_RecentMenuItems.append(item);
			}
		}
		
		// add separator
		pmenu->add( new PMenuItem(separator) );
	}
	
	// now add Local and Global menu items
	PMenuItem *pPersonalHeadBnd, *pPersonalTailBnd = 0;
	PMenuItem *pItem;
	if (bMergePersonalGlobal)
	{
		int nBeforeParse = pmenu->numElements();
		pPersonalHeadBnd = (nBeforeParse ? pmenu->at(nBeforeParse-1) : 0);
		
		// parse including submenus in order to merge correctly
		pmenu->parse(QDir(kde_apps), true);
		pmenu->add(pPersonalTailBnd = new PMenuItem(separator));

		pmenu->setPersonalSectionBounds(pPersonalHeadBnd, pPersonalTailBnd);
		pmenu->mergePersonalEntries(QDir(personal));
	}
	else if (personalFirst)
	{
		int nBeforeParse = pmenu->numElements();
		pPersonalHeadBnd = (nBeforeParse ? pmenu->at(nBeforeParse-1) : 0);
		pmenu->parse(QDir(personal));

		if (pmenu->numElements() != nBeforeParse)
		{
			pmenu->add(new PMenuItem((EntryType)separator));
			pPersonalTailBnd = pmenu->at(pmenu->numElements() - 1);
		}
		pmenu->setPersonalSectionBounds(pPersonalHeadBnd, pPersonalTailBnd);

		pmenu->parse(QDir(kde_apps));
		pmenu->add(pItem = new PMenuItem(separator));
		pmenu->setGlobalSectionBound(pItem);
	}
	else // global first
	{
		pmenu->parse(QDir(kde_apps));
		pmenu->add(pItem = new PMenuItem(separator));
		pmenu->setGlobalSectionBound(pItem);

		int nBeforeParse = pmenu->numElements();
		pPersonalHeadBnd = pmenu->at(nBeforeParse-1);
		pmenu->parse(QDir(personal));

		if (pmenu->numElements() != nBeforeParse)
		{
			pmenu->add(new PMenuItem((EntryType)separator));
			pPersonalTailBnd = pmenu->at(pmenu->numElements() - 1);
		}
		pmenu->setPersonalSectionBounds(pPersonalHeadBnd, pPersonalTailBnd);
	}
															
	PMenuItem* fileBrowser = 0L;
	if (bShowDiskNav)
	{
		fileBrowser = new PMenuItem(submenu, i18n("Disk Navigator"),
			0, "kdisknav.xpm", new PFileMenu(true), 0, 0,
			m_pDiskNavMenu, false, 0, //sven changed
			i18n("Quickly navigate through the filesystem"));
		
		fileBrowser->setEditPopupMenuStatus(EPM_DONT_SHOW);
		pmenu->add( fileBrowser );
	}
	
	if (bShowPanelMenu)
	{

		pmenu_add = new PMenu(*pmenu);
		
		PMenu *panel_menu = new PMenu;
		panel_menu->add(pItem = new PMenuItem(add_but, i18n("Add Application"), 0, 0, pmenu_add,
			0, 0, new myPopupMenu, false, 0,
			i18n("Add an application or a submenu onto the panel")));
		pItem->setEditPopupMenuStatus(EPM_DONT_SHOW);
		
		PMenuItem* pdisknav;
		PMenuItem* pwindowlist;
		
		// --sven: kdisknav button start --
		panel_menu->add( pdisknav = new PMenuItem(prog_com, i18n("Add Disk Navigator"), 0, 0, 0,
			this, SLOT(add_kdisknav()), 0, false, 0,
			i18n("Add a Disk Navigator menu onto the panel")) );
		pdisknav->setEditPopupMenuStatus(EPM_DONT_SHOW);
		
		// --sven: kdisknav button end --
		panel_menu->add( pwindowlist = new PMenuItem(prog_com, i18n("Add Windowlist"), 0, 0, 0,
			this, SLOT(add_windowlist()), 0, false, 0,
			i18n("Add a windowlist menu onto the panel")) );
		pwindowlist->setEditPopupMenuStatus(EPM_DONT_SHOW);

		panel_menu->add(pItem = new PMenuItem(prog_com, i18n("Configure"), 0, 0, 0,
			this, SLOT(configurePanel()), 0, false, 0,
			i18n("Configure panel")) );
		pItem->setEditPopupMenuStatus(EPM_DONT_SHOW);
		
		panel_menu->add(pItem = new PMenuItem(prog_com, i18n("Edit Menus"), 0, 0, 0,
			this, SLOT(editMenus()), 0, false, 0,
			i18n("Add or remove applications in the menu structure ")) );
		pItem->setEditPopupMenuStatus(EPM_DONT_SHOW);
		
		panel_menu->add(pItem = new PMenuItem(prog_com, i18n("Restart"), 0, 0, 0,
			this, SLOT(restart()), 0, false, 0,
			i18n("Restart panel")) );
		pItem->setEditPopupMenuStatus(EPM_DONT_SHOW);
		
		pmenu->add(pItem = new PMenuItem(submenu, i18n("&Panel"), 0, "mini-panel.xpm", panel_menu,
			0, 0, new myPopupMenu, false, 0,
			i18n("Panel")) );
		pItem->setEditPopupMenuStatus(EPM_DONT_SHOW);
		
		panel_popup = panel_menu->getQPopupMenu();
		add_disknav_entry = pdisknav->getId();
		add_windowlist_entry = pwindowlist->getId();
	}
	
	if (bShowRunMenu)
	{
		//PAL - Oct98: Added Run... menu.
		pmenu->add(pItem = new PMenuItem(prog_com, i18n("&Run..."), 0, "mini-run.xpm", 0,
			this, SLOT(call_run()), 0, false, 0,
			i18n("Execute a command")) );
		pItem->setEditPopupMenuStatus(EPM_DONT_SHOW);
	}
	
	if (bShowLockScreenMenu)
	{
		pmenu->add(pItem = new PMenuItem(prog_com, i18n("Lock Screen"), 0, "mini-key.xpm", 0,
			this, SLOT(call_klock()), 0, false, 0,
			i18n("Lock screen")) );
		pItem->setEditPopupMenuStatus(EPM_DONT_SHOW);
	}
	
	pmenu->add(pItem = new PMenuItem(prog_com, i18n("Logout..."), 0, "mini-exit.xpm", 0,
		this, SLOT(ask_logout()), 0, false, 0, i18n("Logout")) );
	pItem->setEditPopupMenuStatus(EPM_DONT_SHOW);
	
	pmenu->createMenu(new myPopupMenu(pmenu), this);
	
	if (fileBrowser)
	{
		PFileMenu::calculateMaxEntriesOnScreen(fileBrowser);
	}
	
	if (has_kdisknav_button)
		panel_popup->setItemChecked(add_disknav_entry, true);
	
	if (has_windowlist_button)
		panel_popup->setItemChecked(add_windowlist_entry, true);
	
	DesktopEntry *pEntry = entries.findButton(kde_button);
	if (pEntry)
	{
		pEntry->popup = pmenu->getQPopupMenu();
	}
	ready_for_event_loop = false;
	kde_button->setCursor(arrowCursor);
}

// BI: 242853 -- 1998/12/23 -- Sam Watts
// Remove certain sub-menus from the K-menu based on the rc file
void kPanel::cleanupMenu(PMenu *menu, QStrList *list)
{
    PMenuItem *currentItem;
    int result;

    for (int i = 0; i < menu->numElements(); i++)
    {
       currentItem = menu->at(i);
       if (currentItem->getType() == submenu)
       {
          result = list->find(currentItem->getFullPathName());
          if (result != -1)
          {
             list->remove(result);
             menu->remove(currentItem);
             i--;
          }
          else
	  {
             cleanupMenu(currentItem->getMenu(),list);
	  }
       }
    }
}

void kPanel::readInConfiguration()
{
	QStrList button;
	QStrList buttondelta;
	
	KConfig *config = KApplication::getKApplication()->getConfig();
	config->setGroup("kpanelButtons");
	
	config->readListEntry("Buttons", button);
	config->readListEntry("ButtonDelta", buttondelta);
	
	float x = 0;
	float y = 0;
	float real_space;
	float delta = 0;

	if (orientation == Qt::Vertical)
	{
		real_space = height();
//		x = margin; y = 0;
	}
	else
	{
		real_space = width();
//		y = margin; x = 0;
	}
	
	QString tmp;
	
	int control = 1;
	
	if (config->hasKey("control"))
	 control = config->readNumEntry("control");
	
	QString button_entry_value;
	for(button_entry_value = button.first(), tmp = buttondelta.first(); 
			button_entry_value;
			button_entry_value = button.next(), tmp = buttondelta.next())
	{
		delta = (delta * real_space) / 10000 + 0.000001; // add error
		if (button_entry_value == "system")
		{
			// the unremovable kde_button!
		}
		else 
		{
			if (orientation == Qt::Horizontal)
				x += delta;
			else
				y += delta;
			PMenuItem* pmi = 0;
		
			bool buttonAdded = false;
			if (button_entry_value == "windowlist")
			{
				addButtonInternal(0, (int)x, (int)y, "windowlist");
				buttonAdded = true;
				has_windowlist_button = true;
			}
			// --sven: kdisknav button start --
			else if (button_entry_value == "kdisknav")
			{
				addButtonInternal(0, (int)x, (int)y, "kdisknav");
				buttonAdded = true;
				has_kdisknav_button = true;
			}
			// --sven: kdisknav button end --
			else 
			{
				pmi = pmenu->searchItem( button_entry_value);
				if (pmi)
				{
					addButtonInternal(pmi, (int)x, (int)y);
					buttonAdded = true;
				}
			}
	
			if (buttonAdded && entries.last())
			{
			 if (orientation == Qt::Horizontal)
				 x += entries.last()->button->width();
			 else
				 y += entries.last()->button->height();
			}
		}

		// assign delta
		delta = (tmp.isNull() ? 0 : tmp.toFloat());
	}	
	
	// kdisknav
	config->setGroup("kdisknav");
	
	if (config->hasKey("MaxRecentFoldersEntries"))
	 max_recent_folders_entries =
		 config->readEntry("MaxRecentFoldersEntries").toUInt();
	else {
	 // config->writeEntry("MaxRecentFoldersEntries", 4);
	 max_recent_folders_entries = 4;
	}
	
	if (config->hasKey("MaxRecentFilesEntries"))
	 max_recent_files_entries =
		 config->readEntry("MaxRecentFilesEntries").toUInt();
	else {
	 // config->writeEntry("MaxRecentFilesEntries", 4);
	 max_recent_files_entries = 4;
	}
	
	if (config->hasKey("MaxNavigableFolderEntries"))
	 max_navigable_folder_entries =
		 config->readEntry("MaxNavigableFolderEntries").toUInt();
	else {
	 // config->writeEntry("MaxNavigableFolderEntries", 200);
	 max_navigable_folder_entries = 200;
	}
	
	if (config->hasKey("ShowDotFiles"))
	 show_dot_files = (config->readEntry("ShowDotFiles") == "on");
	else {
	 // config->writeEntry("ShowDotFiles", "off");
	 show_dot_files = false;
	}
	
	if (config->hasKey("IgnoreCase"))
	 ignore_case = (config->readEntry("IgnoreCase") == "on");
	else {
	 // config->writeEntry("IgnoreCase", "off");
	 ignore_case = false;
	}
	
	if (config->hasKey("RecentFolders"))
	 config->readListEntry("RecentFolders", recent_folders);
	
	if (config->hasKey("RecentFiles"))
	 config->readListEntry("RecentFiles", recent_files);
	
	if (config->hasKey("ShowGlobalSection"))
	 show_shared_section = (config->readEntry("ShowGlobalSection") == "on");
	else
	 show_shared_section = true;
	
	if (config->hasKey("ShowLocalSection"))
	 show_personal_section = (config->readEntry("ShowLocalSection") == "on");
	else {
	 show_personal_section = true;
	}
	
	if (config->hasKey("ShowRecentSection"))
	 show_recent_section = (config->readEntry("ShowRecentSection") == "on");
	else {
	 show_recent_section = true;
	}
	
	if (config->hasKey("ShowOptionEntry"))
	 show_option_entry = (config->readEntry("ShowOptionEntry") == "on");
	else {
	 show_option_entry = true;
	}
}


void kPanel::configurePanel()
{
	execute("kcmkpanel &");
}

void kPanel::editMenus(){
    QString editor = findMenuEditor(kapp->kde_appsdir());
    if ( editor.isEmpty() ){
	QMessageBox::warning( 0, "Panel",
			      i18n("The menu editor is not installed."),
			      i18n("Oops!"));
    }
    else {
	KFM* kfm = new KFM;
	QString com = "file:";
	com.append(editor);
	kfm->exec(com, 0L);
	delete kfm;
    }
}

