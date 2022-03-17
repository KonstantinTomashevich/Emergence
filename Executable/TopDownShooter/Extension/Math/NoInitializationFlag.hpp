#pragma once

namespace Emergence::Math
{
/// \brief Used in math structure constructors to specify that structure should not be initialized.
/// \details In some algorithms it is useful to create structure (like matrix) without initialization and fill it
///          during algorithm next steps. To do that, structures provide constructors with only one argument:
///          reference to an instance of this class. This interface is better that uninitialized default constructor,
///          because it explicitly highlights which instances are initialized and which are not.
class NoInitializationFlag final
{
public:
    static inline const NoInitializationFlag &Confirm () noexcept
    {
        static NoInitializationFlag flag;
        return flag;
    }

private:
    NoInitializationFlag () = default;
};
} // namespace Emergence::Math
