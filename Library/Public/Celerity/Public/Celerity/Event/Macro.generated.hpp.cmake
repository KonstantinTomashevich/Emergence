string (APPEND CONTENT "// AUTOGENERATED BY BUILD SYSTEM, DO NOT MODIFY.\n\n")
string (APPEND CONTENT "#pragma once\n\n")
string (APPEND CONTENT "#include <StandardLayout/MappingRegistration.hpp>\n\n")
string (APPEND CONTENT "// Macros for declaration and implementation of Celerity event structures.\n")
string (APPEND CONTENT "// There could be a lot of small simple events and these macros make event addition easy.\n\n")

string (APPEND CONTENT "#define EMERGENCE_CELERITY_EVENT0_DECLARATION(Class)\\\n")
string (APPEND CONTENT "    struct Class final\\\n")
string (APPEND CONTENT "    {\\\n")
string (APPEND CONTENT "        [[maybe_unused]] uintptr_t alignmentFixer;\\\n")
string (APPEND CONTENT "\\\n")
string (APPEND CONTENT "        bool operator== (const Class &_other) const = default;\\\n")
string (APPEND CONTENT "\\\n")
string (APPEND CONTENT "        struct Reflection final\\\n")
string (APPEND CONTENT "        {\\\n")
string (APPEND CONTENT "            Emergence::StandardLayout::Mapping mapping;\\\n")
string (APPEND CONTENT "        };\\\n")
string (APPEND CONTENT "\\\n")
string (APPEND CONTENT "        static Reflection &Reflect () noexcept;\\\n")
string (APPEND CONTENT "    };\n")

string (APPEND CONTENT "#define EMERGENCE_CELERITY_EVENT0_IMPLEMENTATION(Class)\\\n")
string (APPEND CONTENT "    Class::Reflection &Class::Reflect () noexcept\\\n")
string (APPEND CONTENT "    {\\\n")
string (APPEND CONTENT "        static Reflection reflection = [] ()\\\n")
string (APPEND CONTENT "        {\\\n")
string (APPEND CONTENT "            EMERGENCE_MAPPING_REGISTRATION_BEGIN (Class);\\\n")
string (APPEND CONTENT "            EMERGENCE_MAPPING_REGISTRATION_END ();\\\n")
string (APPEND CONTENT "        }();\\\n")
string (APPEND CONTENT "\\\n")
string (APPEND CONTENT "        return reflection;\\\n")
string (APPEND CONTENT "        }\n")

foreach (ARG_COUNT RANGE 1 5)
    string (APPEND CONTENT "#define EMERGENCE_CELERITY_EVENT${ARG_COUNT}_DECLARATION(Class")
    foreach (ARG_N RANGE 1 ${ARG_COUNT})
        string (APPEND CONTENT ", Field${ARG_N}Type, Field${ARG_N}Name")
    endforeach ()

    string (APPEND CONTENT ") \\\n")
    string (APPEND CONTENT "    struct Class final \\\n")
    string (APPEND CONTENT "    { \\\n")

    foreach (ARG_N RANGE 1 ${ARG_COUNT})
        string (APPEND CONTENT "        Field${ARG_N}Type Field${ARG_N}Name {}; \\\n")
    endforeach ()

    string (APPEND CONTENT "\\\n")
    string (APPEND CONTENT "        bool operator== (const Class &_other) const = default;\\\n")
    string (APPEND CONTENT "\\\n")
    string (APPEND CONTENT "        struct Reflection final \\\n")
    string (APPEND CONTENT "        { \\\n")

    foreach (ARG_N RANGE 1 ${ARG_COUNT})
        string (APPEND CONTENT "            Emergence::StandardLayout::FieldId Field${ARG_N}Name; \\\n")
    endforeach ()

    string (APPEND CONTENT "            Emergence::StandardLayout::Mapping mapping; \\\n")
    string (APPEND CONTENT "        }; \\\n")
    string (APPEND CONTENT "        \\\n")
    string (APPEND CONTENT "        static Reflection &Reflect () noexcept; \\\n")
    string (APPEND CONTENT "    }\n\n")

    string (APPEND CONTENT "#define EMERGENCE_CELERITY_EVENT${ARG_COUNT}_IMPLEMENTATION(Class")
    foreach (ARG_N RANGE 1 ${ARG_COUNT})
        string (APPEND CONTENT ", Field${ARG_N}Name")
    endforeach ()

    string (APPEND CONTENT ") \\\n")
    string (APPEND CONTENT "    Class::Reflection &Class::Reflect () noexcept \\\n")
    string (APPEND CONTENT "    { \\\n")
    string (APPEND CONTENT "        static Reflection reflection = [] () \\\n")
    string (APPEND CONTENT "        { \\\n")
    string (APPEND CONTENT "            EMERGENCE_MAPPING_REGISTRATION_BEGIN (Class);\\\n")

    foreach (ARG_N RANGE 1 ${ARG_COUNT})
        string (APPEND CONTENT "            EMERGENCE_MAPPING_REGISTER_REGULAR (Field${ARG_N}Name);\\\n")
    endforeach ()

    string (APPEND CONTENT "            EMERGENCE_MAPPING_REGISTRATION_END (); \\\n")
    string (APPEND CONTENT "        }(); \\\n")
    string (APPEND CONTENT "\\\n")
    string (APPEND CONTENT "        return reflection; \\\n")
    string (APPEND CONTENT "    }\n\n")
endforeach ()
