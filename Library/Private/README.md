# Private Libraries

If service implementation uses other services, core part of this implementation must be extracted as **private**
library. Therefore, **private** libraries are not designed for direct usage:

- They are not as documented as services and **public** libraries.
- Their interface is not polished.
- They are tested only as parts of corresponding services.

Therefore, it's advised to not use these libraries directly, unless it's absolutely necessary.

Utility libraries, that are not designed for use outside of [Emergence](../../README.md), for
example [APICommon](./APICommon/README.md), are private libraries too.
