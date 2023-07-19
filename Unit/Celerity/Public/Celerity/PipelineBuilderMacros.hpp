#pragma once

#include <API/Common/BlockCast.hpp>

#include <Celerity/Event/CascadeRemover.hpp>
#include <Celerity/PipelineBuilder.hpp>

// This file contains macros, that shorten TaskConstructor query creation calls.
// These macros provide no additional logic and work as syntax sugar, that makes calls less verbose.
// All these macros expect that there is TaskConstructor variable name `_constructor` in the scope.

#define FETCH_SINGLETON(Type) _constructor.FetchSingleton (Type::Reflect ().mapping)
#define MODIFY_SINGLETON(Type) _constructor.ModifySingleton (Type::Reflect ().mapping)

#define INSERT_SHORT_TERM(Type) _constructor.InsertShortTerm (Type::Reflect ().mapping)
#define FETCH_SEQUENCE(Type) _constructor.FetchSequence (Type::Reflect ().mapping)
#define MODIFY_SEQUENCE(Type) _constructor.ModifySequence (Type::Reflect ().mapping)

#define INSERT_LONG_TERM(Type) _constructor.InsertLongTerm (Type::Reflect ().mapping)
#define FETCH_VALUE_1F(Type, Field) _constructor.FetchValue (Type::Reflect ().mapping, {Type::Reflect ().Field})
#define FETCH_VALUE_2F(Type, Field1, Field2)                                                                           \
    _constructor.FetchValue (Type::Reflect ().mapping, {Type::Reflect ().Field1, Type::Reflect ().Field2})

#define MODIFY_VALUE_1F(Type, Field) _constructor.ModifyValue (Type::Reflect ().mapping, {Type::Reflect ().Field})
#define MODIFY_VALUE_2F(Type, Field1, Field2)                                                                          \
    _constructor.ModifyValue (Type::Reflect ().mapping, {Type::Reflect ().Field1, Type::Reflect ().Field2})

#define EDIT_VALUE_1F(Type, Field) _constructor.EditValue (Type::Reflect ().mapping, {Type::Reflect ().Field})
#define EDIT_VALUE_2F(Type, Field1, Field2)                                                                            \
    _constructor.EditValue (Type::Reflect ().mapping, {Type::Reflect ().Field1, Type::Reflect ().Field2})

#define REMOVE_VALUE_1F(Type, Field) _constructor.RemoveValue (Type::Reflect ().mapping, {Type::Reflect ().Field})
#define REMOVE_VALUE_2F(Type, Field1, Field2)                                                                          \
    _constructor.RemoveValue (Type::Reflect ().mapping, {Type::Reflect ().Field1, Type::Reflect ().Field2})

#define FETCH_ASCENDING_RANGE(Type, Field)                                                                             \
    _constructor.FetchAscendingRange (Type::Reflect ().mapping, Type::Reflect ().Field)
#define MODIFY_ASCENDING_RANGE(Type, Field)                                                                            \
    _constructor.ModifyAscendingRange (Type::Reflect ().mapping, Type::Reflect ().Field)
#define EDIT_ASCENDING_RANGE(Type, Field)                                                                              \
    _constructor.EditAscendingRange (Type::Reflect ().mapping, Type::Reflect ().Field)
#define REMOVE_ASCENDING_RANGE(Type, Field)                                                                            \
    _constructor.RemoveAscendingRange (Type::Reflect ().mapping, Type::Reflect ().Field)

#define FETCH_DESCENDING_RANGE(Type, Field)                                                                            \
    _constructor.FetchDescendingRange (Type::Reflect ().mapping, Type::Reflect ().Field)
#define MODIFY_DESCENDING_RANGE(Type, Field)                                                                           \
    _constructor.ModifyDescendingRange (Type::Reflect ().mapping, Type::Reflect ().Field)
#define EDIT_DESCENDING_RANGE(Type, Field)                                                                             \
    _constructor.EditDescendingRange (Type::Reflect ().mapping, Type::Reflect ().Field)
#define REMOVE_DESCENDING_RANGE(Type, Field)                                                                           \
    _constructor.RemoveDescendingRange (Type::Reflect ().mapping, Type::Reflect ().Field)

#define FETCH_SIGNAL(Type, Field, Value)                                                                               \
    _constructor.FetchSignal (Type::Reflect ().mapping, Type::Reflect ().Field,                                        \
                              array_cast<decltype (Type::Field), sizeof (std::uint64_t)> (Value))

#define MODIFY_SIGNAL(Type, Field, Value)                                                                              \
    _constructor.ModifySignal (Type::Reflect ().mapping, Type::Reflect ().Field,                                       \
                               array_cast<decltype (Type::Field), sizeof (std::uint64_t)> (Value))

#define EDIT_SIGNAL(Type, Field, Value)                                                                                \
    _constructor.EditSignal (Type::Reflect ().mapping, Type::Reflect ().Field,                                         \
                             array_cast<decltype (Type::Field), sizeof (std::uint64_t)> (Value))

#define REMOVE_SIGNAL(Type, Field, Value)                                                                              \
    _constructor.RemoveSignal (Type::Reflect ().mapping, Type::Reflect ().Field,                                       \
                               array_cast<decltype (Type::Field), sizeof (std::uint64_t)> (Value))

// It's impossible to fit all data for shape and ray intersection
// queries in one line anyway, therefore there is no macros for them.
// But these macros might be added later.

/// Cascade removers are widely used, therefore we add macro for them too.

#define AS_CASCADE_REMOVER_1F(EventType, ObjectType, Field)                                                            \
    SetExecutor<Emergence::Celerity::CascadeRemover> (                                                                 \
        EventType::Reflect ().mapping, ObjectType::Reflect ().mapping,                                                 \
        Emergence::Container::Vector<Emergence::StandardLayout::FieldId> {ObjectType::Reflect ().Field})
