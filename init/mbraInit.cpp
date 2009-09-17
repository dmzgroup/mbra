#include "mbraInit.h"
#include <dmzApplication.h>
#include <dmzAppShellExt.h>
#include <dmzCommandLine.h>
#include <dmzQtConfigRead.h>
#include <dmzQtConfigWrite.h>
#include <dmzRuntimeConfig.h>
#include <dmzRuntimeConfigToTypesBase.h>
#include <dmzRuntimePluginFactoryLinkSymbol.h>
#include <dmzRuntimeSession.h>
#include <dmzRuntimeVersion.h>
#include <dmzSystemFile.h>
#include <dmzTypesHashTableStringTemplate.h>
#include <dmzXMLUtil.h>
#include <ui_mbraDialog.h>

#include <QtCore/QUrl>
#include <QtGui/QDesktopServices>
#include <QtGui/QCloseEvent>

using namespace dmz;

#include <qdb.h>
static qdb out;

namespace {

static const String MBRAName ("mbra-init");
static const String MBRAFileList ("mbra-file-list");
static const String GeometryName ("geometry");
static const String HeaderStateName ("header-state");

static void
local_restore_session (AppShellInitStruct &init, mbraInit &cInit) {

   Config session = get_session_config (MBRAName, init.app.get_context ());

   Config geometry;

   if (session.lookup_config (GeometryName, geometry)) {

      cInit.restoreGeometry (config_to_qbytearray (geometry));
   }
   else {

      QRect rect = QApplication::desktop ()->availableGeometry (&cInit);
      cInit.move(rect.center () - cInit.rect ().center ());
   }

   Config state;

   if (session.lookup_config (HeaderStateName, state)) {

      QHeaderView *header = cInit.ui.fileTable->horizontalHeader ();

      if (header) { header->restoreState (config_to_qbytearray (state)); }
   }
}


static const Boolean
find_mbra_files (QDialog & dialog, const String &Path, Int32 depth, PathContainer &list) {

   Boolean result (True);

   depth++;

   // Only go 8 directories deep
   if (depth <= 8) {

      PathContainer fileList;

      if (get_file_list (Path, fileList)) {

         dmz::PathContainerIterator it;
         String file;

         while (fileList.get_next (it, file)) {

            String tmp, fileRoot, fileExt;

            split_path_file_ext (file, tmp, fileRoot, fileExt);

            if (fileExt == ".mbra") { list.add_path (Path + file); }
         }
      }

      PathContainer pathList;

      QApplication::sendPostedEvents (0, -1);
      QApplication::processEvents ();
      if (!dialog.isVisible ()) { result = False; }

      if (get_directory_list (Path, pathList)) {

         dmz::PathContainerIterator it;
         String dirName;

         while (result && pathList.get_next (it, dirName)) {

#if defined(__APPLE__) || defined(MACOSX)
            // We need to skip Directories called "Library on the Mac.
            if (dirName != "Library") {

               result = find_mbra_files (dialog, Path + dirName + "/", depth, list);
            }
#else
            result = find_mbra_files (dialog, Path + dirName + "/", depth, list);
#endif
         }
      }
   }

   return result;
}


static const void
local_init_file_list (AppShellInitStruct &init, PathContainer &list) {

   Config session = get_session_config (MBRAFileList, init.app.get_context ());

   if (session) {

      ConfigIterator it;
      Config file;

      while (session.get_next_config (it, file)) {

         const String FileName = config_to_string ("name", file);

         if (is_valid_path (FileName)) { list.add_path (FileName); }
      }
   }
   else {

      QDialog dialog;
      Ui::mbraDialog ui;
      ui.setupUi (&dialog);
      dialog.setModal (true);
      QRect rect = QApplication::desktop ()->availableGeometry (&dialog);
      dialog.move(rect.center () - dialog.rect ().center ());
      dialog.show ();
      dialog.raise ();
      Int32 depth = 0;
      find_mbra_files (dialog, get_home_directory () + "/", depth, list);
   }

   if (list.get_count () > 0) {

      Config fileList (MBRAFileList);

      PathContainerIterator it;
      String file;

      while (list.get_next (it, file)) {

         Config data ("file");
         data.store_attribute ("name", file);
         fileList.add_config (data);
      }

      set_session_config (init.app.get_context (), fileList);
   }
}

};


dmz::mbraInit::mbraInit (AppShellInitStruct &theInit) :
      init (theInit),
      launchNA (False),
      launchFT (False),
      _start (False) {

   ui.setupUi (this);

#if (QT_VERSION < QT_VERSION_CHECK(4, 5, 0))
   ui.fileTable->horizontalHeader()->setDefaultSectionSize(200);
   ui.fileTable->horizontalHeader()->setStretchLastSection(true);
   ui.fileTable->verticalHeader()->setVisible(false);
   ui.fileTable->verticalHeader()->setHighlightSections(true);
#endif

   PathContainer list;
   local_init_file_list (init, list);

   PathContainerIterator it;
   String file;
   int row = 0;

   ui.fileTable->setSortingEnabled (false);

   while (list.get_next (it, file)) {

      QFileInfo info (file.get_buffer ());
      ui.fileTable->setRowCount (row + 1);
      QTableWidgetItem *name = new QTableWidgetItem (
         info.completeBaseName () + "." + info.completeSuffix ());
      name->setData (Qt::UserRole, info.absoluteFilePath ());
      QTableWidgetItem *date = new QTableWidgetItem;
      date->setData (Qt::DisplayRole, info.lastModified ());
      ui.fileTable->setItem (row, 0, name);
      ui.fileTable->setItem (row, 1, date);
      row++;
   }

   ui.fileTable->setSortingEnabled (true);
}


dmz::mbraInit::~mbraInit () {

}


void
dmz::mbraInit::on_fileTable_itemDoubleClicked (QTableWidgetItem * item) {

   on_buttonBox_accepted ();
}


void
dmz::mbraInit::on_naButton_clicked () {

   launchNA = True;
   _start = True;
   close ();
}


void
dmz::mbraInit::on_ftButton_clicked () {

   launchFT = True;
   _start = True;
   close ();
}


void
dmz::mbraInit::on_buttonBox_accepted () {

   QList<QTableWidgetItem *> list = ui.fileTable->selectedItems ();

   if (!list.isEmpty ()) {

      QTableWidgetItem *item = list.first ();

      if (item) { selectedFile = item->data (Qt::UserRole).toString (); }
   }
   else { launchNA = True; }

   _start = True;
   close ();
}     

   
void  
dmz::mbraInit::on_buttonBox_rejected () {
      
   close ();
}  
      

void
dmz::mbraInit::on_buttonBox_helpRequested () {

   const String UrlValue =
      config_to_string ("help.url", init.manifest, "http://dmzdev.org/wiki/mbra");

   if (UrlValue) {

      QUrl Url (UrlValue.get_buffer ());

      QDesktopServices::openUrl (Url);
   }
}


void
dmz::mbraInit::closeEvent (QCloseEvent * event) {

   if (!_start) {

      init.app.quit ("Cancel Button Pressed");
   }
   else {

      Config session (MBRAName);

      session.add_config (qbytearray_to_config (GeometryName, saveGeometry ()));

      QHeaderView *header = ui.fileTable->horizontalHeader ();

      if (header) {

         session.add_config (
            qbytearray_to_config (HeaderStateName, header->saveState ()));
      }
   
      set_session_config (init.app.get_context (), session);
   }

   event->accept ();
}


extern "C" {

DMZ_PLUGIN_FACTORY_LINK_SYMBOL void
dmz_init_mbra (AppShellInitStruct &init) {

   init.fileListPopulated = False;

   if (!init.launchFile) {

      mbraInit minit (init);

      local_restore_session (init, minit);

      if (init.VersionFile) {

         Version version;

         if (xml_to_version (init.VersionFile, version, &init.app.log)) {

            QString vs = minit.windowTitle ();
            vs += " (v";
            const String Tmp = version.get_version ().get_buffer ();
            if (Tmp) { vs += Tmp.get_buffer (); }
            else { vs += "Unknown"; }
            vs += ")";

            minit.setWindowTitle (vs);
         }
      }

      minit.show ();
      minit.raise ();

      while (minit.isVisible ()) {

         QApplication::sendPostedEvents (0, -1);
         QApplication::processEvents (QEventLoop::WaitForMoreEvents);
      }

      if (init.app.is_running ()) {

         if (minit.launchNA || minit.launchFT) {

            CommandLineArgs args ("f");

            if (minit.launchNA) { args.append_arg ("config/start_network.xml"); }
            else if (minit.launchFT) { args.append_arg ("config/start_fault_tree.xml"); }

            CommandLine cl;
            cl.add_args (args);
            init.app.process_command_line (cl);
         }
         else if (!minit.selectedFile.isEmpty ()) {

            init.launchFile = qPrintable (minit.selectedFile);
         }
      }
   }
}

};
