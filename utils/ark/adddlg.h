#ifndef ADDDLG_H
#define ADDDLG_H

#include <qdialog.h>
#include <qpushbt.h>
#include <qgrpbox.h>
#include <qchkbox.h>

class AddOptionsDlg : public QDialog {
	Q_OBJECT
public:
	AddOptionsDlg( QWidget *parent=0, const char *name=0 );
	bool onlyUpdate();
	bool storeFullPath();
        void resizeEvent(QResizeEvent *e);
private:
	QPushButton *ok;
	QPushButton *cancel;
	QGroupBox *gb;
	QCheckBox *updatecb;
	QCheckBox *fullcb;
};

#endif

