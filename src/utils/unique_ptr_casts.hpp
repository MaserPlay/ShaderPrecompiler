#pragma once

#include <memory>

template<typename D, typename B>
std::unique_ptr<D> static_unique_cast_ptr(std::unique_ptr<B>& base)
{
    return std::unique_ptr<D>(static_cast<D*>(base.release()));
}

template<typename D, typename B>
std::unique_ptr<D> static_unique_cast_ptr(std::unique_ptr<B>&& base)
{
    return std::unique_ptr<D>(static_cast<D*>(base.release()));
}

template<typename D, typename B>
std::unique_ptr<D> dynamic_unique_cast_ptr(std::unique_ptr<B>& base)
{
    if (D* d = dynamic_cast<D*>(base.get()); d != NULL)
    {
        return std::unique_ptr<D>(static_cast<D*>(base.release()));
    }
    return NULL;
}

template<typename D, typename B>
std::unique_ptr<D> dynamic_unique_cast_ptr(std::unique_ptr<B>&& base)
{
    if (D* d = dynamic_cast<D*>(base.get()); d != NULL)
    {
        return std::unique_ptr<D>(static_cast<D*>(base.release()));
    }
    return NULL;
}