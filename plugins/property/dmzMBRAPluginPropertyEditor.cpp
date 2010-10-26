#include "dmzMBRAPluginPropertyEditor.h"
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
#include "ui_PropertyEditor.h"

using namespace dmz;

namespace {

struct TabStruct {

   QWidget *widget;
   QFormLayout *layout;

   TabStruct () : widget (0), layout (0) {;}
};

typedef dmz::MBRAPluginPropertyEditor::PropertyWidget pedit;
typedef dmz::MBRAPluginPropertyEditor::PropertyUpdater pupdate;

class LineUpdater : public pupdate {

   public:
      LineUpdater (const Handle AttrHandle, QLineEdit *edit);
      virtual void update_object (const Handle Object, ObjectModule &module);
      virtual QWidget *get_widget ();

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
      LineWidget (const Handle AttrHandle, const String &Name, const int MaxLength);

      virtual pupdate *create_widgets (
         const Handle Object,
         ObjectModule &module,
         QWidget *parent,
         QFormLayout *layout,
         StringSub &sub);

   protected:
      virtual ~LineWidget () {;}

      const int _MaxLength;

   private:
      LineWidget ();
      LineWidget (const LineWidget &);
      LineWidget &operator= (const LineWidget &);
};

class TextUpdater : public pupdate {

   public:
      TextUpdater (const Handle AttrHandle, QTextEdit *edit);
      virtual void update_object (const Handle Object, ObjectModule &module);
      virtual QWidget *get_widget ();

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
         QFormLayout *layout,
         StringSub &sub);

   protected:
      virtual ~TextWidget () {;}

   private:
      TextWidget ();
      TextWidget (const TextWidget &);
      TextWidget &operator= (const TextWidget &);
};

class ScalarUpdater : public pupdate {

   public:
      ScalarUpdater (const Handle AttrHandle, QDoubleSpinBox *edit, const double Scale);
      virtual void update_object (const Handle Object, ObjectModule &module);
      virtual QWidget *get_widget ();

   protected:
      virtual ~ScalarUpdater () {;}
      const double _Scale;
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
         const Boolean Editable,
         const double DefaultValue,
         const double Scale,
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
         QFormLayout *layout,
         StringSub &sub);

   protected:
      virtual ~ScalarWidget () {;}
      const Boolean _Editable;
      const double _DefaultValue;
      const double _Scale;
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

class LinkLabel : public pedit {

   public:
      LinkLabel (
         const Handle AttrHandle,
         const String &Name,
         const Handle TextHandle,
         const Boolean Super);

      virtual pupdate *create_widgets (
         const Handle Object,
         ObjectModule &module,
         QWidget *parent,
         QFormLayout *layout,
         StringSub &sub);

   protected:
      virtual ~LinkLabel () {;}
      const Handle _TextHandle;
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
      virtual QWidget *get_widget ();

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
         QFormLayout *layout,
         StringSub &sub);

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


QWidget *
LineUpdater::get_widget () { return _edit; }


LineWidget::LineWidget (
      const Handle AttrHandle,
      const String &Name,
      const int MaxLength) :
      pedit (AttrHandle, Name),
      _MaxLength (MaxLength) {;}


pupdate *
LineWidget::create_widgets (
      const Handle Object,
      ObjectModule &module,
      QWidget *parent,
      QFormLayout *layout,
      StringSub &sub) {

   const Handle RealObject = get_real_object (Object, module);

   QLabel *label = new QLabel (sub.convert(Name).get_buffer (), parent);

   String text;

   if (!module.lookup_text (RealObject, AttrHandle, text)) { text = ""; }

   QLineEdit *edit= new QLineEdit (text.get_buffer (), parent);

   if (_MaxLength > 0) { edit->setMaxLength (_MaxLength); }

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


QWidget *
TextUpdater::get_widget () { return _edit; }


pupdate *
TextWidget::create_widgets (
      const Handle Object,
      ObjectModule &module,
      QWidget *parent,
      QFormLayout *layout,
      StringSub &sub) {

   const Handle RealObject = get_real_object (Object, module);

   QLabel *label = new QLabel (sub.convert(Name).get_buffer (), parent);

   String text;

   if (!module.lookup_text (RealObject, AttrHandle, text)) { text = ""; }

   QTextEdit *edit = new QTextEdit (text.get_buffer (), parent);
   edit->setTabChangesFocus (true);

   layout->addRow (label, edit);

   return new TextUpdater (AttrHandle, edit);
}


ScalarUpdater::ScalarUpdater (
      const Handle AttrHandle,
      QDoubleSpinBox *edit,
      const double Scale) :
      pupdate (AttrHandle),
      _Scale (Scale > 0.0 ? Scale : 1.0),
      _edit (edit) {;}


void
ScalarUpdater::update_object (const Handle Object, ObjectModule &module) {

   const Handle RealObject = get_real_object (Object, module);
   if (_edit) { module.store_scalar (RealObject, AttrHandle, _edit->value () / _Scale); }
}


QWidget *
ScalarUpdater::get_widget () { return _edit; }


ScalarWidget::ScalarWidget (
      const Handle AttrHandle,
      const String &Name,
      const Boolean Editable,
      const double DefaultValue,
      const double Scale,
      const int Decimals,
      const double Max,
      const double Min,
      const String Prefix,
      const double Step,
      const String Suffix) :
      pedit (AttrHandle, Name),
      _Editable (Editable),
      _DefaultValue (DefaultValue),
      _Scale (Scale > 0.0 ? Scale : 1.0),
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
      QFormLayout *layout,
      StringSub &sub) {

   const Handle RealObject = get_real_object (Object, module);

   QLabel *label = new QLabel (sub.convert(Name).get_buffer (), parent);

   Float64 value (_DefaultValue);

   module.lookup_scalar (RealObject, AttrHandle, value);

   QDoubleSpinBox *edit (0);

   if (_Editable) {

      edit = new QDoubleSpinBox (parent);
      edit->setDecimals (_Decimals);
      edit->setRange (_Min, _Max),
      edit->setPrefix (_Prefix.get_buffer ());
      edit->setSingleStep (_Step);
      edit->setSuffix (_Suffix.get_buffer ());
      edit->setValue (value * _Scale);

      layout->addRow (label, edit);
   }
   else {

      QLabel *vlabel = new QLabel (
         QString (_Prefix.get_buffer ()) +
            QString::number (value * _Scale, 'f', _Decimals) +
            QString (_Suffix.get_buffer ()),
         parent);

      layout->addRow (label, vlabel);
   }

   return _Editable ? new ScalarUpdater (AttrHandle, edit, _Scale) : 0;
}


LinkLabel::LinkLabel (
      const Handle AttrHandle,
      const String &Name,
      const Handle TextHandle,
      const Boolean Super) :
      pedit (AttrHandle, Name),
      _TextHandle (TextHandle),
      _Super (Super) {;}


pupdate *
LinkLabel::create_widgets (
      const Handle Object,
      ObjectModule &module,
      QWidget *parent,
      QFormLayout *layout,
      StringSub &sub) {

   QLabel *label = new QLabel (sub.convert(Name).get_buffer (), parent);

   if (!_TextHandle) {

      Handle linkAttr (0);
      Handle sub (0);
      Handle super (0);

      module.lookup_linked_objects (Object, linkAttr, super, sub);

      String value;

      if (_Super) { module.lookup_text (super, AttrHandle, value); }
      else { module.lookup_text (sub, AttrHandle, value); }

      QLabel *data = new QLabel (value.get_buffer (), parent);

      layout->addRow (label, data);
   }
   else {

      const Handle RealObject = get_real_object (Object, module);

      HandleContainer container;

      if (_Super) { module.lookup_super_links (RealObject, AttrHandle, container); }
      else { module.lookup_sub_links (RealObject, AttrHandle, container); }

      String value;

      module.lookup_text (container.get_first (), _TextHandle, value);

      QLabel *data = new QLabel (value.get_buffer (), parent);

      layout->addRow (label, data);
   }

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


QWidget *
StateUpdater::get_widget () { return _edit; }


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
      QFormLayout *layout,
      StringSub &sub) {

   const Handle RealObject = get_real_object (Object, module);

   QLabel *label = new QLabel (sub.convert(Name).get_buffer (), parent);

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


// Start MBRAPluginPropertyEditor class
dmz::MBRAPluginPropertyEditor::MBRAPluginPropertyEditor (
      const PluginInfo &Info,
      Config &local) :
      Plugin (Info),
      MessageObserver (Info),
      _log (Info),
      _undo (Info),
      _defs (Info),
      _convert (Info),
      _objMod (0),
      _window (0),
      _showFTButton (False),
      _objectDataHandle (0),
      _createdDataHandle (0),
      _ftHandle (0),
      _widgets (0) {

   _init (local);
}


dmz::MBRAPluginPropertyEditor::~MBRAPluginPropertyEditor () {

   if (_widgets) { delete _widgets; _widgets = 0; }

   _tabNameTable.empty ();
}


// Plugin Interface
void
dmz::MBRAPluginPropertyEditor::update_plugin_state (
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
dmz::MBRAPluginPropertyEditor::discover_plugin (
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
dmz::MBRAPluginPropertyEditor::receive_message (
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

            _edit (object, createdHandle != 0);
         }
      }
   }
   else if (Type == _unitsMessage) {

      if (InData) {

         const String Var = _convert.to_string (InData);
         _sub.store (Type.get_name (), Var);
      }
   }
}


// dmzMBRAPluginPropertyEditor Interface
void
dmz::MBRAPluginPropertyEditor::_edit (const Handle Object, const Boolean Created) {

   if (_objMod) {

      QDialog dialog (_window ? _window->get_qt_main_window () : 0);

      Ui::PropertyEditor ui;
      ui.setupUi (&dialog);
      ui.ftCheck->hide ();
      if (!_showFTButton) { ui.ftButton->hide (); }
      dialog.setWindowTitle (_dialogTitle.get_buffer ());

      PropertyWidget *pe (_widgets);
      PropertyUpdater *head (0), *current (0);

      QWidget *prevWidget (0);

      HashTableUInt32Template<TabStruct> tabs;

      const Boolean Tabbed  (_tabNameTable.get_count () > 0);

      if (Tabbed) { ui.stack->setCurrentIndex (0); }
      else { ui.stack->setCurrentIndex (1); }

      while (pe) {

         TabStruct *ts = tabs.lookup (pe->tab);

         if (!ts) {

            ts = new TabStruct;

            if (tabs.store (pe->tab, ts)) {

               String title ("");

               String *ptr = _tabNameTable.lookup (pe->tab);

               if (ptr) { title = *ptr; }

               if (Tabbed) {

                  ts->widget = ui.attributesTab->widget (pe->tab);
               }
               else { ts->widget = ui.attributes; }

               if (!ts->widget) {

                  ts->widget = new QWidget (ui.attributes);

                  ui.attributesTab->insertTab (pe->tab, ts->widget, title.get_buffer ());
               }
               else if (Tabbed) {

                  ui.attributesTab->setTabText (pe->tab, title.get_buffer ());
               }

               ts->layout = new QFormLayout (ts->widget);
            }
         }

         if (ts) {

            PropertyUpdater *next = pe->create_widgets (
               Object,
               *_objMod,
               ts->widget,
               ts->layout,
               _sub);

            if (next) {

               if (!current) { head = current = next; }
               else { current->next = next; current = next; }

               QWidget *widget = next->get_widget ();

               if (widget) {

                  if (prevWidget) { QWidget::setTabOrder (prevWidget, widget); }
                  else { widget->setFocus (); }

                  prevWidget = widget;
               }
            }
         }

         pe = pe->next;
      }

      dialog.move (_window->get_qt_main_window ()->pos ());

      tabs.empty ();

      if (prevWidget && _showFTButton) { QWidget::setTabOrder (prevWidget, ui.ftButton); }

      if (dialog.exec () == QDialog::Accepted) {

         const Handle UndoHandle = _undo.start_record ("Edit Node");

         current = head;

         while (current) {

            current->update_object (Object, *_objMod);
            current = current->next;
         }

         if (ui.ftCheck->isChecked ()) {

            const Handle RealObject = get_real_object (Object, *_objMod);

            Data out;
            out.store_handle (_objectDataHandle, 0, RealObject);
            _ftMessage.send (_ftHandle, &out, 0);
         }

         _undo.stop_record (UndoHandle);
      }
      else if (Created) {

         if (_objMod->is_object (Object)) { _objMod->destroy_object (Object); }
         else if (_objMod->is_link (Object)) { _objMod->unlink_objects (Object); }
      }

      if (head) { delete head; head = 0; }

   }
}


dmz::MBRAPluginPropertyEditor::PropertyWidget *
dmz::MBRAPluginPropertyEditor::_create_widgets (Config &list) {

   ConfigIterator it;
   Config widget;

   PropertyWidget *result (0);

   RuntimeContext *context (get_plugin_runtime_context ());

   while (list.get_prev_config (it, widget)) {

      PropertyWidget *pe (0);

      const String Type = config_to_string ("type", widget);
      const String Name = config_to_string ("name", widget) + ":";
      const Handle AttrHandle = _defs.create_named_handle (
         config_to_string ("attribute", widget, ObjectAttributeDefaultName));

      const UInt32 Tab = config_to_uint32 ("tab", widget, 0);

      if (Type == "line") {

         const int MaxLength = config_to_int32 ("max-length", widget);
         pe = new LineWidget (AttrHandle, Name, MaxLength);
      }
      else if (Type == "text") { pe = new TextWidget (AttrHandle, Name); }
      else if ((Type == "scalar") || (Type == "scalar-label")) {

         const double DefaultValue = config_to_float64 ("default", widget, 0.0);
         const double Scale = config_to_float64 ("scale", widget, 1.0);
         const int Decimals = (int)config_to_int32 ("decimals", widget, 2);
         const double Max = config_to_float64 ("max", widget, 1e+10);
         const double Min = config_to_float64 ("min", widget, 0);
         const String Prefix = config_to_string ("prefix", widget);
         const double Step = config_to_float64 ("step", widget, 1000);
         const String Suffix = config_to_string ("suffix", widget);

         // move the prefix out of the ScalarWidget and append it
         // to the name label to get around a weird qt bug -ss
         String nameWithPrefix (Name);
         if (Prefix) { 
         
            nameWithPrefix.replace_sub (":", " ");
            nameWithPrefix << "(" << Prefix << "):";
         }
         
         pe = new ScalarWidget (
            AttrHandle,
            nameWithPrefix, //Name, see comment above
            Type == "scalar" ? True : False,
            DefaultValue,
            Scale,
            Decimals,
            Max,
            Min,
            "", //Prefix, see comment above
            Step,
            Suffix);
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
                  config_to_state ("name", state, context));

               if (config_to_boolean ("default", state, False)) {

                  se->set_default_state_name (Name);
               }
            }

            pe = se;
         }
      }
      else if (Type == "link-label") {

         pe = new LinkLabel (
            AttrHandle,
            Name,
            config_to_named_handle ("text-attribute", widget, context),
            config_to_boolean ("super", widget));
      }

      if (pe) { pe->tab = Tab; pe->next = result; result = pe; }
   }

   return result;
}


void
dmz::MBRAPluginPropertyEditor::_init (Config &local) {

   RuntimeContext *context = get_plugin_runtime_context ();

   _showFTButton = config_to_boolean ("fault-tree-button.value", local, _showFTButton);

   _editMessage = config_create_message (
      "edit-message.name",
      local,
      "EditObjectAttributesMessage",
      context);

   subscribe_to_message (_editMessage);

   _ftMessage = config_create_message (
      "ft-message.name",
      local,
      "CreateLinkedFaultTreeMessage",
      context);

   _unitsMessage = config_create_message (
         "units-message.name",
         local,
         "UnitsMessage",
         context);

   subscribe_to_message(_unitsMessage);

   _objectDataHandle =
      config_to_named_handle ("attribute.object.name", local, "object", context);

   _createdDataHandle =
      config_to_named_handle ("attribute.created.name", local, "created", context);

   _ftHandle = config_to_named_handle (
      "attribute.fault-tree.name",
      local,
      "dmzMBRAPluginCreateLinkedFaultTree",
      context);

   _dialogTitle = config_to_string ("title.value", local, "Node Properties");

   Config list;

   if (local.lookup_all_config ("property-list.property", list)) {

      _widgets = _create_widgets (list);
   }

   Config tabList;

   if (local.lookup_all_config ("tab-list.tab", tabList)) {

      ConfigIterator it;
      Config tab;
      UInt32 count = 0;

      while (tabList.get_next_config (it, tab)) {

         const String Name = config_to_string ("name", tab);

         if (Name) {

            String *ptr = new String (Name);

            if (ptr && _tabNameTable.store (count, ptr)) { count++; }
            else if (ptr) { delete ptr; ptr = 0; }
         }
      }
   }
}


extern "C" {

DMZ_PLUGIN_FACTORY_LINK_SYMBOL dmz::Plugin *
create_dmzMBRAPluginPropertyEditor (
      const dmz::PluginInfo &Info,
      dmz::Config &local,
      dmz::Config &global) {

   return new dmz::MBRAPluginPropertyEditor (Info, local);
}

};
