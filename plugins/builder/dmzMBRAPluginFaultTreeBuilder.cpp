#include "dmzMBRAPluginFaultTreeBuilder.h"
#include <dmzObjectAttributeMasks.h>
#include <dmzObjectModule.h>
#include <dmzQtModuleCanvas.h>
#include <dmzQtModuleMainWindow.h>
#include <dmzRenderModulePick.h>
#include <dmzRuntimeConfigToTypesBase.h>
#include <dmzRuntimeConfigToNamedHandle.h>
#include <dmzRuntimeData.h>
#include <dmzRuntimePluginFactoryLinkSymbol.h>
#include <dmzRuntimePluginInfo.h>
#include <QtGui/QtGui>
#include "ui_dmzMBRAThreatProperties.h"
#include "ui_dmzMBRAComponentProperties.h"


dmz::MBRAPluginFaultTreeBuilder::MBRAPluginFaultTreeBuilder (
      const PluginInfo &Info,
      Config &local) :
      Plugin (Info),
      MessageObserver (Info),
      ObjectObserverUtil (Info, local),
      _log (Info),
      _appState (Info),
      _defs (Info),
      _undo (Info),
      _canvasModule (0),
      _canvasModuleName (),
      _mainWindowModule (0),
      _mainWindowModuleName (),
      _defaultAttrHandle (0),
      _objectDataHandle (0),
      _createdDataHandle (0),
      _linkAttrHandle (0),
      _naLinkAttrHandle (0),
      _logicAttrHandle (0),
      _hideAttrHandle (0),
      _naNameAttrHandle (0),
      _ftNameAttrHandle (0),
      _activeFTAttrHandle (0),
      _clipBoardHandle (0),
      _clipBoardAttrHandle (0),
      _componentEditTarget (0),
      _threatEditTarget (0),
      _cloneDepth (0),
      _componentAddMessage (),
      _componentEditMessage (),
      _componentDeleteMessage (),
      _threatAddMessage (),
      _threatEditMessage (),
      _threatDeleteMessage (),
      _logicAndMessage (),
      _logicOrMessage (),
      _logicXOrMessage (),
      _rootType (),
      _componentType (),
      _threatType (),
      _logicType (),
      _logicAndMask (),
      _logicOrMask (),
      _logicXOrMask (),
      _linkTable () {

   _init (local);
}


dmz::MBRAPluginFaultTreeBuilder::~MBRAPluginFaultTreeBuilder () {

   _linkTable.empty ();
}


// Plugin Interface
void
dmz::MBRAPluginFaultTreeBuilder::discover_plugin (
      const PluginDiscoverEnum Mode,
      const Plugin *PluginPtr) {

   if (Mode == PluginDiscoverAdd) {

      if (!_canvasModule) {

         _canvasModule = QtModuleCanvas::cast (PluginPtr, _canvasModuleName);
      }

      if (!_mainWindowModule) {

         _mainWindowModule = QtModuleMainWindow::cast (PluginPtr, _mainWindowModuleName);
      }
   }
   else if (Mode == PluginDiscoverRemove) {

      if (_canvasModule && (_canvasModule == QtModuleCanvas::cast (PluginPtr))) {

         _canvasModule = 0;
      }

      if (_mainWindowModule &&
            (_mainWindowModule == QtModuleMainWindow::cast (PluginPtr))) {

         _mainWindowModule = 0;
      }
   }
}


// Message Observer Interface
void
dmz::MBRAPluginFaultTreeBuilder::receive_message (
      const Message &Msg,
      const UInt32 MessageSendHandle,
      const Handle TargetObserverHandle,
      const Data *InData,
      Data *outData) {

   if (InData) {

      Handle obj;

      if (InData->lookup_handle (_objectDataHandle, 0, obj)) {

         if (Msg == _componentAddMessage) { _component_add (obj); }
         else if (Msg == _componentEditMessage) { _component_edit (obj, False); }
         else if (Msg == _componentDeleteMessage) { _component_delete (obj); }
         else if (Msg == _threatAddMessage) { _threat_add (obj); }
         else if (Msg == _threatEditMessage) { _threat_edit (obj, False); }
         else if (Msg == _threatDeleteMessage) { _threat_delete (obj); }
         else if (Msg == _logicAndMessage) { _logic_and (obj); }
         else if (Msg == _logicOrMessage) { _logic_or (obj); }
         else if (Msg == _logicXOrMessage) { _logic_xor (obj); }
         else if (Msg == _cutMessage) { _cut (obj); }
         else if (Msg == _copyMessage) { _copy (obj); }
         else if (Msg == _pasteMessage) { _paste (obj); }
         else if (Msg == _createFromFlaggedMessage) { _create_from_flagged (); }
      }
   }
   else if (Msg == _createFromFlaggedMessage) { _create_from_flagged (); }
}


// Object Observer Interface
void
dmz::MBRAPluginFaultTreeBuilder::create_object (
      const UUID &Identity,
      const Handle ObjectHandle,
      const ObjectType &Type,
      const ObjectLocalityEnum Locality) {

   if (Type.is_of_type (_clipBoardType)) {

      if (ObjectHandle != _clipBoardHandle) {

         _empty_clip_board (ObjectHandle);
      }
   }
}


void
dmz::MBRAPluginFaultTreeBuilder::destroy_object (
      const UUID &Identity,
      const Handle ObjectHandle) {

   if (ObjectHandle == _clipBoardHandle) {

      _clipBoardHandle = 0;
   }
}


void
dmz::MBRAPluginFaultTreeBuilder::remove_object_attribute (
      const UUID &Identity,
      const Handle ObjectHandle,
      const Handle AttributeHandle,
      const Mask &AttrMask) {

   if (AttrMask.contains (ObjectStateMask)) {

      _flaggedNodes.remove (ObjectHandle);
   }
}


void
dmz::MBRAPluginFaultTreeBuilder::link_objects (
      const Handle LinkHandle,
      const Handle AttributeHandle,
      const UUID &SuperIdentity,
      const Handle SuperHandle,
      const UUID &SubIdentity,
      const Handle SubHandle) {

   if (AttributeHandle == _linkAttrHandle) {

      Int32 *count (_linkTable.lookup (SuperHandle));

      if (!count) {

         count = new Int32 (0);

         if (!_linkTable.store (SuperHandle, count)) { delete count; count = 0; }
      }

      if (count) {

         (*count)++;

         if (*count == 2) {

            // Make sure we aren't loading or undoing
            if (_appState.is_mode_normal () && (_cloneDepth == 0)) {

               _create_logic (SuperHandle);
            }
         }
      }
   }
}


void
dmz::MBRAPluginFaultTreeBuilder::unlink_objects (
      const Handle LinkHandle,
      const Handle AttributeHandle,
      const UUID &SuperIdentity,
      const Handle SuperHandle,
      const UUID &SubIdentity,
      const Handle SubHandle) {

   if (AttributeHandle == _linkAttrHandle) {

      Int32 *count (_linkTable.lookup (SuperHandle));

      if (count) {

         (*count)--;

         if (*count == 1) {

            _delete_logic (SuperHandle);
         }
         else if ((*count) == 0) {

            _linkTable.remove (SuperHandle);

            delete count; count = 0;
         }
      }
   }
   else if (AttributeHandle == _clipBoardAttrHandle) {

      if (!_undo.is_in_undo ()) { _component_delete (SubHandle); }
   }
}


void
dmz::MBRAPluginFaultTreeBuilder::update_object_state (
      const UUID &Identity,
      const Handle ObjectHandle,
      const Handle AttributeHandle,
      const Mask &Value,
      const Mask *PreviousValue) {

   const Boolean IsFlagged = Value.contains (_flaggedMask);

   const Boolean WasFlagged =
      PreviousValue ? PreviousValue->contains (_flaggedMask) : False;

   if (IsFlagged && !WasFlagged) { _flaggedNodes.add (ObjectHandle); }
   else if (!IsFlagged && WasFlagged) { _flaggedNodes.remove (ObjectHandle); }
}


void
dmz::MBRAPluginFaultTreeBuilder::_store_object_module (ObjectModule &objMod) {

   if (!_clipBoardHandle) { _empty_clip_board (); }
}


void
dmz::MBRAPluginFaultTreeBuilder::_remove_object_module (ObjectModule &objMod) {

}


void
dmz::MBRAPluginFaultTreeBuilder::_component_add (const Handle Parent) {

   ObjectModule *objMod (get_object_module ());

   if (objMod && Parent) {

      Boolean edited (False);

      const Handle UndoHandle (_undo.start_record ("Add Component"));

      Handle object (objMod->create_object (_componentType, ObjectLocal));

      if (object) {

         _component_edit (object, True);

         if (objMod->is_object (object)) {

            edited = true;
            objMod->activate_object (object);
            objMod->link_objects (_linkAttrHandle, Parent, object);
         }
      }

      if (edited) { _undo.stop_record (UndoHandle); }
      else { _undo.abort_record (UndoHandle); }
   }
}


void
dmz::MBRAPluginFaultTreeBuilder::_component_edit (
      const Handle Object,
      const Boolean Created) {

   if (_componentEditTarget) {

      Data out;
      out.store_handle (_objectDataHandle, 0, Object);
      if (Created) { out.store_handle (_createdDataHandle, 0, Object); }
      _componentEditMessage.send (_componentEditTarget, &out, 0);
   }
}


void
dmz::MBRAPluginFaultTreeBuilder::_component_delete (const Handle Object) {

   ObjectModule *objMod (get_object_module ());

   if (objMod && Object) {

      const Boolean IsRoot (
         objMod->lookup_object_type (Object).is_of_exact_type (_rootType));

      String undoName ("Delete Component");

      if (IsRoot) { undoName = "Delete Fault Tree"; }

      const Handle UndoHandle (_undo.start_record (undoName));

      // delete all objects linked to this Object
      HandleContainer children;

      objMod->lookup_sub_links (Object, _linkAttrHandle, children);

      Handle current (children.get_first ());

      while (current) {

         _component_delete (current);

         current = children.get_next ();
      }

      // then delete this Object
      objMod->destroy_object (Object);

      _undo.stop_record (UndoHandle);
   }
}


void
dmz::MBRAPluginFaultTreeBuilder::_threat_add (const Handle Parent) {

   ObjectModule *objMod (get_object_module ());

   if (objMod && Parent) {

      Boolean edited (False);

      const Handle UndoHandle (_undo.start_record ("Add Threat"));

      Handle object (objMod->create_object (_threatType, ObjectLocal));

      if (object) {

         _threat_edit (object, True);

         if (objMod->is_object (object)) {

            edited = true;
            objMod->activate_object (object);
            objMod->link_objects (_linkAttrHandle, Parent, object);
         }
      }

      if (edited) { _undo.stop_record (UndoHandle); }
      else { _undo.abort_record (UndoHandle); }
   }
}


void
dmz::MBRAPluginFaultTreeBuilder::_threat_edit (
      const Handle Object,
      const Boolean Created) {

   if (_threatEditTarget) {

      Data out;
      out.store_handle (_objectDataHandle, 0, Object);
      if (Created) { out.store_handle (_createdDataHandle, 0, Object); }
      _threatEditMessage.send (_threatEditTarget, &out, 0);
   }
}


void
dmz::MBRAPluginFaultTreeBuilder::_threat_delete (const Handle Object) {

   ObjectModule *objMod (get_object_module ());

   if (objMod && Object) {

      const Handle UndoHandle (_undo.start_record ("Delete Threat"));

      objMod->destroy_object (Object);

      _undo.stop_record (UndoHandle);
   }
}


void
dmz::MBRAPluginFaultTreeBuilder::_logic_and (const Handle Object) {

   ObjectModule *objMod (get_object_module ());

   if (objMod && Object) {

      Mask state;
      if (objMod->lookup_state (Object, _defaultAttrHandle, state)) {

         if (!state.contains (_logicAndMask)) {

            const Handle UndoHandle (_undo.start_record ("Create AND gate"));

            state |= _logicAndMask;
            state &= ~_logicOrMask;
            state &= ~_logicXOrMask;

            objMod->store_state (Object, _defaultAttrHandle, state);

            _undo.stop_record (UndoHandle);
         }
      }
   }
}


void
dmz::MBRAPluginFaultTreeBuilder::_logic_or (const Handle Object) {

   ObjectModule *objMod (get_object_module ());

   if (objMod && Object) {

      Mask state;
      if (objMod->lookup_state (Object, _defaultAttrHandle, state)) {

         if (!state.contains (_logicOrMask)) {

            const Handle UndoHandle (_undo.start_record ("Create OR gate"));

            state &= ~_logicAndMask;
            state |= _logicOrMask;
            state &= ~_logicXOrMask;

            objMod->store_state (Object, _defaultAttrHandle, state);

            _undo.stop_record (UndoHandle);
         }
      }
   }
}


void
dmz::MBRAPluginFaultTreeBuilder::_logic_xor (const Handle Object) {

   ObjectModule *objMod (get_object_module ());

   if (objMod && Object) {

      Mask state;
      if (objMod->lookup_state (Object, _defaultAttrHandle, state)) {

         if (!state.contains (_logicXOrMask)) {

            const Handle UndoHandle (_undo.start_record ("Create XOR gate"));

            state &= ~_logicAndMask;
            state &= ~_logicOrMask;
            state |= _logicXOrMask;

            objMod->store_state (Object, _defaultAttrHandle, state);

            _undo.stop_record (UndoHandle);
         }
      }
   }
}


void
dmz::MBRAPluginFaultTreeBuilder::_create_logic (const Handle Parent) {

   ObjectModule *objMod (get_object_module ());

   if (objMod && Parent) {

      Handle object (objMod->create_object (_logicType, ObjectLocal));

      if (object) {

         objMod->store_state (object, _defaultAttrHandle, _logicOrMask);
         objMod->activate_object (object);

         objMod->link_objects (_logicAttrHandle, Parent, object);
      }
   }
}


void
dmz::MBRAPluginFaultTreeBuilder::_delete_logic (const Handle Parent) {

   ObjectModule *objMod (get_object_module ());

   if (objMod && Parent) {

      HandleContainer logic;
      objMod->lookup_sub_links (Parent, _logicAttrHandle, logic);

      if (logic.get_count ()) {

         objMod->destroy_object (logic.get_first ());
      }
   }
}


dmz::Handle
dmz::MBRAPluginFaultTreeBuilder::_clone_component (
      const Handle Object,
      ObjectModule &objMod) {

   Handle result (objMod.clone_object (Object, ObjectIgnoreLinks));
   objMod.activate_object (result);
   _cloneDepth++;

   HandleContainer list;

   if (objMod.lookup_sub_links (Object, _linkAttrHandle, list)) {

      Handle child = list.get_first ();

      while (child) {

         Handle clone = _clone_component (child, objMod);

         if (clone) { objMod.link_objects (_linkAttrHandle, result, clone); }

         child = list.get_next ();
      }
   }

   if (objMod.lookup_sub_links (Object, _logicAttrHandle, list)) {

      Handle child = list.get_first ();

      if (child) {

         Handle clone = objMod.clone_object (child, ObjectIgnoreLinks);

         if (clone) {

            objMod.activate_object (clone);
            objMod.link_objects (_logicAttrHandle, result, clone);
         }
      }
   }
   
   _cloneDepth--;
   return result;
}


void
dmz::MBRAPluginFaultTreeBuilder::_empty_clip_board (const Handle NewClipBoard) {

   ObjectModule *objMod (get_object_module ());

   if (objMod) {

      if (_clipBoardHandle) { objMod->destroy_object (_clipBoardHandle); }

      if (NewClipBoard) { _clipBoardHandle = NewClipBoard; }
      else {

         _clipBoardHandle = objMod->create_object (_clipBoardType, ObjectLocal);
         objMod->activate_object (_clipBoardHandle);
      }
   }
}


void
dmz::MBRAPluginFaultTreeBuilder::_set_component_hide_state (
      const Handle Obj,
      const Boolean Value,
      ObjectModule &objMod) {

   objMod.store_flag (Obj, _hideAttrHandle, Value);

   HandleContainer list;

   if (objMod.lookup_sub_links (Obj, _linkAttrHandle, list)) {

      Handle child = list.get_first ();

      while (child) {

         _set_component_hide_state (child, Value, objMod);
         child = list.get_next ();
      }
   }

   if (objMod.lookup_sub_links (Obj, _logicAttrHandle, list)) {

      Handle child = list.get_first ();

      while (child) {

         objMod.store_flag (child, _hideAttrHandle, Value);
         child = list.get_next ();
      }
   }
}


void
dmz::MBRAPluginFaultTreeBuilder::_cut (const Handle Parent) {

   ObjectModule *objMod (get_object_module ());

   //_log.error << "Cut: " << Parent << endl;

   if (objMod && Parent) {

      const Handle UndoHandle (_undo.start_record ("Cut"));

      _empty_clip_board ();

      const ObjectType Type = objMod->lookup_object_type (Parent);

      if (Type.is_of_exact_type (_componentType) || Type.is_of_exact_type (_threatType)) {

         objMod->unlink_super_links (Parent, _linkAttrHandle);

         if (objMod->link_objects (_clipBoardAttrHandle, _clipBoardHandle, Parent)) {

            _set_component_hide_state (Parent, True, *objMod);
         }
         else { _component_delete (Parent); }
      }
      else {

         Handle start = 0;

         if (Type.is_of_exact_type (_rootType)) { start = Parent; }
         else if (Type.is_of_exact_type (_logicType)) {

            HandleContainer list;
            objMod->lookup_super_links (Parent, _logicAttrHandle, list);
            start = list.get_first ();
         }

         if (start && _clipBoardHandle) {

            HandleContainer list;
            objMod->lookup_sub_links (start, _linkAttrHandle, list);

            Handle obj = list.get_last ();

            while (obj) {

               objMod->unlink_super_links (obj, _linkAttrHandle);
               objMod->link_objects (_clipBoardAttrHandle, _clipBoardHandle, obj);
               _set_component_hide_state (obj, True, *objMod);
               obj = list.get_prev ();
            }
         }
      }

      _undo.stop_record (UndoHandle);
   }
}


void
dmz::MBRAPluginFaultTreeBuilder::_copy (const Handle Parent) {

   ObjectModule *objMod (get_object_module ());

   //_log.error << "Copy: " << Parent << endl;

   if (objMod && Parent) {

      const Handle UndoHandle (_undo.start_record ("Copy"));

      _empty_clip_board ();

      const ObjectType Type = objMod->lookup_object_type (Parent);

      if (Type.is_of_exact_type (_componentType) || Type.is_of_exact_type (_threatType)) {

         Handle clone = _clone_component (Parent, *objMod);
         _set_component_hide_state (clone, True, *objMod);

         objMod->link_objects (_clipBoardAttrHandle, _clipBoardHandle, clone);
      }
      else {

         Handle start = 0;

         if (Type.is_of_exact_type (_rootType)) { start = Parent; }
         else if (Type.is_of_exact_type (_logicType)) {

            HandleContainer list;
            objMod->lookup_super_links (Parent, _logicAttrHandle, list);
            start = list.get_first ();
         }

         if (start && _clipBoardHandle) {

            HandleContainer list;
            objMod->lookup_sub_links (start, _linkAttrHandle, list);

            Handle obj = list.get_first ();

            while (obj) {

               Handle clone = _clone_component (obj, *objMod);
               _set_component_hide_state (clone, True, *objMod);
               objMod->link_objects (_clipBoardAttrHandle, _clipBoardHandle, clone);
               obj = list.get_next ();
            }
         }
      }

      _undo.stop_record (UndoHandle);
   }
}


void
dmz::MBRAPluginFaultTreeBuilder::_paste (const Handle Parent) {

   ObjectModule *objMod (get_object_module ());

   //_log.error << "Paste: " << Parent << endl;

   if (objMod && Parent) {

      Handle target = Parent;

      const ObjectType Type = objMod->lookup_object_type (Parent);

      if (Type.is_of_exact_type (_logicType)) {

         HandleContainer list;

         if (objMod->lookup_super_links (Parent, _logicAttrHandle, list)) {

            target = list.get_first ();
         }
         else { target = 0; }
      }
      else if (Type.is_of_exact_type (_threatType)) {

         HandleContainer list;

         if (objMod->lookup_super_links (Parent, _linkAttrHandle, list)) {

            target = list.get_first ();
         }
         else { target = 0; }
      }

      const Handle UndoHandle (_undo.start_record ("Paste"));

      HandleContainer list;

      if (target &&
            objMod->lookup_sub_links (_clipBoardHandle, _clipBoardAttrHandle, list)) {

         Handle obj = list.get_first ();

         while (obj) {

            Handle clone = _clone_component (obj, *objMod);

            if (clone) {

               _set_component_hide_state (clone, False, *objMod);
               objMod->link_objects (_linkAttrHandle, target, clone);
            }

            obj = list.get_next ();
         }
      }

      _undo.stop_record (UndoHandle);
   }
}


void
dmz::MBRAPluginFaultTreeBuilder::_create_from_flagged () {

   ObjectModule *objMod = get_object_module ();

   if (objMod) {

      const Handle UndoHandle = _undo.start_record ("Create Composite Fault Tree");

      const Handle Root = objMod->create_object (_rootType, ObjectLocal);
      objMod->store_text (Root, _ftNameAttrHandle, "Root Node");
      objMod->activate_object (Root);

      HandleContainerIterator it;
      Handle node (0);

      while (_flaggedNodes.get_next (it, node)) {

         HandleContainer list;

         if (objMod->lookup_sub_links (node, _naLinkAttrHandle, list)) {

            const Handle FaultTree = list.get_first ();
            const Handle Parent = objMod->create_object (_componentType, ObjectLocal);

            String name;
            objMod->lookup_text (FaultTree, _ftNameAttrHandle, name);
            objMod->store_text (Parent, _ftNameAttrHandle, name);
            objMod->activate_object (Parent);
            _copy (FaultTree);
            _paste (Parent);

            objMod->link_objects (_linkAttrHandle, Root, Parent);
            HandleContainer logic;

            if (objMod->lookup_sub_links (FaultTree, _logicAttrHandle, logic)) {

               const Handle LogicObj = logic.get_first ();

               if (objMod->lookup_sub_links (Parent, _logicAttrHandle, logic)) {

                  const Handle NewLogicObj = logic.get_first ();

                  Mask state;

                  if (objMod->lookup_state (LogicObj, _defaultAttrHandle, state)) {

                     objMod->store_state (NewLogicObj, _defaultAttrHandle, state);
                  }
               }
            }
         }
      }

      _set_component_hide_state (Root, True, *objMod);
      objMod->store_flag (Root, _activeFTAttrHandle, True);
      _set_component_hide_state (Root, False, *objMod);

      _undo.stop_record (UndoHandle);
   }
}


void
dmz::MBRAPluginFaultTreeBuilder::_init (Config &local) {

   RuntimeContext *context (get_plugin_runtime_context ());

   _canvasModuleName = config_to_string ("module.canvas.name", local);
   _mainWindowModuleName = config_to_string ("module.mainWindow.name", local);

   _defaultAttrHandle = activate_default_object_attribute (
      ObjectCreateMask | ObjectDestroyMask | ObjectRemoveAttributeMask | ObjectStateMask);

   _objectDataHandle = config_to_named_handle (
      "attribute.object.name", local, "object", context);

   _createdDataHandle = config_to_named_handle (
      "attribute.created.name", local, "created", context);

   _linkAttrHandle = activate_object_attribute (
      config_to_string ("attribute.logic.name", local, "FT_Link"),
      ObjectLinkMask | ObjectUnlinkMask);

   _clipBoardAttrHandle = activate_object_attribute (
      config_to_string ("attribute.clip-board.value", local, "FT_Clip_Board_Link"),
      ObjectUnlinkMask);

   _logicAttrHandle = config_to_named_handle (
      "attribute.logic.name", local, "FT_Logic_Link", context);

   _hideAttrHandle = config_to_named_handle (
      "attribute.hide.value",
      local,
      ObjectAttributeHideName,
      context);

   _componentEditTarget = config_to_named_handle (
      "thread-edit-component.name",
      local,
      "FaultTreeComponentProperties",
      context);

   _threatEditTarget = config_to_named_handle (
      "thread-edit-target.name",
      local,
      "FaultTreeThreatProperties",
      context);

   _componentAddMessage = config_create_message (
      "message.component.add",
      local,
      "FTComponentAddMessage",
      context);

   _componentEditMessage = config_create_message (
      "message.component.edit",
      local,
      "FTComponentEditMessage",
      context);

   _componentDeleteMessage = config_create_message (
      "message.component.delete",
      local,
      "FTComponentDeleteMessage",
      context);

   _threatAddMessage = config_create_message (
      "message.threat.add",
      local,
      "FTThreatAddMessage",
      context);

   _threatEditMessage = config_create_message (
      "message.threat.edit",
      local,
      "FTThreatEditMessage",
      context);

   _threatDeleteMessage = config_create_message (
      "message.threat.delete",
      local,
      "FTThreatDeleteMessage",
      context);

   _logicAndMessage = config_create_message (
      "message.logic.and",
      local,
      "FTLogicAndMessage",
      context);

   _logicOrMessage = config_create_message (
      "message.logic.or",
      local,
      "FTLogicOrMessage",
      context);

   _logicXOrMessage = config_create_message (
      "message.logic.xor",
      local,
      "FTLogicXOrMessage",
      context);

   _cutMessage = config_create_message ("message.cut", local, "FTCutMessage", context);
   _copyMessage = config_create_message ("message.copy", local, "FTCopyMessage", context);

   _pasteMessage =
      config_create_message ("message.paste", local, "FTPasteMessage", context);

   _createFromFlaggedMessage = config_create_message (
      "message.create-from-flagged-nodes",
      local,
      "FTCreateFromFlaggedNodesMessage",
      context);

   subscribe_to_message (_componentAddMessage);
   subscribe_to_message (_componentEditMessage);
   subscribe_to_message (_componentDeleteMessage);

   subscribe_to_message (_threatAddMessage);
   subscribe_to_message (_threatEditMessage);
   subscribe_to_message (_threatDeleteMessage);

   subscribe_to_message (_logicAndMessage);
   subscribe_to_message (_logicOrMessage);
   subscribe_to_message (_logicXOrMessage);

   subscribe_to_message (_cutMessage);
   subscribe_to_message (_copyMessage);
   subscribe_to_message (_pasteMessage);

   subscribe_to_message (_createFromFlaggedMessage);

   _rootType = config_to_object_type ("type.root", local, "ft_component_root", context);

   _componentType =
      config_to_object_type ("type.component", local, "ft_component", context);

   _threatType = config_to_object_type ("type.threat", local, "ft_threat", context);
   _logicType = config_to_object_type ("type.logic", local, "ft_logic", context);

   _clipBoardType =
      config_to_object_type ("type.clip-board", local, "ft_clip_board", context);

   _defs.lookup_state (
      config_to_string ("state.logic.and", local, "FT_Logic_And"),
      _logicAndMask);

   _defs.lookup_state (
      config_to_string ("state.logic.or", local, "FT_Logic_Or"),
      _logicOrMask);

   _defs.lookup_state (
      config_to_string ("state.logic.xor", local, "FT_Logic_XOr"),
      _logicXOrMask);

   _defs.lookup_state (
      config_to_string ("state.flagged", local, "NA_Node_Flagged"),
      _flaggedMask);

   _naLinkAttrHandle = _defs.create_named_handle (
      config_to_string ("na-ft-link.name", local, "NA_Fault_Tree_Link"));

   _naNameAttrHandle = _defs.create_named_handle (
      config_to_string ("na-name.name", local, "NA_Node_Name"));

   _ftNameAttrHandle = _defs.create_named_handle (
      config_to_string ("ft-name.name", local, "FT_Name"));

   _activeFTAttrHandle = _defs.create_named_handle (
      config_to_string ("ft-active.name", local, "FT_Active_Fault_Tree"));
}


extern "C" {

DMZ_PLUGIN_FACTORY_LINK_SYMBOL dmz::Plugin *
create_dmzMBRAPluginFaultTreeBuilder (
      const dmz::PluginInfo &Info,
      dmz::Config &local,
      dmz::Config &global) {

   return new dmz::MBRAPluginFaultTreeBuilder (Info, local);
}

};
