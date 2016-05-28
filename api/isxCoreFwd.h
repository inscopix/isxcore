#ifndef ISX_CORE_FWD_H
#define ISX_CORE_FWD_H

#include <memory>


#define FWD_DECLARE_WITH_PTRS( C ) \
class C; \
typedef std::shared_ptr<C>  Sp##C##_t; \
typedef std::unique_ptr<C>  Up##C##_t; \
typedef std::weak_ptr<C>    Wp##C##_t;

namespace isx
{
    FWD_DECLARE_WITH_PTRS(Recording);
    FWD_DECLARE_WITH_PTRS(Movie);
    FWD_DECLARE_WITH_PTRS(DispatchQueue);
} // namespace isx


#undef FWD_DECLARE_WITH_PTRS

#endif // ISX_CORE_FWD_H