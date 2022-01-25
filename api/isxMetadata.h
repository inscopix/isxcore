#ifndef ISX_METADATA_H
#define ISX_METADATA_H

#include "isxCellSetFactory.h"
#include "json.hpp"
#include <string>

namespace isx
{
    /// \cond doxygen chokes on enum class inside of namespace
    /// Multicolor channels
    enum class MulticolorChannel_t
    {
        INVALID = 0,
        GREEN,
        RED,
    };
    /// \endcond doxygen chokes on enum class inside of namespace

    /// \cond doxygen chokes on enum class inside of namespace
    /// Method used for generating a cell set
    enum class CellSetMethod_t
    {
        UNAVAILABLE = 0,
        PCAICA,
        CNMFE,
        MANUAL,
        APPLIED
    };
    /// \endcond doxygen chokes on enum class inside of namespace

    /// \cond doxygen chokes on enum class inside of namespace
    /// Type of spatial footprints in a cell set
    enum class CellSetType_t
    {
        UNAVAILABLE = 0,
        ANALOG,
        BINARY,
    };
    /// \endcond doxygen chokes on enum class inside of namespace

    /// \cond doxygen chokes on enum class inside of namespace
    /// Units of the traces in a cell set
    enum class CellSetUnits_t
    {
        UNAVAILABLE = 0,
        RAW,           // manual ROI and applied contours (average of raw pixel values within ROI)
        DF_OVER_F,     // PCA-ICA (DF/F)
        DF,            // CNMFe (estimate of the “true” dF, i.e. temporal traces which are on the same scale of pixel intensity as the raw movie)
        DF_OVER_NOISE  // CNMFe (trace divided by its estimate noise level)
    };
    /// \endcond doxygen chokes on enum class inside of namespace

    /// \cond doxygen chokes on enum class inside of namespace
    /// Type of data stored in a vessel set
    enum class VesselSetType_t
    {
        VESSEL_DIAMETER = 0,
        RBC_VELOCITY,
    };
    /// \endcond doxygen chokes on enum class inside of namespace

    /// \cond doxygen chokes on enum class inside of namespace
    /// Units of the traces in a vessel set
    enum class VesselSetUnits_t
    {
        // units used for vessel diameter
        PIXELS = 0,
        MICRONS,
        // units used for rbc velocity
        PIXELS_PER_SECOND,
        MICRONS_PER_SECOND
    };
    /// \endcond doxygen chokes on enum class inside of namespace

    /// \cond doxygen chokes on enum class inside of namespace
    /// Type of projection image
    enum class ProjectionType
    {
        MEAN = 0,
        MIN,
        MAX,
        STANDARD_DEVIATION
    };
    /// \endcond doxygen chokes on enum class inside of namespace

    /// \cond doxygen chokes on enum class inside of namespace
    /// Type of base plate unit used to capture data
    /// Enum values are set in order to maintain backwards compatibility with IDPS 1.7
    enum class BasePlateType_t
    {
        UNAVAILABLE = 0,
        CUSTOM = 1,
        CRANIAL_WINDOW = 17,

        BP1 = 19,
        BP2 = 20,
        BP3 = 21,
        BP4 = 22,
        BP5 = 23,
        BP6 = 24,

        BP7 = 2,
        BP8 = 3,
        BP9 = 4,
        BP10 = 5,
        BP11 = 6,
        BP12 = 7,
        BP13 = 8,
        BP14 = 9,
        BP15 = 10,

        BP16 = 25,
        BP17 = 26,
        BP18 = 27,
        BP19 = 28,
        BP20 = 29,
        BP21 = 30,
        BP22 = 31,
        BP23 = 32,
        BP24 = 33,
        BP25 = 34,
        BP26 = 35,
        BP27 = 36,

        BP28 = 13,
        BP29 = 14,
        BP30 = 15,
        BP31 = 18,
        BP32 = 16,
    };
    /// \endcond doxygen chokes on enum class inside of namespace

    const std::map<BasePlateType_t, std::string> basePlateMap =
    {
        {BasePlateType_t::UNAVAILABLE, "None"},
        {BasePlateType_t::CUSTOM, "Custom"},
        {BasePlateType_t::CRANIAL_WINDOW, "Cranial Window or No Lens"},
        {BasePlateType_t::BP1, "ProView Lens Probe 0.5 mm x 4.0 mm"},
        {BasePlateType_t::BP2, "Lens Probe 1.0 mm x 4.2 mm"},
        {BasePlateType_t::BP3, "Lens Probe 1.0 mm x 9.0 mm"},
        {BasePlateType_t::BP4, "Prism Probe 0.85 mm x 3.3 mm"},
        {BasePlateType_t::BP5, "Prism Probe 1.0 mm x 4.3 mm"},
        {BasePlateType_t::BP6, "Prism Probe 1.0 mm x 9.1 mm"},
        {BasePlateType_t::BP7, "ProView Integrated Lens 0.5 mm x 4.0 mm"},
        {BasePlateType_t::BP8, "ProView Integrated Lens 0.5 mm x 6.1 mm"},
        {BasePlateType_t::BP9, "ProView Integrated Lens 0.5 mm x 8.4 mm"},
        {BasePlateType_t::BP10, "ProView Integrated Lens 0.6 mm x 7.3 mm"},
        {BasePlateType_t::BP11, "ProView Integrated Lens 1.0 mm x 4.2 mm"},
        {BasePlateType_t::BP12, "ProView Integrated Lens 1.0 mm x 9.0 mm"},
        {BasePlateType_t::BP13, "ProView Integrated Lens 1.0 mm x 13.7 mm"},
        {BasePlateType_t::BP14, "ProView Prism Integrated Lens 1.0 mm x 4.3 mm"},
        {BasePlateType_t::BP15, "ProView Prism Integrated Lens 1.0 mm x 9.1 mm"},
        {BasePlateType_t::BP16, "Lens Probe 0.5 mm x 4.0 mm"},
        {BasePlateType_t::BP17, "Lens Probe 0.5 mm x 6.1 mm"},
        {BasePlateType_t::BP18, "Lens Probe 0.5 mm x 8.4 mm"},
        {BasePlateType_t::BP19, "Lens Probe 0.6 mm x 7.3 mm"},
        {BasePlateType_t::BP20, "ProView Lens Probe 0.5 mm x 6.1 mm"},
        {BasePlateType_t::BP21, "ProView Lens Probe 0.5 mm x 8.4 mm"},
        {BasePlateType_t::BP22, "ProView Lens Probe 0.6 mm x 7.3 mm"},
        {BasePlateType_t::BP23, "ProView Lens Probe 1.0 mm x 4.2 mm"},
        {BasePlateType_t::BP24, "ProView Lens Probe 1.0 mm x 9.0 mm"},
        {BasePlateType_t::BP25, "ProView Prism Probe 0.85 mm x 3.3 mm"},
        {BasePlateType_t::BP26, "ProView Prism Probe 1.0 mm x 4.3 mm"},
        {BasePlateType_t::BP27, "ProView Prism Probe 1.0 mm x 9.1 mm"},
        {BasePlateType_t::BP28, "ProView DC Integrated Lens 0.5 mm x 5.6 mm Li"},
        {BasePlateType_t::BP29, "ProView DC Integrated Lens 0.66 mm x 7.5 mm Li"},
        {BasePlateType_t::BP30, "ProView DC Integrated Lens 0.75 mm x 8.6 mm Li"},
        {BasePlateType_t::BP31, "ProView DC Integrated Lens 1.0 mm x 4.2 mm"},
        {BasePlateType_t::BP32, "ProView DC Integrated Lens 1.0 mm x 11.7 mm Li"},
    };

    /// Scaling is dependant upon efocus and the integrated base plate type. We store a mapping
    /// from the integrated base plate type to the microns/pixels scaling ratio at 0 and 200 efocus.
    /// Note: pixel scaling is only available for intergrated base plates
    const std::map<BasePlateType_t, std::pair<double, double>> integratedBasePlateToScaling=
        {
                {BasePlateType_t::CRANIAL_WINDOW, std::make_pair(0.82, 0.80)},

                {BasePlateType_t::BP1, std::make_pair(0.67,0.81)},

                {BasePlateType_t::BP7, std::make_pair(0.67,0.81)},
                {BasePlateType_t::BP8, std::make_pair(0.63,0.79)},
                {BasePlateType_t::BP9, std::make_pair(0.62,0.78)},
                {BasePlateType_t::BP10, std::make_pair(0.61,0.69)},
                {BasePlateType_t::BP11, std::make_pair(0.77,0.80)},
                {BasePlateType_t::BP12, std::make_pair(0.75,0.78)},
                {BasePlateType_t::BP13, std::make_pair(0.76,0.78)},
                {BasePlateType_t::BP14, std::make_pair(0.90,0.97)},
                {BasePlateType_t::BP15, std::make_pair(0.90,0.98)},

                {BasePlateType_t::BP20, std::make_pair(0.63,0.79)},
                {BasePlateType_t::BP21, std::make_pair(0.62,0.78)},
                {BasePlateType_t::BP22, std::make_pair(0.61,0.69)},
                {BasePlateType_t::BP23, std::make_pair(0.77,0.80)},
                {BasePlateType_t::BP24, std::make_pair(0.75,0.78)},
                {BasePlateType_t::BP25, std::make_pair(0.91,1.02)},
                {BasePlateType_t::BP26, std::make_pair(0.90,0.97)},
                {BasePlateType_t::BP27, std::make_pair(0.90,0.98)},

                {BasePlateType_t::BP28, std::make_pair(0.79,0.80)},
                {BasePlateType_t::BP29, std::make_pair(0.80,0.80)},
                {BasePlateType_t::BP30, std::make_pair(0.80,0.80)},
                {BasePlateType_t::BP31, std::make_pair(0.77, 0.80)},
                {BasePlateType_t::BP32, std::make_pair(0.81, 0.80)},
        };

    const std::map<std::string, BasePlateType_t> probeIdToBasePlate=
        {
            {"100-002172", BasePlateType_t::BP1},
            {"130-000143", BasePlateType_t::BP2},
            {"130-000304", BasePlateType_t::BP3},
            {"130-000248", BasePlateType_t::BP4},
            {"130-000247", BasePlateType_t::BP5},
            {"130-000444", BasePlateType_t::BP6},

            {"1050-004417", BasePlateType_t::BP7},
            {"1050-004415", BasePlateType_t::BP8},
            {"1050-004414", BasePlateType_t::BP9},
            {"1050-004413", BasePlateType_t::BP10},
            {"1050-004637", BasePlateType_t::BP11},
            {"1050-004416", BasePlateType_t::BP12},
            {"1050-004418", BasePlateType_t::BP13},
            {"1050-004419", BasePlateType_t::BP14},
            {"1050-004420", BasePlateType_t::BP15},

            {"1050-002181", BasePlateType_t::BP16},
            {"1050-002182", BasePlateType_t::BP17},
            {"1050-002183", BasePlateType_t::BP18},
            {"1050-002179", BasePlateType_t::BP19},
            {"1050-002211", BasePlateType_t::BP20},
            {"1050-002212", BasePlateType_t::BP21},
            {"1050-002208", BasePlateType_t::BP22},
            {"1050-002202", BasePlateType_t::BP23},
            {"1050-002214", BasePlateType_t::BP24},
            {"1050-002204", BasePlateType_t::BP25},
            {"1050-002203", BasePlateType_t::BP26},
            {"1050-002213", BasePlateType_t::BP27},

            {"1050-005441", BasePlateType_t::BP28},
            {"1050-005442", BasePlateType_t::BP29},
            {"1050-005443", BasePlateType_t::BP30},
            {"1050-005473", BasePlateType_t::BP31},
            {"1050-005475", BasePlateType_t::BP32},
        };

    /// \cond doxygen chokes on enum class inside of namespace
    /// Vector specifying the order the baseplates should be listed in the metadata view
    const std::vector<BasePlateType_t> basePlateOrder =
    {
        BasePlateType_t::UNAVAILABLE,
        BasePlateType_t::CUSTOM,
        BasePlateType_t::CRANIAL_WINDOW,
        BasePlateType_t::BP1,
        BasePlateType_t::BP2,
        BasePlateType_t::BP3,
        BasePlateType_t::BP4,
        BasePlateType_t::BP5,
        BasePlateType_t::BP6,
        BasePlateType_t::BP7,
        BasePlateType_t::BP8,
        BasePlateType_t::BP9,
        BasePlateType_t::BP10,
        BasePlateType_t::BP11,
        BasePlateType_t::BP12,
        BasePlateType_t::BP13,
        BasePlateType_t::BP14,
        BasePlateType_t::BP15,
        BasePlateType_t::BP16,
        BasePlateType_t::BP17,
        BasePlateType_t::BP18,
        BasePlateType_t::BP19,
        BasePlateType_t::BP20,
        BasePlateType_t::BP21,
        BasePlateType_t::BP22,
        BasePlateType_t::BP23,
        BasePlateType_t::BP24,
        BasePlateType_t::BP25,
        BasePlateType_t::BP26,
        BasePlateType_t::BP27,
        BasePlateType_t::BP28,
        BasePlateType_t::BP29,
        BasePlateType_t::BP30,
        BasePlateType_t::BP31,
        BasePlateType_t::BP32,
    };
    /// \endcond doxygen chokes on enum class inside of namespace

    /// Struct for cell-set-specific metadata
    struct CellSetMetadata
    {
        /// empty constructor
        CellSetMetadata()
        {
        }

        /// fully specified constructor
        CellSetMetadata(
            const CellSetMethod_t method,
            const CellSetType_t type,
            const CellSetUnits_t units)
        : m_method(method)
        , m_type(type)
        , m_units(units)
        {
        }

        CellSetMethod_t  m_method = CellSetMethod_t::UNAVAILABLE;  ///< method used to generate the cell set
        CellSetType_t  m_type = CellSetType_t::UNAVAILABLE;        ///< type of footprints in the cell set
        CellSetUnits_t  m_units = CellSetUnits_t::UNAVAILABLE;     ///< units of the traces in the cell set
    };

    /// Struct for vessel-set-specific metadata
    struct VesselSetMetadata
    {
        /// empty constructor
        VesselSetMetadata()
        {
        }

        /// fully specified constructor
        VesselSetMetadata(
            const VesselSetUnits_t units,
            const ProjectionType projectionType,
            const double timeWindow,
            const double timeIncrement,
            const double inputMovieFps = 0.0,
            const std::map<std::string, std::vector<int>> clippedVessels = {},
            const std::map<std::string, std::vector<int>> noSignificantVessels = {},
            const std::map<std::string, std::vector<int>> directionChangedVessels = {}
            )
            : m_units(units)
            , m_projectionType(projectionType)
            , m_timeWindow(timeWindow)
            , m_timeIncrement(timeIncrement)
            , m_inputMovieFps(inputMovieFps)
            , m_clippedVessels(clippedVessels)
            , m_noSignificantVessels(noSignificantVessels)
            , m_directionChangedVessels(directionChangedVessels)
        {
        }

        VesselSetUnits_t m_units;        ///< units of the traces in the vessel set
        ProjectionType m_projectionType; ///< type of projection stored in the vessel set
        double m_timeWindow;             ///< the length of the time window in seconds
        double m_timeIncrement;          ///< the length of the time increment in seconds
        double m_inputMovieFps;          ///< the fps of the input movie
        std::map<std::string, std::vector<int>> m_clippedVessels; ///< map of vessel to timepoints where clipping occurred in the rbc algo
        std::map<std::string, std::vector<int>> m_noSignificantVessels; ///< map of vessel to timepoints where no signficant pixels were detected in the rbc algo
        std::map<std::string, std::vector<int>> m_directionChangedVessels; ///< map of vessel to timepoints where direction travelled significant changed between offsets
    };

    /// Struct for holding pre-motion-correction metadata
    struct PreMotionCorrMetadata
    {
        /// empty constructor
        PreMotionCorrMetadata()
        {
        }

        /// fully specified constructor
        PreMotionCorrMetadata(
            const size_t topLeftX,
            const size_t topLeftY,
            const size_t width,
            const size_t height)
            : m_topLeftX(topLeftX)
            , m_topLeftY(topLeftY)
            , m_width(width)
            , m_height(height)
        {
        }

        size_t m_topLeftX = 0;   ///< top left corner x coordinate
        size_t m_topLeftY = 0;   ///< top left corner y coordinate
        size_t m_width;          ///< width of the data prior to motion correction
        size_t m_height;         ///< height of the data prior to motion correction
    };

    /// Struct for holding pre-processing metadata
    struct PreprocessMetadata
    {
        /// empty constructor
        PreprocessMetadata()
        {
        }

        /// fully specified constructor
        PreprocessMetadata(
                size_t spatialDs,
                size_t temporalDs)
                : m_spatialDs(spatialDs)
                , m_temporalDs(temporalDs)
        {
        }

        size_t m_spatialDs = 1;        ///< spatial downsampling factor
        size_t m_temporalDs = 1;       ///< temportal downsampling factor
    };

    template <typename T>
    nlohmann::json getExtraPropertiesJSON(T & inData)
    {
        using json = nlohmann::json;
        json extraProps = json::parse(inData->getExtraProperties());
        if (!extraProps["idps"].is_null() && !extraProps["idps"].is_object() && extraProps["idps"].is_string())
        {
            extraProps["idps"] = json::object();
        }
        return extraProps;
    }

    // Getters
    inline std::string getCellSetMethodString(CellSetMethod_t cellSetMethod)
    {
        switch(cellSetMethod)
        {
            case CellSetMethod_t::PCAICA:
                return "pca-ica";
            case CellSetMethod_t::CNMFE:
                return "cnmfe";
            case CellSetMethod_t::MANUAL:
                return "manual";
            case CellSetMethod_t::APPLIED:
                return "applied";
            case CellSetMethod_t::UNAVAILABLE:
                return "";
            default:
                return "";
        }
    }

    inline std::string getCellSetTypeString(CellSetType_t cellSetType)
    {
        switch(cellSetType)
        {
            case CellSetType_t::ANALOG:
                return "analog";
            case CellSetType_t::BINARY:
                return "binary";
            case CellSetType_t::UNAVAILABLE:
                return "";
            default:
                return "";
        }
    }

    inline std::string getCellSetUnitsString(CellSetUnits_t cellSetUnits)
    {
        switch(cellSetUnits)
        {
            case CellSetUnits_t::RAW:
                return "raw";
            case CellSetUnits_t::DF_OVER_F:
                return "dF over F";
            case CellSetUnits_t::DF:
                return "dF";
            case CellSetUnits_t::DF_OVER_NOISE:
                return "dF over noise";
            case CellSetUnits_t::UNAVAILABLE:
                return "";
            default:
                return "";
        }
    }

    inline std::string getVesselSetTypeString(VesselSetType_t vesselSetType)
    {
        switch(vesselSetType)
        {
            case VesselSetType_t::VESSEL_DIAMETER:
                return "vessel diameter";
            case VesselSetType_t::RBC_VELOCITY:
                return "red blood cell velocity";
            default:
                return "";
        }
    }

    inline std::string getVesselSetUnitsString(VesselSetUnits_t vesselSetUnits, const bool inShorthand = false)
    {
        switch(vesselSetUnits)
        {
            case VesselSetUnits_t::PIXELS:
                return inShorthand ? "px" : "pixels";
            case VesselSetUnits_t::MICRONS:
                return inShorthand ? "um" : "microns";
            case VesselSetUnits_t::PIXELS_PER_SECOND:
                return inShorthand ? "px/s" : "pixels per second";
            case VesselSetUnits_t::MICRONS_PER_SECOND:
                return inShorthand ? "um/s" : "microns per second";
            default:
                return "";
        }
    }

    inline std::string getVesselSetProjectionTypeString(ProjectionType projectionType)
    {
        switch(projectionType)
        {
            case ProjectionType::MEAN:
                return "mean";
            case ProjectionType::MAX:
                return "max";
            case ProjectionType::MIN:
                return "min";
            case ProjectionType::STANDARD_DEVIATION:
                return "standard deviation";
            default:
                return "";
        }
    }

    template <class T>
    CellSetMethod_t getCellSetMethod(T & inData)
    {
        using json = nlohmann::json;
        json extraProps = getExtraPropertiesJSON(inData);
        if (!extraProps["idps"]["cellset"]["method"].is_null())
        {
            std::string method = extraProps["idps"]["cellset"]["method"].get<std::string>();
            if (method == "pca-ica") return CellSetMethod_t::PCAICA;
            if (method == "cnmfe") return CellSetMethod_t::CNMFE;
            if (method == "manual") return CellSetMethod_t::MANUAL;
            if (method == "applied") return CellSetMethod_t::APPLIED;
        }
        return CellSetMethod_t::UNAVAILABLE;
    }

    template <class T>
    CellSetType_t getCellSetType(T & inData)
    {
        using json = nlohmann::json;
        json extraProps = getExtraPropertiesJSON(inData);
        if (!extraProps["idps"]["cellset"]["type"].is_null())
        {
            std::string method = extraProps["idps"]["cellset"]["type"].get<std::string>();
            if (method == "analog") return CellSetType_t::ANALOG;
            if (method == "binary") return CellSetType_t::BINARY;
        }
        return CellSetType_t::UNAVAILABLE;
    }

    template <class T>
    CellSetUnits_t getCellSetUnits(T & inData)
    {
        using json = nlohmann::json;
        json extraProps = getExtraPropertiesJSON(inData);
        if (!extraProps["idps"]["cellset"]["units"].is_null())
        {
            std::string method = extraProps["idps"]["cellset"]["units"].get<std::string>();
            if (method == "raw") return CellSetUnits_t::RAW;
            if (method == "dF over F") return CellSetUnits_t::DF_OVER_F;
            if (method == "dF") return CellSetUnits_t::DF;
            if (method == "dF over noise") return CellSetUnits_t::DF_OVER_NOISE;
        }
        return CellSetUnits_t::UNAVAILABLE;
    }

    template <class T>
    PreprocessMetadata getPreprocessMetadata(T & inData)
    {
        using json = nlohmann::json;
        json extraProps = getExtraPropertiesJSON(inData);

        PreprocessMetadata preprocessMetadata;

        if (!extraProps["idps"]["spatialDownsampling"].is_null()) {

            preprocessMetadata.m_spatialDs = extraProps["idps"]["spatialDownsampling"].get<size_t>();
        }
        if (!extraProps["idps"]["temporalDownsampling"].is_null()) {
            preprocessMetadata.m_temporalDs = extraProps["idps"]["temporalDownsampling"].get<size_t>();
        }

        return preprocessMetadata;
    }

    template <typename T>
    std::vector<size_t> getInterpolatedFrames(T & inData)
    {
        using json = nlohmann::json;
        json extraProps = getExtraPropertiesJSON(inData);

        if (extraProps["idps"]["interpolatedFrames"].is_null())
        {
            ISX_THROW(ExceptionUserInput, "No interpolated frames in metadata");
        }
        return extraProps["idps"]["interpolatedFrames"].get<std::vector<size_t>>();
    }

    template <class T>
    VesselSetType_t getVesselSetType(T & inData)
    {
        using json = nlohmann::json;
        json extraProps = getExtraPropertiesJSON(inData);
        if (!extraProps["idps"]["vesselset"]["type"].is_null())
        {
            std::string method = extraProps["idps"]["vesselset"]["type"].get<std::string>();
            if (method == "vessel diameter")
            {
                return VesselSetType_t::VESSEL_DIAMETER;
            }
            else if (method == "red blood cell velocity")
            {
                return VesselSetType_t::RBC_VELOCITY;
            }
        }

        // default
        return VesselSetType_t::VESSEL_DIAMETER;
    }

    template <class T>
    VesselSetUnits_t getVesselSetUnits(T & inData)
    {
        using json = nlohmann::json;
        json extraProps = getExtraPropertiesJSON(inData);
        if (!extraProps["idps"]["vesselset"]["units"].is_null())
        {
            std::string method = extraProps["idps"]["vesselset"]["units"].get<std::string>();
            if (method == "pixels")
            {
                return VesselSetUnits_t::PIXELS;
            }
            else if (method == "microns")
            {
                return VesselSetUnits_t::MICRONS;
            }
            else if (method == "pixels per second")
            {
                return VesselSetUnits_t::PIXELS_PER_SECOND;
            }
            else if (method == "microns per second")
            {
                return VesselSetUnits_t::MICRONS_PER_SECOND;
            }
        }

        // default
        return VesselSetUnits_t::PIXELS;
    }

    template <class T>
    ProjectionType getVesselSetProjectionType(T & inData)
    {
        using json = nlohmann::json;
        json extraProps = getExtraPropertiesJSON(inData);
        if (!extraProps["idps"]["vesselset"]["projectionType"].is_null())
        {
            std::string projectionType = extraProps["idps"]["vesselset"]["projectionType"].get<std::string>();
            if (projectionType == "mean")
            {
                return ProjectionType::MEAN;
            }
            else if (projectionType == "max")
            {
                return ProjectionType::MAX;
            }
            else if (projectionType == "min")
            {
                return ProjectionType::MIN;
            }
            else if (projectionType == "standard deviation")
            {
                return ProjectionType::STANDARD_DEVIATION;
            }
        }

        // default
        return ProjectionType::STANDARD_DEVIATION;
    }

    template <class T>
    double getVesselSetTimeWindow(T & inData)
    {
        using json = nlohmann::json;
        json extraProps = getExtraPropertiesJSON(inData);
        if (extraProps["idps"]["vesselset"]["timeWindow"].is_null())
        {
            ISX_THROW(ExceptionUserInput, "No time window in vessel set metadata");
        }
        return extraProps["idps"]["vesselset"]["timeWindow"].get<double>();
        
    }

    template <class T>
    double getVesselSetTimeIncrement(T & inData)
    {
        using json = nlohmann::json;
        json extraProps = getExtraPropertiesJSON(inData);
        if (extraProps["idps"]["vesselset"]["timeIncrement"].is_null())
        {
            ISX_THROW(ExceptionUserInput, "No time increment in vessel set metadata");
        }
        return extraProps["idps"]["vesselset"]["timeIncrement"].get<double>();
    }

    template <class T>
    double getVesselSetInputMovieFps(T & inData)
    {
        using json = nlohmann::json;
        json extraProps = getExtraPropertiesJSON(inData);
        if (extraProps["idps"]["vesselset"]["inputMovieFps"].is_null())
        {
            ISX_THROW(ExceptionUserInput, "No input movie fps in vessel set metadata");
        }
        return extraProps["idps"]["vesselset"]["inputMovieFps"].get<double>();
    }

    inline std::string getVesselTimepointsString(std::map<std::string, std::vector<int>> inData)
    {
        using json = nlohmann::json;
        json j;

        for (auto it = inData.cbegin(); it != inData.cend(); it++) {
            j[it->first] = it->second;
        }
        return j.dump();
        
    }

    template <class T>
    BasePlateType_t getBasePlateType(T & inData)
    {
        using json = nlohmann::json;
        json extraProps = getExtraPropertiesJSON(inData);
        if (!extraProps["idps"]["integratedBasePlate"].is_null())
        {
            // read IDPS metadata
            std::string ibp = extraProps["idps"]["integratedBasePlate"].get<std::string>();
            return static_cast<BasePlateType_t>(stoi(ibp));
        }
        else if (!extraProps["probe"].is_null())
        {
            // read IDAS metadata
            std::string name = extraProps["probe"]["name"].get<std::string>();
            ISX_ASSERT(!name.empty());

            BasePlateType_t probeType;
            if (name == "Custom")
            {
                probeType = BasePlateType_t::CUSTOM;
            }
            else if (name == "None")
            {
                probeType = BasePlateType_t::UNAVAILABLE;
            }
            else if (name == "Cranial Window or No Lens")
            {
                probeType = BasePlateType_t::CRANIAL_WINDOW;
            }
            else // Integrated lens
            {
                std::string probeId = extraProps["probe"]["id"].get<std::string>();
                const bool probeMappingExists = probeIdToBasePlate.find(probeId) != probeIdToBasePlate.end();
                ISX_ASSERT(probeMappingExists, "Failed to map IDAS probe ID " + probeId + " to a base plate type in IDPS.");
                if (probeMappingExists)
                {
                    probeType = probeIdToBasePlate.at(probeId);
                }
                else
                {
                    probeType = BasePlateType_t::UNAVAILABLE;
                }
            }
            return probeType;
        }
        return BasePlateType_t::UNAVAILABLE;
    }

    template <class T>
    uint16_t getEfocus(const T & inData)
    {
        /*
        * Read efocus value. If not from a multiplane movie,
        *  get focus value from microscope key
        */
        using json = nlohmann::json;
        json extraProps = getExtraPropertiesJSON(inData);
        auto idps = extraProps.find("idps");
        if (idps != extraProps.end())
        {
            auto efocus = idps->find("efocus");
            if (efocus != idps->end())
            {
                return *efocus;
            }
        }

        auto microscope = extraProps.find("microscope");
        if (microscope != extraProps.end())
        {
            auto efocus = microscope->find("focus");
            if (efocus != microscope->end())
            {
                return *efocus;
            }
        }

        return 0;
    }

    template<class T>
    std::vector<uint16_t> getEfocusSeries(const std::vector<T> & inDataSeries)
    {
        std::vector<uint16_t> efocusSeries;
        for (auto & data : inDataSeries)
        {
            efocusSeries.push_back(getEfocus(data));
        }
        return efocusSeries;
    }

    template<class T>
    bool hasPixelScaling(T & inData)
    {
        using json = nlohmann::json;
        json extraProps = getExtraPropertiesJSON(inData);

        BasePlateType_t basePlateType = getBasePlateType(inData);
        return integratedBasePlateToScaling.find(basePlateType) != integratedBasePlateToScaling.end();
    }

    template <class T>
    double getMicronsPerPixel(T & inData)
    {
        using json = nlohmann::json;
        json extraProps = getExtraPropertiesJSON(inData);

        uint16_t efocus = getEfocus(inData);
        BasePlateType_t basePlateType = getBasePlateType(inData);
        if (integratedBasePlateToScaling.find(basePlateType) == integratedBasePlateToScaling.end())
        {
            ISX_THROW(isx::ExceptionUserInput, "No pixel scaling for non-integrated base plate");
        }

        std::pair<double, double> efocusData = integratedBasePlateToScaling.at(basePlateType);

        // Linearly interpolate the efocus scale factor
        double micronsPerPixel = ((efocusData.second - efocusData.first) / 200) * (efocus) + efocusData.first;

        // Ratio must not be zero
        if (micronsPerPixel == 0)
        {
            ISX_THROW(isx::Exception, "Unit ratio of microns per pixel is zero. Unable to convert between units");
        }

        // Account for spatial downsampling factor in conversion ratio
        micronsPerPixel *= getSpatialDownSamplingFactor(inData);
        return micronsPerPixel;
    }

    template <class T>
    PreMotionCorrMetadata getPreMotionCorrMetadata(T & inData)
    {
        PreMotionCorrMetadata preMotionCorrMetadata;
        using json = nlohmann::json;
        json extraProps = getExtraPropertiesJSON(inData);
        if (!extraProps["idps"]["pre_mc"].is_null())
        {
            preMotionCorrMetadata.m_topLeftX = extraProps["idps"]["pre_mc"]["topLeft"]["x"].get<size_t>();
            preMotionCorrMetadata.m_topLeftY = extraProps["idps"]["pre_mc"]["topLeft"]["y"].get<size_t>();
            preMotionCorrMetadata.m_width = extraProps["idps"]["pre_mc"]["width"].get<size_t>();
            preMotionCorrMetadata.m_height = extraProps["idps"]["pre_mc"]["height"].get<size_t>();
        }
        return preMotionCorrMetadata;
    }

    template <class T>
    size_t getSpatialDownSamplingFactor(T & inData)
    {
        size_t downSamplingFactor = 1;

        // read from IDAS
        using json = nlohmann::json;
        json extraProps = getExtraPropertiesJSON(inData);
        if (extraProps.find("microscope") != extraProps.end())
        {
            if (!extraProps["microscope"]["downSamplingFactor"].is_null())
            {
                downSamplingFactor *= extraProps["microscope"]["downSamplingFactor"].get<size_t>();
            }
        }        

        // read from IDPS
        PreprocessMetadata preProcessMetadata = getPreprocessMetadata(inData);
        downSamplingFactor *= preProcessMetadata.m_spatialDs;

        return downSamplingFactor;
    }

    template <class T>
    bool getMotionCorrPadding(T & inData)
    {
        using json = nlohmann::json;
        json extraProps = getExtraPropertiesJSON(inData);
        if (!extraProps["idps"]["pre_mc"].is_null()) {
            return extraProps["idps"]["mc_padding"].get<bool>();
        }

        ISX_THROW(isx::Exception, "Metadata for motion correction padding not found.");
    }

    // Setters
    template <typename T>
    void setCellSetMethod(T & inData, CellSetMethod_t cellSetMethod)
    {
        if (cellSetMethod == CellSetMethod_t::UNAVAILABLE) return;
        using json = nlohmann::json;
        json extraProps = getExtraPropertiesJSON(inData);
        extraProps["idps"]["cellset"]["method"] = getCellSetMethodString(cellSetMethod);
        inData->setExtraProperties(extraProps.dump());
    }

    template <typename T>
    void setCellSetType(T & inData, CellSetType_t cellSetType)
    {
        if (cellSetType == CellSetType_t::UNAVAILABLE) return;
        using json = nlohmann::json;
        json extraProps = getExtraPropertiesJSON(inData);
        extraProps["idps"]["cellset"]["type"] = getCellSetTypeString(cellSetType);
        inData->setExtraProperties(extraProps.dump());
    }

    template <typename T>
    void setCellSetUnits(T & inData, CellSetUnits_t cellSetUnits)
    {
        if (cellSetUnits == CellSetUnits_t::UNAVAILABLE) return;
        using json = nlohmann::json;
        json extraProps = getExtraPropertiesJSON(inData);
        extraProps["idps"]["cellset"]["units"] = getCellSetUnitsString(cellSetUnits);
        inData->setExtraProperties(extraProps.dump());
    }

    template <typename T>
    void setCellSetMetadata(T & inData, CellSetMetadata cellSetMetadata)
    {
        setCellSetMethod(inData, cellSetMetadata.m_method);
        setCellSetType(inData, cellSetMetadata.m_type);
        setCellSetUnits(inData, cellSetMetadata.m_units);
    }


    template <typename T>
    void setPreMotionCorrMetadata(T & inData, PreMotionCorrMetadata preMotionCorrMetadata)
    {
        using json = nlohmann::json;
        json extraProps = getExtraPropertiesJSON(inData);
        extraProps["idps"]["pre_mc"]["topLeft"]["x"] = preMotionCorrMetadata.m_topLeftX;
        extraProps["idps"]["pre_mc"]["topLeft"]["y"] = preMotionCorrMetadata.m_topLeftY;
        extraProps["idps"]["pre_mc"]["width"] = preMotionCorrMetadata.m_width;
        extraProps["idps"]["pre_mc"]["height"] = preMotionCorrMetadata.m_height;
        inData->setExtraProperties(extraProps.dump());
    }

    template <typename T>
    void setBasePlateType(T & inData, BasePlateType_t basePlateType) {
        using json = nlohmann::json;
        json extraProps = getExtraPropertiesJSON(inData);

        size_t index = size_t(basePlateType);
        std::string basePlateString = std::to_string(index);

        // Pad with zeros
        std::string zeros(std::to_string(basePlateMap.size() - 1).size() - basePlateString.size(), '0');
        basePlateString.insert(0, zeros);

        extraProps["idps"]["integratedBasePlate"] = basePlateString;

        inData->setExtraProperties(extraProps.dump());
    }

    template <typename T1, typename T2>
    void setPreprocessMetadata(T1 & inDataSrc, T2 & inDataDest, isize_t spatialDsFactor, isize_t temporalDsFactor)
    {
        using json = nlohmann::json;
        json extraProps = getExtraPropertiesJSON(inDataSrc);

        if (!extraProps["idps"]["spatialDownsampling"].is_null())
        {
            extraProps["idps"]["spatialDownsampling"] = extraProps["idps"]["spatialDownsampling"].get<size_t>() * spatialDsFactor;
        }
        else
        {
            extraProps["idps"]["spatialDownsampling"] = spatialDsFactor;
        }

        if (!extraProps["idps"]["temporalDownsampling"].is_null())
        {
            extraProps["idps"]["temporalDownsampling"] = extraProps["idps"]["temporalDownsampling"].get<size_t>() * temporalDsFactor;
        }
        else
        {
            extraProps["idps"]["temporalDownsampling"] = temporalDsFactor;
        }

        inDataDest->setExtraProperties(extraProps.dump());
    }


    template <typename T>
    std::string setInterpolatedFrames(T & inData, std::vector<isx::isize_t> interpolatedFrames)
    {
        using json = nlohmann::json;
        json extraProps = getExtraPropertiesJSON(inData);

        extraProps["idps"]["interpolatedFrames"] = interpolatedFrames;
        return extraProps.dump();
    }

    // Helper functions to deal with string representations of the extra properties
    template <typename T>
    std::string addCellSetMetadata(T & inData, CellSetMetadata cellSetMetadata)
    {
        using json = nlohmann::json;
        json extraProps = getExtraPropertiesJSON(inData);

        if (cellSetMetadata.m_method != CellSetMethod_t ::UNAVAILABLE)
        {
            extraProps["idps"]["cellset"]["method"] = getCellSetMethodString(cellSetMetadata.m_method);
        }

        if (cellSetMetadata.m_type != CellSetType_t::UNAVAILABLE)
        {
            extraProps["idps"]["cellset"]["type"] = getCellSetTypeString(cellSetMetadata.m_type);
        }

        if (cellSetMetadata.m_units != CellSetUnits_t::UNAVAILABLE)
        {
            extraProps["idps"]["cellset"]["units"] = getCellSetUnitsString(cellSetMetadata.m_units);
        }

        return extraProps.dump();
    }

    template <typename T>
    std::string setVesselSetMetadata(T & inData, VesselSetMetadata vesselSetMetadata)
    {
        using json = nlohmann::json;
        json extraProps = getExtraPropertiesJSON(inData);
        extraProps["idps"]["vesselset"]["units"] = getVesselSetUnitsString(vesselSetMetadata.m_units);
        extraProps["idps"]["vesselset"]["projectionType"] = getVesselSetProjectionTypeString(vesselSetMetadata.m_projectionType);
        extraProps["idps"]["vesselset"]["timeWindow"] = vesselSetMetadata.m_timeWindow;
        extraProps["idps"]["vesselset"]["timeIncrement"] = vesselSetMetadata.m_timeIncrement;
        if (vesselSetMetadata.m_inputMovieFps > 0.0)
        {
            extraProps["idps"]["vesselset"]["inputMovieFps"] = vesselSetMetadata.m_inputMovieFps;
        }
        if (!vesselSetMetadata.m_clippedVessels.empty())
        {
            extraProps["idps"]["vesselset"]["clippedVessels"] = getVesselTimepointsString(vesselSetMetadata.m_clippedVessels);
        }
        if (!vesselSetMetadata.m_noSignificantVessels.empty())
        {
            extraProps["idps"]["vesselset"]["noSignificantVessels"] = getVesselTimepointsString(vesselSetMetadata.m_noSignificantVessels);
        }
        if (!vesselSetMetadata.m_directionChangedVessels.empty())
        {
            extraProps["idps"]["vesselset"]["directionChangedVessels"] = getVesselTimepointsString(vesselSetMetadata.m_directionChangedVessels);
        }
        return extraProps.dump();
    }

    template <typename Src, typename Dst>
    void transferPreMotionCorrMetadata(Src & src, Dst & dst)
    {
        using json = nlohmann::json;
        json extraProps = getExtraPropertiesJSON(src);
        dst->setExtraProperties(extraProps.dump());

        PreMotionCorrMetadata preMotionCorrMetadata(
            static_cast<size_t>(src->getSpacingInfo().getTopLeft().getX().toDouble()),
            static_cast<size_t>(src->getSpacingInfo().getTopLeft().getY().toDouble()),
            src->getSpacingInfo().getNumPixels().getWidth(),
            src->getSpacingInfo().getNumPixels().getHeight());
        setPreMotionCorrMetadata(dst, preMotionCorrMetadata);
    }

    template <typename Dst>
    void setMotionCorrPadding(Dst & dst, const bool value)
    {
        using json = nlohmann::json;
        json extraProps = getExtraPropertiesJSON(dst);
        extraProps["idps"]["mc_padding"] = value;
        dst->setExtraProperties(extraProps.dump());
    }

    // Multicolor metadata
    inline bool hasMulticolorMetadata(std::string inExtraProperties)
    {
        using json = nlohmann::json;
        json extraProps = json::parse(inExtraProperties);
        return !extraProps["microscope"]["dualColor"].is_null();
    }

    template <typename T>
    inline bool hasMulticolorMetadata(const T & inData)
    {
        using json = nlohmann::json;
        json extraProps = getExtraPropertiesJSON(inData);
        return !extraProps["microscope"]["dualColor"].is_null();
    }

    template <typename T>
    inline std::string getMulticolorChannelName(const T & inData, const std::string defaultChannelName)
    {
        using json = nlohmann::json;
        json extraProps = getExtraPropertiesJSON(inData);

        // IDPS metadata
        if (!extraProps["idps"]["channel"].is_null())
        {
            return extraProps["idps"]["channel"].get<std::string>();
        }

        // IDAS metadata
        if (!extraProps["microscope"]["dualColor"]["enabled"].is_null() && extraProps["microscope"]["dualColor"]["mode"].get<std::string>()=="single")
        {
            return extraProps["microscope"]["dualColor"]["single"]["channel"].get<std::string>();
        }

        return defaultChannelName;
    }

    template <typename T>
    bool isMulticolorMultiplexingData(const T & inData)
    {
        using json = nlohmann::json;
        json extraProps = getExtraPropertiesJSON(inData);
        if (!extraProps["microscope"]["dualColor"]["mode"].is_null())
        {
            return extraProps["microscope"]["dualColor"]["mode"].get<std::string>() == "multiplexing";
        }
        return false;
    }

    template <typename T>
    bool requiresResizingAfterMotionCorr(const T & inData)
    {
        using json = nlohmann::json;
        json extraProps = getExtraPropertiesJSON(inData);

        bool hasPreMotionCorrMetadata = !extraProps["idps"]["pre_mc"].is_null();
        bool hasMotionCorrPaddingMetadata = !extraProps["idps"]["mc_padding"].is_null();

        return (!hasMotionCorrPaddingMetadata && hasPreMotionCorrMetadata) 
            || (hasMotionCorrPaddingMetadata && !getMotionCorrPadding(inData));
    }

    template <typename T>
    std::vector<std::pair<MulticolorChannel_t, size_t>> getMulticolorMuxRatio(const T & inData)
    {
        using json = nlohmann::json;
        json extraProps = getExtraPropertiesJSON(inData);

        if (!extraProps["microscope"]["dualColor"]["muxRatio"].is_null())
        {
            MulticolorChannel_t startChannel = (extraProps["microscope"]["dualColor"]["startChannel"].get<std::string>() == "green") ? MulticolorChannel_t::GREEN : MulticolorChannel_t::RED;
            if (startChannel == MulticolorChannel_t::GREEN)
            {
                return {
                    {MulticolorChannel_t::GREEN, extraProps["microscope"]["dualColor"]["muxRatio"]["led1"].get<size_t>()},
                    {MulticolorChannel_t::RED, extraProps["microscope"]["dualColor"]["muxRatio"]["led2"].get<size_t>()}
                };
            }
            else
            {
                return {
                    {MulticolorChannel_t::RED, extraProps["microscope"]["dualColor"]["muxRatio"]["led2"].get<size_t>()},
                    {MulticolorChannel_t::GREEN, extraProps["microscope"]["dualColor"]["muxRatio"]["led1"].get<size_t>()}
                };
            }
        }

        // assume default 1:1 mux ratio for movies from older versions of IDAS
        return {{MulticolorChannel_t::GREEN, 1}, {MulticolorChannel_t::RED, 1}};;
    }
} // namespace isx

#endif // ISX_METADATA_H
