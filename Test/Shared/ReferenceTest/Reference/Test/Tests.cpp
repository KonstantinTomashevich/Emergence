#include <Reference/Test/Tests.hpp>

namespace Emergence::Reference::Test
{
using namespace Tasks;

Scenario ConstructAndDestructSingle ()
{
    return {
        // clang-format off
        CheckStatus {false},
        Create {"original"},
        CheckStatus {true},
        Delete {"original"},
        CheckStatus {false},
        // clang-format on
    };
}

Scenario ConstructAndDestructMultiple ()
{
    return {
        // clang-format off
        CheckStatus {false},
        Create {"first"},
        CheckStatus {true},
        Create {"second"},
        CheckStatus {true},
        Delete {"first"},
        CheckStatus {true},
        Delete {"second"},
        CheckStatus {false},
        // clang-format on
    };
}

Scenario MoveChain ()
{
    return {
        // clang-format off
        CheckStatus {false},
        Create {"original"},
        CheckStatus {true},

        Move {"original", "first"},
        CheckStatus {true},

        Move {"first", "second"},
        CheckStatus {true},

        Move {"second", "third"},
        CheckStatus {true},

        Delete {"third"},
        CheckStatus {false},
        // clang-format on
    };
}

Scenario MoveCopy ()
{
    return {
        // clang-format off
        CheckStatus {false},
        Create {"original"},
        CheckStatus {true},

        Copy {"original", "copied"},
        CheckStatus {true},

        Move {"copied", "moved"},
        CheckStatus {true},

        Delete {"original"},
        CheckStatus {true},

        Delete {"moved"},
        CheckStatus {false},
        // clang-format on
    };
}

Scenario CopyMultiple ()
{
    return {
        // clang-format off
        CheckStatus {false},
        Create {"original"},
        CheckStatus {true},

        Copy {"original", "first"},
        CheckStatus {true},

        Copy {"first", "second"},
        CheckStatus {true},

        Copy {"second", "third"},
        CheckStatus {true},

        Delete {"original"},
        CheckStatus {true},

        Delete {"first"},
        CheckStatus {true},

        Delete {"second"},
        CheckStatus {true},

        Delete {"third"},
        CheckStatus {false},
        // clang-format on
    };
}

Scenario CopyAssignMultiple ()
{
    return {
        // clang-format off
        CheckStatus {false},
        Create {"original"},
        Create {"first"},
        Create {"second"},
        Create {"third"},
        CheckStatus {true},

        CopyAssign {"original", "first"},
        CheckStatus {true},

        CopyAssign {"original", "second"},
        CheckStatus {true},

        CopyAssign {"first", "second"},
        CheckStatus {true},

        CopyAssign {"second", "third"},
        CheckStatus {true},

        Delete {"original"},
        CheckStatus {true},

        Delete {"first"},
        CheckStatus {true},

        Delete {"second"},
        CheckStatus {true},

        Delete {"third"},
        CheckStatus {false},
        // clang-format on
    };
}

Scenario MoveAssignChain ()
{
    return {
        // clang-format off
        CheckStatus {false},
        Create {"original"},
        CheckStatus {true},
        Create {"first"},
        Create {"second"},
        Create {"third"},
        CheckStatus {true},

        MoveAssign {"original", "first"},
        CheckStatus {true},

        MoveAssign {"first", "second"},
        CheckStatus {true},

        MoveAssign {"second", "third"},
        CheckStatus {true},

        Delete {"third"},
        CheckStatus {false},
        // clang-format on
    };
}
} // namespace Emergence::Reference::Test
