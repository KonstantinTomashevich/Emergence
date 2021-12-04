#pragma once

#include <OgreApplicationContext.h>

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

    OgreBites::ApplicationContextBase context;
};
