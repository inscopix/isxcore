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
    /// Type of integrated base plate unit used to capture data
    enum class IntegratedBasePlateType_t
    {
        UNAVAILABLE = 0,
        CUSTOM,
        IBP1,
        IBP2,
        IBP3,
        IBP4,
        IBP5,
        IBP6,
        IBP7,
        IBP8,
        IBP9,
        IBP10,
        IBP11,
        IBP12,
        IBP13,
        IBP14,
        IBP15,
        CRANIAL_WINDOW
    };
    /// \endcond doxygen chokes on enum class inside of namespace

    const std::map<IntegratedBasePlateType_t, std::string> integratedBasePlateMap =
    {
        {IntegratedBasePlateType_t::UNAVAILABLE, "None"},
        {IntegratedBasePlateType_t::CUSTOM, "Custom"},
        {IntegratedBasePlateType_t::IBP1, "0.5mm x 4.0mm"},
        {IntegratedBasePlateType_t::IBP2, "0.5mm x 6.1mm"},
        {IntegratedBasePlateType_t::IBP3, "0.5mm x 8.4mm"},
        {IntegratedBasePlateType_t::IBP4, "0.6mm x 7.3mm"},
        {IntegratedBasePlateType_t::IBP5, "1.0mm x 4.0mm"},
        {IntegratedBasePlateType_t::IBP6, "1.0mm x 9.0mm"},
        {IntegratedBasePlateType_t::IBP7, "1.0mm x 13.7mm"},
        {IntegratedBasePlateType_t::IBP8, "Prism 1.0mm x 4.3mm"},
        {IntegratedBasePlateType_t::IBP9, "Prism 1.0mm x 9.1mm"},
        {IntegratedBasePlateType_t::IBP10, "Mouse Dorsal Striatum Camk2a (1.0mm x 4.0mm)"},
        {IntegratedBasePlateType_t::IBP11, "Mouse Dorsal Striatum CAG.Flex (1.0mm x 4.0mm )"},
        {IntegratedBasePlateType_t::IBP12, "0.5mm x 5.6mm"},
        {IntegratedBasePlateType_t::IBP13, "0.66mmx 7.5mm"},
        {IntegratedBasePlateType_t::IBP14, "0.75mm x 8.65mm"},
        {IntegratedBasePlateType_t::IBP15, "1.0mm x 11.7mm"},
        {IntegratedBasePlateType_t::CRANIAL_WINDOW, "Cranial Window or No Lens"}
    };

    /// Scaling is dependant upon efocus and the integrated base plate type. We store a mapping
    /// from the integrated base plate type to the microns/pixels scaling ratio at 0 and 200 efocus.
    const std::map<IntegratedBasePlateType_t, std::pair<double, double>> integratedBasePlateToScaling=
        {
                {IntegratedBasePlateType_t::UNAVAILABLE, std::make_pair(0,0)},
                {IntegratedBasePlateType_t::CUSTOM, std::make_pair(0,0)},
                {IntegratedBasePlateType_t::IBP1, std::make_pair(0.672,0.812)},
                {IntegratedBasePlateType_t::IBP2, std::make_pair(0.626,0.788)},
                {IntegratedBasePlateType_t::IBP3, std::make_pair(0.621,0.780)},
                {IntegratedBasePlateType_t::IBP4, std::make_pair(0.612,0.686)},
                {IntegratedBasePlateType_t::IBP5, std::make_pair(0.733,0.796)},
                {IntegratedBasePlateType_t::IBP6, std::make_pair(0.745,0.780)},
                {IntegratedBasePlateType_t::IBP7, std::make_pair(0.745,0.780)},
                {IntegratedBasePlateType_t::IBP8, std::make_pair(0.901,0.970)},
                {IntegratedBasePlateType_t::IBP9, std::make_pair(0.901,0.982)},
                {IntegratedBasePlateType_t::IBP10, std::make_pair(0.733,0.796)},
                {IntegratedBasePlateType_t::IBP11, std::make_pair(0.733,0.796)},
                {IntegratedBasePlateType_t::IBP12, std::make_pair(0.788,0.796)},
                {IntegratedBasePlateType_t::IBP13, std::make_pair(0.804,0.796)},
                {IntegratedBasePlateType_t::IBP14, std::make_pair(0.804,0.796)}, // use same scaling as 0.66mm x 7.5mm
                {IntegratedBasePlateType_t::IBP15, std::make_pair(0.812, 0.796)},
                {IntegratedBasePlateType_t::CRANIAL_WINDOW, std::make_pair(0.820, 0.796)},
        };

    const std::map<std::string, IntegratedBasePlateType_t> probeIdToIntegratedBasePlate=
        {
                {"1050-004417", IntegratedBasePlateType_t::IBP1},
                {"1050-004415", IntegratedBasePlateType_t::IBP2},
                {"1050-004414", IntegratedBasePlateType_t::IBP3},
                {"1050-004413", IntegratedBasePlateType_t::IBP4},
                {"1050-004637", IntegratedBasePlateType_t::IBP5},
                {"1050-004416", IntegratedBasePlateType_t::IBP6},
                {"1050-004418", IntegratedBasePlateType_t::IBP7},
                {"1050-004419", IntegratedBasePlateType_t::IBP8},
                {"1050-004420", IntegratedBasePlateType_t::IBP9},
                {"1050-004474", IntegratedBasePlateType_t::IBP10},
                {"1050-004724", IntegratedBasePlateType_t::IBP11},
                {"1050-005441", IntegratedBasePlateType_t::IBP12},
                {"1050-005442", IntegratedBasePlateType_t::IBP13},
                {"1050-005443", IntegratedBasePlateType_t::IBP14},
                {"1050-005473", IntegratedBasePlateType_t::IBP5},
                {"1050-005475", IntegratedBasePlateType_t::IBP15},
        };

    /// \cond doxygen chokes on enum class inside of namespace
    /// Vector specifying the order the baseplates should be listed in the metadata view
    const std::vector<IntegratedBasePlateType_t> integratedBasePlateOrder =
    {
        IntegratedBasePlateType_t::UNAVAILABLE,
        IntegratedBasePlateType_t::CUSTOM,
        IntegratedBasePlateType_t::CRANIAL_WINDOW,
        IntegratedBasePlateType_t::IBP1,
        IntegratedBasePlateType_t::IBP2,
        IntegratedBasePlateType_t::IBP3,
        IntegratedBasePlateType_t::IBP4,
        IntegratedBasePlateType_t::IBP5,
        IntegratedBasePlateType_t::IBP6,
        IntegratedBasePlateType_t::IBP7,
        IntegratedBasePlateType_t::IBP8,
        IntegratedBasePlateType_t::IBP9,
        IntegratedBasePlateType_t::IBP10,
        IntegratedBasePlateType_t::IBP11,
        IntegratedBasePlateType_t::IBP12,
        IntegratedBasePlateType_t::IBP13,
        IntegratedBasePlateType_t::IBP14,
        IntegratedBasePlateType_t::IBP15,
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

        /// constructor without clippedVessels
        VesselSetMetadata(
            const VesselSetUnits_t units,
            const ProjectionType projectionType,
            const double timeWindow,
            const double timeIncrement
            )
            : m_units(units)
            , m_projectionType(projectionType)
            , m_timeWindow(timeWindow)
            , m_timeIncrement(timeIncrement)
        {
        }

        /// fully specified constructor
        VesselSetMetadata(
            const VesselSetUnits_t units,
            const ProjectionType projectionType,
            const double timeWindow,
            const double timeIncrement,
            const std::map<std::string, std::vector<int>> clippedVessels
            )
            : m_units(units)
            , m_projectionType(projectionType)
            , m_timeWindow(timeWindow)
            , m_timeIncrement(timeIncrement)
            , m_clippedVessels(clippedVessels)
        {
        }

        VesselSetUnits_t m_units;        ///< units of the traces in the vessel set
        ProjectionType m_projectionType; ///< type of projection stored in the vessel set
        double m_timeWindow;             ///< the length of the time window in seconds
        double m_timeIncrement;          ///< the length of the time increment in seconds
        std::map<std::string, std::vector<int>> m_clippedVessels; ///< the clipped vessels with frame # in the vessel set
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

    inline std::string getVesselSetUnitsString(VesselSetUnits_t vesselSetUnits)
    {
        switch(vesselSetUnits)
        {
            case VesselSetUnits_t::PIXELS:
                return "pixels";
            case VesselSetUnits_t::MICRONS:
                return "microns";
            case VesselSetUnits_t::PIXELS_PER_SECOND:
                return "pixels per second";
            case VesselSetUnits_t::MICRONS_PER_SECOND:
                return "microns per second";
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
        if (!extraProps["idps"]["vesselset"]["timeWindow"].is_null())
        {
            return extraProps["idps"]["vesselset"]["timeWindow"].get<double>();
        }
        return 0;
    }

    template <class T>
    double getVesselSetTimeIncrement(T & inData)
    {
        using json = nlohmann::json;
        json extraProps = getExtraPropertiesJSON(inData);
        if (!extraProps["idps"]["vesselset"]["timeIncrement"].is_null())
        {
            return extraProps["idps"]["vesselset"]["timeIncrement"].get<double>();
        }
        return 0;
    }

    inline std::string getClippedVesselsString(std::map<std::string, std::vector<int>> inData)
    {
        std::string val = "{";
        std::string converted = "";
        for (auto it = inData.cbegin(); it != inData.cend(); it++) {
            std::vector<int> vals = it->second;
            for (size_t i = 0; i < vals.size(); i++) {
                converted += std::to_string(vals.at(i));
                if (i < vals.size() -1 ) {
                    converted += ",";
                }
            }
            val += (it->first) + ":[" + converted + "], ";
            converted = "";
        }

        val += "}";
        
        return val;
    }

    template <class T>
    IntegratedBasePlateType_t getIntegratedBasePlateType(T & inData)
    {
        using json = nlohmann::json;
        json extraProps = getExtraPropertiesJSON(inData);
        if (!extraProps["idps"]["integratedBasePlate"].is_null())
        {
            // read IDPS metadata
            std::string ibp = extraProps["idps"]["integratedBasePlate"].get<std::string>();
            return static_cast<IntegratedBasePlateType_t>(stoi(ibp));
        }
        else if (!extraProps["probe"].is_null())
        {
            // read IDAS metadata
            std::string name = extraProps["probe"]["name"].get<std::string>();
            ISX_ASSERT(!name.empty());

            IntegratedBasePlateType_t probeType;
            if (name == "Custom")
            {
                probeType = IntegratedBasePlateType_t::CUSTOM;
            }
            else if (name == "None")
            {
                probeType = IntegratedBasePlateType_t::UNAVAILABLE;
            }
            else if (name == "Cranial Window or No Lens")
            {
                probeType = IntegratedBasePlateType_t::CRANIAL_WINDOW;
            }
            else // Integrated lens
            {
                std::string probeId = extraProps["probe"]["id"].get<std::string>();
                const bool probeMappingExists = probeIdToIntegratedBasePlate.find(probeId) != probeIdToIntegratedBasePlate.end();
                ISX_ASSERT(probeMappingExists, "Failed to map IDAS probe ID " + probeId + " to an integrated base plate type in IDPS.");
                if (probeMappingExists)
                {
                    probeType = probeIdToIntegratedBasePlate.at(probeId);
                }
                else
                {
                    probeType = IntegratedBasePlateType_t::UNAVAILABLE;
                }
            }
            return probeType;
        }
        return IntegratedBasePlateType_t::UNAVAILABLE;
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

    template <class T>
    double getMicronsPerPixel(T & inData)
    {
        using json = nlohmann::json;
        json extraProps = getExtraPropertiesJSON(inData);

        uint16_t efocus = getEfocus(inData);
        IntegratedBasePlateType_t integratedBasePlateType = getIntegratedBasePlateType(inData);
        if (integratedBasePlateType == IntegratedBasePlateType_t::UNAVAILABLE ||
            integratedBasePlateType == IntegratedBasePlateType_t::CUSTOM) return 0;
        std::pair<double, double> efocusData = integratedBasePlateToScaling.at(integratedBasePlateType);

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
    void setIntegratedBasePlateType(T & inData, IntegratedBasePlateType_t integratedBasePlateType) {
        using json = nlohmann::json;
        json extraProps = getExtraPropertiesJSON(inData);

        size_t index = size_t(integratedBasePlateType);
        std::string integratedBasePlateString = std::to_string(index);

        // Pad with zeros
        std::string zeros(std::to_string(integratedBasePlateMap.size() - 1).size() - integratedBasePlateString.size(), '0');
        integratedBasePlateString.insert(0, zeros);

        extraProps["idps"]["integratedBasePlate"] = integratedBasePlateString;

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
        extraProps["idps"]["planesDeinterleaved"] = true;
        extraProps["idps"]["channelsDeinterleaved"] = true;

        inDataDest->setExtraProperties(extraProps.dump());
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
        extraProps["idps"]["vesselset"]["clippedVessels"] = getClippedVesselsString(vesselSetMetadata.m_clippedVessels);
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
