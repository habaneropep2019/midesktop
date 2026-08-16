// -*- C++ -*-

/*
  pmenu.h
  
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

#ifndef PMENU_H
#define PMENU_H

#include <qdir.h>
#include <qpixmap.h>
#include <qlist.h>
#include <qfileinf.h>
#include <qpopmenu.h>
#include <kapp.h>
#include <time.h>

#include <cstdlib>

class PMenu;
class PFileMenu;
class kPanel;

class myPopupMenu: public QPopupMenu
{
  Q_OBJECT
public:
  myPopupMenu( QWidget *parent=0, const char *name=0 );
  myPopupMenu( PMenu* _parentMenu );
  ~myPopupMenu() {}
  int height();
  int width();
  int id;        // if this object is a submenu, the submenu id

private:	// Disabled copy constructor and operator=
  myPopupMenu( const myPopupMenu & ) {}
  myPopupMenu &operator=( const myPopupMenu & ) { return *this; }

  virtual bool x11Event( XEvent * xe);
  virtual void mousePressEvent ( QMouseEvent * );

public:
  int maxEntriesOnScreen();
  int entryHeight();
	PMenu* getParentMenu() { return parentMenu; };

  PMenu* parentMenu;
  static int keyStatus;   // ShiftButton | ControlButton | AltButton
};


enum EntryType { empty, separator, submenu, unix_com, prog_com, label, add_but, dirbrowser, url };

#define __cc0 (void*)0

// available edit popup menu status flags
const int EPM_DONT_SHOW = 0x1;
const int EPM_CANT_DELETE = 0x2;
const int EPM_CANT_CHANGE = 0x4;
const int EPM_CANT_RENAME = 0x8;

//////////////////////////////////////////////////////////////////////////////

class PMenuItem : public QObject
{
  Q_OBJECT
  friend QString &operator<<(QString &, PMenuItem &);
  friend PMenu;
  friend PFileMenu;
public:

#ifdef DISKNAV_DEBUG
  virtual void dump();
#endif

  PMenuItem();
  PMenuItem( EntryType e, QString text=QString(), QString command=QString(),
	     QString pixmap=QString(), PMenu *submenu=0,
	     QObject *receiver=0, char *member=0, QPopupMenu *cm=0,
	     bool readonly = FALSE,
	     QString dir=QString(), QString comment=QString() );

  PMenuItem( PMenuItem &item );
  ~PMenuItem ();

  short     parse( QFileInfo *fi, PMenu *menu = 0L );
  short     parse( QString abs_file_path );
  void      setType( EntryType e) { entry_type = e; }
  EntryType getType() { return entry_type; }
  void      setMenu( PMenu *menu );
  PMenu    *getMenu() { return sub_menu; }
  QPopupMenu *getQPopupMenu() { return cmenu; }
  void      removeMenu();
  QString   getSaveName();
  const char* getBigIconName(){ return big_pixmap_name;};
  const char* getText() { return text_name; };
  QString   getFullPathName(){ return dir_path + "/" + real_name; };
  const char* getComment(){ return comment; }
  const char* getDirPath(){ return dir_path; }
  const QPixmap * getPixmap() { return &pixmap; }

  int getId(){ return id; }
  void setRealName(QString _real_name) { real_name = _real_name; }

  bool writeConfig( QDir dir );

	void setEditPopupMenuStatus(unsigned s) { m_nEditPopupMenuStatus = s; };
	unsigned getEditPopupMenuStatus() { return m_nEditPopupMenuStatus; };
	
	bool isModified();

	void insertIntoQMenu(QPopupMenu *menu);

public slots:
  void exec();
  void addFolderToRecentList();

signals:
  void showToolTip(QString);
  void addButton(PMenuItem *);

protected slots:
  void highlighted() { emit showToolTip(comment); }
  void execAddButton() { emit addButton(this); }

protected:
  static bool  use_kfm;
  static bool  in_kwm_mode;
  QString      text_name;
  QString      real_name;
  QString      pixmap_name;
  QString      big_pixmap_name;
  QPixmap      pixmap;
  EntryType    entry_type;
  QString      command_name;
  QString      dir_path;
  PMenu       *sub_menu;
  myPopupMenu  *cmenu;
  QObject     *recv;
  char        *memb;
  QString      comment;
	int m_nAccel;

private:
  int id;
	unsigned m_nEditPopupMenuStatus;
	time_t m_nLastModificationTime;
};

class PMenu : public QObject
{
  Q_OBJECT
public:
  PMenu ();
  PMenu ( PMenu &menu );
  ~PMenu ();

#ifdef DISKNAV_DEBUG
  virtual void dump();
#endif

  int numElements() { return list.count(); };
	PMenuItem *at(int item) { return list.at(item); };

  void       add ( PMenuItem *item);
  void       insert ( PMenuItem *item, int index );
  void       move( short item_id, short new_pos);
  void       remove(PMenuItem *item);
  short      parse (QDir d, bool bParseSubdirs = false, PMenuItem *pInsertBefore = 0);
  short      parseLazy (const QDir & d );
  virtual void createMenu( QPopupMenu *menu, kPanel *panel, bool add_button = FALSE);
  void       create_pixmap( QPixmap &buf, PMenuItem *item, QPopupMenu *menu );
  void       set_net_recv( QObject *receiver, char *member );
  PMenuItem *searchItem( QString name );

  PMenuItem *searchItem( int id );
  bool       copyFiles( QString source, QString dest);
  int        count() { return list.count(); }

#ifdef DISKNAV_DEBUG
  PMenuItem *searchItem( PMenu* item );  // DEBUG
#endif

  QPopupMenu *getQPopupMenu() { return cmenu; }

  void setAltSort( bool alternateSort ) { altSort = alternateSort; }
  bool getAltSort() { return altSort; }
  void clearSubmenus();

	bool insertItem(PMenu *pMenu, int nId, bool bMove);
	bool deleteItem(int nId);
	void update();
	bool checkWritablePath(QString & strPath);
	void createNewFolder();

signals:

public slots:
  virtual void aboutToShow();
  virtual void aboutToShowAddMenu();

protected slots:
  virtual void highlighted(int id);

private:
	bool skipDuplicatedEntry(QString &strFullPath, QFileInfo *fi, bool bNoLocalCheck = false);
	bool isSameAppKdelnk(const char *szGlobalFile, const char *szLocalFile);

  bool parsed;

protected:
  QString uniqueFileName(QString name, QString dir_name);
  virtual void parseBeforeShowing(bool is_add_menu);
	bool removeItem(int nId, const char *szMoveToDir);

	// merges directory contents into menu between certain items
	bool mergeInto(QDir dir, PMenuItem *pHeadBnd = 0, PMenuItem *pTailBnd = 0);
  bool addFromDrop(QString a, PMenuItem *pHeadBnd = 0, PMenuItem *pTailBnd = 0);

  QList<PMenuItem> list;
  myPopupMenu      *cmenu;
  PMenuItem*       parentItem;

  bool altSort;
	QString m_strMergedDir;

  static PMenuItem* menu_editor_item;
	static bool m_bShowEmptyFolders;

public:
	static QStrList* m_pMenusToClear;
};

inline void PMenuItem::setMenu( PMenu *menu )
{
  if( sub_menu )
    delete sub_menu;
  sub_menu = menu;
}

inline void PMenuItem::removeMenu()
{
  if( sub_menu )
    delete sub_menu;
  sub_menu = NULL;
}


// 
// class RootMenu definition
//
class RootMenu : public PMenu
{
  Q_OBJECT

public:
  RootMenu ();
	void setPersonalSectionBounds(PMenuItem* pHeadBnd, PMenuItem* pTail = 0);
	void setGlobalSectionBound(PMenuItem* pTail) { m_pGlobalTailBnd = pTail; };

	bool mergePersonalEntries(QDir dir);
  bool addPersonalFromDrop(QString a);

	static void setMergePersonalGlobal(bool b) { m_bMergePersonalGlobal = b; };
	static bool isPersonalGlobalMerged() { return m_bMergePersonalGlobal; };

protected:
  virtual void parseBeforeShowing(bool is_add_menu);

protected:
	PMenuItem*  m_pPersonalHeadBnd;	// head boundary
	PMenuItem*  m_pPersonalTailBnd; // tail boundary
	PMenuItem*  m_pGlobalTailBnd; 
	static bool m_bMergePersonalGlobal;
};

inline void RootMenu::setPersonalSectionBounds(PMenuItem* pHeadBnd, PMenuItem* pTailBnd)
{
	m_pPersonalHeadBnd = pHeadBnd;
	m_pPersonalTailBnd = pTailBnd;
}

//
// class PFileMenu definition
//
class PFileMenu : public PMenu
{
  Q_OBJECT
public:
  friend PMenu;
  friend PMenuItem;

#ifdef DISKNAV_DEBUG
  virtual void dump();
#endif

  PFileMenu(bool isRoot = false);
  PFileMenu(QString& _path);
  PFileMenu(const char* _path);
  virtual ~PFileMenu();

  void setId(int _id) { this->id = _id; }
  int parseDir(QDir d, bool addOpenFolderEntry = true);
  int parseTail();

  PMenuItem* newDirBrowserItem(const QFileInfo* fi, bool useCurrentPath);
  PMenuItem* newLinkItem(const QFileInfo* fi, bool useCurrentPath);
  PMenuItem* newFileItem(const QFileInfo* fi, bool useCurrentPath);
  void buildRootMenu();

  static bool updateRecentFolders(QString _path);
  static bool updateRecentFiles(QString _path);
  static void calculateMaxEntriesOnScreen(PMenuItem* menu);

public slots:
  virtual void aboutToShow();
  void deactivated(int _id);
  void optionsDlg();
  void openFolder();
  void end() { exit(0); }   // DMALLOC DEBUG

protected:

  static bool updateRecentList(EntryType, QString _path, QStrList& recentlist,
                               int max_size);


  static void insertRecentItem(EntryType type, const char* _path,
                                const char* fn = 0);

  static void removeLessRecentItem(EntryType type, QStrList& recentlist);

  bool addFile(QFileInfo* fi, bool useDefaultPath = true);
  bool addFile(QString _path, bool useDefaultPath = true);

  void addTailMenu(QFileInfoListIterator* tail);
  void copyTailFileInfo(QFileInfoListIterator& it);
  void fixMenuPosition();

#ifdef DISKNAV_DEBUG
  void doSelfCheck();
#endif

private:
  int id;
  QFileInfoListIterator* tail;
  QFileInfoList finfos;
  QString path;
  bool isClean;
  static PFileMenu* root;
  static int maxEntriesOnScreen;
  static int entryHeight;
  PFileMenu* lastActivated;
};

#endif // PMENU_H
