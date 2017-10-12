#ifndef ISX_EXPORT_H
#define ISX_EXPORT_H

#include "isxCoreFwd.h"
#include "isxAsyncTaskHandle.h"
#include "isxTrace.h"

#include <string>
#include <map>

namespace isx
{

class Time;

/// \cond doxygen chokes on enum class inside of namespace
/// Enum to select how to write time stamps in exported file
enum class WriteTimeRelativeTo
{
    FIRST_DATA_ITEM,    ///< write time stamps relative to start time of data set
    UNIX_EPOCH          ///< write time stamps relative to unix epoch
};
/// \endcond doxygen chokes on enum class inside of namespace

/// Write logical traces with names to an output stream, which is used when
/// exporting logical GPIO traces and events.
///
/// \param  inStream        The stream to which to append.
/// \param  inTraces        Outer dimension corresponds to number of named traces,
///                         inner dimension to number of segments.
/// \param  inNames         The names of the traces.
/// \param  inNameHeader    The string to use for the header corresponding to names
///                         (e.g. "Cell Name", "Channel Name").
/// \param  inBaseTime      The 0 time, with which to write relative to.
/// \param  inCheckInCB     Check-in callback function that is periodically invoked
///                         with progress and to tell algo whether to cancel / abort.
/// \return                 True if cancelled.
bool writeLogicalTraces(
        std::ofstream & inStream,
        const std::vector<std::vector<SpLogicalTrace_t>> & inTraces,
        const std::vector<std::string> & inNames,
        const std::string & inNameHeader,
        const Time & inBaseTime,
        AsyncCheckInCB_t inCheckInCB = [](float){return false;});

/// Write regular traces with names to an output stream, which is used when
/// exporting analog GPIO and cellset traces.
///
/// \param  inStream        The stream to which to append.
/// \param  inTraces        Outer dimension corresponds to number of segments,
///                         inner dimension to number of named traces.
/// \param  inNames         The names of the traces.
/// \param  inStatuses      The cell statuses or empty if there are no statuses.
/// \param  inBaseTime      The 0 time, with which to write relative to.
/// \param  inCheckInCB     Check-in callback function that is periodically invoked
///                         with progress and to tell algo whether to cancel / abort.
/// \return                 True if cancelled.
bool writeTraces(
        std::ofstream & inStream,
        const std::vector<std::vector<SpFTrace_t>> & inTraces,
        const std::vector<std::string> & inNames,
        const std::vector<std::string> & inStatuses,
        const Time & inBaseTime,
        AsyncCheckInCB_t inCheckInCB = [](float){return false;});

} // namespace isx

#endif // ISX_EXPORT_H
