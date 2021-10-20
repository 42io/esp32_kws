#pragma once

// C++ compile time check idiom: Non-Copyable Class
// TODO: inheritance approach bloats the code size

class NonCopyable {
protected:
    NonCopyable( NonCopyable const& )              = delete;
    NonCopyable( NonCopyable && )                  = delete;
    NonCopyable& operator = ( NonCopyable const& ) = delete;
    NonCopyable& operator = ( NonCopyable && )     = delete;
    NonCopyable() = default;
};