/* This file is part of the KDE project
   Copyright (C) 2006-2012 Jarosław Staniek <staniek@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef KDB_ALTER_H
#define KDB_ALTER_H

#include "KDbUtils.h"
#include "KDbResult.h"
#include "KDbTristate.h"
#include "KDbTableSchema.h"

#include <QList>
#include <QHash>

class KDbConnection;

//! @short A tool for handling altering database table schema.
/*! In relational (and other) databases, table schema altering is not an easy task.
 It may be considered as easy if there is no data that user wants to keep while
 the table schema is altered. Otherwise, if the table is alredy filled with data,
 there could be no easy algorithm like:
 1. Drop existing table
 2. Create new one with altered schema.

 Instead, more complex algorithm is needed. To perform the table schema alteration,
 a list of well defined atomic operations is used as a "recipe".

 1. Look at the current data, and:
 1.1. analyze what values will be removed (in case of impossible conversion
      or table field removal);
 1.2. analyze what values can be converted (e.g. from numeric types to text), and so on.
 2. Optimize the atomic actions knowing that sometimes a compilation of one action
    and another that's opposite to the first means "do nothing". The optimization
    is a simulating of actions' execution.
    For example, when both action A="change field name from 'city' to 'town'"
    and action B="change field name from 'town' to 'city'" is specified, the compilation
    of the actions means "change field name from 'city' to 'city'", what is a NULL action.
    On the other hand, we need to execute all the actions on the destination table
    in proper order, and not just drop them. For the mentioned example, between actions
    A and B there can be an action like C="change the type of field 'city' to LongText".
    If A and B were simply removed, C would become invalid (there is no 'city' field).
 3. Ask user whether she agrees with the results of analysis mentioned in 1.
 3.2. Additionally, it may be possible to get some hints from the user, as humans usually
      know more about logic behind the altered table schema than any machine.
      If the user provided hints about the altering, apply them to the actions list.
 4. Create (empty) destination table schema with temporary name, using
    the information collected so far.
 5. Copy the data from the source to destionation table. Convert values,
    move them between fields, using the information collected.
 6. Remove the source table.
 7. Rename the destination table to the name previously assigned for the source table.

 Notes:
 * The actions 4 to 7 should be performed within a database transaction.
 * [todo] We want to take care about database relationships as well.
    For example, is a table field is removed, relationships related to this field should
    be also removed (similar rules as in the Query Designer).
 * Especially, care about primary keys and uniquess (indices). Recreate them when needed.
   The problem could be if such analysis may require to fetch the entire table data
   to the client side. Use "SELECT INTO" statements if possible to avoid such a treat.

 The KDbAlterTableHandler is used in Kexi's Table Designer.
 Already opened KDbConnection object is needed.

 Use case:
 @code
  KDbConnection *conn = ...

  // add some actions (in reality this is performed by tracking user's actions)
  // Actions 1, 2 will require physical table altering PhysicalAltering
  // Action 3 will only require changes in kexi__fields
  // Action 4 will only require changes in extended table schema written in kexi__objectdata
  AlterTable::ActionList list;

  // 1. rename the "city" field to "town"
  list << new ChangeFieldPropertyAction("city", "name", "town")

  // 2. change type of "town" field to "LongText"
    << new ChangeFieldPropertyAction("town", "type", "LongText")

  // 3. set caption of "town" field to "Town"
    << new ChangeFieldPropertyAction("town", "caption", "Town")

  // 4. set visible decimal places to 4 for "cost" field
    << new ChangeFieldPropertyAction("cost", "visibleDecimalPlaces", 4)

  KDbAlterTableHandler::execute( *conn );

 @endcode

 Actions for Alter
*/
class KDB_EXPORT KDbAlterTableHandler : public KDbResultable
{
public:
    class ChangeFieldPropertyAction;
    class RemoveFieldAction;
    class InsertFieldAction;
    class MoveFieldPositionAction;

    //! Defines flags for possible altering requirements; can be combined.
    enum AlteringRequirements {
        /*! Physical table altering is required; e.g. ALTER TABLE ADD COLUMN. */
        PhysicalAlteringRequired = 1,

        /*! Data conversion is required; e.g. converting integer
         values to string after changing column type from integer to text. */
        DataConversionRequired = 2,

        /*! Changes to the main table schema (in kexi__fields) required,
         this does not require physical changes for the table;
         e.g. changing value of the "caption" or "description" property. */
        MainSchemaAlteringRequired = 4,

        /*! Only changes to extended table schema required,
         this does not require physical changes for the table;
         e.g. changing value of the "visibleDecimalPlaces" property
         or any of the custom properties. */
        ExtendedSchemaAlteringRequired = 8,

        /*! Convenience flag, changes to the main or extended schema is required. */
        SchemaAlteringRequired = ExtendedSchemaAlteringRequired | MainSchemaAlteringRequired
    };

    class ActionBase;
    //! For collecting actions related to a single field
    typedef KDbUtils::AutodeletedHash<QByteArray, ActionBase*> ActionDict;
    typedef KDbUtils::AutodeletedHash<int, ActionDict*> ActionDictDict; //!< for collecting groups of actions by field UID
    typedef QHash<QByteArray, ActionBase*>::Iterator ActionDictIterator;
    typedef QHash<QByteArray, ActionBase*>::ConstIterator ActionDictConstIterator;
    typedef QHash<int, ActionDict*>::Iterator ActionDictDictIterator;
    typedef QHash<int, ActionDict*>::ConstIterator ActionDictDictConstIterator;
    typedef QVector<ActionBase*> ActionsVector; //!< for collecting actions related to a single field

    //! Defines a type for action list.
    typedef QList<ActionBase*> ActionList;

    //! Defines a type for action list's iterator.
    typedef QList<ActionBase*>::ConstIterator ActionListIterator;

    //! Abstract base class used for implementing all the AlterTable actions.
    class KDB_EXPORT ActionBase
    {
    public:
        virtual ~ActionBase();

        ChangeFieldPropertyAction& toChangeFieldPropertyAction();
        RemoveFieldAction& toRemoveFieldAction();
        InsertFieldAction& toInsertFieldAction();
        MoveFieldPositionAction& toMoveFieldPositionAction();

        //! @return true if the action is NULL; used in the Table Designer
        //! for temporarily collecting actions that have no effect at all.
        inline bool isNull() const {
            return m_null;
        }

        //! Controls debug options for actions. Used in debugString() and debug().
        class DebugOptions
        {
        public:
            inline DebugOptions() : showUID(true), showFieldDebug(false) {}

            //! true if UID should be added to the action debug string (the default)
            bool showUID;

            //! true if the field associated with the action (if exists) should
            //! be appended to the debug string (default is false)
            bool showFieldDebug;
        };

        inline virtual QString debugString(const DebugOptions& debugOptions = DebugOptions()) {
            Q_UNUSED(debugOptions); return QLatin1String("ActionBase");
        }

//! @todo add QDebug operator <<
        void debug(const DebugOptions& debugOptions = DebugOptions());

    protected:
        //! @internal, used for constructing null action
        explicit ActionBase(bool null);

        //! Sets requirements for altering; used internally by KDbAlterTableHandler object
        inline void setAlteringRequirements(int alteringRequirements) {
            m_alteringRequirements = alteringRequirements;
        }

        inline int alteringRequirements() const {
            return m_alteringRequirements;
        }

        inline virtual void updateAlteringRequirements() {}

        /*! Simplifies @a fieldActions dictionary. If this action has to be inserted
         Into the dictionary, an ActionDict is created first and then a copy of this action
         is inserted into it. */
        inline virtual void simplifyActions(ActionDictDict *fieldActions) {
            Q_UNUSED(fieldActions);
        }

        /*! After calling simplifyActions() for each action,
         shouldBeRemoved() is called for them as an additional step.
         This is used for ChangeFieldPropertyAction items so actions
         that do not change property values are removed. */
        inline virtual bool shouldBeRemoved(ActionDictDict *fieldActions) {
            Q_UNUSED(fieldActions); return false;
        }

        inline virtual tristate updateTableSchema(KDbTableSchema* table, KDbField* field,
                                           QHash<QString, QString>* fieldHash) {
            Q_UNUSED(table); Q_UNUSED(field); Q_UNUSED(fieldHash); return true;
        }

    private:
        //! Performs physical execution of this action.
        inline virtual tristate execute(KDbConnection* /*conn*/, KDbTableSchema* /*table*/) {
            return true;
        }

        //! requirements for altering; used internally by KDbAlterTableHandler object
        int m_alteringRequirements;

        //! @internal used for "simplify" algorithm
        int m_order;

        const bool m_null;

        friend class KDbAlterTableHandler;
    };

    //! Abstract base class used for implementing table field-related actions.
    class KDB_EXPORT FieldActionBase : public ActionBase
    {
    public:
        FieldActionBase(const QString& fieldName, int uid);
        ~FieldActionBase() override;

        //! @return field name for this action
        inline QString fieldName() const {
            return m_fieldName;
        }

        /*! @return field's unique identifier
         This id is needed because in the meantime there can be more than one
         field sharing the same name, so we need to identify them unambiguously.
         After the (valid) altering is completed all the names will be unique.

         Example scenario when user exchanged the field names:
         1. At the beginning: [field A], [field B]
         2. Rename the 1st field to B: [field B], [field B]
         3. Rename the 2nd field to A: [field B], [field A] */
        inline int uid() const {
            return m_fieldUID;
        }

        //! Sets field name for this action
        inline void setFieldName(const QString& fieldName) {
            m_fieldName = fieldName;
        }

    protected:
        //! @internal, used for constructing null action
        explicit FieldActionBase(bool null);

        //! field's unique identifier, @see uid()
        int m_fieldUID;
    private:
        QString m_fieldName;
    };

    /*! Defines an action for changing a single property value of a table field.
     Supported properties are currently:
     "name", "type", "caption", "description", "unsigned", "maxLength", "precision",
     "defaultWidth", "defaultValue", "primaryKey", "unique", "notNull", "allowEmpty",
     "autoIncrement", "indexed", "visibleDecimalPlaces"

     More to come.
    */
    class KDB_EXPORT ChangeFieldPropertyAction : public FieldActionBase
    {
    public:
        ChangeFieldPropertyAction(const QString& fieldName,
                                  const QString& propertyName, const QVariant& newValue, int uid);

        //! Creates null action
        ChangeFieldPropertyAction();

        ~ChangeFieldPropertyAction() override;

        inline QString propertyName() const {
            return m_propertyName;
        }
        inline QVariant newValue() const {
            return m_newValue;
        }
        QString debugString(const DebugOptions& debugOptions = DebugOptions()) override;

        void simplifyActions(ActionDictDict *fieldActions) override;

        bool shouldBeRemoved(ActionDictDict *fieldActions) override;

        tristate updateTableSchema(KDbTableSchema *table, KDbField *field,
                                   QHash<QString, QString> *fieldHash) override;

    protected:
        //! @internal, used for constructing null action
        explicit ChangeFieldPropertyAction(bool null);

        void updateAlteringRequirements() override;

        //! Performs physical execution of this action.
        tristate execute(KDbConnection* conn, KDbTableSchema* table) override;

        QString m_propertyName;
        QVariant m_newValue;
    };

    //! Defines an action for removing a single table field.
    class KDB_EXPORT RemoveFieldAction : public FieldActionBase
    {
    public:
        RemoveFieldAction(const QString& fieldName, int uid);

        ~RemoveFieldAction() override;

        QString debugString(const DebugOptions& debugOptions = DebugOptions()) override;

        void simplifyActions(ActionDictDict *fieldActions) override;

        tristate updateTableSchema(KDbTableSchema *table, KDbField *field,
                                   QHash<QString, QString> *fieldHash) override;

    protected:
        //! @internal, used for constructing null action
        explicit RemoveFieldAction(bool null);

        void updateAlteringRequirements() override;

        //! Performs physical execution of this action.
        tristate execute(KDbConnection* conn, KDbTableSchema* table) override;
    };

    //! Defines an action for inserting a single table field.
    class KDB_EXPORT InsertFieldAction : public FieldActionBase
    {
    public:
        InsertFieldAction(int fieldIndex, KDbField *newField, int uid);

        //! copy ctor
        InsertFieldAction(const InsertFieldAction& action);

        //! Creates null action
        InsertFieldAction();

        ~InsertFieldAction() override;

        inline int index() const {
            return m_index;
        }
        inline void setIndex(int index) {
            m_index = index;
        }
        inline const KDbField* field() const {
            return m_field;
        }
        void setField(KDbField* field);
        QString debugString(const DebugOptions& debugOptions = DebugOptions()) override;

        void simplifyActions(ActionDictDict *fieldActions) override;

        tristate updateTableSchema(KDbTableSchema *table, KDbField *field,
                                   QHash<QString, QString> *fieldHash) override;

    protected:
        //! @internal, used for constructing null action
        explicit InsertFieldAction(bool null);

        void updateAlteringRequirements() override;

        //! Performs physical execution of this action.
        tristate execute(KDbConnection* conn, KDbTableSchema* table) override;

        int m_index;

    private:
        KDbField *m_field;
    };

    /*! Defines an action for moving a single table field to a different
     position within table schema. */
    class KDB_EXPORT MoveFieldPositionAction : public FieldActionBase
    {
    public:
        MoveFieldPositionAction(int fieldIndex, const QString& fieldName, int uid);

        ~MoveFieldPositionAction() override;

        inline int index() const {
            return m_index;
        }
        QString debugString(const DebugOptions& debugOptions = DebugOptions()) override;

        void simplifyActions(ActionDictDict *fieldActions) override;

    protected:
        //! @internal, used for constructing null action
        explicit MoveFieldPositionAction(bool null);

        void updateAlteringRequirements() override;

        //! Performs physical execution of this action.
        tristate execute(KDbConnection* conn, KDbTableSchema* table) override;

        int m_index;
    };

    explicit KDbAlterTableHandler(KDbConnection* conn);

    ~KDbAlterTableHandler() override;

    /*! Appends @a action for the alter table tool. */
    void addAction(ActionBase* action);

    /*! Provided for convenience, @see addAction(const ActionBase& action). */
    KDbAlterTableHandler& operator<< (ActionBase* action);

    /*! Removes an action from the alter table tool at index @a index. */
    void removeAction(int index);

    /*! Removes all actions from the alter table tool. */
    void clear();

    /*! Sets @a actions for the alter table tool. Previous actions are cleared.
     @a actions will be owned by the KDbAlterTableHandler object. */
    void setActions(const ActionList& actions);

    /*! @return a list of actions for this AlterTable object.
     Use ActionBase::ListIterator to iterate over the list items. */
    const ActionList& actions() const;

    //! Arguments for KDbAlterTableHandler::execute().
    class ExecutionArguments
    {
    public:
        inline ExecutionArguments()
                : debugString(nullptr)
                , requirements(0)
                , result(false)
                , simulate(false)
                , onlyComputeRequirements(false) {
        }
        /*! If not 0, debug is directed here. Used only in the alter table test suite. */
        QString* debugString;
        /*! Requrements computed, a combination of AlteringRequirements values. */
        int requirements;
        /*! Set to true on success, to false on failure. */
        tristate result;
        /*! Used only in the alter table test suite. */
        bool simulate;
        /*! Set to true if requirements should be computed
         and the execute() method should return afterwards. */
        bool onlyComputeRequirements;
    private:
        Q_DISABLE_COPY(ExecutionArguments)
    };

    /*! Performs table alteration using predefined actions for table named @a tableName,
     assuming it already exists. The KDbConnection object passed to the constructor must exist,
     must be connected and a database must be used. The connection must not be read-only.

     If args.simulate is true, the execution is only simulated, i.e. al lactions are processed
     like for regular execution but no changes are performed physically.
     This mode is used only for debugging purposes.

    @todo For some cases, table schema can completely change, so it will be needed
     to refresh all objects depending on it.
     Implement this!

     Sets args.result to true on success, to false on failure or when the above requirements are not met
     (then, you can get a detailed error message from KDbObject).
     When the action has been cancelled (stopped), args.result is set to cancelled value.
     If args.debugString is not 0, it will be filled with debugging output.
     @return the new table schema object created as a result of schema altering.
     The old table is returned if recreating table schema was not necessary or args.simulate is true.
     0 is returned if args.result is not true. */
    KDbTableSchema* execute(const QString& tableName, ExecutionArguments* args);

    //! Displays debug information about all actions collected by the handler.
    void debug();

    /*! Like execute() with simulate set to true, but debug is directed to debugString.
     This function is used only in the alter table test suite. */
//  tristate simulateExecution(const QString& tableName, QString& debugString);

    /*! Helper. @return a combination of AlteringRequirements values decribing altering type required
     when a given property field's @a propertyName is altered.
     Used internally KDbAlterTableHandler. Moreover it can be also used in the Table Designer's code
     as a temporary replacement before KDbAlterTableHandler is fully implemented.
     Thus, it is possible to identify properties that have no PhysicalAlteringRequired flag set
     (e.g. caption or extended properties like visibleDecimalPlaces. */
    static int alteringTypeForProperty(const QByteArray& propertyName);

private:
    Q_DISABLE_COPY(KDbAlterTableHandler)
    class Private;
    Private * const d;
};

#endif
