#pragma once

#include <Celerity/PipelineBuilder.hpp>

#include <SyntaxSugar/BlockCast.hpp>

// This file contains macros, that shorten TaskConstructor query creation calls.
// These macros provide no additional logic and work as syntax sugar, that makes calls less verbose.

#define MFetchSingleton(Type) FetchSingleton (Type::Reflect ().mapping)   // NOLINT(readability-identifier-naming)
#define MModifySingleton(Type) ModifySingleton (Type::Reflect ().mapping) // NOLINT(readability-identifier-naming)

#define MInsertShortTerm(Type) InsertShortTerm (Type::Reflect ().mapping) // NOLINT(readability-identifier-naming)
#define MFetchSequence(Type) FetchSequence (Type::Reflect ().mapping)     // NOLINT(readability-identifier-naming)
#define MModifySequence(Type) ModifySequence (Type::Reflect ().mapping)   // NOLINT(readability-identifier-naming)

// NOLINTNEXTLINE(readability-identifier-naming)
#define MInsertLongTerm(Type) InsertLongTerm (Type::Reflect ().mapping)

// NOLINTNEXTLINE(readability-identifier-naming)
#define MFetchValue1F(Type, Field) FetchValue (Type::Reflect ().mapping, {Type::Reflect ().Field})

// NOLINTNEXTLINE(readability-identifier-naming)
#define MModifyValue1F(Type, Field) ModifyValue (Type::Reflect ().mapping, {Type::Reflect ().Field})

// NOLINTNEXTLINE(readability-identifier-naming)
#define MEditValue1F(Type, Field) EditValue (Type::Reflect ().mapping, {Type::Reflect ().Field})

// NOLINTNEXTLINE(readability-identifier-naming)
#define MRemoveValue1F(Type, Field) RemoveValue (Type::Reflect ().mapping, {Type::Reflect ().Field})

// NOLINTNEXTLINE(readability-identifier-naming)
#define MFetchAscendingRange(Type, Field) FetchAscendingRange (Type::Reflect ().mapping, Type::Reflect ().Field)

// NOLINTNEXTLINE(readability-identifier-naming)
#define MModifyAscendingRange(Type, Field) ModifyAscendingRange (Type::Reflect ().mapping, Type::Reflect ().Field)

// NOLINTNEXTLINE(readability-identifier-naming)
#define MEditAscendingRange(Type, Field) EditAscendingRange (Type::Reflect ().mapping, Type::Reflect ().Field)

// NOLINTNEXTLINE(readability-identifier-naming)
#define MRemoveAscendingRange(Type, Field) RemoveAscendingRange (Type::Reflect ().mapping, Type::Reflect ().Field)

// NOLINTNEXTLINE(readability-identifier-naming)
#define MFetchDescendingRange(Type, Field) FetchDescendingRange (Type::Reflect ().mapping, Type::Reflect ().Field)

// NOLINTNEXTLINE(readability-identifier-naming)
#define MModifyDescendingRange(Type, Field) ModifyDescendingRange (Type::Reflect ().mapping, Type::Reflect ().Field)

// NOLINTNEXTLINE(readability-identifier-naming)
#define MEditDescendingRange(Type, Field) EditDescendingRange (Type::Reflect ().mapping, Type::Reflect ().Field)

// NOLINTNEXTLINE(readability-identifier-naming)
#define MRemoveDescendingRange(Type, Field) RemoveDescendingRange (Type::Reflect ().mapping, Type::Reflect ().Field)

// NOLINTNEXTLINE(readability-identifier-naming)
#define MFetchSignal(Type, Field, Value)                                                                               \
    FetchSignal (Type::Reflect ().mapping, Type::Reflect ().Field,                                                     \
                 array_cast<decltype (Type::Field), sizeof (uint64_t)> (Value))

// NOLINTNEXTLINE(readability-identifier-naming)
#define MModifySignal(Type, Field, Value)                                                                              \
    ModifySignal (Type::Reflect ().mapping, Type::Reflect ().Field,                                                    \
                  array_cast<decltype (Type::Field), sizeof (uint64_t)> (Value))

// NOLINTNEXTLINE(readability-identifier-naming)
#define MEditSignal(Type, Field, Value)                                                                                \
    EditSignal (Type::Reflect ().mapping, Type::Reflect ().Field,                                                      \
                array_cast<decltype (Type::Field), sizeof (uint64_t)> (Value))

// NOLINTNEXTLINE(readability-identifier-naming)
#define MRemoveSignal(Type, Field, Value)                                                                              \
    RemoveSignal (Type::Reflect ().mapping, Type::Reflect ().Field,                                                    \
                  array_cast<decltype (Type::Field), sizeof (uint64_t)> (Value))

// It's impossible to fit all data for shape and ray intersection
// queries in one line anyway, therefore there is no macros for them.
