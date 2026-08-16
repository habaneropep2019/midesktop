/* 
   kpanel.h - Part of the KDE project.
  
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

#ifndef KPANEL_H
#define KPANEL_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <qwidget.h>
#include <qlabel.h>
#include <qbttngrp.h>
#include <qpushbt.h>
#include <qpixmap.h>
#include <qcolor.h>
#include <qpopmenu.h>
#include <qdrawutl.h>
#include <qtoolbutton.h>

#include "pmenu.h"

// KDE includes
#include <kconfig.h>
#include <kwmmapp.h>
#include <kiconloader.h>
#include <X11/Xlib.h>

class QBoxLayout;
class QTimer;
class QScrollView;
class QLineEdit;
class KProcess;
class KToolBoxManager;
class QToolButton;

enum Orientation {horizontal, vertical};
enum Position {top_left, bottom_right};
enum TaskbarPosition {hidden, top, bottom, taskbar_top_left};

void restart_the_panel();
extern char* applications_path;
extern char* personal_applications_path;
extern char* personal_applications_path_name;
extern bool ready_for_event_loop;


enum {
  OP_MAXIMIZE = 5000,
  OP_RESTORE,
  OP_ICONIFY,
  OP_UNICONIFY,
  OP_MOVE,
  OP_RESIZE,
  OP_CLOSE,
  OP_STICKY,
  OP_ONTO_CURRENT_DESKTOP,
  OP_ICONIFY_OTHER_WINDOWS
};

enum StyleSize { tiny = 0, normal, large };


//
// class myPushButton
//
class myPushButton: public QPushButton
{
  Q_OBJECT
public:
  myPushButton ( QWidget *parent=0, const char* name=0);
  ~myPushButton ();
  bool flat;
  int last_button;
  static myPushButton* most_recent_pressed;
  Window swallowed_window;	

protected:
  void enterEvent( QEvent * );
  void leaveEvent( QEvent * );
  void mousePressEvent( QMouseEvent *e);
  void mouseReleaseEvent( QMouseEvent *e);
  void mouseMoveEvent( QMouseEvent *e);
  void paint( QPainter *_painter );
  void drawButton( QPainter *p ){paint(p);}
  virtual void drawButtonLabel( QPainter *painter );
  bool never_flat;
  bool flat_means_down;
  bool draw_down;
};


//
// class myFrame
//
class myFrame: public QFrame
{
  Q_OBJECT
public:
  myFrame( bool _autoHide, unsigned int delay, QWidget *parent=0, const char* name=0, WFlags f=0);
  ~myFrame(){};
  bool autoHidden;
  bool autoHide;
  int hide_delay;

signals:
  void showMe();
  void hideMe();

protected:
  void enterEvent( QEvent * );

 private slots:
 void hideTimerDone();

private:
  QTimer *hideTimer;
};

//
// class myTaskButton
//
class myTaskButton: public myPushButton
{
  Q_OBJECT
public:
  myTaskButton ( QWidget *parent=0, const char* name=0 );
  ~myTaskButton();
  void setText(const char*);
  void setActive(bool value = TRUE);
  bool isActive() const;
  static void setNoActive();
  Window win;
  int virtual_desktop;
	bool m_bSticky;

signals:
	void clicked(myTaskButton*);
	void pressed(myTaskButton*);

private slots:
	void btnClicked();
	void btnPressed();

 protected:
  virtual void drawButtonLabel( QPainter *painter );
 private:
  QString s;
  static myTaskButton* active;
};


//
// class DesktopEntry
//
class DesktopEntry 
{
public:
  DesktopEntry();
  DesktopEntry(myPushButton* pBtn);
  myPushButton* button;
  QPopupMenu* popup;
  PMenuItem* pmi;
  KDNDDropZone* drop_zone;

  QString swallow;
  Window swallowed;
  int app_id;
  QString identity;
  QPixmap* icon[4];

private:
	void init();
};

//
// class DesktopEntryList
//
class DesktopEntriesList : public QList<DesktopEntry>
{
public:
	DesktopEntriesList();
	DesktopEntry * findButton(QWidget* pBtn);
};

//
// class ScrollableButtonsArea
//
class ScrollableButtonsArea : public QWidget
{
	Q_OBJECT
public:
	ScrollableButtonsArea(QWidget *parent);

	QWidget *getButtonsArea() { return m_pButtonsArea;};
	void layoutComponents();
	void setOrientation(Orientation orientation) { m_Orientation = orientation; };
	Orientation getOrientation() { return m_Orientation; };
  void buttonsAreaResized();
  bool bScrollButtonsDisplayed() { return scroll_buttons_displayed; };          

	void insertEntry(QWidget *pEntry, int x, int y);
	void addEntry(QWidget *pEntry);
	void removeEntry(QWidget *pEntry, bool bShiftRemaining = true);
	void moveEntry(QWidget *pEntry, int nXOld, int nYOld);
	void moveEntry(QWidget *pEntry, const QPoint & OldPos);
	void fillEmptyPosition(int nX, int nY);
	void checkButtonsAreaSize();
	QList<QWidget> &getEntries() { return m_Entries; };
	int getNumEntries() { return m_Entries.count(); };
	QWidget *getEntryAt(int i) { return (i >= 0 && i < getNumEntries() ? m_Entries.at(i) : 0); };
	void setBoxSize(int nBoxWidth, int nBoxHeight) { m_nBoxWidth = nBoxWidth, m_nBoxHeight = nBoxHeight; }; 
	int getBoxWidth() { return m_nBoxWidth; };
	int getBoxHeight() { return m_nBoxHeight; }; 
	int getMaxLength();
	
	// special mode (an extended horizontal mode)
	void setAllowMultipleRows(bool bSet) { m_bAllowMultipleRows = bSet; if (bSet) m_Orientation = Qt::Horizontal; };
	bool isAllowsMultipleRows() { return m_bAllowMultipleRows; };
	void setTrailingEmptySpace(int n) { m_nTrailingFreeSpace = n; };
	
	void setFlexibleElementSize(int nMin, int nMax) { m_nMinSize = nMin; m_nMaxSize = nMax; };
protected slots:
  void scrollTopLeft ();
  void scrollBottomRight ();
  void scrollTopLeft (int step);
  void scrollBottomRight (int step);

protected:
  void enableDisableScrollButtons();
  void showHideScrollButtons();     
	void createArrowPixmaps();
  
	void resizeEvent( QResizeEvent * );
	void changeItemsSize(int nNewWd, int nNewHg);

	QScrollView *m_pScroller;
	QWidget *m_pButtonsArea;
	Orientation m_Orientation;

  QPushButton *top_left_scroll_button;    
  QPushButton *bottom_right_scroll_button;
  bool scroll_buttons_displayed;          

  // The maximum width of the scroll area (when scroll buttons
  // aren't displayed)
  int scroll_area_max_size;
  // The minimum width of the scroll area (when scroll buttons
  // are displayed)
  int scroll_area_min_size;

	bool m_bArrowPixmapsCreated;

	int m_nButtonsAreaSize;

	QList<QWidget> m_Entries;

	int m_nBoxWidth, m_nBoxHeight; 
	int m_nTrailingFreeSpace;

	bool m_bAllowMultipleRows;
	int m_nNumColumns; // number of possible columns

	int m_nMinSize, m_nMaxSize;
};
	
//
// class Taskbar
//
class Taskbar : public QObject
{
	Q_OBJECT
public:
	Taskbar(QWidget *parent, bool bDocked, int  nCurrentDesktop);

	void setOrientation(Orientation orientation) { getButtonsArea()->setOrientation(orientation); };
	myTaskButton *createTaskButton();
	void removeAll();
	bool isEmpty() { return (getNumEntries() <= 0); };
	void remove(myTaskButton *pEntry);
	void insert(myTaskButton *pEntry);
	void taskChangedDesktop(myTaskButton *pEntry, int nNewDesktop);

	ScrollableButtonsArea *getButtonsArea() { return m_pButtonsArea; };

	int getNumEntries() { return m_Buttons.count(); };
	myTaskButton *getEntryAt(int i) { return m_Buttons.at(i); };
	
	void setButtonWidthLimits(int nMin, int nMax);
	void setButtonSize(int nWidth, int nHeight);

	int getButtonWidth() { return m_nButtonWidth; };
	
	int getHeight() { return m_nHeight; };
	void setShowsDesktopSpecificTasks(bool); 
	bool isShowDesktopSpecificTasks() { return m_bShowDesktopSpecificTasks; };
	void changedCurrentDesktop(int n);
	void updateButtonArea();
	int getMaxLength() { return getButtonsArea()->getMaxLength(); };

public slots:
	void taskbarModeDesktop();
	void taskbarModeAll();

private:
	void insertIntoButtonArea(myTaskButton *pEntry);

protected:
	ScrollableButtonsArea *m_pButtonsArea;
	bool m_bDocked;
	bool m_bShowDesktopSpecificTasks;
	int m_nButtonWidth;
	int m_nHeight;
	QList<myTaskButton> m_Buttons;
	int m_nCurrentDesktop;
};

//
// class CResizeBar
//
class CResizeBar : public QFrame
{
	Q_OBJECT
public:
	CResizeBar(QWidget *parent, Orientation orientation, bool bFixedPos = false);

signals:
	void barMoved(int);

protected:
	virtual void mousePressEvent(QMouseEvent *e);
	virtual void mouseReleaseEvent(QMouseEvent *e);
	virtual void mouseMoveEvent(QMouseEvent *e);

	bool m_bMove;
	bool m_bFixedPos;
	Orientation m_Orientation;
	int m_nPrevPos;
};

//
// class DesktopSwitchButton 	
//
class DesktopSwitchButton : public QToolButton
{
  Q_OBJECT
public:
	DesktopSwitchButton(QWidget *parent, int nID);
	static int getPrefferedWidth() { return 16 + 6; };
	static int getMinimumHeight	() { return 16 + 5; };
protected:
	void drawButtonLabel(QPainter * pPainter);
	void createPixmap(int nID);

	QPixmap m_Pixmap;
};

//
// class ToolTip
//
class ToolTip : public QObject
{
	Q_OBJECT
public:
	ToolTip();

	void hide();
	void show();
	int getToolTipsDelay() { return menu_tool_tips; };
	void setToolTipsDelay(int n) { menu_tool_tips = n; };
	void enable(bool b) { menu_tool_tips_disabled = !b; };
	bool isVisible();

public slots:
	void showToolTip(QString s);

protected slots:
	void tipSleepTimerDone();
	void tipTimerDone();

protected:
	QLabel *info_label;
	QTimer *tipTimer;
	QTimer *tipSleepTimer;

	int menu_tool_tips;
	bool menu_tool_tips_disabled;
};

//
// KMenuEventFilter
//
class KMenuEventFilter : public QObject
{
	Q_OBJECT
public:
	KMenuEventFilter(ToolTip *pToolTip);

public slots:
	void addNewRootMenuItem();
	void addNewRootMenuFolder(); 

protected slots:
	void activated(int nId);
	void processExited(KProcess*);
	void processExitedRoot(KProcess*);

protected:
	bool eventFilter(QObject *, QEvent *);
	bool deleteItem(bool bCurrentVsSaved, const char *strDestDirectory);

	ToolTip *m_pToolTip;
	QPopupMenu *m_pCurrentPopup;
	QPopupMenu *m_pEditPopup;
	QPopupMenu *m_pNewPopup;

	PMenu * m_pCurrentMenu;
	int m_nCurrentId;

	bool m_bIsCopied;
	bool m_bIsCut;
	PMenu *m_pSavMenu;
	int m_nSavId;
};

//
// class kPanel
//
class kPanel : public QFrame
{
	Q_OBJECT
public:
	kPanel(KWMModuleApplication* kwmapp_arg,
	 QWidget *parent=0, const char *name=0 );
	~kPanel() {}
	
	
	void show();
	void doGeometry (bool do_not_change_taskbar = FALSE);
	
	void showSystem();
	PMenu* getKMenu() { return pmenu; };

public slots:

	// kwm communication
	void kwmInit();
	void windowAdd(Window);
	void dialogWindowAdd(Window);
	void windowRemove(Window);
	void windowChange(Window);
	void windowActivate(Window);
	void windowIconChanged(Window);
	void windowRaise(Window);
	void kwmDesktopChange(int);
	void kwmDesktopNameChange(int, QString);
	void kwmDesktopNumberChange(int);
	void kwmCommandReceived(QString);
	void dockWindowAdd(Window);
	void dockWindowRemove(Window);

/*   void playSound(QString); */

	void windowlistActivated(int);
	
	void taskbarClicked(myTaskButton*);
	void taskbarPressed(myTaskButton*);
	
	void button_clicked();
	void button_pressed();
	
	void add_windowlist();
	// --sven: kdisknav button start --
	void add_kdisknav();
	// --sven: kdisknav button end --
	void ask_logout();
	void call_help();
	void call_run();  // PAL - Oct98
	void call_klock();
	
	void showPanel();
	
	void hidePanelLeft ();
	void hidePanelRight();
	void showPanelFromLeft (bool smooth = TRUE);
	void showPanelFromRight(bool smooth = TRUE);
	
	void miniButtons (int); // sven
	void desktop_change(int);
	
	void standalonePanelButtonClicked();
	void standalonePanelButton2Clicked();
	
	void configurePanel();
	void editMenus();
		
	void to_desktop_menu_activated(int item); // PAL - Oct98
	
	void restart();
	
	
	void slotDropEvent( KDNDDropZone *_zone );
	
	void addButton(PMenuItem* pmi);
	
	void kdisplayPaletteChanged();
	
	void hideTimerDone();
	
	// the autoHideTaskbar feature
	void showTaskbar();
	void hideTaskbar();
	
	QWidget * parentOfSwallowed(Window);
	
	void launchSwallowedApplications();
	void parseMenus();

	bool addToMenuFromDrop(QString & strPath);

private slots:
	void slotUpdateClock();
	void resizeBarMoved(int);
	void desktopPreview();
	void desktopPreviewProcessExited(KProcess *);
	void slotHotSpot (int hs);

signals:

public:
	void arrow_on_pixmap(QPixmap* pm, ArrowType rt);
	void updateRecentMenuItems(const char *szItem, bool bRecentFilesChanged);

protected:
	void resizeEvent( QResizeEvent * );
	void enterEvent( QEvent *);
	void leaveEvent( QEvent * );
	
	bool eventFilter(QObject *, QEvent *);
	void mousePressEvent(QWidget *, QMouseEvent * );

private:
	// BI: 242853 -- 1998/12/23 -- Sam Watts
	// Add the ability to remove submenus from the Kmenu
	void     cleanupMenu(PMenu *menu, QStrList *list);
	void     writeInitStyle(KConfig *config, int size);
	bool     initConfigFile(KConfig *config);
	void     createPixmap();

	void startButtonMove(QWidget *pWidget2Move, QMouseEvent *evt = 0);
	void stopButtonMove(bool bCancel = false);
	void doButtonMove();

	QPixmap mBackTexture;
	KWMModuleApplication* kwmmapp;
	Window* callbacklist;
	
	DesktopEntriesList entries;
	
	ScrollableButtonsArea *m_pButtonsArea;
		
	myTaskButton* last_taskbar_button_pressed; // PAL - Oct98
	
	QFrame *control_group;
	QButtonGroup *desktopbar;
	QPushButton *exit_button;
	QPushButton *lock_button;
	QPushButton *panel_button;
	QPushButton *panel_button_standalone;
	QPushButton *panel_button2;
	QPushButton *panel_button_standalone2;
	QFrame *panel_button_frame_standalone;
	QFrame *panel_button_frame_standalone2;
	
	bool m_bTaskbarDocked;
	Taskbar *taskbar;
	myFrame* taskbar_frame;
	int taskbar_height;
	myTaskButton* taskButtonFromWindow(Window);
	
	QTimer *hideTimer;
	
	QPopupMenu *popup_item;
	
	RootMenu* pmenu;
	PMenu* pmenu_add;
	PMenu* p_pmenu;

	int wId;
	
	QLabel *label_date;
	QWidget *dock_area;
	
	QWidget *moving_button;
	QPoint moving_button_offset;
	// tu
	QPoint moving_button_oldpos;
	
	QWidget *wait_cursor_button;
	QLineEdit *edit_button; // used for the editable desktop buttons
	
	KDNDDropZone* drop_zone;
	
	QToolButton *m_pBtnDesktopPreview;

	// options
	Orientation orientation;
	Position position;
	TaskbarPosition taskbar_position;
	
	int box_width;
	int box_height;
	int margin;
	int dbhs;
	int dbrows;
	int number_of_desktops;
	
	int tbhs;
	int tbmhs;
	
	bool foldersFirst;
	bool autoHide;
	bool autoHidden;
	unsigned int autoHideDelay;
	int autoHideSpeed;
	
	bool autoHideTaskbar;
	unsigned int autoHideTaskbarDelay;
	int autoHideTaskbarSpeed;
	
	bool clockAmPm;
	bool clockBeats;
	
	int hide_show_animation;
	
	// tools
	QString findMenuEditor(const QString& folder);
	
	QPixmap create_arrow_pixmap(QPixmap pm);
	
	void set_button_text(QButton* button, const char* s);
	int show_popup(QPopupMenu* popup, QWidget* button, bool isTaskButton = False);
	void init_popup(QPopupMenu* popup);
	void set_label_date();
	void delete_button(QWidget* button);
	void cleanup();
	void restore_editbutton( bool takeit ); // Stephan
	
	void addButtonInternal(PMenuItem* pmi, int x = -1, int y = -1, QString name = "");
	void doPanelReposition();
	
	//layout
	void layoutDesktopBar();
	void layoutTaskbar(bool bSizeChangeOnly = false);
	int numberOfTaskbarRows();
	void layoutDockArea();
	
	void load_and_set_some_fonts();
	
	// kdisknav
	friend class PFileMenu;
	
	QStrList recent_folders;
	QStrList recent_files;
	
	uint max_recent_folders_entries;
	uint max_recent_files_entries;
	int head_recent_id;
	uint max_navigable_folder_entries;
	
	bool show_dot_files;
	bool ignore_case;   // ignore case when sorting
	bool show_shared_section;
	bool show_personal_section;
	bool show_recent_section;
	bool show_option_entry;
	bool bShowDiskNav;
	bool m_bDesktopSwitchButtonWithText;
	
	// development
	myPushButton* kde_button;
	
	QPopupMenu* windowlist;
	// --sven: kdisknav button start --
	myPopupMenu *m_pDiskNavMenu; //kdisknav; 
	// --sven: kdisknav button end --
	void generateWindowlist(QPopupMenu*);
	
	QPixmap load_pixmap(const char* name, bool is_folder = false);
	
	void writeOutConfiguration();
	void readInConfiguration();
	
	void showMiniPanel(); // sven
	void hideMiniPanel ();
	
	int currentDesktop;
	
	Bool panelHidden[8+1];
	Bool panelHiddenLeft[8+1];
	Bool panelCurrentlyHidden;
	Bool panelCurrentlyLeft;
	Bool miniPanelHidden;
	
	QFrame *miniPanelFrame;
	QButtonGroup *miniPanel;
	QPushButton *miniSystem;
	QPushButton *miniDesk;
	QPushButton *miniDiskNav;
	
	int clock_timer_id;
	
	bool initing;
	
	QStrList swallowed_applications;
	
	QRect taskbar_frame_geometry;
	
	QPixmap defaultPixmap;
	
	bool has_kdisknav_button;
	bool has_windowlist_button;
	int add_disknav_entry;
	int add_windowlist_entry;
	QPopupMenu* panel_popup;
	
	// contains items currently in Recent section of KMenu
	QList<PMenuItem> m_RecentMenuItems;

	QBoxLayout *m_pLayout; 

	QFrame *m_pDesktopPreviewFrame; // a parent frame of virtual dsktop preview
	KProcess *m_pPagerProcess;	

// variables used for kpanel reposition
	bool m_bPrepareMovePanel;
	KToolBoxManager *m_pMoveMgr;
	int m_nMovePos;

public:
	KMenuEventFilter *m_pKMenuEventFilter;
	ToolTip *m_pToolTip;
};

void arrow_on_pixmap(QWidget *w, QPixmap* pm, Qt::ArrowType rt);

#endif // KPANEL_H
