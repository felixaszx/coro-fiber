#include "internal.hxx"

using namespace fiber;

property::property() noexcept
    : impl_(new impl)
{
}

property::~property() noexcept = default;
