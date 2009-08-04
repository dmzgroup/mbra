#include "dmzMBRAPluginNAProperties.h"
#include <dmzObjectCalc.h>
#include <dmzObjectConsts.h>
#include <dmzObjectModule.h>
#include <dmzQtModuleMainWindow.h>
#include <dmzRuntimeConfig.h>
#include <dmzRuntimeConfigToTypesBase.h>
#include <dmzRuntimeConfigToNamedHandle.h>
#include <dmzRuntimeData.h>
#include <dmzRuntimePluginFactoryLinkSymbol.h>
#include <dmzRuntimePluginInfo.h>

#include <QtGui/QtGui>
#include "ui_NodeProperties.h"

using namespace dmz;

namespace {

typedef dmz::MBRAPluginNAProperties::PropertyEditor pedit;
typedef dmz::MBRAPluginNAProperties::PropertyUpdater pupdate;

class LineUpdater : public pupdate {

   public:
      LineUpdater (const Handle AttrHandle, QLineEdit *edit);
      virtual void update_object (const Handle Object, ObjectModule &module);

   protected:
      virtual ~LineUpdater () {;}
      QLineEdit *_edit;

   private:
      LineUpdater ();
      LineUpdater (const LineUpdater &);
      LineUpdater &operator= (const LineUpdater &);
};

class LineEditor : public pedit {

   public:
      LineEditor (const Handle AttrHandle, const String &Name) :
            pedit (AttrHandle, Name) {;}

      virtual pupdate *create_widgets (
         const Handle Object,
         ObjectModule &module,
         QWidget *parent,
         QFormLayout *layout);

   protected:
      virtual ~LineEditor () {;}

   private:
      LineEditor ();
      LineEditor (const LineEditor &);
      LineEditor &operator= (const LineEditor &);
};

class TextUpdater : public pupdate {

   public:
      TextUpdater (const Handle AttrHandle, QTextEdit *edit);
      virtual void update_object (const Handle Object, ObjectModule &module);

   protected:
      virtual ~TextUpdater () {;}
      QTextEdit *_edit;

   private:
      TextUpdater ();
      TextUpdater (const TextUpdater &);
      TextUpdater &operator= (const TextUpdater &);
};

class TextEditor : public pedit {

   public:
      TextEditor (const Handle AttrHandle, const String &Name) :
            pedit (AttrHandle, Name) {;}

      virtual pupdate *create_widgets (
         const Handle Object,
         ObjectModule &module,
         QWidget *parent,
         QFormLayout *layout);

   protected:
      virtual ~TextEditor () {;}

   private:
      TextEditor ();
      TextEditor (const TextEditor &);
      TextEditor &operator= (const TextEditor &);
};

class ScalarUpdater : public pupdate {

   public:
      ScalarUpdater (const Handle AttrHandle, QDoubleSpinBox *edit);
      virtual void update_object (const Handle Object, ObjectModule &module);

   protected:
      virtual ~ScalarUpdater () {;}
      QDoubleSpinBox *_edit;

   private:
      ScalarUpdater ();
      ScalarUpdater (const ScalarUpdater &);
      ScalarUpdater &operator= (const ScalarUpdater &);
};

class ScalarEditor : public pedit {

   public:
      ScalarEditor (
         const Handle AttrHandle,
         const String &Name,
         const int Decimals,
         const double Max,
         const double Min,
         const String Prefix,
         const double Step,
         const String Suffix);

      virtual pupdate *create_widgets (
         const Handle Object,
         ObjectModule &module,
         QWidget *parent,
         QFormLayout *layout);

   protected:
      virtual ~ScalarEditor () {;}
      const int _Decimals;
      const double _Max;
      const double _Min;
      const String _Prefix;
      const double _Step;
      const String _Suffix;

   private:
      ScalarEditor ();
      ScalarEditor (const ScalarEditor &);
      ScalarEditor &operator= (const ScalarEditor &);
};

class CalcEditor : public pedit {

   public:
      CalcEditor (
         const Handle AttrHandle,
         const String &Name,
         ObjectAttributeCalculator *calc);

      virtual pupdate *create_widgets (
         const Handle Object,
         ObjectModule &module,
         QWidget *parent,
         QFormLayout *layout);

   protected:
      virtual ~CalcEditor () {;}

      ObjectAttributeCalculator *_calc;

   private:
      CalcEditor ();
      CalcEditor (const CalcEditor &);
      CalcEditor &operator= (const CalcEditor &);
};

};

LineUpdater::LineUpdater (const Handle AttrHandle, QLineEdit *edit) :
      pupdate (AttrHandle),
      _edit (edit) {;}


void
LineUpdater::update_object (const Handle Object, ObjectModule &module) {

   if (_edit) {

      module.store_text (Object, AttrHandle, _edit->text ().toAscii ().data ());
   }
}


pupdate *
LineEditor::create_widgets (
      const Handle Object,
      ObjectModule &module,
      QWidget *parent,
      QFormLayout *layout) {

   QLabel *label = new QLabel (Name.get_buffer (), parent);

   String text;

   if (!module.lookup_text (Object, AttrHandle, text)) { text = ""; }

   QLineEdit *edit= new QLineEdit (text.get_buffer (), parent);

   layout->addRow (label, edit);

   return new LineUpdater (AttrHandle, edit);
}


TextUpdater::TextUpdater (const Handle AttrHandle, QTextEdit *edit) :
      pupdate (AttrHandle),
      _edit (edit) {;}


void
TextUpdater::update_object (const Handle Object, ObjectModule &module) {

   if (_edit) {

      module.store_text (Object, AttrHandle, _edit->toPlainText ().toAscii ().data ());
   }
}


pupdate *
TextEditor::create_widgets (
      const Handle Object,
      ObjectModule &module,
      QWidget *parent,
      QFormLayout *layout) {

   QLabel *label = new QLabel (Name.get_buffer (), parent);

   String text;

   if (!module.lookup_text (Object, AttrHandle, text)) { text = ""; }

   QTextEdit *edit= new QTextEdit (text.get_buffer (), parent);

   layout->addRow (label, edit);

   return new TextUpdater (AttrHandle, edit);
}


ScalarUpdater::ScalarUpdater (const Handle AttrHandle, QDoubleSpinBox *edit) :
      pupdate (AttrHandle),
      _edit (edit) {;}


void
ScalarUpdater::update_object (const Handle Object, ObjectModule &module) {

   if (_edit) { module.store_scalar (Object, AttrHandle, _edit->value ()); } }


ScalarEditor::ScalarEditor (
      const Handle AttrHandle,
      const String &Name,
      const int Decimals,
      const double Max,
      const double Min,
      const String Prefix,
      const double Step,
      const String Suffix) :
      pedit (AttrHandle, Name),
      _Decimals (Decimals),
      _Max (Max),
      _Min (Min),
      _Prefix (Prefix),
      _Step (Step),
      _Suffix (Suffix) {;}


pupdate *
ScalarEditor::create_widgets (
      const Handle Object,
      ObjectModule &module,
      QWidget *parent,
      QFormLayout *layout) {

   QLabel *label = new QLabel (Name.get_buffer (), parent);

   Float64 value (0.0);

   module.lookup_scalar (Object, AttrHandle, value);

   QDoubleSpinBox *edit = new QDoubleSpinBox (parent);
   edit->setDecimals (_Decimals);
   edit->setRange (_Min, _Max),
   edit->setPrefix (_Prefix.get_buffer ());
   edit->setSingleStep (_Step);
   edit->setSuffix (_Suffix.get_buffer ());
   edit->setValue (value);

   layout->addRow (label, edit);

   return new ScalarUpdater (AttrHandle, edit);
}


CalcEditor::CalcEditor (
      const Handle AttrHandle,
      const String &Name,
      ObjectAttributeCalculator *calc) :
      pedit (AttrHandle, Name),
      _calc (calc) {;}


#include <qdb.h>
static qdb out;
pupdate *
CalcEditor::create_widgets (
      const Handle Object,
      ObjectModule &module,
      QWidget *parent,
      QFormLayout *layout) {

   QLabel *label = new QLabel (Name.get_buffer (), parent);

   Float64 value (0.0);

   if (_calc) {

      _calc->store_object_module (&module);
      value = _calc->calculate (Object);
out << "Value: " << value << endl;
      _calc->store_object_module (0);
   }

   QLabel *data = new QLabel (parent);
   data->setNum (value);

   layout->addRow (label, data);

   return 0;
}

// Start MBRAPluginNAProperties class
dmz::MBRAPluginNAProperties::MBRAPluginNAProperties (
      const PluginInfo &Info,
      Config &local) :
      Plugin (Info),
      MessageObserver (Info),
      _log (Info),
      _undo (Info),
      _defs (Info),
      _objMod (0),
      _window (0),
      _objectDataHandle (0),
      _createdDataHandle (0),
      _objectEditors (0) {

   _init (local);
}


dmz::MBRAPluginNAProperties::~MBRAPluginNAProperties () {

   if (_objectEditors) { delete _objectEditors; _objectEditors = 0; }
}


// Plugin Interface
void
dmz::MBRAPluginNAProperties::update_plugin_state (
      const PluginStateEnum State,
      const UInt32 Level) {

   if (State == PluginStateInit) {

   }
   else if (State == PluginStateStart) {

   }
   else if (State == PluginStateStop) {

   }
   else if (State == PluginStateShutdown) {

   }
}


void
dmz::MBRAPluginNAProperties::discover_plugin (
      const PluginDiscoverEnum Mode,
      const Plugin *PluginPtr) {

   if (Mode == PluginDiscoverAdd) {

      if (!_objMod) { _objMod = ObjectModule::cast (PluginPtr); }
      if (!_window) { _window = QtModuleMainWindow::cast (PluginPtr); }
   }
   else if (Mode == PluginDiscoverRemove) {

      if (_objMod && (_objMod == ObjectModule::cast (PluginPtr))) { _objMod = 0; }
      if (_window && (_window == QtModuleMainWindow::cast (PluginPtr))) { _window = 0; }
   }
}


// Message Observer Interface
void
dmz::MBRAPluginNAProperties::receive_message (
      const Message &Type,
      const UInt32 MessageSendHandle,
      const Handle TargetObserverHandle,
      const Data *InData,
      Data *outData) {

   if (Type == _editMessage) {

      if (InData && _objMod) {

         Handle object (0);

         if (InData->lookup_handle (_objectDataHandle, 0, object)) {

            Handle createdHandle (0);
            InData->lookup_handle (_createdDataHandle, 0, createdHandle);

            qApp->processEvents ();

            if (_objMod->is_object (object)) {

               _edit_node (object, createdHandle != 0);
            }
            else if (_objMod->is_link (object)) {

               _edit_link (object, createdHandle != 0);
            }
         }
      }
   }
}


// dmzMBRAPluginNAProperties Interface
void
dmz::MBRAPluginNAProperties::_edit_node (const Handle Object, const Boolean Created) {

   if (_objMod && _window) {

      QDialog dialog (_window->get_widget ());

      Ui::NodeProperties ui;
      ui.setupUi (&dialog);

      QFormLayout *layout = new QFormLayout (ui.attributes);

      PropertyEditor *pe (_objectEditors);
      PropertyUpdater *head (0), *current (0);

      while (pe) {

         PropertyUpdater *next = pe->create_widgets (
            Object,
            *_objMod,
            &dialog,
            layout);

         if (!current) { head = current = next; }
         else if (next) { current->next = next; current = next; }

         pe = pe->next;
      }

      if (dialog.exec () == QDialog::Accepted) {

         current = head;

         while (current) {

            current->update_object (Object, *_objMod);
            current = current->next;
         }
      }
      else if (Created) { _objMod->destroy_object (Object); }

      if (head) { delete head; head = 0; }
   }
}


void
dmz::MBRAPluginNAProperties::_edit_link (const Handle Link, const Boolean Created) {

}


dmz::MBRAPluginNAProperties::PropertyEditor *
dmz::MBRAPluginNAProperties::_create_editors (Config &list) {

   ConfigIterator it;
   Config editor;

   PropertyEditor *result (0);

   while (list.get_prev_config (it, editor)) {

      PropertyEditor *pe (0);

      const String Type = config_to_string ("type", editor);
      const String Name = config_to_string ("name", editor);
      const Handle AttrHandle = _defs.create_named_handle (
         config_to_string ("attribute", editor, ObjectAttributeDefaultName));

      if (Type == "line") { pe = new LineEditor (AttrHandle, Name); }
      else if (Type == "text") { pe = new TextEditor (AttrHandle, Name); }
      else if (Type == "scalar") {

         const int Decimals = (int)config_to_int32 ("decimals", editor, 2);
         const double Max = config_to_float64 ("max", editor, 1e+10);
         const double Min = config_to_float64 ("min", editor, 0);
         const String Prefix = config_to_string ("prefix", editor);
         const double Step = config_to_float64 ("step", editor, 1000);
         const String Suffix = config_to_string ("suffix", editor);

         pe = new ScalarEditor (
            AttrHandle,
            Name,
            Decimals,
            Max,
            Min,
            Prefix,
            Step,
            Suffix);
      }
      else if (Type == "calc") {

         ConfigIterator it;
         Config root;
         editor.get_first_config (it, root);

         ObjectAttributeCalculator *calc = config_to_object_attribute_calculator (
            "",
            root,
            get_plugin_runtime_context (),
            &_log);

if (!calc) { out << "Not calc was create!" << endl; }

         pe = new CalcEditor (AttrHandle, Name, calc);
      }

      if (pe) { pe->next = result; result = pe; }
   }

   return result;
}


void
dmz::MBRAPluginNAProperties::_init (Config &local) {

   RuntimeContext *context = get_plugin_runtime_context ();

   _editMessage = config_create_message (
      "message.edit",
      local,
      "EditObjectAttributesMessage",
      context);

   subscribe_to_message (_editMessage);

   _objectDataHandle =
      config_to_named_handle ("attribute.object.name", local, "object", context);

   _createdDataHandle =
      config_to_named_handle ("attribute.created.name", local, "created", context);

   Config list;

   if (local.lookup_all_config ("object-editor-list.editor", list)) {

      _objectEditors = _create_editors (list);
   }
}


extern "C" {

DMZ_PLUGIN_FACTORY_LINK_SYMBOL dmz::Plugin *
create_dmzMBRAPluginNAProperties (
      const dmz::PluginInfo &Info,
      dmz::Config &local,
      dmz::Config &global) {

   return new dmz::MBRAPluginNAProperties (Info, local);
}

};
