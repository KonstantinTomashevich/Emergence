#include <Memory/Profiler/Test/AllocationGroup.hpp>
#include <Memory/Profiler/Test/Capture.hpp>

#include <Testing/SetupMain.hpp>

// Use "include marker", so linker will not omit translation units with autoregistered tests.
static const bool ALLOCATION_GROUP_TEST_INCLUDE_MARKER =
    Emergence::Memory::Profiler::Test::AllocationGroupTestIncludeMarker ();
static const bool CAPTURE_TEST_INCLUDE_MARKER = Emergence::Memory::Profiler::Test::CaptureTestIncludeMarker ();
