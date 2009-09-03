#ifndef DMZ_MBRA_PLUGIN_LINK_EDITOR_DOT_H
#define DMZ_MBRA_PLUGIN_LINK_EDITOR_DOT_H

#include <dmzObjectObserverUtil.h>
#include <dmzQtWidget.h>
#include <dmzRuntimeLog.h>
#include <dmzRuntimeObjectType.h>
#include <dmzRuntimePlugin.h>
#include <dmzRuntimeUndo.h>
#include <dmzTypesHashTableHandleTemplate.h>

#include <QtGui/QWidget>

#include <ui_LinkEditor.h>

namespace dmz {

   class MBRAPluginLinkEditor :
         public QWidget,
         public Plugin,
         public ObjectObserverUtil,
         public QtWidget {

      Q_OBJECT

      public:
         MBRAPluginLinkEditor (const PluginInfo &Info, Config &local);
         ~MBRAPluginLinkEditor ();

         // Plugin Interface
         virtual void update_plugin_state (
            const PluginStateEnum State,
            const UInt32 Level);

         virtual void discover_plugin (
            const PluginDiscoverEnum Mode,
            const Plugin *PluginPtr);

         // Object Observer Interface
         virtual void create_object (
            const UUID &Identity,
            const Handle ObjectHandle,
            const ObjectType &Type,
            const ObjectLocalityEnum Locality);

         virtual void destroy_object (const UUID &Identity, const Handle ObjectHandle);

         virtual void link_objects (
            const Handle LinkHandle,
            const Handle AttributeHandle,
            const UUID &SuperIdentity,
            const Handle SuperHandle,
            const UUID &SubIdentity,
            const Handle SubHandle);

         virtual void unlink_objects (
            const Handle LinkHandle,
            const Handle AttributeHandle,
            const UUID &SuperIdentity,
            const Handle SuperHandle,
            const UUID &SubIdentity,
            const Handle SubHandle);

         virtual void update_link_attribute_object (
            const Handle LinkHandle,
            const Handle AttributeHandle,
            const UUID &SuperIdentity,
            const Handle SuperHandle,
            const UUID &SubIdentity,
            const Handle SubHandle,
            const UUID &AttributeIdentity,
            const Handle AttributeObjectHandle,
            const UUID &PrevAttributeIdentity,
            const Handle PrevAttributeObjectHandle);

         virtual void update_object_flag (
            const UUID &Identity,
            const Handle ObjectHandle,
            const Handle AttributeHandle,
            const Boolean Value,
            const Boolean *PreviousValue);

         virtual void update_object_text (
            const UUID &Identity,
            const Handle ObjectHandle,
            const Handle AttributeHandle,
            const String &Value,
            const String *PreviousValue);

         // QtWidget Interface
         virtual QWidget *get_qt_widget ();

      protected slots:
         void on_linkButton_clicked ();
         void on_unlinkButton_clicked ();

      protected:
         struct LinkStruct {

            QListWidgetItem *item;
            String naName;
            String ftName;

            LinkStruct () : item (0) {;}
            ~LinkStruct () { if (item) { delete item; item = 0; } ;}
         };

         void _init (Config &local);

         Log _log;
         Undo _undo;

         Ui::LinkEditor _ui;

         Handle _linkAttrHandle;
         Handle _naNameAttrHandle;
         Handle _ftNameAttrHandle;

         ObjectType _naNodeType;
         ObjectType _ftRootType;

         HashTableHandleTemplate<QListWidgetItem> _objTable;
         HashTableHandleTemplate<LinkStruct> _linkTable;
         HashTableHandleTemplate<LinkStruct> _superTable;
         HashTableHandleTemplate<LinkStruct> _subTable;

      private:
         MBRAPluginLinkEditor ();
         MBRAPluginLinkEditor (const MBRAPluginLinkEditor &);
         MBRAPluginLinkEditor &operator= (const MBRAPluginLinkEditor &);

   };
};

#endif // DMZ_MBRA_PLUGIN_LINK_EDITOR_DOT_H
