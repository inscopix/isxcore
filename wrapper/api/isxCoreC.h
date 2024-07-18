#ifndef ISX_CORE_C_H
#define ISX_CORE_C_H

#if ISX_OS_WIN32
#define ISX_DLL_EXPORT __declspec(dllexport)
#else
#define ISX_DLL_EXPORT
#endif

#include <stddef.h>
#include <stdint.h>

/// These structs and functions are mostly undocumented as they
/// are simply wrappers around the C++ API.

#ifdef __cplusplus
extern "C"
{
#endif // def __cplusplus

//////////////////////
// API version methods
//////////////////////

ISX_DLL_EXPORT
int
isx_get_version_string_size(
    size_t * out_version_string_size
);

ISX_DLL_EXPORT
int
isx_get_version_string(
    char * out_version_string,
    const size_t in_version_string_size
);

////////////////////////////
// Shared struct definitions
////////////////////////////

typedef struct IsxRatio
{
    int64_t num;
    int64_t den;
} IsxRatio;

typedef struct IsxTime
{
    IsxRatio secs_since_epoch;
    int32_t utc_offset;
} IsxTime;

typedef struct IsxIndexRange
{
    size_t first;
    size_t last;
} IsxIndexRange;

typedef struct IsxTimingInfo
{
    IsxTime start;
    IsxRatio step;
    size_t num_samples;
    size_t * dropped;
    size_t num_dropped;
    size_t * cropped_first;
    size_t * cropped_last;
    size_t num_cropped;
    size_t * blank;
    size_t num_blank;
} IsxTimingInfo;

ISX_DLL_EXPORT
int
isx_timing_info_get_secs_since_start(
    IsxTimingInfo * in_timing_info,
    int64_t * out_secs_since_start_num,
    int64_t * out_secs_since_start_den
);

/// \param  out_mask    This would ideally be bool *, but MATLAB can't handle that.
ISX_DLL_EXPORT
int
isx_timing_info_get_valid_sample_mask(
    IsxTimingInfo * in_timing_info,
    uint8_t * out_mask
);

typedef struct IsxSpacingInfo
{
    size_t num_cols;
    size_t num_rows;
    IsxRatio pixel_width;
    IsxRatio pixel_height;
    IsxRatio left;
    IsxRatio top;
} IsxSpacingInfo;

///////////////////////////
// Movie struct and methods
///////////////////////////

typedef struct IsxMovie
{
    size_t id;
    IsxTimingInfo timing;
    IsxSpacingInfo spacing;
    int data_type;
    bool read_only;
    char * file_path;
} IsxMovie;

ISX_DLL_EXPORT
int
isx_read_movie(
    const char * in_file_path,
    IsxMovie ** out_movie
);

ISX_DLL_EXPORT
int
isx_write_movie(
    const char * in_file_path,
    IsxTimingInfo in_timing,
    IsxSpacingInfo in_spacing,
    int in_data_type,
    const bool in_has_frame_header_footer,
    IsxMovie ** out_movie
);

ISX_DLL_EXPORT
int
isx_movie_get_frame_data_u16(
    IsxMovie * in_movie,
    size_t in_index,
    uint16_t * out_frame_data
);

ISX_DLL_EXPORT
int
isx_movie_get_frame_data_u16_with_header_footer(
    IsxMovie * in_movie,
    size_t in_index,
    uint16_t * out_frame_data
);

ISX_DLL_EXPORT
int
isx_movie_get_frame_header(
    IsxMovie * in_movie,
    size_t in_index,
    uint16_t * out_header
);

ISX_DLL_EXPORT
int
isx_movie_get_frame_footer(
    IsxMovie * in_movie,
    size_t in_index,
    uint16_t * out_header
);

ISX_DLL_EXPORT
int
isx_movie_get_frame_data_f32(
    IsxMovie * in_movie,
    size_t in_index,
    float * out_frame_data
);

ISX_DLL_EXPORT
int
isx_movie_get_frame_data_u8(
    IsxMovie * in_movie,
    size_t in_index,
    uint8_t * out_frame_data
);

ISX_DLL_EXPORT
int
isx_movie_write_frame_u16(
    IsxMovie * in_movie,
    size_t in_index,
    uint16_t * in_frame_data
);

ISX_DLL_EXPORT
int
isx_movie_write_frame_u16_with_header_footer(
    IsxMovie * in_movie,
    size_t in_index,
    uint16_t * in_buffer
);

ISX_DLL_EXPORT
int
isx_movie_write_frame_u16_with_header_footer_separately(
    IsxMovie * in_movie,
    uint16_t * in_header,
    uint16_t * in_pixels,
    uint16_t * in_footer
);

ISX_DLL_EXPORT
int
isx_movie_write_frame_f32(
    IsxMovie * in_movie,
    size_t in_index,
    float * in_frame_data
);

ISX_DLL_EXPORT
int
isx_movie_set_extra_properties(
    IsxMovie * in_movie,
    const char * in_extra_properties
);

ISX_DLL_EXPORT
int
isx_movie_get_extra_properties(
    IsxMovie * in_movie,
    char * out_extra_properties,
    const size_t in_extra_properties_size
);

ISX_DLL_EXPORT
int
isx_movie_get_extra_properties_for_matlab(
    IsxMovie * in_movie,
    char ** out_extra_properties,
    const size_t in_extra_properties_size
);

ISX_DLL_EXPORT
int
isx_movie_get_extra_properties_size(
    IsxMovie * in_movie,
    size_t * out_extra_properties_size
);

ISX_DLL_EXPORT
int
isx_movie_get_acquisition_info(
    IsxMovie * in_movie,
    char * out_acquisition_info,
    const size_t in_acquisition_info_size
);

ISX_DLL_EXPORT
int
isx_movie_get_acquisition_info_for_matlab(
    IsxMovie * in_movie,
    char ** out_acquisition_info,
    const size_t in_acquisition_info_size
);

ISX_DLL_EXPORT
int
isx_movie_get_acquisition_info_size(
    IsxMovie * in_movie,
    size_t * out_acquisition_info_size
);

ISX_DLL_EXPORT
int
isx_movie_flush(
    IsxMovie * in_movie
);

ISX_DLL_EXPORT
int
isx_movie_delete(
    IsxMovie * in_movie
);

ISX_DLL_EXPORT
int
isx_movie_flush_with_timing_info(
    IsxMovie * in_movie,
    IsxTimingInfo in_timing
);

ISX_DLL_EXPORT
int
isx_movie_get_frame_timestamp(
    IsxMovie * in_movie,
    size_t in_index,
    uint64_t * out_timestamp
);

/////////////////////////////
// CellSet struct and methods
/////////////////////////////

typedef struct IsxCellSet
{
    size_t id;
    IsxTimingInfo timing;
    IsxSpacingInfo spacing;
    size_t num_cells;
    bool roi_set;
    bool read_only;
    char * file_path;
} IsxCellSet;

ISX_DLL_EXPORT
int
isx_read_cell_set(
    const char * in_file_path,
    bool in_read_only,
    IsxCellSet ** out_cell_set
);

ISX_DLL_EXPORT
int
isx_write_cell_set(
    const char * in_file_path,
    IsxTimingInfo in_timing,
    IsxSpacingInfo in_spacing,
    bool in_is_roi_set,
    IsxCellSet ** out_cell_set
);

ISX_DLL_EXPORT
int
isx_cell_set_get_name(
    IsxCellSet * in_cell_set,
    size_t in_index,
    size_t in_cell_name_size,
    char * out_cell_name
);

ISX_DLL_EXPORT
int
isx_cell_set_get_name_for_matlab(
    IsxCellSet * in_cell_set,
    size_t in_index,
    size_t in_cell_name_size,
    char ** out_cell_name
);

ISX_DLL_EXPORT
int
isx_cell_set_get_trace(
    IsxCellSet * in_cell_set,
    size_t in_index,
    float * out_trace
);

ISX_DLL_EXPORT
int
isx_cell_set_get_image(
    IsxCellSet * in_cell_set,
    size_t in_index,
    float * out_image
);

ISX_DLL_EXPORT
int
isx_cell_set_get_status(
    IsxCellSet * in_cell_set,
    size_t in_index,
    int * out_status
);

ISX_DLL_EXPORT
int
isx_cell_set_set_status(
    IsxCellSet * in_cell_set,
    size_t in_index,
    int in_status
);

ISX_DLL_EXPORT
int
isx_cell_set_write_image_trace(
    IsxCellSet * in_cell_set,
    size_t in_index,
    float * in_image,
    float * in_trace,
    const char * in_name
);

ISX_DLL_EXPORT
int
isx_cell_set_flush(
    IsxCellSet * in_cell_set
);

ISX_DLL_EXPORT
int
isx_cell_set_delete(
    IsxCellSet * in_cell_set
);

ISX_DLL_EXPORT
int
isx_cell_set_get_acquisition_info(
    IsxCellSet * in_cell_set,
    char * out_acquisition_info,
    const size_t in_acquisition_info_size
);

ISX_DLL_EXPORT
int
isx_cell_set_get_acquisition_info_for_matlab(
    IsxCellSet * in_cell_set,
    char ** out_acquisition_info,
    const size_t in_acquisition_info_size
);

ISX_DLL_EXPORT
int
isx_cell_set_get_acquisition_info_size(
    IsxCellSet * in_cell_set,
    size_t * out_acquisition_info_size
);

typedef struct IsxVesselSet
{
    size_t id;
    IsxTimingInfo timing;
    IsxSpacingInfo spacing;
    size_t num_vessels;
    bool read_only;
    char * file_path;
} IsxVesselSet;

ISX_DLL_EXPORT
int
isx_read_vessel_set(
    const char * in_file_path,
    bool in_read_only,
    IsxVesselSet ** out_vessel_set
);

ISX_DLL_EXPORT
int
isx_write_vessel_set(
    const char * in_file_path,
    IsxTimingInfo in_timing,
    IsxSpacingInfo in_spacing,
    const int in_vessel_set_type,
    IsxVesselSet ** out_vessel_set
);

ISX_DLL_EXPORT
int
isx_vessel_set_get_name(
    IsxVesselSet * in_vessel_set,
    size_t in_index,
    size_t in_vessel_name_size,
    char * out_vessel_name
);

ISX_DLL_EXPORT
int
isx_vessel_set_get_name_for_matlab(
    IsxVesselSet * in_vessel_set,
    size_t in_index,
    size_t in_vessel_name_size,
    char ** out_vessel_name
);

ISX_DLL_EXPORT
int
isx_vessel_set_get_trace(
    IsxVesselSet * in_vessel_set,
    size_t in_index,
    float * out_trace
);

ISX_DLL_EXPORT
int
isx_vessel_set_get_image(
    IsxVesselSet * in_vessel_set,
    size_t in_index,
    float * out_image
);

ISX_DLL_EXPORT
int
isx_vessel_set_get_line_endpoints(
    IsxVesselSet * in_vessel_set,
    size_t in_index,
    int64_t * out_line
);

ISX_DLL_EXPORT
int
isx_vessel_set_get_type(
    IsxVesselSet * in_vessel_set,
    int * type
);

ISX_DLL_EXPORT
int
isx_vessel_set_get_center_trace(
    IsxVesselSet * in_vessel_set,
    size_t in_index,
    float * out_trace
);

ISX_DLL_EXPORT
int
isx_vessel_set_get_direction_trace(
    IsxVesselSet * in_vessel_set,
    size_t in_index,
    float * out_trace
);

ISX_DLL_EXPORT
int
isx_vessel_set_is_correlation_saved(
    IsxVesselSet * in_vessel_set,
    int * out_saved
);

ISX_DLL_EXPORT
int
isx_vessel_set_get_correlation_size(
    IsxVesselSet * in_vessel_set,
    size_t in_index,
    size_t * bbox_size
);

ISX_DLL_EXPORT
int
isx_vessel_set_get_correlations(
    IsxVesselSet * in_vessel_set,
    size_t in_index,
    size_t in_frame_number,
    float * out_correlations
);

ISX_DLL_EXPORT
int
isx_vessel_set_get_status(
    IsxVesselSet * in_vessel_set,
    size_t in_index,
    int * out_status
);

ISX_DLL_EXPORT
int
isx_vessel_set_set_status(
    IsxVesselSet * in_vessel_set,
    size_t in_index,
    int in_status
);

ISX_DLL_EXPORT
int
isx_vessel_set_write_vessel_diameter_data(
    IsxVesselSet * in_vessel_set,
    size_t in_index,
    float * in_image,
    int64_t * in_line,
    float * in_trace,
    float * in_center_trace,
    const char * in_name
);

ISX_DLL_EXPORT
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
);

ISX_DLL_EXPORT
int
isx_vessel_set_flush(
    IsxVesselSet * in_vessel_set
);

ISX_DLL_EXPORT
int
isx_vessel_set_delete(
    IsxVesselSet * in_vessel_set
);

ISX_DLL_EXPORT
int
isx_vessel_set_get_acquisition_info(
    IsxVesselSet * in_vessel_set,
    char * out_acquisition_info,
    const size_t in_acquisition_info_size
);

ISX_DLL_EXPORT
int
isx_vessel_set_get_acquisition_info_for_matlab(
    IsxVesselSet * in_vessel_set,
    char ** out_acquisition_info,
    const size_t in_acquisition_info_size
);

ISX_DLL_EXPORT
int
isx_vessel_set_get_acquisition_info_size(
    IsxVesselSet * in_vessel_set,
    size_t * out_acquisition_info_size
);

///////////////////////////
// Event struct and methods
///////////////////////////

typedef struct IsxEvents
{
    size_t id;
    IsxTimingInfo timing;
    size_t num_cells;
    bool read_only;
    char * file_path;
} IsxEvents;

ISX_DLL_EXPORT
int
isx_read_events(
    const char * in_file_path,
    IsxEvents ** out_events
);

ISX_DLL_EXPORT
int
isx_write_events(
    const char * in_file_path,
    IsxTimingInfo in_timing,
    const char ** in_channels,
    const size_t in_num_channels,
    IsxEvents ** out_events
);

ISX_DLL_EXPORT
int
isx_events_write_cell(
    IsxEvents * in_events,
    uint64_t in_index,
    size_t in_num_packets,
    uint64_t * in_usecs_since_start,
    float * in_values
);

// NOTE: Must be called after isx_events_get_cell_count to get size of destination array
ISX_DLL_EXPORT
int
isx_events_get_cell(
    IsxEvents * in_events,
    const char * in_name,
    uint64_t * out_usecs_since_start,
    float * out_values
);

ISX_DLL_EXPORT
int
isx_events_get_cell_count(
    IsxEvents * in_events,
    const char * in_name,
    size_t * out_count
);

ISX_DLL_EXPORT
int
isx_events_get_cell_name(
    IsxEvents * in_events,
    size_t in_index,
    size_t in_cell_name_size,
    char * out_cell_name
);

ISX_DLL_EXPORT
int
isx_events_get_cell_name_for_matlab(
    IsxEvents * in_events,
    size_t in_index,
    size_t in_cell_name_size,
    char ** out_cell_name
);

ISX_DLL_EXPORT
int
isx_events_flush(
    IsxEvents * in_events
);

ISX_DLL_EXPORT
int
isx_events_delete(
    IsxEvents * in_events
);

ISX_DLL_EXPORT
int
isx_events_get_acquisition_info(
    IsxEvents * in_events,
    char * out_acquisition_info,
    const size_t in_acquisition_info_size
);

ISX_DLL_EXPORT
int
isx_events_get_acquisition_info_for_matlab(
    IsxEvents * in_events,
    char ** out_acquisition_info,
    const size_t in_acquisition_info_size
);

ISX_DLL_EXPORT
int
isx_events_get_acquisition_info_size(
    IsxEvents * in_events,
    size_t * out_acquisition_info_size
);

///////////////////////////
// GPIO struct and methods
///////////////////////////

typedef struct IsxGpio
{
    size_t id;
    IsxTimingInfo timing;
    size_t num_channels;
    bool read_only;
    char * file_path;
} IsxGpio;

ISX_DLL_EXPORT
int
isx_read_gpio(
    const char * in_file_path,
    IsxGpio ** out_gpio
);

ISX_DLL_EXPORT
int
isx_gpio_get_channel_count(
    IsxGpio * in_gpio,
    const char * in_name,
    size_t * out_count
);

ISX_DLL_EXPORT
int
isx_gpio_get_channel(
    IsxGpio * in_gpio,
    const char * in_name,
    uint64_t * out_usecs_since_start,
    float * out_values
);

ISX_DLL_EXPORT
int
isx_gpio_get_channel_name(
    IsxGpio * in_gpio,
    size_t in_index,
    size_t in_channel_name_size,
    char * out_channel_name
);

ISX_DLL_EXPORT
int
isx_gpio_delete(
    IsxGpio * in_gpio
);

ISX_DLL_EXPORT
int
isx_gpio_get_acquisition_info(
    IsxGpio * in_gpio,
    char * out_acquisition_info,
    const size_t in_acquisition_info_size
);

ISX_DLL_EXPORT
int
isx_gpio_get_acquisition_info_size(
    IsxGpio * in_gpio,
    size_t * out_acquisition_info_size
);

////////////
// Functions
////////////

ISX_DLL_EXPORT
int
isx_initialize();

ISX_DLL_EXPORT
int
isx_shutdown();

ISX_DLL_EXPORT
int
isx_get_data_type_u16();

ISX_DLL_EXPORT
int
isx_get_data_type_f32();

ISX_DLL_EXPORT
int
isx_get_data_type_u8();

ISX_DLL_EXPORT
int
isx_get_cell_status_accepted();

ISX_DLL_EXPORT
int
isx_get_cell_status_undecided();

ISX_DLL_EXPORT
int
isx_get_cell_status_rejected();

ISX_DLL_EXPORT
int
isx_get_vessel_status_accepted();

ISX_DLL_EXPORT
int
isx_get_vessel_status_undecided();

ISX_DLL_EXPORT
int
isx_get_vessel_status_rejected();

ISX_DLL_EXPORT
int
isx_get_vessel_type_vessel_diameter();

ISX_DLL_EXPORT
int
isx_get_vessel_type_rbc_velocity();

// These return version numbers, not error codes.
ISX_DLL_EXPORT
int
isx_get_core_version_major();

ISX_DLL_EXPORT
int
isx_get_core_version_minor();

ISX_DLL_EXPORT
int
isx_get_core_version_patch();

ISX_DLL_EXPORT
int
isx_get_core_version_build();

ISX_DLL_EXPORT
int
isx_get_is_with_algos();

// Note: This is not thread-safe. Multi-threaded invocation of the fun.ctions here will certainly break
// this if there is more than one exception.  To make this thread-safe we'll need to design a different
// API to retrieve the exception string that would probably also make this harder to use.
ISX_DLL_EXPORT
const char *
isx_get_last_exception_string();

ISX_DLL_EXPORT
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
);

ISX_DLL_EXPORT
int
isx_export_movie_tiff(
    size_t in_num_input_files,
    const char ** in_input_file_paths,
    const char * in_output_file_path,
    bool in_write_invalid_frames
);

ISX_DLL_EXPORT
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
);

ISX_DLL_EXPORT
int
isx_export_movie_timestamps_to_csv(
    size_t in_num_input_files,
    const char ** in_input_file_paths,
    const char * in_output_file_path,
    int in_format
);

ISX_DLL_EXPORT
int
isx_export_nvision_movie_tracking_frame_data_to_csv(
    size_t in_num_input_files,
    const char ** in_input_file_paths,
    const char * in_output_file_path,
    int in_format
);

ISX_DLL_EXPORT
int
isx_export_nvision_movie_tracking_zone_data_to_csv(
    size_t in_num_input_files,
    const char ** in_input_file_paths,
    const char * in_output_file_path
);

ISX_DLL_EXPORT
int
isx_get_time_reference_start();

ISX_DLL_EXPORT
int
isx_get_time_reference_unix();

ISX_DLL_EXPORT
int
isx_get_time_reference_tsc();

ISX_DLL_EXPORT
int
isx_export_event_set(
    const size_t in_num_input_files,
    const char ** in_input_file_paths,
    const char * in_output_file_path,
    const int in_write_time_relative_to,
    const char * in_props_file_path,
    const bool in_sparse_output,
    const bool in_write_amplitude
);

ISX_DLL_EXPORT
int
isx_export_gpio_set(
    const size_t in_num_input_files,
    const char ** in_input_file_paths,
    const char * in_output_file_path,
    const char * in_inter_isxd_file_dir,
    const int in_write_time_relative_to
);

ISX_DLL_EXPORT
int
isx_export_gpio_isxd(
    const char * in_input_file_path,
    const char * in_output_dir_path
);

ISX_DLL_EXPORT
int
isx_decompress(
    const char * in_input_file_path,
    const char * in_output_dir_path
);

ISX_DLL_EXPORT
int
isx_align_start_times(
    const char * in_input_ref_file_path,
    const size_t in_num_input_align_files,
    const char ** in_input_align_file_paths
);

ISX_DLL_EXPORT
int
isx_export_aligned_timestamps(
    const char * in_input_ref_file_path,
    const size_t in_num_input_align_files,
    const char ** in_input_align_file_paths,
    const char * in_input_ref_name,
    const char ** in_input_align_names,
    const char * in_output_file_path,
    int in_write_time_relative_to
);

#ifdef __cplusplus
}
#endif // def __cplusplus

#endif // define ISX_CORE_C_H
