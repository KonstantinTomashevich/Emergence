#include <Reference/Test/Tests.hpp>

namespace Emergence::Reference::Test
{
using namespace Tasks;

Scenario ConstructAndDestructSingle ()
{
    return
        {
            CheckStatus {false},
            Create {"original"},
            CheckStatus {true},
            Delete {"original"},
            CheckStatus {false},
        };
}

Scenario ConstructAndDestructMultiple ()
{
    return
        {
            CheckStatus {false},
            Create {"first"},
            CheckStatus {true},
            Create {"second"},
            CheckStatus {true},
            Delete {"first"},
            CheckStatus {true},
            Delete {"second"},
            CheckStatus {false},
        };
}

Scenario MoveChain ()
{
    return
        {
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
        };
}

Scenario MoveCopy ()
{
    return
        {
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
        };
}

Scenario CopyMultiple ()
{
    return
        {
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
        };
}

Scenario CopyAssignMultiple ()
{
    return
        {
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
        };
}

Scenario MoveAssignChain ()
{
    return
        {
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
        };
}
} // namespace Emergence::Reference::Test
