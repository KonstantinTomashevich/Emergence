#pragma once

#include <Celerity/PipelineBuilder.hpp>

#include <SyntaxSugar/BlockCast.hpp>

// TODO: Try to use these macro in other tasks, not only physics.

// This file contains macros, that shorten TaskConstructor query creation calls.
// These macros provide no additional logic and work as syntax sugar, that makes calls less verbose.

#define MFetchSingleton(Type) FetchSingleton (Type::Reflect ().mapping)
#define MModifySingleton(Type) ModifySingleton (Type::Reflect ().mapping)

#define MInsertShortTerm(Type) InsertShortTerm (Type::Reflect ().mapping)
#define MFetchSequence(Type) FetchSequence (Type::Reflect ().mapping)
#define MModifySequence(Type) ModifySequence (Type::Reflect ().mapping)

#define MInsertLongTerm(Type) InsertLongTerm (Type::Reflect ().mapping)
#define MFetchValue1F(Type, Field) FetchValue (Type::Reflect ().mapping, {Type::Reflect ().Field})
#define MModifyValue1F(Type, Field) ModifyValue (Type::Reflect ().mapping, {Type::Reflect ().Field})
#define MEditValue1F(Type, Field) EditValue (Type::Reflect ().mapping, {Type::Reflect ().Field})
#define MRemoveValue1F(Type, Field) RemoveValue (Type::Reflect ().mapping, {Type::Reflect ().Field})

#define MFetchAscendingRange(Type, Field) FetchAscendingRange (Type::Reflect ().mapping, Type::Reflect ().Field)
#define MModifyAscendingRange(Type, Field) ModifyAscendingRange (Type::Reflect ().mapping, Type::Reflect ().Field)
#define MEditAscendingRange(Type, Field) EditAscendingRange (Type::Reflect ().mapping, Type::Reflect ().Field)
#define MRemoveAscendingRange(Type, Field) RemoveAscendingRange (Type::Reflect ().mapping, Type::Reflect ().Field)

#define MFetchDescendingRange(Type, Field) FetchDescendingRange (Type::Reflect ().mapping, Type::Reflect ().Field)
#define MModifyDescendingRange(Type, Field) ModifyDescendingRange (Type::Reflect ().mapping, Type::Reflect ().Field)
#define MEditDescendingRange(Type, Field) EditDescendingRange (Type::Reflect ().mapping, Type::Reflect ().Field)
#define MRemoveDescendingRange(Type, Field) RemoveDescendingRange (Type::Reflect ().mapping, Type::Reflect ().Field)

#define MFetchSignal(Type, Field, Value)                                                                               \
    FetchSignal (Type::Reflect ().mapping, Type::Reflect ().Field,                                                     \
                 array_cast<decltype (Type::Field), sizeof (uint64_t)> (Value))

#define MModifySignal(Type, Field, Value)                                                                              \
    ModifySignal (Type::Reflect ().mapping, Type::Reflect ().Field,                                                    \
                  array_cast<decltype (Type::Field), sizeof (uint64_t)> (Value))

#define MEditSignal(Type, Field, Value)                                                                                \
    EditSignal (Type::Reflect ().mapping, Type::Reflect ().Field,                                                      \
                array_cast<decltype (Type::Field), sizeof (uint64_t)> (Value))

#define MRemoveSignal(Type, Field, Value)                                                                              \
    RemoveSignal (Type::Reflect ().mapping, Type::Reflect ().Field,                                                    \
                  array_cast<decltype (Type::Field), sizeof (uint64_t)> (Value))

// It's impossible to fit all data for shape and ray intersection
// queries in one line anyway, therefore there is no macros for them.
