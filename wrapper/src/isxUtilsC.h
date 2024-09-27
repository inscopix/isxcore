#ifndef ISX_UTILS_C_H
#define ISX_UTILS_C_H

#include "isxAsync.h"
#include "isxDataSet.h"
#include <string>
#include <functional>
#include <QCoreApplication>

// Note: This is not thread-safe. Multi-threaded invocation of the functions here will certainly break
// this if there is more than one exception.  To make this thread-safe we'll need to design a different
// API to retrieve the exception string that would probably also make this harder to use.
std::string &
isx_get_last_exception_string_internal();

using OpFunc_t = std::function<void(void)>;
using AsyncOpFunc_t = std::function<isx::AsyncTaskStatus(void)>;

/// Use this function to run all C++ operations to handle exceptions
/// and return a C error code.
///
/// \param  in_op       The operation to run.
/// \return             0 if the operation ran successfully.
///                     -1 otherwise, in which case check isx_get_last_exception_string
///                     for the last error message.
int
isx_process_op(const OpFunc_t & in_op);

/// Similar to isx_process_op, except that it's specialized for operations
/// that return async statuses (e.g. algorithms).
int
isx_process_async_op(const AsyncOpFunc_t & in_op);

/// \param  in_file_paths   The paths of the files to check.
/// \param  in_exp_types    The expected data set types.
/// \throw  isx::ExceptionFileIO    If any of the files do not exist.
/// \throw  isx::ExceptionUserInput If any of the files is not of one of the expected types.
void
isx_check_input_file_path(const std::string & in_file_path, const std::vector<isx::DataSet::Type> in_exp_types);

/// \param  in_file_path    The paths of the file to check.
/// \param  in_exp_ext      If not empty, the expected extension.
///                         If any of the actual extensions do not match, a warning will be issued.
/// \throw  isx::ExceptionFileIO    If any of the files already exist.
void
isx_check_output_file_path(const std::string & in_file_path, const std::string & in_exp_ext = "");

/// A convenience function that deletes a pointer and sets it to nullptr.
///
template <typename T>
void
delete_and_nullify(T *& in_pointer)
{
    delete in_pointer;
    in_pointer = nullptr;
}

/// A convenience function that deletes a pointer to an array
/// and sets it to nullptr.
template <typename T>
void
delete_and_nullify_array(T *& in_pointer)
{
    delete[] in_pointer;
    in_pointer = nullptr;
}

/// A convenience function to delete only if the ret_code is not 0.
///
/// \return     The same return code passed in so that this can be
///             conveniently called with return.
template <typename T>
int
isx_checked_delete_and_nullify(
        const int in_ret_code,
        T *& in_pointer)
{
    if (in_ret_code != 0)
    {
        delete_and_nullify(in_pointer);
    }
    return in_ret_code;
}

#endif // ISX_UTILS_C_H
