#ifndef ISX_VESSEL_SET_FILE_H
#define ISX_VESSEL_SET_FILE_H

#include <fstream>
#include "isxCoreFwd.h"
#include "isxTimingInfo.h"
#include "isxSpacingInfo.h"
#include "isxImage.h"
#include "isxTrace.h"
#include "isxJsonUtils.h"
#include "isxVesselSet.h"


namespace isx
{

/// A class for a file containing all vessels extracted from a movie
///
class VesselSetFile
{

public:
    /// Empty constructor.
    ///
    /// This creates a valid c++ object but an invalid vessel set file.
    VesselSetFile();

    /// Read constructor.
    ///
    /// This opens an existing vessel set file and reads information from its
    /// header.
    ///
    /// \param  inFileName  The name of the vessel set file.
    /// \param  enableWrite     Set to true to open in read-write mode
    ///
    /// \throw  isx::ExceptionFileIO    If reading the vessel set file fails.
    /// \throw  isx::ExceptionDataIO    If parsing the vessel set file fails.
    VesselSetFile(const std::string & inFileName, bool enableWrite = false);

    /// Write constructor.
    ///
    /// This opens a new file, writes header information.
    ///
    /// \param  inFileName      The name of the vessel set file.
    /// \param  inTimingInfo    The timing information of the vessel set.
    /// \param  inSpacingInfo   The spacing information of the vessel set.
    /// \param  inVesselSetType The type of the vessel set (vessel diameter or rbc velocity).
    ///
    /// \throw  isx::ExceptionFileIO    If writing the vessel set file fails.
    /// \throw  isx::ExceptionDataIO    If formatting the vessel set data fails.
    VesselSetFile(const std::string & inFileName,
                const TimingInfo & inTimingInfo,
                const SpacingInfo & inSpacingInfo,
                const VesselSetType_t inVesselSetType);

    /// Destructor.
    ///
    ~VesselSetFile();

    /// \return True if the vessel set file is valid, false otherwise.
    ///
    bool isValid() const;

    /// Close this file for writing.  This writes the header containing
    /// metadata at the end of the file.  Any attempts to write data after
    /// this is called will result in an exception.
    ///
    void
    closeForWriting();

    /// \return     The name of the file.
    ///
    std::string getFileName() const;

    /// \return the number of vessel contained in the file
    ///
    const isize_t numberOfVessels();

    /// \return     The timing information read from the vessel set.
    ///
    const isx::TimingInfo & getTimingInfo() const;

    /// \return     The spacing information read from the vessel set.
    ///
    const isx::SpacingInfo & getSpacingInfo() const;

    /// Read vessel data for vessel ID.
    /// \param inVesselId the vessel of interest
    /// \return a shared pointer to the trace data for the input vessel.
    /// \throw  isx::ExceptionFileIO    If trying to access nonexistent vessel or reading fails.
    SpFTrace_t readTrace(isize_t inVesselId);

    /// \return a shared pointer to the projection image for the input vessel
    /// \throw  isx::ExceptionFileIO    If trying to access nonexistent vessel or reading fails.
    SpImage_t readProjectionImage();

    /// \return the line endpoints for the input vessel
    /// \throw  isx::ExceptionFileIO    If trying to access nonexistent vessel or reading fails.
    SpVesselLine_t readLineEndpoints(isize_t inVesselId);

    /// \return the direction trace for the input vessel
    /// \throw  isx::ExceptionFileIO    If trying to access nonexistent vessel or reading fails.
    SpFTrace_t readDirectionTrace(isize_t inVesselId);

    /// \return the correlation triptych for a particular velocity measurement of a vessel
    /// \throw  isx::ExceptionFileIO    If trying to access nonexistent vessel or reading fails.
    SpVesselCorrelations_t readCorrelations(isize_t inVesselId, isize_t inFrameNumber);

    /// Write vessel data
    /// \param inVesselId the vessel of interest
    /// \param inProjectionImage the projection image to write
    /// \param inLineEndpoints the endpoints of the line to write
    /// \param inData the trace to write
    /// \param inName the vessel name (will be truncated to 15 characters, if longer). If no name is provided, a default will be created using the vessel id
    /// \param  inDirectionTrace    The direction of velocity if the vessel set is an rbc velocity type
    /// \param  inCorrTrace         The correlation triptychs used to estimate velocity if the vessel set is an rbc velocity type
    /// If vessel ID already exists, it will overwrite its data. Otherwise, it will be appended
    /// \throw  isx::ExceptionFileIO    If trying to access nonexistent vessel or writing fails.
    /// \throw  isx::ExceptionDataIO    If image data is of an unexpected data type.
    /// \throw  isx::ExceptionFileIO    If called after calling closeForWriting().
    void writeVesselData(isize_t inVesselId, const Image & inProjectionImage, const SpVesselLine_t & inLineEndpoints,
                         Trace<float> & inData, const std::string & inName = std::string(),
                         const SpFTrace_t & inDirectionTrace = nullptr,
                         const SpVesselCorrelationsTrace_t & inCorrTrace = nullptr);

    /// \return the status of the vessel
    /// \param inVesselId the vessel of interest
    /// \throw  isx::ExceptionFileIO    If trying to access nonexistent vessel or reading fails.
    VesselSet::VesselStatus getVesselStatus(isize_t inVesselId);

    /// \return the status of the vessel
    /// \param inVesselId the vessel of interest
    /// \throw  isx::ExceptionFileIO    If trying to access nonexistent vessel or reading fails.
    Color getVesselColor(isize_t inVesselId);

    /// \return the status of the vessel
    /// \param inVesselId the vessel of interest
    /// \throw  isx::ExceptionFileIO    If trying to access nonexistent vessel or reading fails.
    std::string
    getVesselStatusString(isize_t inVesselId);

    /// Set a vessel in the set to be valid/invalid (used for rejecting or accepting segmented vessel)
    /// \param inVesselId the vessel of interest
    /// \param inStatus accepted/rejected/undecided status
    /// \throw  isx::ExceptionFileIO    If trying to access nonexistent vessel or reading fails.
    /// \throw  isx::ExceptionFileIO    If called after calling closeForWriting().
    void setVesselStatus(isize_t inVesselId, VesselSet::VesselStatus inStatus);

    /// Set a vessel color
    /// \param inVesselId the vessel of interest
    /// \param inColor new color
    void setVesselColor(isize_t inVesselId, const Color& inColor);

    /// Set a vessel colors
    /// \param inColors new colors
    void setVesselColors(const IdColorPairs& inColor);

    /// Get the name for a vessel in the set
    /// \param inVesselId the vessel of interest
    /// \return a string with the name
    std::string getVesselName(isize_t inVesselId);

    /// Set the vessel name in the vessel header
    /// \param inVesselId the vessel of interest
    /// \param inName the assigned name (it will be truncated to 15 characters, if longer than that)
    /// \throw  isx::ExceptionFileIO    If called after calling closeForWriting().
    void setVesselName(isize_t inVesselId, const std::string & inName);

    /// Indicates whether a vessel is active in the file or not
    /// \return true if vessel has a valid trace
    bool isVesselActive(isize_t inVesselId) const;

    /// Set the vessel activity flag
    /// \param inVesselId the vessel of interest
    /// \param inActive whether the vessel is active or not
    void setVesselActive(isize_t inVesselId, bool inActive);

    /// get the efocus values
    /// \return  efocus values
    std::vector<uint16_t> getEfocusValues();

    /// set the efocus values
    /// \param inEfocus efocus values
    void setEfocusValues(const std::vector<uint16_t> & inEfocus);

    std::string
    getExtraProperties() const;

    void
    setExtraProperties(const std::string & inProperties);

    SpacingInfo
    getOriginalSpacingInfo() const;

    VesselSetType_t
    getVesselSetType() const;

    SizeInPixels_t
    getCorrelationSize(isize_t inVesselId) const;

    /// \return flag indicating whether correlation heatmaps were saved to disk
    ///
    bool
    isCorrelationSaved() const;
    
private:

    /// True if the vessel set file is valid, false otherwise.
    bool m_valid = false;

    /// The total number of vessels in the set.
    isize_t m_numVessels = 0;

    /// The name of the vessel set file.
    std::string m_fileName;

    /// The timing information of the vessel set.
    TimingInfo m_timingInfo;

    /// The spacing information of the vessel set.
    SpacingInfo m_spacingInfo;

    /// The header offset.
    std::ios::pos_type m_headerOffset;

    /// The vessel names
    VesselNames_t m_vesselNames;

    /// Vessel validity flags
    VesselStatuses_t m_vesselStatuses;

    /// Vessel validity flags
    VesselColors_t m_vesselColors;

    /// Flag indicating whether a vessel is active in this file
    VesselActivities_t m_vesselActivity;

    /// The type of vessel set (rbc velocity or vessel diameter)
    VesselSetType_t m_vesselSetType = VesselSetType_t::VESSEL_DIAMETER;
    
    /// Flag indicating whether direction was saved to vessel set file
    bool m_directionSaved = false;

    /// Size of correlation heatmaps for rbc velocity vessel set
    std::vector<SizeInPixels_t> m_correlationSizes;

    /// Efocus values for each vessel, used by Multiplane registration
    std::vector<uint16_t> m_efocusValues = {0};

    /// The file stream
    std::fstream m_file;

    /// The stream open mode
    std::ios_base::openmode m_openmode;

    bool m_fileClosedForWriting = false;

    const static size_t s_version = 5;

    /// Size of global vessel set
    isize_t m_sizeGlobalVS = 0;

    /// The extra properties to write in the JSON footer.
    json m_extraProperties = nullptr;

    /// Read the header to populate information about the vessel set.
    ///
    /// \throw  isx::ExceptionFileIO    If reading the header from the file fails.
    /// \throw  isx::ExceptionDataIO    If parsing the header fails.
    void readHeader();

    /// Write the header containing information about the vessel set.
    ///
    /// \throw  isx::ExceptionFileIO    If writing the vessel set file fails.
    /// \throw  isx::ExceptionDataIO    If formatting the vessel set data fails.
    void writeHeader();

    /// Seek to a specific vessel in the file for a read operation
    /// \param inVesselId the vessel ID
    /// \param readTrace if true position to read trace, if false position to read line endpoints
    /// \throw  isx::ExceptionFileIO    If seeking to a nonexistent vessel or reading fails.
    void seekToVesselForRead(isize_t inVesselId, const bool readTrace);

    /// Seek to a specific vessel in the file for a write operation
    /// \param inVesselId the vessel ID
    /// \throw  isx::ExceptionFileIO    If seeking to a nonexistent vessel or reading fails.
    void seekToVesselForWrite(isize_t inVesselId);

    /// Seek to the projection image in the file for a read operation
    /// \throw  isx::ExceptionFileIO    If reading fails.
    void seekToProjectionImageForRead();

    /// \return the size of the projection image in bytes
    ///
    isize_t projectionImageSizeInBytes();

    /// \return the size of the line endpoints in bytes
    ///
    isize_t lineEndpointsSizeInBytes();

    /// \return the size of the trace in bytes (in the vessel header)
    ///
    isize_t traceSizeInBytes();

    /// \return the size of the direction trace in bytes
    ///
    isize_t directionSizeInBytes();

    /// \return the size of the correlation heatmap for a vessel in bytes
    /// \param inVesselId the vessel ID
    ///
    isize_t correlationSizeInBytes(isize_t inVesselId);

    /// \return the size of the correlation tritych trace for a vessel in bytes
    ///
    isize_t correlationTraceSizeInBytes(isize_t inVesselId);

    /// \return the size of a vessel in bytes
    ///
    isize_t vesselDataSizeInBytes(isize_t inVesselId);

    /// \return the offset of a vessel in bytes
    ///
    isize_t vesselOffsetInBytes(isize_t inVesselId);

    /// Flush the stream
    ///
    void flush();

    /// Replace empty vessel names with names of format V%0Xu.
    /// X is the width of the number string and is automatically determined by
    /// the number of vessels.
    void replaceEmptyNames();

    /// Saves the vessel set type to the metadata extra properties
    void saveVesselSetType();

    /// \return list correlation sizes as json
    ///
    json convertVesselSetCorrelationSizesToJson();

};

} // namespace isx

#endif // ISX_VESSEL_SET_FILE_H
