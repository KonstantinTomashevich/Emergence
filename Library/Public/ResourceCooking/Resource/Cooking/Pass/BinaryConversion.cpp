#include <Container/StringBuilder.hpp>

#include <Log/Log.hpp>

#include <Resource/Cooking/Pass/BinaryConversion.hpp>

#include <Serialization/Binary.hpp>
#include <Serialization/Yaml.hpp>

#include <VirtualFileSystem/Reader.hpp>
#include <VirtualFileSystem/Writer.hpp>

namespace Emergence::Resource::Cooking
{
using namespace Memory::Literals;

struct BinaryConversionHeap final
{
    BinaryConversionHeap () noexcept = default;

    BinaryConversionHeap (const BinaryConversionHeap &_other) = delete;

    BinaryConversionHeap (BinaryConversionHeap &&_other) = delete;

    ~BinaryConversionHeap () noexcept
    {
        if (currentBuffer)
        {
            heap.Release (currentBuffer, currentBufferSize);
        }
    }

    EMERGENCE_DELETE_ASSIGNMENT (BinaryConversionHeap);

    static constexpr std::size_t INITIAL_SIZE = 1024u;
    static constexpr std::size_t INITIAL_ALIGNMENT = 16u;

    Memory::Heap heap {Memory::Profiler::AllocationGroup {"BinaryConversionAlgorithm"_us}};
    void *currentBuffer = heap.Acquire (INITIAL_SIZE, INITIAL_ALIGNMENT);
    std::size_t currentBufferSize = INITIAL_SIZE;
    std::size_t currentBufferAlignment = INITIAL_ALIGNMENT;
};

bool BinaryConversionPass (Context &_context) noexcept
{
    // TODO: It might be better to use multithreading? Is there any sense or are we limited by drive read speed?

    EMERGENCE_LOG (INFO, "Resource::Cooking: Binary conversion pass started.");
    BinaryConversionHeap conversionHeap;
    const VirtualFileSystem::Entry passDirectory = _context.GetPassIntermediateRealDirectory ("BinaryConversion");

    for (auto cursor = _context.GetSourceList ().VisitAllObjects (); Container::Optional<ObjectData> object = *cursor;
         ++cursor)
    {
        switch (object->format)
        {
        case Provider::ObjectFormat::BINARY:
            _context.GetTargetList ().AddObject (*object);
            break;

        case Provider::ObjectFormat::YAML:
        {
            EMERGENCE_LOG (INFO, "Resource::Cooking: Converting \"", object->id, "\" of type \"",
                           object->type.GetName (), "\" to binary.");

            const Container::Utf8String fileName = object->entry.GetFullName ();
            VirtualFileSystem::Entry outputEntry {passDirectory, fileName};

            if (outputEntry)
            {
                if (outputEntry.GetLastWriteTime () >= object->entry.GetLastWriteTime ())
                {
                    EMERGENCE_LOG (INFO, "Resource::Cooking: Conversion skipped, already up to date.");
                    break;
                }
            }
            else
            {
                outputEntry = _context.GetVirtualFileSystem ().CreateFile (passDirectory, fileName);
                if (!outputEntry)
                {
                    EMERGENCE_LOG (ERROR, "Resource::Cooking: Unable to create conversion output file \"", fileName,
                                   "\".");
                    return false;
                }
            }

            VirtualFileSystem::Reader reader {object->entry, VirtualFileSystem::OpenMode::BINARY};
            if (!reader)
            {
                EMERGENCE_LOG (ERROR, "Resource::Cooking: Unable to open \"", object->entry.GetFullName (),
                               "\" for read.");
                return false;
            }

            VirtualFileSystem::Writer writer {outputEntry, VirtualFileSystem::OpenMode::BINARY};
            if (!writer)
            {
                EMERGENCE_LOG (ERROR, "Resource::Cooking: Unable to open \"", outputEntry.GetFullPath (),
                               "\" for write.");
                return false;
            }

            if (conversionHeap.currentBufferSize < object->type.GetObjectSize () ||
                conversionHeap.currentBufferAlignment < object->type.GetObjectAlignment ())
            {
                const std::size_t newSize = std::max (conversionHeap.currentBufferSize, object->type.GetObjectSize ());
                const std::size_t newAlignment =
                    std::max (conversionHeap.currentBufferAlignment, object->type.GetObjectAlignment ());

                if (newAlignment != conversionHeap.currentBufferAlignment)
                {
                    conversionHeap.heap.Release (conversionHeap.currentBuffer, conversionHeap.currentBufferSize);
                    conversionHeap.currentBuffer = conversionHeap.heap.Acquire (newSize, newAlignment);
                    conversionHeap.currentBufferSize = newSize;
                    conversionHeap.currentBufferAlignment = newAlignment;
                }
                else
                {
                    conversionHeap.currentBuffer = conversionHeap.heap.Resize (
                        conversionHeap.currentBuffer, newAlignment, conversionHeap.currentBufferSize, newSize);
                    conversionHeap.currentBufferSize = newSize;
                }
            }

            object->type.Construct (conversionHeap.currentBuffer);
            if (!Serialization::Yaml::DeserializeObject (
                    reader.InputStream (), conversionHeap.currentBuffer, object->type,
                    _context.GetInitialResourceProvider ().GetPatchableTypesRegistry ()))
            {
                EMERGENCE_LOG (ERROR, "Resource::Cooking: Unable to deserialize \"", object->entry.GetFullName (),
                               "\" content.");

                object->type.Destruct (conversionHeap.currentBuffer);
                return false;
            }

            Serialization::Binary::SerializeObject (writer.OutputStream (), conversionHeap.currentBuffer, object->type);
            object->type.Destruct (conversionHeap.currentBuffer);

            object->entry = outputEntry;
            object->format = Provider::ObjectFormat::BINARY;
            _context.GetTargetList ().AddObject (*object);

            EMERGENCE_LOG (INFO, "Resource::Cooking: Conversion successful.");
            break;
        }
        }
    }

    for (auto cursor = _context.GetSourceList ().VisitAllThirdParty ();
         Container::Optional<ThirdPartyData> thirdParty = *cursor; ++cursor)
    {
        _context.GetTargetList ().AddThirdParty (*thirdParty);
    }

    EMERGENCE_LOG (INFO, "Resource::Cooking: Binary conversion pass finished successfully.");
    _context.OnPassFinished ();
    return true;
}
} // namespace Emergence::Resource::Cooking
