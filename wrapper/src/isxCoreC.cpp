#include "isxCoreC.h"
#include "isxUtilsC.h"

#include "isxMovieFactory.h"
#include "isxCellSetFactory.h"
#include "isxMovieExporter.h"
#include "isxIndexRange.h"
#include "isxLog.h"
#include "isxEvents.h"
#include "isxWritableEvents.h"
#include "isxWritableMovie.h"
#include "isxPathUtils.h"
#include "isxEventsExporter.h"
#include "isxDataSet.h"
#include "isxGpio.h"
#include "isxGpioExporter.h"
#include "isxGpioImporter.h"
#include "isxVesselSetFactory.h"
#include "isxDataSet.h"
#include "isxDecompression.h"
#include "isxSynchronize.h"
#include "isxSeriesUtils.h"

#include <string.h>
#include <functional>
#include <memory>

//////////////////
// Local utilities
//////////////////

namespace
{

// things that need to stay alive
std::unique_ptr<std::pair<int, std::string>> g_core_app_args;
std::unique_ptr<QCoreApplication> g_core_app;

isx::isize_t g_movie_id = 0;
std::map <isx::isize_t, isx::SpMovie_t> g_open_movies;

isx::isize_t g_writable_movie_id = 0;
std::map <isx::isize_t, isx::SpWritableMovie_t> g_open_writable_movies;

isx::isize_t g_cell_set_id = 0;
std::map <isx::isize_t, isx::SpCellSet_t> g_open_cell_sets;

isx::isize_t g_events_id = 0;
std::map <isx::isize_t, isx::SpEvents_t> g_open_events;

isx::isize_t g_gpio_id = 0;
std::map <isx::isize_t, isx::SpGpio_t> g_open_gpios;

isx::isize_t g_vessel_set_id = 0;
std::map <isx::isize_t, isx::SpVesselSet_t> g_open_vessel_sets;

std::map <std::pair<isx::isize_t, std::string>, isx::SpLogicalTrace_t> g_open_logical_traces;

isx::isize_t g_writable_events_id = 0;
std::map <isx::isize_t, isx::SpWritableEvents_t> g_open_writable_events;

void
delete_timing(IsxTimingInfo & in_timing)
{
    delete_and_nullify_array(in_timing.dropped);
    delete_and_nullify_array(in_timing.cropped_first);
    delete_and_nullify_array(in_timing.cropped_last);
    delete_and_nullify_array(in_timing.blank);
}

void
isx_movie_flush_internal(
    IsxMovie * in_movie,
    const isx::TimingInfo * in_timing
)
{
    if (in_movie->read_only)
    {
        if (in_timing != nullptr)
        {
            ISX_LOG_WARNING("Trying to flush read-only movie with timing info.");
        }
    }
    else
    {
        isx::SpWritableMovie_t movie = g_open_writable_movies[in_movie->id];
        if (in_timing == nullptr)
        {
            movie->closeForWriting();
        }
        else
        {
            movie->closeForWriting(*in_timing);
        }
    }
}

void
isx_cell_set_flush_internal(
    IsxCellSet * in_cell_set
)
{
    if (!(in_cell_set->read_only))
    {
        isx::SpCellSet_t cell_set = g_open_cell_sets[in_cell_set->id];
        cell_set->closeForWriting();
    }
}

void
isx_events_flush_internal(
    IsxEvents * in_events
)
{
    if (!in_events->read_only)
    {
        isx::SpWritableEvents_t events = g_open_writable_events[in_events->id];
        events->closeForWriting();
    }
}

void
isx_vessel_set_flush_internal(
    IsxVesselSet * in_vessel_set
)
{
    if (!(in_vessel_set->read_only))
    {
        isx::SpVesselSet_t vessel_set = g_open_vessel_sets[in_vessel_set->id];
        vessel_set->closeForWriting();
    }
}

void
isx_check_cell_index(
    const size_t in_index,
    const size_t in_num_cells)
{
    if (in_index >= in_num_cells)
    {
        ISX_THROW(isx::ExceptionUserInput, "Cell index ", in_index, " is too large. ",
                "There are only ", in_num_cells, " cells.");
    }
}

void
isx_check_vessel_index(
    const size_t in_index,
    const size_t in_num_vessels)
{
    if (in_index >= in_num_vessels)
    {
        ISX_THROW(isx::ExceptionUserInput, "Vessel index ", in_index, " is too large. ",
                "There are only ", in_num_vessels, " vessels.");
    }
}

void
isx_check_channel_index(
    const size_t in_index,
    const size_t in_num_channels)
{
    if (in_index >= in_num_channels)
    {
        ISX_THROW(isx::ExceptionUserInput, "Channel index ", in_index, " is too large. ",
                "There are only ", in_num_channels, " channels.");
    }
}

int
isx_cell_set_get_name_internal(
    IsxCellSet * in_cell_set,
    size_t in_index,
    size_t in_cell_name_size,
    char * out_cell_name
)
{
    return isx_process_op([=]()
    {
        const isx::SpCellSet_t cell_set = g_open_cell_sets[in_cell_set->id];
        isx_check_cell_index(in_index, cell_set->getNumCells());
        const std::string cell_name = cell_set->getCellName(isx::isize_t(in_index));
        isx::copyCppStringToCString(cell_name, out_cell_name, in_cell_name_size);
    });
}

int
isx_events_get_cell_name_internal(
    IsxEvents * in_events,
    size_t in_index,
    size_t in_cell_name_size,
    char * out_cell_name
)
{
    return isx_process_op([=]()
    {
        const isx::SpEvents_t events = g_open_events[in_events->id];
        isx_check_cell_index(in_index, events->numberOfCells());
        const std::string cell_name = events->getCellNamesList().at(in_index);
        isx::copyCppStringToCString(cell_name, out_cell_name, in_cell_name_size);
    });
}

int
isx_vessel_set_get_name_internal(
    IsxVesselSet * in_vessel_set,
    size_t in_index,
    size_t in_vessel_name_size,
    char * out_vessel_name
)
{
    return isx_process_op([=]()
    {
        const isx::SpVesselSet_t vessel_set = g_open_vessel_sets[in_vessel_set->id];
        isx_check_vessel_index(in_index, vessel_set->getNumVessels());
        const std::string vessel_name = vessel_set->getVesselName(isx::isize_t(in_index));
        isx::copyCppStringToCString(vessel_name, out_vessel_name, in_vessel_name_size);
    });
}

int
isx_movie_get_extra_properties_internal(
    IsxMovie * in_movie,
    char * out_extra_properties,
    const size_t in_extra_properties_size
)
{
    return isx_process_op([=]()
    {
        const isx::SpMovie_t movie = g_open_movies[in_movie->id];
        const std::string extra_props = movie->getExtraProperties();
        if (extra_props.size() >= in_extra_properties_size)
        {
            ISX_THROW(isx::ExceptionDataIO, "Properties string contains ", extra_props.size(),
                " characters plus the null terminator, but only instructed to write ",
                in_extra_properties_size, " characters.");
        }
        else
        {
            isx::copyCppStringToCString(extra_props, out_extra_properties, in_extra_properties_size);
        }
    });
}

template <typename T>
std::string
isx_get_acquisition_info_internal(
    const std::shared_ptr<T> in_data_set,
    char * out_acquisition_info = nullptr,
    const size_t in_acquisition_info_size = 0
)
{
    const isx::DataSet::Properties props = {};
    const auto dataType = isx::readDataSetType(in_data_set->getFileName(), props);
    const std::string extra_props = in_data_set->getExtraProperties();
    const std::string acq_info = (dataType == isx::DataSet::Type::NVISION_MOVIE) ? 
        isx::getNVisionAcquisitionInfoFromExtraProps(extra_props) : isx::getAcquisitionInfoFromExtraProps(extra_props);

    if (out_acquisition_info != nullptr)
    {
        if (acq_info.size() >= in_acquisition_info_size)
        {
            ISX_THROW(isx::ExceptionDataIO, "Acquisition info string contains ", acq_info.size(),
                " characters plus the null terminator, but only instructed to write ",
                in_acquisition_info_size, " characters.");
        }
        else
        {
            isx::copyCppStringToCString(acq_info, out_acquisition_info, in_acquisition_info_size);
        }
    }

    return acq_info;
}

isx::Ratio
convert_ratio_c_to_cpp(const IsxRatio & in_ratio)
{
    return isx::Ratio(in_ratio.num, in_ratio.den);
}

IsxRatio
convert_ratio_cpp_to_c(const isx::Ratio & in_ratio)
{
    IsxRatio ratio;
    ratio.num = int64_t(in_ratio.getNum());
    ratio.den = int64_t(in_ratio.getDen());
    return ratio;
}

isx::TimingInfo
convert_timing_info_c_to_cpp(const IsxTimingInfo & in_timing)
{
    std::vector<isx::isize_t> dropped(in_timing.num_dropped);
    if (in_timing.dropped != nullptr)
    {
        for (size_t i = 0; i < in_timing.num_dropped; ++i)
        {
            dropped[i] = in_timing.dropped[i];
        }
    }

    std::vector<isx::IndexRange> cropped(in_timing.num_cropped);
    if ((in_timing.cropped_first != nullptr) && (in_timing.cropped_last != nullptr))
    {
        for (size_t i = 0; i < in_timing.num_cropped; ++i)
        {
            cropped[i] = isx::IndexRange(in_timing.cropped_first[i], in_timing.cropped_last[i]);
        }
    }

    std::vector<isx::isize_t> blank(in_timing.num_blank);
    if (in_timing.dropped != nullptr)
    {
        for (size_t i = 0; i < in_timing.num_blank; ++i)
        {
            blank[i] = in_timing.blank[i];
        }
    }

    return isx::TimingInfo(
            isx::Time(isx::DurationInSeconds(convert_ratio_c_to_cpp(in_timing.start.secs_since_epoch)), in_timing.start.utc_offset),
            isx::DurationInSeconds(convert_ratio_c_to_cpp(in_timing.step)),
            in_timing.num_samples,
            dropped,
            cropped,
            blank
    );
}

void
convert_timing_info_cpp_to_c(
    const isx::TimingInfo & in_timing,
    IsxTimingInfo * out_timing
)
{
    const isx::Time start = in_timing.getStart();
    out_timing->start.secs_since_epoch = convert_ratio_cpp_to_c(start.getSecsSinceEpoch());
    out_timing->start.utc_offset = start.getUtcOffset();
    out_timing->step = convert_ratio_cpp_to_c(in_timing.getStep());
    out_timing->num_samples = in_timing.getNumTimes();

    const std::vector<isx::isize_t> dropped = in_timing.getDroppedFrames();
    out_timing->num_dropped = dropped.size();
    out_timing->dropped = nullptr;
    if (out_timing->num_dropped > 0)
    {
        out_timing->dropped = new size_t[out_timing->num_dropped];
        for (size_t i = 0; i < out_timing->num_dropped; ++i)
        {
            out_timing->dropped[i] = dropped[i];
        }
    }

    const std::vector<isx::IndexRange> cropped = in_timing.getCropped();
    out_timing->num_cropped = cropped.size();
    out_timing->cropped_first = nullptr;
    out_timing->cropped_last = nullptr;
    if (out_timing->num_cropped > 0)
    {
        out_timing->cropped_first = new size_t[out_timing->num_cropped];
        out_timing->cropped_last = new size_t[out_timing->num_cropped];
        for (size_t i = 0; i < out_timing->num_cropped; ++i)
        {
            out_timing->cropped_first[i] = cropped[i].m_first;
            out_timing->cropped_last[i] = cropped[i].m_last;
        }
    }

    const std::vector<isx::isize_t> blank = in_timing.getBlankFrames();
    out_timing->num_blank = blank.size();
    out_timing->blank = nullptr;
    if (out_timing->num_blank > 0)
    {
        out_timing->blank = new size_t[out_timing->num_blank];
        for (size_t i = 0; i < out_timing->num_blank; ++i)
        {
            out_timing->blank[i] = blank[i];
        }
    }

}

isx::SpacingInfo
convert_spacing_info_c_to_cpp(
    const IsxSpacingInfo & in_spacing
)
{
    return isx::SpacingInfo(
            isx::SizeInPixels_t(in_spacing.num_cols, in_spacing.num_rows),
            isx::SizeInMicrons_t(convert_ratio_c_to_cpp(in_spacing.pixel_width), convert_ratio_c_to_cpp(in_spacing.pixel_height)),
            isx::PointInMicrons_t(convert_ratio_c_to_cpp(in_spacing.left), convert_ratio_c_to_cpp(in_spacing.top))
    );
}

void
convert_spacing_info_cpp_to_c(
    const isx::SpacingInfo & in_spacing,
    IsxSpacingInfo * out_spacing
)
{
    out_spacing->num_cols = size_t(in_spacing.getNumColumns());
    out_spacing->num_rows = size_t(in_spacing.getNumRows());

    const isx::SizeInMicrons_t pixel_size = in_spacing.getPixelSize();
    out_spacing->pixel_width = convert_ratio_cpp_to_c(pixel_size.getWidth());
    out_spacing->pixel_height = convert_ratio_cpp_to_c(pixel_size.getHeight());

    const isx::PointInMicrons_t top_left = in_spacing.getTopLeft();
    out_spacing->left = convert_ratio_cpp_to_c(top_left.getX());
    out_spacing->top = convert_ratio_cpp_to_c(top_left.getY());
}

template <typename T>
void
isx_movie_get_frame_data_internal(
    IsxMovie * in_movie,
    const size_t in_index,
    T * out_frame_data
)
{
    const isx::SpMovie_t movie = g_open_movies[in_movie->id];
    const isx::SpVideoFrame_t frame = movie->getFrame(in_index);
    std::memcpy(reinterpret_cast<char *>(out_frame_data), frame->getPixels(), frame->getImageSizeInBytes());
}

template <typename T>
int
isx_movie_write_frame_internal(
        IsxMovie * in_movie,
        const size_t in_index,
        const T * in_frame_data
)
{
    return isx_process_op([=]()
    {
        isx::SpWritableMovie_t movie = g_open_writable_movies[in_movie->id];
        if (movie->getTimingInfo().isIndexValid(in_index))
        {
            isx::SpVideoFrame_t frame = movie->makeVideoFrame(isx::isize_t(in_index));
            std::memcpy(frame->getPixels(), reinterpret_cast<const char *>(in_frame_data), frame->getImageSizeInBytes());
            movie->writeFrame(frame);
        }
        else
        {
            ISX_LOG_WARNING("Attempted to write invalid frame ", in_index, ". Skipping.");
        }
    });
}

char *
make_canonical_file_path(const char * in_file_path)
{
    const std::string canonical_path = isx::getCanonicalPath(in_file_path);
    const size_t canonical_path_size = canonical_path.size() + 1;
    char * out_canonical_path = new char[canonical_path_size];
    isx::copyCppStringToCString(canonical_path, out_canonical_path, canonical_path_size);
    return out_canonical_path;
}

} // namespace

//////////////////////
// API version methods
//////////////////////

int
isx_get_version_string_size(
    size_t * out_version_string_size
)
{
    return isx_process_op([=]()
    {
        const std::string version_string = isx::CoreVersionString();
        *out_version_string_size = version_string.size() + 1;
    });
}

int
isx_get_version_string(
    char * out_version_string,
    const size_t in_version_string_size
)
{
    return isx_process_op([=]()
    {
        const std::string version_string = isx::CoreVersionString();
        isx::copyCppStringToCString(version_string, out_version_string, in_version_string_size);
    });
}

////////////////////////////////
// TimingInfo struct and methods
////////////////////////////////

int
isx_timing_info_get_secs_since_start(
    IsxTimingInfo * in_timing_info,
    int64_t * out_secs_since_start_num,
    int64_t * out_secs_since_start_den
)
{
    return isx_process_op([=]()
    {
        const isx::TimingInfo timing_info = convert_timing_info_c_to_cpp(*in_timing_info);
        const isx::Time start = timing_info.getStart();
        for (isx::isize_t t = 0; t < timing_info.getNumTimes(); ++t)
        {
            const isx::DurationInSeconds secs_since_start = timing_info.convertIndexToStartTime(t) - start;
            out_secs_since_start_num[t] = int64_t(secs_since_start.getNum());
            out_secs_since_start_den[t] = int64_t(secs_since_start.getDen());
        }
    });
}

int
isx_timing_info_get_valid_sample_mask(
    IsxTimingInfo * in_timing_info,
    uint8_t * out_mask
)
{
    return isx_process_op([=]()
    {
        const isx::TimingInfo timing_info = convert_timing_info_c_to_cpp(*in_timing_info);
        for (isx::isize_t t = 0; t < timing_info.getNumTimes(); ++t)
        {
            out_mask[t] = uint8_t(timing_info.isIndexValid(t));
        }
    });
}


///////////////////////////
// Movie struct and methods
///////////////////////////

int
isx_read_movie(
    const char * in_file_path,
    IsxMovie ** out_movie
)
{
    const int ret_code = isx_process_op([=]()
    {
        isx_check_input_file_path(in_file_path, {isx::DataSet::Type::MOVIE, isx::DataSet::Type::IMAGE, isx::DataSet::Type::NVISION_MOVIE});

        isx::SpMovie_t movie = isx::readMovie(in_file_path);

        g_open_movies[g_movie_id] = movie;

        *out_movie = new IsxMovie;

        (*out_movie)->id = size_t(g_movie_id);
        (*out_movie)->data_type = int(movie->getDataType());
        (*out_movie)->read_only = true;
        (*out_movie)->file_path = make_canonical_file_path(in_file_path);

        convert_timing_info_cpp_to_c(movie->getTimingInfo(), &((*out_movie)->timing));
        convert_spacing_info_cpp_to_c(movie->getSpacingInfo(), &((*out_movie)->spacing));

        g_movie_id++;
    });

    return isx_checked_delete_and_nullify(ret_code, *out_movie);
}

int
isx_write_movie(
    const char * in_file_path,
    IsxTimingInfo in_timing,
    IsxSpacingInfo in_spacing,
    int in_data_type,
    const bool in_has_frame_header_footer,
    IsxMovie ** out_movie
)
{
    const int ret_code = isx_process_op([=]()
    {
        if (isx::isNVisionMovieFileExtension(in_file_path))
        {
            ISX_THROW(isx::Exception, "Cannot write isxb movies.");
        }

        isx_check_output_file_path(in_file_path, "isxd");

        const auto timing = convert_timing_info_c_to_cpp(in_timing);
        const auto spacing = convert_spacing_info_c_to_cpp(in_spacing);

        isx::SpWritableMovie_t movie = isx::writeMosaicMovie(in_file_path, timing, spacing, isx::DataType(in_data_type), in_has_frame_header_footer);

        g_open_writable_movies[g_writable_movie_id] = movie;

        *out_movie = new IsxMovie;

        (*out_movie)->id = size_t(g_writable_movie_id);
        (*out_movie)->data_type= in_data_type;
        (*out_movie)->read_only = false;
        (*out_movie)->file_path = make_canonical_file_path(in_file_path);
        // We convert back so that we get a deep copy in case the client deletes
        // things in the input structs.
        convert_timing_info_cpp_to_c(timing, &((*out_movie)->timing));
        convert_spacing_info_cpp_to_c(spacing, &((*out_movie)->spacing));

        g_writable_movie_id++;
    });

    return isx_checked_delete_and_nullify(ret_code, *out_movie);
}

int
isx_movie_get_frame_data_u16(
    IsxMovie * in_movie,
    size_t in_index,
    uint16_t * out_frame_data
)
{
    ISX_ASSERT(in_movie->data_type == int(isx::DataType::U16));
    return isx_process_op([=]()
    {
        isx_movie_get_frame_data_internal(in_movie, in_index, out_frame_data);
    });
}

int
isx_movie_get_frame_data_u16_with_header_footer(
    IsxMovie * in_movie,
    size_t in_index,
    uint16_t * out_frame_data
)
{
    ISX_ASSERT(in_movie->data_type == int(isx::DataType::U16));
    return isx_process_op([=]()
    {
        const isx::SpMovie_t movie = g_open_movies[in_movie->id];

        const std::vector<uint16_t> header = movie->getFrameHeader(in_index);
        std::memcpy(out_frame_data, header.data(), sizeof(uint16_t) * header.size());

        const isx::SpVideoFrame_t frame = movie->getFrame(in_index);
        std::memcpy(out_frame_data + header.size(), frame->getPixels(), frame->getImageSizeInBytes());

        const std::vector<uint16_t> footer = movie->getFrameFooter(in_index);
        std::memcpy(out_frame_data + header.size() + frame->getImage().getSpacingInfo().getTotalNumPixels(), footer.data(), sizeof(uint16_t) * footer.size());
    });
}

int
isx_movie_get_frame_header(
    IsxMovie * in_movie,
    size_t in_index,
    uint16_t * out_header
)
{
    return isx_process_op([=]()
    {
        const isx::SpMovie_t movie = g_open_movies[in_movie->id];
        const std::vector<uint16_t> header = movie->getFrameHeader(in_index);
        std::memcpy(out_header, header.data(), sizeof(uint16_t) * header.size());
    });
}

int
isx_movie_get_frame_footer(
    IsxMovie * in_movie,
    size_t in_index,
    uint16_t * out_footer
)
{
    return isx_process_op([=]()
    {
        const isx::SpMovie_t movie = g_open_movies[in_movie->id];
        const std::vector<uint16_t> footer = movie->getFrameFooter(in_index);
        std::memcpy(out_footer, footer.data(), sizeof(uint16_t) * footer.size());
    });
}

int
isx_movie_get_frame_data_f32(
    IsxMovie * in_movie,
    size_t in_index,
    float * out_frame_data
)
{
    ISX_ASSERT(in_movie->data_type == int(isx::DataType::F32));
    return isx_process_op([=]()
    {
        isx_movie_get_frame_data_internal(in_movie, in_index, out_frame_data);
    });
}

int
isx_movie_get_frame_data_u8(
    IsxMovie * in_movie,
    size_t in_index,
    uint8_t * out_frame_data
)
{
    ISX_ASSERT(in_movie->data_type == int(isx::DataType::U8));
    return isx_process_op([=]()
    {
        isx_movie_get_frame_data_internal(in_movie, in_index, out_frame_data);
    });
}

int
isx_movie_write_frame_u16(
    IsxMovie *  in_movie,
    size_t in_index,
    uint16_t * in_frame_data
)
{
    ISX_ASSERT(in_movie->data_type == int(isx::DataType::U16));
    return isx_movie_write_frame_internal(in_movie, in_index, in_frame_data);
}

int
isx_movie_write_frame_u16_with_header_footer(
    IsxMovie *  in_movie,
    size_t in_index,
    uint16_t * in_buffer
)
{
    // TODO : Remove in_index from the interface.
    // Leaving in for now so that I do not break the hub.
    (void)in_index;
    ISX_ASSERT(in_movie->data_type == int(isx::DataType::U16));
    return isx_process_op([=]()
    {
        isx::SpWritableMovie_t movie = g_open_writable_movies[in_movie->id];
        movie->writeFrameWithHeaderFooter(in_buffer);
    });
}

int
isx_movie_write_frame_u16_with_header_footer_separately(
    IsxMovie *  in_movie,
    uint16_t * in_header,
    uint16_t * in_pixels,
    uint16_t * in_footer
)
{
    ISX_ASSERT(in_movie->data_type == int(isx::DataType::U16));
    return isx_process_op([=]()
    {
        isx::SpWritableMovie_t movie = g_open_writable_movies[in_movie->id];
        movie->writeFrameWithHeaderFooter(in_header, in_pixels, in_footer);
    });
}

int
isx_movie_write_frame_f32(
    IsxMovie * in_movie,
    size_t in_index,
    float * in_frame_data
)
{
    ISX_ASSERT(in_movie->data_type == int(isx::DataType::F32));
    return isx_movie_write_frame_internal(in_movie, in_index, in_frame_data);
}

int
isx_movie_set_extra_properties(
    IsxMovie * in_movie,
    const char * in_properties
)
{
    return isx_process_op([=]()
    {
        isx::SpWritableMovie_t movie = g_open_writable_movies[in_movie->id];
        movie->setExtraProperties(in_properties);
    });
}

int
isx_movie_get_extra_properties(
    IsxMovie * in_movie,
    char * out_extra_properties,
    const size_t in_extra_properties_size
)
{
    return isx_movie_get_extra_properties_internal(in_movie, out_extra_properties, in_extra_properties_size);
}

int
isx_movie_get_extra_properties_for_matlab(
    IsxMovie * in_movie,
    char ** out_extra_properties,
    const size_t in_extra_properties_size
)
{
    return isx_movie_get_extra_properties_internal(in_movie, *out_extra_properties, in_extra_properties_size);
}

int
isx_movie_get_extra_properties_size(
    IsxMovie * in_movie,
    size_t * out_extra_properties_size
)
{
    return isx_process_op([=]()
    {
        const isx::SpMovie_t movie = g_open_movies[in_movie->id];
        // +1 for the null terminator
        *out_extra_properties_size = movie->getExtraProperties().size() + 1;
    });
}

int
isx_movie_flush(
    IsxMovie * in_movie
)
{
    return isx_process_op([=]()
    {
        isx_movie_flush_internal(in_movie, nullptr);
    });
}

int
isx_movie_flush_with_timing_info(
    IsxMovie * in_movie,
    IsxTimingInfo in_timing
)
{
    return isx_process_op([=]()
    {
        const isx::TimingInfo ti = convert_timing_info_c_to_cpp(in_timing);
        isx_movie_flush_internal(in_movie, &ti);
    });
}

int
isx_movie_delete(
    IsxMovie * in_movie
)
{
    return isx_process_op([=]()
    {
        if (in_movie != nullptr)
        {
            isx_movie_flush_internal(in_movie, nullptr);
            if (in_movie->read_only)
            {
                g_open_movies.erase(in_movie->id);
            }
            else
            {
                g_open_writable_movies.erase(in_movie->id);
            }
            delete_timing(in_movie->timing);
            delete in_movie->file_path;
            delete in_movie;
        }
    });
}

ISX_DLL_EXPORT
int
isx_movie_get_acquisition_info(
    IsxMovie * in_movie,
    char * out_acquisition_info,
    const size_t in_acquisition_info_size
)
{
    return isx_process_op([=]()
    {
        isx_get_acquisition_info_internal(g_open_movies[in_movie->id], out_acquisition_info, in_acquisition_info_size);
    });
}

ISX_DLL_EXPORT
int
isx_movie_get_acquisition_info_for_matlab(
    IsxMovie * in_movie,
    char ** out_acquisition_info,
    const size_t in_acquisition_info_size
)
{
    return isx_process_op([=]()
    {
        isx_get_acquisition_info_internal(g_open_movies[in_movie->id], *out_acquisition_info, in_acquisition_info_size);
    });
}

ISX_DLL_EXPORT
int
isx_movie_get_acquisition_info_size(
    IsxMovie * in_movie,
    size_t * out_acquisition_info_size
)
{
    return isx_process_op([=]()
    {
        const std::string info_str = isx_get_acquisition_info_internal(g_open_movies[in_movie->id]);
        *out_acquisition_info_size = info_str.size() + 1;
    });
}

int
isx_movie_get_frame_timestamp(
    IsxMovie * in_movie,
    size_t in_index,
    uint64_t * out_timestamp
)
{
    return isx_process_op([=]()
    {
        const isx::SpMovie_t movie = g_open_movies[in_movie->id];
        if (!movie->hasFrameTimestamps())
        {
            ISX_THROW(isx::ExceptionUserInput, "No frame timestamps stored in movie.");
        }

        uint64_t timestamp = movie->getFrameTimestamp(in_index);
        *out_timestamp = timestamp;
    });
}

/////////////////////////////
// CellSet struct and methods
/////////////////////////////

int
isx_read_cell_set(
    const char * in_file_path,
    bool in_read_only,
    IsxCellSet ** out_cell_set
)
{
    const int ret_code = isx_process_op([=]()
    {
        isx_check_input_file_path(in_file_path, {isx::DataSet::Type::CELLSET});

        isx::SpCellSet_t cell_set = isx::readCellSet(in_file_path, !in_read_only);

        g_open_cell_sets[g_cell_set_id] = cell_set;

        *out_cell_set = new IsxCellSet;

        (*out_cell_set)->id = size_t(g_cell_set_id);
        (*out_cell_set)->read_only = in_read_only;
        (*out_cell_set)->roi_set = cell_set->isRoiSet();
        (*out_cell_set)->num_cells = size_t(cell_set->getNumCells());
        (*out_cell_set)->file_path = make_canonical_file_path(in_file_path);

        convert_timing_info_cpp_to_c(cell_set->getTimingInfo(), &((*out_cell_set)->timing));
        convert_spacing_info_cpp_to_c(cell_set->getSpacingInfo(), &((*out_cell_set)->spacing));

        g_cell_set_id++;
    });

    return isx_checked_delete_and_nullify(ret_code, *out_cell_set);
}

int
isx_cell_set_get_name(
    IsxCellSet * in_cell_set,
    size_t in_index,
    size_t in_cell_name_size,
    char * out_cell_name
)
{
    return isx_cell_set_get_name_internal(in_cell_set, in_index, in_cell_name_size, out_cell_name);
}

int
isx_cell_set_get_name_for_matlab(
    IsxCellSet * in_cell_set,
    size_t in_index,
    size_t in_cell_name_size,
    char ** out_cell_name
)
{
    return isx_cell_set_get_name_internal(in_cell_set, in_index, in_cell_name_size, *out_cell_name);
}

int
isx_cell_set_get_trace(
    IsxCellSet * in_cell_set,
    size_t in_index,
    float * out_trace
)
{
    return isx_process_op([=]()
    {
        const isx::SpCellSet_t cell_set = g_open_cell_sets[in_cell_set->id];
        isx_check_cell_index(in_index, cell_set->getNumCells());
        const isx::TimingInfo & ti = cell_set->getTimingInfo();
        const isx::SpFTrace_t trace = cell_set->getTrace(isx::isize_t(in_index));
        float * values = trace->getValues();
        for (size_t p = 0; p < ti.getNumTimes(); ++p)
        {
            out_trace[p] = values[p];
        }
    });
}

int
isx_cell_set_get_image(
    IsxCellSet * in_cell_set,
    size_t in_index,
    float * out_image
)
{
    return isx_process_op([=]()
    {
        const isx::SpCellSet_t cell_set = g_open_cell_sets[in_cell_set->id];
        isx_check_cell_index(in_index, cell_set->getNumCells());
        const isx::SpImage_t image = cell_set->getImage(isx::isize_t(in_index));
        const size_t num_pixels = image->getSpacingInfo().getTotalNumPixels();
        const float * pixels = image->getPixelsAsF32();
        for (size_t p = 0; p < num_pixels; ++p)
        {
            out_image[p] = pixels[p];
        }
    });
}

int
isx_write_cell_set(
    const char * in_file_path,
    IsxTimingInfo in_timing,
    IsxSpacingInfo in_spacing,
    bool in_roi_set,
    IsxCellSet ** out_cell_set
)
{
    const int ret_code = isx_process_op([=]()
    {
        isx_check_output_file_path(in_file_path, "isxd");

        const auto timing = convert_timing_info_c_to_cpp(in_timing);
        const auto spacing = convert_spacing_info_c_to_cpp(in_spacing);

        isx::SpCellSet_t cell_set = isx::writeCellSet(in_file_path, timing, spacing, in_roi_set);

        g_open_cell_sets[g_cell_set_id] = cell_set;

        *out_cell_set = new IsxCellSet;

        (*out_cell_set)->id = size_t(g_cell_set_id);
        (*out_cell_set)->roi_set = in_roi_set;
        (*out_cell_set)->read_only = false;
        (*out_cell_set)->num_cells = 0;
        (*out_cell_set)->file_path = make_canonical_file_path(in_file_path);
        // We convert back so that we get a deep copy in case the client deletes
        // things in the input structs.
        convert_timing_info_cpp_to_c(timing, &((*out_cell_set)->timing));
        convert_spacing_info_cpp_to_c(spacing, &((*out_cell_set)->spacing));

        g_cell_set_id++;
    });

    return ret_code;
}

int
isx_cell_set_flush(
    IsxCellSet * in_cell_set
)
{
    return isx_process_op([=]()
    {
        isx_cell_set_flush_internal(in_cell_set);
    });
}

int
isx_cell_set_delete(
    IsxCellSet * in_cell_set
)
{
    return isx_process_op([=]()
    {
        if (in_cell_set != nullptr)
        {
            isx_cell_set_flush_internal(in_cell_set);
            g_open_cell_sets.erase(in_cell_set->id);
            delete_timing(in_cell_set->timing);
            delete in_cell_set->file_path;
            delete in_cell_set;
        }
    });
}

int
isx_cell_set_get_status(
    IsxCellSet * in_cell_set,
    size_t in_index,
    int * out_status
)
{
    return isx_process_op([=]()
    {
        isx::SpCellSet_t cell_set = g_open_cell_sets[in_cell_set->id];
        isx_check_cell_index(in_index, cell_set->getNumCells());
        *out_status = int(cell_set->getCellStatus(isx::isize_t(in_index)));
    });
}

int
isx_cell_set_set_status(
    IsxCellSet * in_cell_set,
    size_t in_index,
    int in_status
)
{
    return isx_process_op([=]()
    {
        isx::SpCellSet_t cell_set = g_open_cell_sets[in_cell_set->id];
        isx_check_cell_index(in_index, cell_set->getNumCells());
        cell_set->setCellStatus(isx::isize_t(in_index), isx::CellSet::CellStatus(in_status));
    });
}

int
isx_cell_set_write_image_trace(
    IsxCellSet * in_cell_set,
    size_t in_index,
    float * in_image,
    float * in_trace,
    const char * in_name
)
{
    return isx_process_op([=]()
    {
        if (!in_cell_set->read_only)
        {
            isx::SpCellSet_t cell_set = g_open_cell_sets[in_cell_set->id];
            const auto si = cell_set->getSpacingInfo();
            const auto ti = cell_set->getTimingInfo();

            const auto image = std::make_shared<isx::Image>(si, si.getNumColumns()*sizeof(float), 1, isx::DataType::F32);
            std::memcpy(image->getPixelsAsF32(), in_image, image->getImageSizeInBytes());

            auto trace = std::make_shared<isx::FTrace_t>(ti);
            float * trace_data = trace->getValues();
            for (size_t i = 0; i < ti.getNumTimes(); i++)
            {
                if (ti.isIndexValid(i))
                {
                    trace_data[i] = in_trace[i];
                }
                else
                {
                    trace_data[i] = std::numeric_limits<float>::quiet_NaN();
                }
            }

            cell_set->writeImageAndTrace(isx::isize_t(in_index), image, trace, std::string(in_name));
        }
    });
}

ISX_DLL_EXPORT
int
isx_cell_set_get_acquisition_info(
    IsxCellSet * in_cell_set,
    char * out_acquisition_info,
    const size_t in_acquisition_info_size
)
{
    return isx_process_op([=]()
    {
        isx_get_acquisition_info_internal(g_open_cell_sets[in_cell_set->id], out_acquisition_info, in_acquisition_info_size);
    });
}

ISX_DLL_EXPORT
int
isx_cell_set_get_acquisition_info_for_matlab(
    IsxCellSet * in_cell_set,
    char ** out_acquisition_info,
    const size_t in_acquisition_info_size
)
{
    return isx_process_op([=]()
    {
        isx_get_acquisition_info_internal(g_open_cell_sets[in_cell_set->id], *out_acquisition_info, in_acquisition_info_size);
    });
}

ISX_DLL_EXPORT
int
isx_cell_set_get_acquisition_info_size(
    IsxCellSet * in_cell_set,
    size_t * out_acquisition_info_size
)
{
    return isx_process_op([=]()
    {
        const std::string info_str = isx_get_acquisition_info_internal(g_open_cell_sets[in_cell_set->id]);
        *out_acquisition_info_size = info_str.size() + 1;
    });
}

///////////////////////////
// Event struct and methods
///////////////////////////

int
isx_read_events(
    const char * in_file_path,
    IsxEvents ** out_events
)
{
    const int ret_code = isx_process_op([=]()
    {
        isx_check_input_file_path(in_file_path, {isx::DataSet::Type::EVENTS});

        isx::SpEvents_t events = isx::readEvents(in_file_path);

        g_open_events[g_events_id] = events;

        *out_events = new IsxEvents;

        (*out_events)->id = size_t(g_events_id);
        (*out_events)->num_cells = size_t(events->numberOfCells());
        (*out_events)->read_only = true;
        (*out_events)->file_path = make_canonical_file_path(in_file_path);

        convert_timing_info_cpp_to_c(events->getTimingInfo(), &((*out_events)->timing));

        g_events_id++;
    });

    return isx_checked_delete_and_nullify(ret_code, *out_events);
}

int
isx_write_events(
    const char * in_file_path,
    IsxTimingInfo in_timing,
    const char ** in_channels,
    const size_t in_num_channels,
    IsxEvents ** out_events
)
{
    const int ret_code = isx_process_op([=]()
    {
        isx_check_output_file_path(in_file_path);

        std::vector<std::string> channels;
        for (size_t c = 0; c < in_num_channels; ++c)
        {
            channels.push_back(in_channels[c]);
        }

        const isx::TimingInfo timing = convert_timing_info_c_to_cpp(in_timing);
        const std::vector<isx::DurationInSeconds> steps(in_num_channels, timing.getStep());

        isx::SpWritableEvents_t events = isx::writeEvents(in_file_path, channels, steps);
        events->setTimingInfo(timing);

        g_open_writable_events[g_writable_events_id] = events;

        *out_events = new IsxEvents;

        (*out_events)->id = size_t(g_writable_events_id);
        (*out_events)->read_only = false;
        (*out_events)->num_cells = in_num_channels;
        (*out_events)->file_path = make_canonical_file_path(in_file_path);

        // We convert back so that we get a deep copy in case the client deletes
        // things in the input struct.
        convert_timing_info_cpp_to_c(timing, &((*out_events)->timing));

        g_writable_events_id++;
    });

    return isx_checked_delete_and_nullify(ret_code, *out_events);
}

int
isx_events_get_cell_count(
    IsxEvents * in_events,
    const char * in_name,
    size_t * out_count
)
{
    return isx_process_op([=]()
    {
        *out_count = 0;
        if (in_events->read_only)
        {
            auto event = g_open_events[in_events->id];
            auto trace = event->getLogicalData(in_name);

            std::pair<isx::isize_t, std::string> trace_info(in_events->id, in_name);

            g_open_logical_traces[trace_info] = trace;

            *out_count = trace->getValues().size();
        }
    });
}

int
isx_events_get_cell(
    IsxEvents * in_events,
    const char * in_name,
    uint64_t * out_usecs_since_start,
    float * out_values
)
{
    return isx_process_op([=]()
    {
        if (in_events->read_only)
        {
            const isx::SpEvents_t events = g_open_events[in_events->id];
            const isx::Time start = events->getTimingInfo().getStart();

            std::pair<isx::isize_t, std::string> trace_info(in_events->id, in_name);
            auto trace = g_open_logical_traces[trace_info];

            std::map<isx::Time, float> values = trace->getValues();
            size_t index = 0;
            for (const auto & value : values)
            {
                const isx::Ratio secs_since_start = value.first - start;
                if (secs_since_start < 0)
                {
                    ISX_THROW(isx::ExceptionDataIO, "Found negative offset for event ", index);
                }
                out_usecs_since_start[index] = isx::DurationInSeconds(secs_since_start).toMicroseconds();
                out_values[index] = value.second;
                index++;
            }

            g_open_logical_traces.erase(trace_info);
        }
    });
}

int
isx_events_get_cell_name(
    IsxEvents * in_events,
    size_t in_index,
    size_t in_cell_name_size,
    char * out_cell_name
)
{
    return isx_events_get_cell_name_internal(in_events, in_index, in_cell_name_size, out_cell_name);
}

int
isx_events_get_cell_name_for_matlab(
    IsxEvents * in_events,
    size_t in_index,
    size_t in_cell_name_size,
    char ** out_cell_name
)
{
    return isx_events_get_cell_name_internal(in_events, in_index, in_cell_name_size, *out_cell_name);
}

int
isx_events_write_cell(
    IsxEvents * in_events,
    uint64_t in_index,
    size_t in_num_packets,
    uint64_t * in_usecs_since_start,
    float * in_values
)
{
    return isx_process_op([=]()
    {
        if (!in_events->read_only)
        {
            auto events = g_open_writable_events[in_events->id];
            for (size_t e = 0; e < in_num_packets; e++)
            {
                events->writeDataPkt(in_index, in_usecs_since_start[e], in_values[e]);
            }
        }
    });
}

int
isx_events_flush(
    IsxEvents * in_events
)
{
    return isx_process_op([=]()
    {
        isx_events_flush_internal(in_events);
    });
}

int
isx_events_delete(
    IsxEvents * in_events
)
{
    return isx_process_op([=]()
    {
        if (in_events != nullptr)
        {
            isx_events_flush_internal(in_events);
            if (in_events->read_only)
            {
                g_open_events.erase(in_events->id);
            }
            else
            {
                g_open_writable_events.erase(in_events->id);
            }
            delete_timing(in_events->timing);
            delete in_events->file_path;
            delete in_events;
        }
    });
}

ISX_DLL_EXPORT
int
isx_events_get_acquisition_info(
    IsxEvents * in_events,
    char * out_acquisition_info,
    const size_t in_acquisition_info_size
)
{
    return isx_process_op([=]()
    {
        isx_get_acquisition_info_internal(g_open_events[in_events->id], out_acquisition_info, in_acquisition_info_size);
    });
}

ISX_DLL_EXPORT
int
isx_events_get_acquisition_info_for_matlab(
    IsxEvents * in_events,
    char ** out_acquisition_info,
    const size_t in_acquisition_info_size
)
{
    return isx_process_op([=]()
    {
        isx_get_acquisition_info_internal(g_open_events[in_events->id], *out_acquisition_info, in_acquisition_info_size);
    });
}

ISX_DLL_EXPORT
int
isx_events_get_acquisition_info_size(
    IsxEvents * in_events,
    size_t * out_acquisition_info_size
)
{
    return isx_process_op([=]()
    {
        const std::string info_str = isx_get_acquisition_info_internal(g_open_events[in_events->id]);
        *out_acquisition_info_size = info_str.size() + 1;
    });
}

///////////////////////////
// GPIO struct and methods
///////////////////////////

int
isx_read_gpio(
    const char * in_file_path,
    IsxGpio ** out_gpio
)
{
    const int ret_code = isx_process_op([=]()
    {
        isx_check_input_file_path(in_file_path, {isx::DataSet::Type::GPIO, isx::DataSet::Type::IMU});
        isx::SpGpio_t gpio = isx::readGpio(in_file_path);

        g_open_gpios[g_gpio_id] = gpio;

        *out_gpio = new IsxGpio;

        (*out_gpio)->id = size_t(g_gpio_id);
        (*out_gpio)->num_channels = size_t(gpio->numberOfChannels());
        (*out_gpio)->read_only = true;
        (*out_gpio)->file_path = make_canonical_file_path(in_file_path);

        convert_timing_info_cpp_to_c(gpio->getTimingInfo(), &((*out_gpio)->timing));

        g_gpio_id++;
    });

    return isx_checked_delete_and_nullify(ret_code, *out_gpio);
}

int
isx_gpio_get_channel_count(
    IsxGpio * in_gpio,
    const char * in_name,
    size_t * out_count
)
{
    return isx_process_op([=]()
    {
        *out_count = 0;
        if (in_gpio->read_only)
        {
            auto gpio = g_open_gpios[in_gpio->id];
            auto trace = gpio->getLogicalData(in_name);

            std::pair<isx::isize_t, std::string> trace_info(in_gpio->id, in_name);

            g_open_logical_traces[trace_info] = trace;

            *out_count = trace->getValues().size();
        }
    });
}

int
isx_gpio_get_channel(
    IsxGpio * in_gpio,
    const char * in_name,
    uint64_t * out_usecs_since_start,
    float * out_values
)
{
    return isx_process_op([=]()
    {
        if (in_gpio->read_only)
        {
            const isx::SpGpio_t gpio = g_open_gpios[in_gpio->id];
            const isx::Time start = gpio->getTimingInfo().getStart();

            std::pair<isx::isize_t, std::string> trace_info(in_gpio->id, in_name);
            auto trace = g_open_logical_traces[trace_info];

            std::map<isx::Time, float> values = trace->getValues();
            size_t index = 0;
            for (const auto & value : values)
            {
                const isx::Ratio secs_since_start = value.first - start;
                if (secs_since_start < 0)
                {
                    ISX_THROW(isx::ExceptionDataIO, "Found negative offset for event ", index);
                }
                out_usecs_since_start[index] = isx::DurationInSeconds(secs_since_start).toMicroseconds();
                out_values[index] = value.second;
                index++;
            }

            g_open_logical_traces.erase(trace_info);
        }
    });
}

int
isx_gpio_get_channel_name(
    IsxGpio * in_gpio,
    size_t in_index,
    size_t in_channel_name_size,
    char * out_channel_name
)
{
    return isx_process_op([=]()
    {
        const isx::SpGpio_t gpio = g_open_gpios[in_gpio->id];
        isx_check_channel_index(in_index, gpio->numberOfChannels());
        const std::string channel_name = gpio->getChannelList().at(in_index);
        isx::copyCppStringToCString(channel_name, out_channel_name, in_channel_name_size);
    });
}

int
isx_gpio_delete(
    IsxGpio * in_gpio
)
{
    return isx_process_op([=]()
    {
        if (in_gpio != nullptr)
        {
            g_open_gpios.erase(in_gpio->id);
            delete_timing(in_gpio->timing);
            delete in_gpio->file_path;
            delete in_gpio;
        }
    });
}

ISX_DLL_EXPORT
int
isx_gpio_get_acquisition_info(
    IsxGpio * in_gpio,
    char * out_acquisition_info,
    const size_t in_acquisition_info_size
)
{
    return isx_process_op([=]()
    {
        isx_get_acquisition_info_internal(g_open_gpios[in_gpio->id], out_acquisition_info, in_acquisition_info_size);
    });
}

ISX_DLL_EXPORT
int
isx_gpio_get_acquisition_info_size(
    IsxGpio * in_gpio,
    size_t * out_acquisition_info_size
)
{
    return isx_process_op([=]()
    {
        const std::string info_str = isx_get_acquisition_info_internal(g_open_gpios[in_gpio->id]);
        *out_acquisition_info_size = info_str.size() + 1;
    });
}

/////////////////////////////
// VesselSet struct and methods
/////////////////////////////

int
isx_read_vessel_set(
    const char * in_file_path,
    bool in_read_only,
    IsxVesselSet ** out_vessel_set
)
{
    const int ret_code = isx_process_op([=]()
    {
        isx_check_input_file_path(in_file_path, {isx::DataSet::Type::VESSELSET});

        isx::SpVesselSet_t vessel_set = isx::readVesselSet(in_file_path, !in_read_only);

        g_open_vessel_sets[g_vessel_set_id] = vessel_set;

        *out_vessel_set = new IsxVesselSet;

        (*out_vessel_set)->id = size_t(g_vessel_set_id);
        (*out_vessel_set)->read_only = in_read_only;
        (*out_vessel_set)->num_vessels = size_t(vessel_set->getNumVessels());
        (*out_vessel_set)->file_path = make_canonical_file_path(in_file_path);

        convert_timing_info_cpp_to_c(vessel_set->getTimingInfo(), &((*out_vessel_set)->timing));
        convert_spacing_info_cpp_to_c(vessel_set->getSpacingInfo(), &((*out_vessel_set)->spacing));

        g_vessel_set_id++;
    });

    return isx_checked_delete_and_nullify(ret_code, *out_vessel_set);
}

int
isx_vessel_set_get_name(
    IsxVesselSet * in_vessel_set,
    size_t in_index,
    size_t in_vessel_name_size,
    char * out_vessel_name
)
{
    return isx_vessel_set_get_name_internal(in_vessel_set, in_index, in_vessel_name_size, out_vessel_name);
}

int
isx_vessel_set_get_name_for_matlab(
    IsxVesselSet * in_vessel_set,
    size_t in_index,
    size_t in_vessel_name_size,
    char ** out_vessel_name
)
{
    return isx_vessel_set_get_name_internal(in_vessel_set, in_index, in_vessel_name_size, *out_vessel_name);
}

int
isx_vessel_set_get_trace(
    IsxVesselSet * in_vessel_set,
    size_t in_index,
    float * out_trace
)
{
    return isx_process_op([=]()
    {
        const isx::SpVesselSet_t vessel_set = g_open_vessel_sets[in_vessel_set->id];
        isx_check_vessel_index(in_index, vessel_set->getNumVessels());
        const isx::TimingInfo & ti = vessel_set->getTimingInfo();
        const isx::SpFTrace_t trace = vessel_set->getTrace(isx::isize_t(in_index));
        float * values = trace->getValues();
        for (size_t p = 0; p < ti.getNumTimes(); ++p)
        {
            out_trace[p] = values[p];
        }
    });
}

int
isx_vessel_set_get_image(
    IsxVesselSet * in_vessel_set,
    size_t in_index,
    float * out_image
)
{
    return isx_process_op([=]()
    {
        const isx::SpVesselSet_t vessel_set = g_open_vessel_sets[in_vessel_set->id];
        isx_check_vessel_index(in_index, vessel_set->getNumVessels());
        const isx::SpImage_t image = vessel_set->getImage(isx::isize_t(in_index));
        const size_t num_pixels = image->getSpacingInfo().getTotalNumPixels();
        const float * pixels = image->getPixelsAsF32();
        for (size_t p = 0; p < num_pixels; ++p)
        {
            out_image[p] = pixels[p];
        }
    });
}

int
isx_vessel_set_get_line_endpoints(
    IsxVesselSet * in_vessel_set,
    size_t in_index,
    int64_t * out_line
)
{
    return isx_process_op([=]()
    {
        const isx::SpVesselSet_t vessel_set = g_open_vessel_sets[in_vessel_set->id];
        isx_check_vessel_index(in_index, vessel_set->getNumVessels());
        const isx::SpImage_t image = vessel_set->getImage(isx::isize_t(in_index));
        const isx::SpVesselLine_t line_endpoints = vessel_set->getLineEndpoints(isx::isize_t(in_index));
        size_t numPoints = vessel_set->getVesselSetType() == isx::VesselSetType_t::VESSEL_DIAMETER ? 2 : 4;

        // The reason that the X and Y points are filled in in such a weird order is that
        //      numpy converts 1D arrays to 2D arrays by simply cutting the values.
        //      For example, [1, 2, 3, 4] gets turned into [[1, 2], [3, 4]]. Thus, in order
        //      to reserve the X and Y points, we must place it like [x1, y1, x2, y2].

        for (size_t i = 0; i < numPoints; ++i)
        {
            out_line[2 * i] = line_endpoints->m_contour[i].getX();
            out_line[2 * i + 1] = line_endpoints->m_contour[i].getY();
        }
    });
}

int
isx_vessel_set_get_type(
    IsxVesselSet * in_vessel_set,
    int * type
)
{
    return isx_process_op([=]()
    {
        isx::SpVesselSet_t vessel_set = g_open_vessel_sets[in_vessel_set->id];
        *type = int(vessel_set->getVesselSetType());
    });
}

int
isx_vessel_set_get_center_trace(
    IsxVesselSet * in_vessel_set,
    size_t in_index,
    float * out_trace
)
{
    return isx_process_op([=]()
    {
        const isx::SpVesselSet_t vessel_set = g_open_vessel_sets[in_vessel_set->id];
        isx_check_vessel_index(in_index, vessel_set->getNumVessels());
        const isx::TimingInfo & ti = vessel_set->getTimingInfo();
        const isx::SpFTrace_t trace = vessel_set->getCenterTrace(isx::isize_t(in_index));
        if (!trace)
        {
            ISX_THROW(isx::ExceptionUserInput, "No center point traces saved to vessel set file.");
        }
        float * values = trace->getValues();
        for (size_t p = 0; p < ti.getNumTimes(); ++p)
        {
            out_trace[p] = values[p];
        }
    });
}

int
isx_vessel_set_get_direction_trace(
    IsxVesselSet * in_vessel_set,
    size_t in_index,
    float * out_trace
)
{
    return isx_process_op([=]()
    {
        const isx::SpVesselSet_t vessel_set = g_open_vessel_sets[in_vessel_set->id];
        isx_check_vessel_index(in_index, vessel_set->getNumVessels());
        const isx::TimingInfo & ti = vessel_set->getTimingInfo();
        const isx::SpFTrace_t trace = vessel_set->getDirectionTrace(isx::isize_t(in_index));
        if (!trace)
        {
            ISX_THROW(isx::ExceptionUserInput, "No direction traces saved to vessel set file.");
        }
        float * values = trace->getValues();
        for (size_t p = 0; p < ti.getNumTimes(); ++p)
        {
            out_trace[p] = values[p];
        }
    });
}

int
isx_vessel_set_is_correlation_saved(
    IsxVesselSet * in_vessel_set,
    int * out_saved
)
{
    return isx_process_op([=]()
    {
        isx::SpVesselSet_t vessel_set = g_open_vessel_sets[in_vessel_set->id];
        *out_saved = static_cast<int>(vessel_set->isCorrelationSaved());
    });
}

int
isx_vessel_set_get_correlation_size(
    IsxVesselSet * in_vessel_set,
    size_t in_index,
    size_t * bbox_size
)
{
    return isx_process_op([=]()
    {
        const isx::SpVesselSet_t vessel_set = g_open_vessel_sets[in_vessel_set->id];
        isx_check_vessel_index(in_index, vessel_set->getNumVessels());
        if (!vessel_set->isCorrelationSaved())
        {
            ISX_THROW(isx::ExceptionUserInput, "No correlation heatmaps saved to vessel set file.");
        }

        isx::SizeInPixels_t size = vessel_set->getCorrelationSize(in_index);
        bbox_size[0] = size.getHeight();
        bbox_size[1] = size.getWidth();
    });
}

int
isx_vessel_set_get_correlations(
    IsxVesselSet * in_vessel_set,
    size_t in_index,
    size_t in_frame_number,
    float * out_correlations
)
{
    return isx_process_op([=]()
    {
        const isx::SpVesselSet_t vessel_set = g_open_vessel_sets[in_vessel_set->id];
        isx_check_vessel_index(in_index, vessel_set->getNumVessels());
        if (!vessel_set->isCorrelationSaved())
        {
            ISX_THROW(isx::ExceptionUserInput, "No correlation heatmaps saved to vessel set file.");
        }

        isx::SpVesselCorrelations_t triptych = vessel_set->getCorrelations(in_index, in_frame_number);
        const size_t num_pixels = triptych->getTotalNumPixels() * 3;
        const float * pixels = triptych->getValues();
        for (size_t p = 0; p < num_pixels; ++p)
        {
            out_correlations[p] = pixels[p];
        }
    });
}

int
isx_write_vessel_set(
    const char * in_file_path,
    IsxTimingInfo in_timing,
    IsxSpacingInfo in_spacing,
    const int in_vessel_set_type,
    IsxVesselSet ** out_vessel_set
)
{
    const int ret_code = isx_process_op([=]()
    {
        isx_check_output_file_path(in_file_path, "isxd");

        const auto timing = convert_timing_info_c_to_cpp(in_timing);
        const auto spacing = convert_spacing_info_c_to_cpp(in_spacing);
        const auto type = static_cast<isx::VesselSetType_t>(in_vessel_set_type);

        isx::SpVesselSet_t vessel_set = isx::writeVesselSet(in_file_path, timing, spacing, type);

        g_open_vessel_sets[g_vessel_set_id] = vessel_set;

        *out_vessel_set = new IsxVesselSet;

        (*out_vessel_set)->id = size_t(g_vessel_set_id);
        (*out_vessel_set)->read_only = false;
        (*out_vessel_set)->num_vessels = 0;
        (*out_vessel_set)->file_path = make_canonical_file_path(in_file_path);
        // We convert back so that we get a deep copy in case the client deletes
        // things in the input structs.
        convert_timing_info_cpp_to_c(timing, &((*out_vessel_set)->timing));
        convert_spacing_info_cpp_to_c(spacing, &((*out_vessel_set)->spacing));

        g_vessel_set_id++;
    });

    return ret_code;
}

int
isx_vessel_set_flush(
    IsxVesselSet * in_vessel_set
)
{
    return isx_process_op([=]()
    {
        isx_vessel_set_flush_internal(in_vessel_set);
    });
}

int
isx_vessel_set_delete(
    IsxVesselSet * in_vessel_set
)
{
    return isx_process_op([=]()
    {
        if (in_vessel_set != nullptr)
        {
            isx_vessel_set_flush_internal(in_vessel_set);
            g_open_vessel_sets.erase(in_vessel_set->id);
            delete_timing(in_vessel_set->timing);
            delete in_vessel_set->file_path;
            delete in_vessel_set;
        }
    });
}

int
isx_vessel_set_get_status(
    IsxVesselSet * in_vessel_set,
    size_t in_index,
    int * out_status
)
{
    return isx_process_op([=]()
    {
        isx::SpVesselSet_t vessel_set = g_open_vessel_sets[in_vessel_set->id];
        isx_check_vessel_index(in_index, vessel_set->getNumVessels());
        *out_status = int(vessel_set->getVesselStatus(isx::isize_t(in_index)));
    });
}

int
isx_vessel_set_set_status(
    IsxVesselSet * in_vessel_set,
    size_t in_index,
    int in_status
)
{
    return isx_process_op([=]()
    {
        isx::SpVesselSet_t vessel_set = g_open_vessel_sets[in_vessel_set->id];
        isx_check_vessel_index(in_index, vessel_set->getNumVessels());
        vessel_set->setVesselStatus(isx::isize_t(in_index), isx::VesselSet::VesselStatus(in_status));
    });
}

int
isx_vessel_set_write_vessel_diameter_data(
    IsxVesselSet * in_vessel_set,
    size_t in_index, 
    float * in_image,
    int64_t * in_line,
    float * in_trace,
    float * in_center_trace,
    const char * in_name
)
{
    return isx_process_op([=]()
    {
        if (!in_vessel_set->read_only)
        {
            isx::SpVesselSet_t vessel_set = g_open_vessel_sets[in_vessel_set->id];
            const auto si = vessel_set->getSpacingInfo();
            const auto ti = vessel_set->getTimingInfo();

            // Assumption: There are only two points.
            isx::PointInPixels_t m_p1 = isx::PointInPixels_t(in_line[0], in_line[1]);
            isx::PointInPixels_t m_p2 = isx::PointInPixels_t(in_line[2], in_line[3]);
            isx::SpVesselLine_t line = std::make_shared<isx::VesselLine>(isx::VesselLine({m_p1, m_p2}));

            const auto image = std::make_shared<isx::Image>(si, si.getNumColumns()*sizeof(float), 1, isx::DataType::F32);
            std::memcpy(image->getPixelsAsF32(), in_image, image->getImageSizeInBytes());

            auto trace = std::make_shared<isx::FTrace_t>(ti);
            float * trace_data = trace->getValues();

            auto center_trace = std::make_shared<isx::FTrace_t>(ti);
            float * center_trace_data = center_trace->getValues();
            for (size_t i = 0; i < ti.getNumTimes(); i++)
            {
                if (ti.isIndexValid(i))
                {
                    trace_data[i] = in_trace[i];
                    center_trace_data[i] = in_center_trace[i];
                }
                else
                {
                    trace_data[i] = std::numeric_limits<float>::quiet_NaN();
                    center_trace_data[i] = std::numeric_limits<float>::quiet_NaN();
                }
            }

            if (in_index == 0)
            {
                vessel_set->writeImage(image);
            }
            vessel_set->writeVesselDiameterData(isx::isize_t(in_index), line, trace, center_trace, std::string(in_name));
        }
    });
}

int
isx_vessel_set_write_rbc_velocity_data(
    IsxVesselSet * in_vessel_set,
    size_t in_index, 
    float * in_image,
    int64_t * in_line,
    float * in_trace,
    float * in_direction_trace,
    size_t in_correlation_width,
    size_t in_correlation_height,
    float * in_correlations_trace,  
    const char * in_name
)
{
    return isx_process_op([=]()
    {
        if (!in_vessel_set->read_only)
        {
            isx::SpVesselSet_t vessel_set = g_open_vessel_sets[in_vessel_set->id];
            const auto si = vessel_set->getSpacingInfo();
            const auto ti = vessel_set->getTimingInfo();

            // Assumption: There are only four points.
            isx::SpVesselLine_t line = std::make_shared<isx::VesselLine>(isx::VesselLine());
            for (size_t i = 0; i < 4; i++)
            {
                size_t offset = 2 * i;
                line->m_contour.push_back(isx::PointInPixels_t(in_line[offset], in_line[offset + 1]));
            }

            const auto image = std::make_shared<isx::Image>(si, si.getNumColumns()*sizeof(float), 1, isx::DataType::F32);
            std::memcpy(image->getPixelsAsF32(), in_image, image->getImageSizeInBytes());

            auto trace = std::make_shared<isx::FTrace_t>(ti);
            float * trace_data = trace->getValues();

            auto direction_trace = std::make_shared<isx::FTrace_t>(ti);
            float * direction_trace_data = direction_trace->getValues();

            auto corr_trace = in_correlations_trace ? std::make_shared<isx::VesselCorrelationsTrace>(
                ti, isx::SizeInPixels_t(in_correlation_width, in_correlation_height)
            ) : nullptr;

            for (size_t i = 0; i < ti.getNumTimes(); i++)
            {
                if (ti.isIndexValid(i))
                {
                    trace_data[i] = in_trace[i];
                    direction_trace_data[i] = in_direction_trace[i];
                }
                else
                {
                    trace_data[i] = std::numeric_limits<float>::quiet_NaN();
                    direction_trace_data[i] = std::numeric_limits<float>::quiet_NaN();
                }

                if (corr_trace)
                {
                    const size_t numPixels = in_correlation_width * in_correlation_height * 3;
                    size_t offset = numPixels * i;
                    auto correlations = corr_trace->getValue(i);
                    float * correlations_data = correlations->getValues();
                    for (size_t p = 0; p < numPixels; p++)
                    {
                        correlations_data[p] = in_correlations_trace[offset];
                        offset++;
                    }
                }
            }

            if (in_index == 0)
            {
                vessel_set->writeImage(image);
            }
            vessel_set->writeVesselVelocityData(isx::isize_t(in_index), line, trace, direction_trace, corr_trace, std::string(in_name));
        }
    });
}

ISX_DLL_EXPORT
int
isx_vessel_set_get_acquisition_info(
    IsxVesselSet * in_vessel_set,
    char * out_acquisition_info,
    const size_t in_acquisition_info_size
)
{
    return isx_process_op([=]()
    {
        isx_get_acquisition_info_internal(g_open_vessel_sets[in_vessel_set->id], out_acquisition_info, in_acquisition_info_size);
    });
}

ISX_DLL_EXPORT
int
isx_vessel_set_get_acquisition_info_for_matlab(
    IsxVesselSet * in_vessel_set,
    char ** out_acquisition_info,
    const size_t in_acquisition_info_size
)
{
    return isx_process_op([=]()
    {
        isx_get_acquisition_info_internal(g_open_vessel_sets[in_vessel_set->id], *out_acquisition_info, in_acquisition_info_size);
    });
}

ISX_DLL_EXPORT
int
isx_vessel_set_get_acquisition_info_size(
    IsxVesselSet * in_vessel_set,
    size_t * out_acquisition_info_size
)
{
    return isx_process_op([=]()
    {
        const std::string info_str = isx_get_acquisition_info_internal(g_open_vessel_sets[in_vessel_set->id]);
        *out_acquisition_info_size = info_str.size() + 1;
    });
}

////////////
// Functions
////////////

int
isx_initialize()
{
    return isx_process_op([=]()
    {
        if (!g_core_app)
        {
            g_core_app_args.reset(new std::pair<int, std::string>(1, "g_core_app"));
            char * testing = &g_core_app_args->second[0];
            g_core_app.reset(new QCoreApplication(g_core_app_args->first, &testing));

            isx::CoreInitialize();
        }
    });
}

int
isx_shutdown()
{
    return isx_process_op([=]()
    {
        if (g_core_app)
        {
            isx::CoreShutdown();
            g_core_app.reset();
        }
    });
}

int
isx_get_data_type_u16()
{
    return int(isx::DataType::U16);
}

int
isx_get_data_type_f32()
{
    return int(isx::DataType::F32);
}

int
isx_get_data_type_u8()
{
    return int(isx::DataType::U8);
}

int
isx_get_cell_status_accepted()
{
    return int(isx::CellSet::CellStatus::ACCEPTED);
}

int
isx_get_cell_status_undecided()
{
    return int(isx::CellSet::CellStatus::UNDECIDED);
}

int
isx_get_cell_status_rejected()
{
    return int(isx::CellSet::CellStatus::REJECTED);
}

int
isx_get_vessel_status_accepted()
{
    return int(isx::VesselSet::VesselStatus::ACCEPTED);
}

int
isx_get_vessel_status_undecided()
{
    return int(isx::VesselSet::VesselStatus::UNDECIDED);
}

int
isx_get_vessel_status_rejected()
{
    return int(isx::VesselSet::VesselStatus::REJECTED);
}

int
isx_get_vessel_type_vessel_diameter()
{
    return int(isx::VesselSetType_t::VESSEL_DIAMETER);
}

int
isx_get_vessel_type_rbc_velocity()
{
    return int(isx::VesselSetType_t::RBC_VELOCITY);
}

int
isx_get_core_version_major()
{
    return isx::CoreVersionMajor();
}

int
isx_get_core_version_minor()
{
    return isx::CoreVersionMinor();
}

int
isx_get_core_version_patch()
{
    return isx::CoreVersionPatch();
}

int
isx_get_core_version_build()
{
    return isx::CoreVersionBuild();
}

int
isx_get_is_with_algos()
{
    return isx::isWithAlgos();
}

const char *
isx_get_last_exception_string()
{
    return isx_get_last_exception_string_internal().c_str();
}

int
isx_export_movie_nwb(
    size_t in_num_input_files,
    const char ** in_input_file_paths,
    const char * in_output_file_path,
    const char * in_identifier,
    const char * in_session_description,
    const char * in_comments,
    const char * in_description,
    const char * in_experiment_description,
    const char * in_experimenter,
    const char * in_institution,
    const char * in_lab,
    const char * in_session_id
)
{
    return isx_process_async_op([=]()
    {
        isx_check_output_file_path(in_output_file_path);

        std::vector<isx::SpMovie_t> input_movies;
        for (int64_t i = 0; i < int64_t(in_num_input_files); ++i)
        {
            isx_check_input_file_path(in_input_file_paths[i], {isx::DataSet::Type::MOVIE});
            input_movies.push_back(isx::readMovie(in_input_file_paths[i]));
        }

        isx::MovieExporterParamsWrapper params = isx::makeMovieExporterParamsWrapper(isx::MovieExporterParams::Type::NWB);
        params.setOutputFileName(in_output_file_path);
        params.setSources(input_movies);
        params.setAdditionalInfo(
            std::string(in_identifier),
            std::string(in_session_description),
            std::string(in_comments),
            std::string(in_description),
            std::string(in_experiment_description),
            std::string(in_experimenter),
            std::string(in_institution),
            std::string(in_lab),
            std::string(in_session_id)
        );

        auto output_params = std::make_shared<isx::MovieExporterOutputParams>();
        auto res = isx::runMovieExport(params, output_params, [](float) {return false; });
        return res;
    });
}

int
isx_export_movie_tiff(
    size_t in_num_input_files,
    const char ** in_input_file_paths,
    const char * in_output_file_path,
    bool in_write_invalid_frames
)
{
    return isx_process_async_op([=]()
    {
        isx_check_output_file_path(in_output_file_path);

        std::vector<isx::SpMovie_t> input_movies;
        for (int64_t i = 0; i < int64_t(in_num_input_files); ++i)
        {
            isx_check_input_file_path(in_input_file_paths[i], {isx::DataSet::Type::MOVIE});
            input_movies.push_back(isx::readMovie(in_input_file_paths[i]));
        }

        isx::MovieExporterParamsWrapper params = isx::makeMovieExporterParamsWrapper(isx::MovieExporterParams::Type::TIFF);
        params.setOutputFileName(in_output_file_path);
        params.setSources(input_movies);
        params.setWriteDroppedAndCroppedParameter(bool(in_write_invalid_frames));

        auto output_params = std::make_shared<isx::MovieExporterOutputParams>();
        auto res = isx::runMovieExport(params, output_params, [](float) {return false; });
        return res;
    });
}

int
isx_export_movie_mp4(
    size_t in_num_input_files,
    const char ** in_input_file_paths,
    const char * in_output_file_path,
    double in_compression_quality,
    bool in_write_invalid_frames,
    int in_frame_rate_format,
    bool in_draw_bounding_box,
    bool in_draw_bounding_box_center,
    bool in_draw_zones
)
{
    return isx_process_async_op([=]()
    {
        isx_check_output_file_path(in_output_file_path);

        std::vector<isx::SpMovie_t> input_movies;
        for (int64_t i = 0; i < int64_t(in_num_input_files); ++i)
        {
            isx_check_input_file_path(in_input_file_paths[i], {isx::DataSet::Type::MOVIE, isx::DataSet::Type::NVISION_MOVIE});
            input_movies.push_back(isx::readMovie(in_input_file_paths[i]));
        }

        isx::MovieExporterParamsWrapper params = isx::makeMovieExporterParamsWrapper(isx::MovieExporterParams::Type::MP4);
        params.setOutputFileName(in_output_file_path);
        params.setSources(input_movies);
        params.setWriteDroppedAndCroppedParameter(bool(in_write_invalid_frames));
        params.setBitRateFraction(in_compression_quality);
        params.setFrameRateFormat(isx::MovieExporterParams::FrameRateFormat(in_frame_rate_format));


        const isx::DataSet::Type type = isx::readDataSetType(in_input_file_paths[0], {});
        if (type == isx::DataSet::Type::NVISION_MOVIE)
        {
            const auto trackingParams = std::shared_ptr<isx::NVisionMovieTrackingExporterParams>(
                new isx::NVisionMovieTrackingExporterParams()
            );
            trackingParams->m_drawBoundingBox = in_draw_bounding_box;
            trackingParams->m_drawBoundingBoxCenter = in_draw_bounding_box_center;
            trackingParams->m_drawZones = in_draw_zones;
            params.m_trackingParams = trackingParams;
        }

        auto output_params = std::make_shared<isx::MovieExporterOutputParams>();
        auto res = isx::runMovieExport(params, output_params, [](float) {return false; });
        return res;
    });
}

int
isx_export_movie_timestamps_to_csv(
    size_t in_num_input_files,
    const char ** in_input_file_paths,
    const char * in_output_file_path,
    int in_format
)
{
    return isx_process_async_op([=]()
    {
        isx_check_output_file_path(in_output_file_path);

        std::vector<isx::SpMovie_t> input_movies;
        for (int64_t i = 0; i < int64_t(in_num_input_files); ++i)
        {
            isx_check_input_file_path(in_input_file_paths[i], {isx::DataSet::Type::MOVIE, isx::DataSet::Type::NVISION_MOVIE});
            input_movies.push_back(isx::readMovie(in_input_file_paths[i]));
        }

        // sort movies by start time
        std::sort(input_movies.begin(), input_movies.end(), [](isx::SpMovie_t a, isx::SpMovie_t b)
            {
                return a->getTimingInfo().getStart() < b->getTimingInfo().getStart();
            });
        // movies are sorted by start time now, check if they meet requirements for series
        std::string errorMessage;
        for (size_t i = 1; i < input_movies.size(); ++i)
        {
            if (!isx::checkNewMemberOfSeries({input_movies[i - 1]}, input_movies[i], errorMessage))
            {
                ISX_THROW(isx::ExceptionSeries, errorMessage);
            }
        }

        const auto format = isx::WriteTimeRelativeTo(in_format);
        isx::MovieTimestampExporterParams params(in_output_file_path, format);
        params.setSources(input_movies);

        auto res = isx::runMovieTimestampExport(params, [](float) {return false; });
        return res;
    });
}

int
isx_export_nvision_movie_tracking_frame_data_to_csv(
    size_t in_num_input_files,
    const char ** in_input_file_paths,
    const char * in_output_file_path,
    int in_format
)
{
    return isx_process_async_op([=]()
    {
        isx_check_output_file_path(in_output_file_path);

        std::vector<isx::SpMovie_t> input_movies;
        for (int64_t i = 0; i < int64_t(in_num_input_files); ++i)
        {
            isx_check_input_file_path(in_input_file_paths[i], {isx::DataSet::Type::NVISION_MOVIE});
            input_movies.push_back(isx::readMovie(in_input_file_paths[i]));
        }

        // sort movies by start time
        std::sort(input_movies.begin(), input_movies.end(), [](isx::SpMovie_t a, isx::SpMovie_t b)
            {
                return a->getTimingInfo().getStart() < b->getTimingInfo().getStart();
            });
        // movies are sorted by start time now, check if they meet requirements for series
        std::string errorMessage;
        for (size_t i = 1; i < input_movies.size(); ++i)
        {
            if (!isx::checkNewMemberOfSeries({input_movies[i - 1]}, input_movies[i], errorMessage))
            {
                ISX_THROW(isx::ExceptionSeries, errorMessage);
            }
        }

        const auto format = isx::WriteTimeRelativeTo(in_format);
        isx::NVisionMovieTrackingExporterParams params;
        params.m_srcs = input_movies;
        params.m_frameTrackingDataOutputFilename = in_output_file_path;
        params.m_writeTimeRelativeTo = format;

        auto res = isx::runNVisionTrackingExporter(params, [](float) {return false; });
        return res;
    });
}

int
isx_export_nvision_movie_tracking_zone_data_to_csv(
    size_t in_num_input_files,
    const char ** in_input_file_paths,
    const char * in_output_file_path
)
{
    return isx_process_async_op([=]()
    {
        isx_check_output_file_path(in_output_file_path);

        std::vector<isx::SpMovie_t> input_movies;
        for (int64_t i = 0; i < int64_t(in_num_input_files); ++i)
        {
            isx_check_input_file_path(in_input_file_paths[i], {isx::DataSet::Type::NVISION_MOVIE});
            input_movies.push_back(isx::readMovie(in_input_file_paths[i]));
        }

        // sort movies by start time
        std::sort(input_movies.begin(), input_movies.end(), [](isx::SpMovie_t a, isx::SpMovie_t b)
            {
                return a->getTimingInfo().getStart() < b->getTimingInfo().getStart();
            });
        // movies are sorted by start time now, check if they meet requirements for series
        std::string errorMessage;
        for (size_t i = 1; i < input_movies.size(); ++i)
        {
            if (!isx::checkNewMemberOfSeries({input_movies[i - 1]}, input_movies[i], errorMessage))
            {
                ISX_THROW(isx::ExceptionSeries, errorMessage);
            }
        }

        isx::NVisionMovieTrackingExporterParams params;
        params.m_srcs = input_movies;
        params.m_zonesOutputFilename = in_output_file_path;

        auto res = isx::runNVisionTrackingExporter(params, [](float) {return false; });
        return res;
    });
}

int
isx_get_time_reference_start()
{
    return int(isx::WriteTimeRelativeTo::FIRST_DATA_ITEM);
}

int
isx_get_time_reference_unix()
{
    return int(isx::WriteTimeRelativeTo::UNIX_EPOCH);
}

int
isx_get_time_reference_tsc()
{
    return int(isx::WriteTimeRelativeTo::TSC);
}

int
isx_export_event_set(
    const size_t in_num_input_files,
    const char ** in_input_file_paths,
    const char * in_output_file_path,
    const int in_write_time_relative_to,
    const char * in_props_file_path,
    const bool in_sparse_output,
    const bool in_write_amplitude
)
{
    return isx_process_async_op([=]()
    {
        isx_check_output_file_path(in_output_file_path);

        std::vector<isx::SpEvents_t> input_event_sets;
        for (size_t i = 0; i < in_num_input_files; ++i)
        {
            isx_check_input_file_path(in_input_file_paths[i], {isx::DataSet::Type::EVENTS});
            input_event_sets.push_back(isx::readEvents(in_input_file_paths[i]));
        }

        const std::string props_file_path(in_props_file_path);
        if (!props_file_path.empty())
        {
            isx_check_output_file_path(props_file_path.c_str());
        }

        isx::EventsExporterParams params(
            input_event_sets,
            std::string(in_output_file_path),
            isx::WriteTimeRelativeTo(in_write_time_relative_to),
            props_file_path,
            false,
            in_sparse_output,
            in_write_amplitude);

        auto output_params = std::make_shared<isx::EventsExporterOutputParams>();
        return isx::runEventsExporter(params, output_params, [](float) {return false;});
    });
}

int
isx_export_gpio_set(
    const size_t in_num_input_files,
    const char ** in_input_file_paths,
    const char * in_output_file_path,
    const char * in_inter_isxd_file_dir,
    const int in_write_time_relative_to
)
{
    return isx_process_async_op([=]()
    {
        isx_check_output_file_path(in_output_file_path);

        std::vector<isx::SpGpio_t> input_gpio_sets;
        for (size_t i = 0; i < in_num_input_files; ++i)
        {
            isx_check_input_file_path(in_input_file_paths[i], {isx::DataSet::Type::GPIO, isx::DataSet::Type::IMU});
            input_gpio_sets.push_back(isx::readGpio(in_input_file_paths[i], std::string(in_inter_isxd_file_dir)));
        }

        isx::GpioExporterParams params(
            input_gpio_sets,
            std::string(in_output_file_path),
            isx::WriteTimeRelativeTo(in_write_time_relative_to));

        auto output_params = std::make_shared<isx::GpioExporterOutputParams>();
        
        isx::DataSet::Type refType = input_gpio_sets[0]->getEventBasedFileType();
        
        // Use dedicated exporter function for .imu files
        std::string extension = isx::getExtension(in_input_file_paths[0]);
        if (refType == isx::DataSet::Type::IMU)
        {
            return isx::runIMUExporter(params, output_params, [](float) {return false;});
        }
        else
        {
            return isx::runGpioExporter(params, output_params, [](float) {return false;});
        }
    });
}

int
isx_export_gpio_isxd(
    const char * in_input_file_path,
    const char * in_output_dir_path
)
{
    return isx_process_async_op([=]()
    {
        isx_check_input_file_path(in_input_file_path, {isx::DataSet::Type::GPIO, isx::DataSet::Type::IMU});
        const isx::GpioDataParams inputParams(in_output_dir_path, in_input_file_path);
        auto outputParams = std::make_shared<isx::GpioDataOutputParams>();
        return runGpioDataImporter(inputParams, outputParams, [](float){return false;});
    });
}

int
isx_decompress(
    const char * in_input_file_path,
    const char * in_output_dir_path
)
{
    return isx_process_async_op([=]()
    {
        const isx::DecompressParams inputParams(in_output_dir_path, in_input_file_path);
        auto outputParams = std::make_shared<isx::DecompressOutputParams>();
        return runDecompression(inputParams, outputParams, [](float){return false;});
    });
}

int
isx_align_start_times(
    const char * in_input_ref_file_path,
    const size_t in_num_input_align_files,
    const char ** in_input_align_file_paths
)
{
    return isx_process_async_op([=]()
    {
        std::vector<std::string> input_align_file_paths(in_num_input_align_files);
        for (size_t i = 0; i < in_num_input_align_files; i++)
        {
            input_align_file_paths[i] = in_input_align_file_paths[i];
        }

        return isx::alignStartTimes(
            in_input_ref_file_path,
            input_align_file_paths
        );
    });
}

int
isx_export_aligned_timestamps(
    const char * in_input_ref_file_path,
    const size_t in_num_input_align_files,
    const char ** in_input_align_file_paths,
    const char * in_input_ref_name,
    const char ** in_input_align_names,
    const char * in_output_file_path,
    int in_write_time_relative_to
)
{
    return isx_process_async_op([=](){
        std::vector<std::vector<std::string>> input_align_file_paths(in_num_input_align_files);
        std::vector<std::string> input_align_names(in_num_input_align_files);
        for (size_t i = 0; i < in_num_input_align_files; i++)
        {
            input_align_file_paths[i] = {in_input_align_file_paths[i]};
            input_align_names[i] = in_input_align_names[i];
        }    

        const isx::ExportAlignedTimestampsParams inputParams(
            {in_input_ref_file_path},
            input_align_file_paths,
            in_input_ref_name,
            input_align_names,
            in_output_file_path,
            isx::WriteTimeRelativeTo(in_write_time_relative_to)
        );
        auto outputParams = std::make_shared<isx::ExportAlignedTimestampsOutputParams>();
        return exportAlignedTimestamps(inputParams, outputParams, [](float){return false;});
    });
}
