#include "isxUtilsC.h"
#include "isxLog.h"
#include "isxAssert.h"
#include "isxPathUtils.h"
#include "isxException.h"

namespace
{
    std::string g_last_exception_string;
}

std::unique_ptr<std::pair<int, std::string>> g_core_app_args;
std::unique_ptr<QCoreApplication> g_core_app;

isx::isize_t g_movie_id = 0;
std::map <isx::isize_t, isx::SpMovie_t> g_open_movies;

isx::isize_t g_writable_movie_id = 0;
std::map <isx::isize_t, isx::SpWritableMovie_t> g_open_writable_movies;

isx::isize_t g_image_id = 0;
std::map <isx::isize_t, isx::SpVideoFrame_t> g_open_images;

isx::isize_t g_cell_set_id = 0;
std::map <isx::isize_t, isx::SpCellSet_t> g_open_cell_sets;

isx::isize_t g_events_id = 0;
std::map <isx::isize_t, isx::SpEvents_t> g_open_events;

std::map <std::pair<isx::isize_t, std::string>, isx::SpLogicalTrace_t> g_open_logical_traces;

isx::isize_t g_writable_events_id = 0;
std::map <isx::isize_t, isx::SpWritableEvents_t> g_open_writable_events;

std::string &
isx_get_last_exception_string_internal()
{
    return g_last_exception_string;
}

int
isx_process_op(const OpFunc_t & in_op)
{
    int ret_code = 0;

    try
    {
        in_op();
    }
    catch (const std::exception & e)
    {
        g_last_exception_string = e.what();
        ret_code = -1;
    }
    catch (...)
    {
        g_last_exception_string = "Unknown exception";
        ret_code = -1;
    }

    return ret_code;
}

int
isx_process_async_op(const AsyncOpFunc_t & in_op)
{
    auto res = isx::AsyncTaskStatus::PENDING;

    try
    {
        res = isx::AsyncTaskStatus::PROCESSING;
        res = in_op();
    }
    catch (std::exception & e)
    {
        g_last_exception_string = e.what();
        res = isx::AsyncTaskStatus::ERROR_EXCEPTION;
    }
    catch (...)
    {
        g_last_exception_string = "Unknown exception";
        res = isx::AsyncTaskStatus::ERROR_EXCEPTION;
    }

    return res == isx::AsyncTaskStatus::COMPLETE ? 0 : -1;
}

void
isx_check_input_file_path(const std::string & in_file_path, const std::vector<isx::DataSet::Type> in_exp_types)
{
    if (!isx::pathExists(in_file_path))
    {
        ISX_THROW(isx::ExceptionFileIO, "File does not exist: ", in_file_path);
    }

    const isx::DataSet::Type type = isx::readDataSetType(in_file_path, {});
    if (std::find(in_exp_types.begin(), in_exp_types.end(), type) == in_exp_types.end())
    {
        // Check ambiguous case of .hdf5 which readDataSetType only classfies as either Movie or Image
        // Accept hdf5 file if GPIO type is in expected types vector
        std::string extension = isx::getExtension(in_file_path);
        if (extension == "hdf5")
        {
            if (std::find(in_exp_types.begin(), in_exp_types.end(), isx::DataSet::Type::GPIO) != in_exp_types.end())
            {
                return;
            }
        }
        std::string exp_types_string;
        for (size_t i = 0; i < in_exp_types.size(); ++i)
        {
            exp_types_string += isx::DataSet::getTypeString(in_exp_types[i]);
            if (i != (in_exp_types.size() - 1))
            {
                exp_types_string += " or ";
            }
        }

        ISX_THROW(isx::ExceptionUserInput,
                "Expected data set to be of type: ", exp_types_string, ". ",
                "Instead it is of type: ", isx::DataSet::getTypeString(type), ". ");
    }
}

void
isx_check_output_file_path(const std::string & in_file_path, const std::string & in_exp_ext)
{
    if (isx::pathExists(in_file_path))
    {
        ISX_THROW(isx::ExceptionFileIO, "File already exists: ", in_file_path);
    }

    if (!in_exp_ext.empty())
    {
        const std::string ext = isx::getExtension(in_file_path);
        if (ext != in_exp_ext)
        {
            ISX_LOG_WARNING("Expected extension: ", in_exp_ext, ". ",
                    "Instead extension is: ", ext, ". ");
        }
    }
}
