#ifndef MBRA_INIT_DOT_H
#define MBRA_INIT_DOT_H

#include <dmzAppShellExt.h>
#include <QtGui/QtGui>

#include <ui_mbraLaunch.h>

namespace dmz {

class mbraInit : public QWidget {

   Q_OBJECT

   public:
      mbraInit (AppShellInitStruct &init);
      ~mbraInit ();

      AppShellInitStruct &init;
      Ui::mbraLaunch ui;
      QString selectedFile;

   protected slots:
      void on_fileTable_itemDoubleClicked (QTableWidgetItem * item);
      void on_naButton_clicked ();
      void on_ftButton_clicked ();
      void on_buttonBox_accepted ();
      void on_buttonBox_rejected ();
      void on_buttonBox_helpRequested ();

   protected:
      virtual void closeEvent (QCloseEvent * event);

      Boolean _start;
};

};

#endif // MBRA_INIT_DOT_H
