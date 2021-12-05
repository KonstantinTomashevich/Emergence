#include <MuteWarnings.hpp>

#pragma once

BEGIN_MUTING_WARNING
#include <OgreApplicationContext.h>
END_MUTING_WARNING

class SharedApplicationContext final
{
public:
    SharedApplicationContext (const SharedApplicationContext &_other) = delete;

    SharedApplicationContext (SharedApplicationContext &&_other) = delete;

    static OgreBites::ApplicationContextBase *Get () noexcept;

    SharedApplicationContext &operator= (const SharedApplicationContext &_other) = delete;

    SharedApplicationContext &operator= (SharedApplicationContext &&_other) = delete;

private:
    SharedApplicationContext () noexcept;

    ~SharedApplicationContext () noexcept;

    OgreBites::ApplicationContextBase *context = nullptr;
};
