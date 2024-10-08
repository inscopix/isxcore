#ifndef ISX_CORE_FWD_H
#define ISX_CORE_FWD_H

#include <memory>
#include <functional>

#define FWD_DECLARE_WITH_PTRS( C ) \
class C; \
typedef std::shared_ptr<C>  Sp##C##_t; \
typedef std::unique_ptr<C>  Up##C##_t; \
typedef std::weak_ptr<C>    Wp##C##_t;

namespace isx
{
    FWD_DECLARE_WITH_PTRS(Movie);
    FWD_DECLARE_WITH_PTRS(WritableMovie);
    FWD_DECLARE_WITH_PTRS(Image);
    FWD_DECLARE_WITH_PTRS(VideoFrame);
    FWD_DECLARE_WITH_PTRS(Recording);
    FWD_DECLARE_WITH_PTRS(CellSet);
    FWD_DECLARE_WITH_PTRS(VesselSet);
    FWD_DECLARE_WITH_PTRS(MovieSeries);
    FWD_DECLARE_WITH_PTRS(ProjectItem);
    FWD_DECLARE_WITH_PTRS(Group);
    FWD_DECLARE_WITH_PTRS(Series);
    FWD_DECLARE_WITH_PTRS(DataSet);
    FWD_DECLARE_WITH_PTRS(Project);
    FWD_DECLARE_WITH_PTRS(ProjectFile);
    FWD_DECLARE_WITH_PTRS(Hdf5FileHandle);
    FWD_DECLARE_WITH_PTRS(DispatchQueueInterface);
    FWD_DECLARE_WITH_PTRS(DispatchQueueWorker);
    FWD_DECLARE_WITH_PTRS(AsyncTaskHandle);
    FWD_DECLARE_WITH_PTRS(Gpio);
    FWD_DECLARE_WITH_PTRS(Events);
    FWD_DECLARE_WITH_PTRS(WritableEvents);
    FWD_DECLARE_WITH_PTRS(LogicalTrace);

} // namespace isx

#undef FWD_DECLARE_WITH_PTRS

#endif // ISX_CORE_FWD_H
