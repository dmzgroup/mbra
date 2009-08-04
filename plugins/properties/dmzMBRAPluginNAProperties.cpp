#include "dmzMBRAPluginNAProperties.h"
#include <dmzObjectCalc.h>
#include <dmzObjectConsts.h>
#include <dmzObjectModule.h>
#include <dmzQtModuleMainWindow.h>
#include <dmzRuntimeConfig.h>
#include <dmzRuntimeConfigToState.h>
#include <dmzRuntimeConfigToTypesBase.h>
#include <dmzRuntimeConfigToNamedHandle.h>
#include <dmzRuntimeData.h>
#include <dmzRuntimePluginFactoryLinkSymbol.h>
#include <dmzRuntimePluginInfo.h>
#include <dmzTypesHashTableStringTemplate.h>
#include <dmzTypesMask.h>

#include <QtGui/QtGui>
#include "ui_NodeProperties.h"

using namespace dmz;

namespace {

typedef dmz::MBRAPluginNAProperties::PropertyWidget pedit;
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

class LineWidget : public pedit {

   public:
      LineWidget (const Handle AttrHandle, const String &Name) :
            pedit (AttrHandle, Name) {;}

      virtual pupdate *create_widgets (
         const Handle Object,
         ObjectModule &module,
         QWidget *parent,
         QFormLayout *layout);

   protected:
      virtual ~LineWidget () {;}

   private:
      LineWidget ();
      LineWidget (const LineWidget &);
      LineWidget &operator= (const LineWidget &);
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

class TextWidget : public pedit {

   public:
      TextWidget (const Handle AttrHandle, const String &Name) :
            pedit (AttrHandle, Name) {;}

      virtual pupdate *create_widgets (
         const Handle Object,
         ObjectModule &module,
         QWidget *parent,
         QFormLayout *layout);

   protected:
      virtual ~TextWidget () {;}

   private:
      TextWidget ();
      TextWidget (const TextWidget &);
      TextWidget &operator= (const TextWidget &);
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

class ScalarWidget : public pedit {

   public:
      ScalarWidget (
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
      virtual ~ScalarWidget () {;}
      const int _Decimals;
      const double _Max;
      const double _Min;
      const String _Prefix;
      const double _Step;
      const String _Suffix;

   private:
      ScalarWidget ();
      ScalarWidget (const ScalarWidget &);
      ScalarWidget &operator= (const ScalarWidget &);
};

class CalcLabel : public pedit {

   public:
      CalcLabel (
         const Handle AttrHandle,
         const String &Name,
         ObjectAttributeCalculator *calc);

      virtual pupdate *create_widgets (
         const Handle Object,
         ObjectModule &module,
         QWidget *parent,
         QFormLayout *layout);

   protected:
      virtual ~CalcLabel () {;}

      ObjectAttributeCalculator *_calc;

   private:
      CalcLabel ();
      CalcLabel (const CalcLabel &);
      CalcLabel &operator= (const CalcLabel &);
};

class LinkLabel : public pedit {

   public:
      LinkLabel (
         const Handle AttrHandle,
         const String &Name,
         const Boolean Super);

      virtual pupdate *create_widgets (
         const Handle Object,
         ObjectModule &module,
         QWidget *parent,
         QFormLayout *layout);

   protected:
      virtual ~LinkLabel () {;}
      const Boolean _Super;

   private:
      LinkLabel ();
      LinkLabel (const LinkLabel &);
      LinkLabel &operator= (const LinkLabel &);
};

class StateWidget;

class StateUpdater : public pupdate {

   public:
      StateUpdater (const Handle AttrHandle, QComboBox *edit, StateWidget &table);
      virtual void update_object (const Handle Object, ObjectModule &module);

   protected:
      virtual ~StateUpdater () {;}
      QComboBox *_edit;
      StateWidget &_table;

   private:
      StateUpdater ();
      StateUpdater (const StateUpdater &);
      StateUpdater &operator= (const StateUpdater &);
};

class StateWidget : public pedit {

   public:
      StateWidget (const Handle AttrHandle, const String &Name) :
            pedit (AttrHandle, Name) {;}

      Boolean add_state (const String &Name, const Mask &State);
      void set_default_state_name (const String &Name);
      Mask get_state_mask () const;
      Mask lookup_state (const String &Name) const;

      virtual pupdate *create_widgets (
         const Handle Object,
         ObjectModule &module,
         QWidget *parent,
         QFormLayout *layout);

   protected:
      String _defaultStateName;
      Mask _mask;
      HashTableStringTemplate<Mask> _table;

      virtual ~StateWidget () { _table.empty (); }

   private:
      StateWidget ();
      StateWidget (const StateWidget &);
      StateWidget &operator= (const StateWidget &);
};


inline static Handle
get_real_object (const Handle Object, ObjectModule &module) {

   return module.is_link (Object) ?
      module.lookup_link_attribute_object (Object) :
      Object;
}

};


LineUpdater::LineUpdater (const Handle AttrHandle, QLineEdit *edit) :
      pupdate (AttrHandle),
      _edit (edit) {;}


void
LineUpdater::update_object (const Handle Object, ObjectModule &module) {

   const Handle RealObject = get_real_object (Object, module);

   if (_edit) {

      module.store_text (RealObject, AttrHandle, _edit->text ().toAscii ().data ());
   }
}


pupdate *
LineWidget::create_widgets (
      const Handle Object,
      ObjectModule &module,
      QWidget *parent,
      QFormLayout *layout) {

   const Handle RealObject = get_real_object (Object, module);

   QLabel *label = new QLabel (Name.get_buffer (), parent);

   String text;

   if (!module.lookup_text (RealObject, AttrHandle, text)) { text = ""; }

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

      const Handle RealObject = get_real_object (Object, module);

      module.store_text (
         RealObject,
         AttrHandle,
         _edit->toPlainText ().toAscii ().data ());
   }
}


pupdate *
TextWidget::create_widgets (
      const Handle Object,
      ObjectModule &module,
      QWidget *parent,
      QFormLayout *layout) {

   const Handle RealObject = get_real_object (Object, module);

   QLabel *label = new QLabel (Name.get_buffer (), parent);

   String text;

   if (!module.lookup_text (RealObject, AttrHandle, text)) { text = ""; }

   QTextEdit *edit= new QTextEdit (text.get_buffer (), parent);

   layout->addRow (label, edit);

   return new TextUpdater (AttrHandle, edit);
}


ScalarUpdater::ScalarUpdater (const Handle AttrHandle, QDoubleSpinBox *edit) :
      pupdate (AttrHandle),
      _edit (edit) {;}


void
ScalarUpdater::update_object (const Handle Object, ObjectModule &module) {

   const Handle RealObject = get_real_object (Object, module);
   if (_edit) { module.store_scalar (RealObject, AttrHandle, _edit->value ()); }
}


ScalarWidget::ScalarWidget (
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
ScalarWidget::create_widgets (
      const Handle Object,
      ObjectModule &module,
      QWidget *parent,
      QFormLayout *layout) {

   const Handle RealObject = get_real_object (Object, module);

   QLabel *label = new QLabel (Name.get_buffer (), parent);

   Float64 value (0.0);

   module.lookup_scalar (RealObject, AttrHandle, value);

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


CalcLabel::CalcLabel (
      const Handle AttrHandle,
      const String &Name,
      ObjectAttributeCalculator *calc) :
      pedit (AttrHandle, Name),
      _calc (calc) {;}


pupdate *
CalcLabel::create_widgets (
      const Handle Object,
      ObjectModule &module,
      QWidget *parent,
      QFormLayout *layout) {

   const Handle RealObject = get_real_object (Object, module);

   QLabel *label = new QLabel (Name.get_buffer (), parent);

   Float64 value (0.0);

   if (_calc) {

      _calc->store_object_module (&module);
      value = _calc->calculate (RealObject);
      _calc->store_object_module (0);
   }

   QLabel *data = new QLabel (parent);
   data->setNum (value);

   layout->addRow (label, data);

   return 0;
}


LinkLabel::LinkLabel (
      const Handle AttrHandle,
      const String &Name,
      const Boolean Super) :
      pedit (AttrHandle, Name),
      _Super (Super) {;}


pupdate *
LinkLabel::create_widgets (
      const Handle Object,
      ObjectModule &module,
      QWidget *parent,
      QFormLayout *layout) {

   QLabel *label = new QLabel (Name.get_buffer (), parent);

   Handle linkAttr (0);
   Handle sub (0);
   Handle super (0);

   module.lookup_linked_objects (Object, linkAttr, super, sub);

   String value;

   if (_Super) { module.lookup_text (super, AttrHandle, value); }
   else { module.lookup_text (sub, AttrHandle, value); }

   QLabel *data = new QLabel (value.get_buffer (), parent);

   layout->addRow (label, data);

   return 0;
}


StateUpdater::StateUpdater (
      const Handle AttrHandle,
      QComboBox *edit,
      StateWidget &table) :
      pupdate (AttrHandle),
      _edit (edit),
      _table (table) {;}


void
StateUpdater::update_object (const Handle Object, ObjectModule &module) {

   const Handle RealObject = get_real_object (Object, module);

   if (_edit) {

      Mask state;
      module.lookup_state (RealObject, AttrHandle, state);

      const Mask StateMask = _table.get_state_mask ();
      const Mask Value = _table.lookup_state (_edit->currentText ().toAscii ().data ());

      state.unset (StateMask);
      state |= Value;

      module.store_state (RealObject, AttrHandle, state);
   }
}


Boolean
StateWidget::add_state (const String &Name, const Mask &State) {

   Mask *ptr = new Mask (State);

   if (ptr && !_table.store (Name, ptr)) { delete ptr; ptr = 0; }
   else if (ptr) { _mask |= State; }

   return ptr != 0;
}


void
StateWidget::set_default_state_name (const String &Name) { _defaultStateName = Name; }


Mask
StateWidget::get_state_mask () const { return _mask; }


Mask
StateWidget::lookup_state (const String &Name) const {

   Mask *ptr = _table.lookup (Name);

   return ptr ? *ptr : Mask ();
}


pupdate *
StateWidget::create_widgets (
      const Handle Object,
      ObjectModule &module,
      QWidget *parent,
      QFormLayout *layout) {

   const Handle RealObject = get_real_object (Object, module);

   QLabel *label = new QLabel (Name.get_buffer (), parent);

   Mask state;

   String stateName (_defaultStateName);

   if (module.lookup_state (RealObject, AttrHandle, state)) {

      HashTableStringIterator it;
      Mask *current (0);

      Boolean done = False;

      while (!done && _table.get_next (it, current)) {

         if (state.contains (*current)) { stateName = it.get_hash_key (); done = true; }
      }
   }
   else {

   }

   QComboBox *edit = new QComboBox (parent);

   HashTableStringIterator it;
   Mask *current (0);

   while (_table.get_next (it, current)) {

      edit->addItem (it.get_hash_key ().get_buffer ());
   }

   if (stateName) {

      const int Index = edit->findText (stateName.get_buffer ());
      if (Index >= 0) { edit->setCurrentIndex (Index); }
   }

   layout->addRow (label, edit);

   return new StateUpdater (AttrHandle, edit, *this);
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
      _objectWidgets (0),
      _linkWidgets (0) {

   _init (local);
}


dmz::MBRAPluginNAProperties::~MBRAPluginNAProperties () {

   if (_objectWidgets) { delete _objectWidgets; _objectWidgets = 0; }
   if (_linkWidgets) { delete _linkWidgets; _linkWidgets = 0; }
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

      PropertyWidget *pe (_objectWidgets);
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

         const Handle UndoHandle = _undo.start_record ("Edit Node");

         current = head;

         while (current) {

            current->update_object (Object, *_objMod);
            current = current->next;
         }

         _undo.stop_record (UndoHandle);
      }
      else if (Created) { _objMod->destroy_object (Object); }

      if (head) { delete head; head = 0; }
   }
}


void
dmz::MBRAPluginNAProperties::_edit_link (const Handle Link, const Boolean Created) {

   if (_objMod && _window) {

      QDialog dialog (_window->get_widget ());

      Ui::NodeProperties ui;
      ui.setupUi (&dialog);
      dialog.setWindowTitle ("Link Properties");

      QFormLayout *layout = new QFormLayout (ui.attributes);

      PropertyWidget *pe (_linkWidgets);
      PropertyUpdater *head (0), *current (0);

      while (pe) {

         PropertyUpdater *next = pe->create_widgets (
            Link,
            *_objMod,
            &dialog,
            layout);

         if (!current) { head = current = next; }
         else if (next) { current->next = next; current = next; }

         pe = pe->next;
      }

      if (dialog.exec () == QDialog::Accepted) {

         const Handle UndoHandle = _undo.start_record ("Edit Link");

         current = head;

         while (current) {

            current->update_object (Link, *_objMod);
            current = current->next;
         }

         _undo.stop_record (UndoHandle);
      }
      else if (Created) { _objMod->unlink_objects (Link); }

      if (head) { delete head; head = 0; }
   }

}


dmz::MBRAPluginNAProperties::PropertyWidget *
dmz::MBRAPluginNAProperties::_create_widgets (Config &list) {

   ConfigIterator it;
   Config widget;

   PropertyWidget *result (0);

   while (list.get_prev_config (it, widget)) {

      PropertyWidget *pe (0);

      const String Type = config_to_string ("type", widget);
      const String Name = config_to_string ("name", widget);
      const Handle AttrHandle = _defs.create_named_handle (
         config_to_string ("attribute", widget, ObjectAttributeDefaultName));

      if (Type == "line") { pe = new LineWidget (AttrHandle, Name); }
      else if (Type == "text") { pe = new TextWidget (AttrHandle, Name); }
      else if (Type == "scalar") {

         const int Decimals = (int)config_to_int32 ("decimals", widget, 2);
         const double Max = config_to_float64 ("max", widget, 1e+10);
         const double Min = config_to_float64 ("min", widget, 0);
         const String Prefix = config_to_string ("prefix", widget);
         const double Step = config_to_float64 ("step", widget, 1000);
         const String Suffix = config_to_string ("suffix", widget);

         pe = new ScalarWidget (
            AttrHandle,
            Name,
            Decimals,
            Max,
            Min,
            Prefix,
            Step,
            Suffix);
      }
      else if (Type == "calc-label") {

         ConfigIterator it;
         Config root;
         widget.get_first_config (it, root);

         ObjectAttributeCalculator *calc = config_to_object_attribute_calculator (
            "",
            root,
            get_plugin_runtime_context (),
            &_log);

         pe = new CalcLabel (AttrHandle, Name, calc);
      }
      else if (Type == "state") {

         StateWidget *se = new StateWidget (AttrHandle, Name);

         if (se) {

            Config stateList;

            widget.lookup_all_config ("state", stateList);

            ConfigIterator it;
            Config state;

            while (stateList.get_next_config (it, state)) {

               const String Name = config_to_string ("label", state);

               se->add_state (
                  Name,
                  config_to_state ("name", state, get_plugin_runtime_context ()));

               if (config_to_boolean ("default", state, False)) {

                  se->set_default_state_name (Name);
               }
            }

            pe = se;
         }
      }
      else if (Type == "link-label") {

         pe = new LinkLabel (AttrHandle, Name, config_to_boolean ("super", widget));
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

   if (local.lookup_all_config ("object-widget-list.widget", list)) {

      _objectWidgets = _create_widgets (list);
   }

   if (local.lookup_all_config ("link-widget-list.widget", list)) {

      _linkWidgets = _create_widgets (list);
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
