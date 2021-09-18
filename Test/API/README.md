# Test API

Different services may share common logic primitives, for example:

- Instances of some service objects behave as handles to actual implementation instances.
- Service provides API for storing and accessing objects.

Instead of duplicating test cases for such pieces of common logic, we use agnostic declarative tests, that can be
executed by each service separately using appropriate driver. These tests are called API tests and their libraries are
stored in this directory.
