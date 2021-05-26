#ifndef ISX_METADATA_H
#define ISX_METADATA_H

#include "isxCellSetFactory.h"
#include "json.hpp"
#include <string>

namespace isx
{
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
        DF,            // CNMFe (estimate of the “true” dF, i.e. temporal traces which are on the same scale of pixel intensity as the raw movie, calculated as the scaled dF divided by the average pixel intensity of the nth percentile of brightest pixels in the spatial footprint)
        SCALED_DF,     // CNMFe (average fluorescence activity of all pixels in the neuron - this is how the dF data is scaled by different pixels in the ROI)
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
        PIXELS = 0,
        MICRONS
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
    };
    /// \endcond doxygen chokes on enum class inside of namespace

    const std::map<IntegratedBasePlateType_t, std::string> integratedBasePlateMap =
    {
        {IntegratedBasePlateType_t::UNAVAILABLE, "None"},
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
    };

    /// Scaling is dependant upon efocus and the integrated base plate type. We store a mapping
    /// from the integrated base plate type to the microns/pixels scaling ratio at 0 and 200 efocus.
    const std::map<IntegratedBasePlateType_t, std::pair<double, double>> integratedBasePlateToScaling=
        {
                {IntegratedBasePlateType_t::UNAVAILABLE, std::make_pair(0,0)},
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
        };

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
            const VesselSetType_t type,
            const VesselSetUnits_t units,
            const ProjectionType projectionType,
            const uint64_t timeWindow,
            const uint64_t timeIncrement)
            : m_type(type)
            , m_units(units)
            , m_projectionType(projectionType)
            , m_timeWindow(timeWindow)
            , m_timeIncrement(timeIncrement)
        {
        }

        VesselSetType_t  m_type;         ///< type of data stored in the vessel set
        VesselSetUnits_t m_units;        ///< units of the traces in the vessel set
        ProjectionType m_projectionType; ///< type of projection stored in the vessel set
        uint64_t m_timeWindow;           ///< the length of the time window in milliseconds
        uint64_t m_timeIncrement;        ///< the length of the time increment in milliseconds
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
            case CellSetUnits_t::SCALED_DF:
                return "scaled dF";
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
            if (method == "scaled dF") return CellSetUnits_t::SCALED_DF;
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
        }
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
    }

    template <class T>
    uint64_t getVesselSetTimeWindow(T & inData)
    {
        using json = nlohmann::json;
        json extraProps = getExtraPropertiesJSON(inData);
        if (!extraProps["idps"]["vesselset"]["timeWindow"].is_null())
        {
            return extraProps["idps"]["vesselset"]["timeWindow"].get<uint64_t>();
        }
        return 0;
    }

    template <class T>
    uint64_t getVesselSetTimeIncrement(T & inData)
    {
        using json = nlohmann::json;
        json extraProps = getExtraPropertiesJSON(inData);
        if (!extraProps["idps"]["vesselset"]["timeIncrement"].is_null())
        {
            return extraProps["idps"]["vesselset"]["timeIncrement"].get<uint64_t>();
        }
        return 0;
    }

    template <class T>
    IntegratedBasePlateType_t getIntegratedBasePlateType(T & inData)
    {
        using json = nlohmann::json;
        json extraProps = getExtraPropertiesJSON(inData);
        if (!extraProps["integratedBasePlate"].is_null())
        {
            std::string ibp = extraProps["integratedBasePlate"].get<std::string>();
            return static_cast<IntegratedBasePlateType_t>(stoi(ibp));
        }
        return IntegratedBasePlateType_t::UNAVAILABLE;
    }

    template <class T>
    double getEfocusScaling(T & inData)
    {
        using json = nlohmann::json;
        json extraProps = getExtraPropertiesJSON(inData);

        uint16_t efocus = 0;

        if (!extraProps["idps"]["efocus"].is_null())
        {
            efocus = extraProps["idps"]["efocus"].get<uint16_t>();
        }

        IntegratedBasePlateType_t integratedBasePlateType = getIntegratedBasePlateType(inData);
        if (integratedBasePlateType == IntegratedBasePlateType_t::UNAVAILABLE) return 0;
        std::pair<double, double> efocusData = integratedBasePlateToScaling.at(integratedBasePlateType);

        // Linearly interpolate the efocus scale factor
        double efocusScaling = ((efocusData.second - efocusData.first) / 200) * (efocus) + efocusData.first;

        return efocusScaling;
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
    void setVesselSetType(T & inData, VesselSetType_t vesselSetType)
    {
        using json = nlohmann::json;
        json extraProps = getExtraPropertiesJSON(inData);
        extraProps["idps"]["vesselset"]["type"] = getVesselSetTypeString(vesselSetType);
        inData->setExtraProperties(extraProps.dump());
    }

    template <typename T>
    void setVesselSetUnits(T & inData, VesselSetUnits_t vesselSetUnits)
    {
        using json = nlohmann::json;
        json extraProps = getExtraPropertiesJSON(inData);
        extraProps["idps"]["vesselset"]["units"] = getVesselSetUnitsString(vesselSetUnits);
        inData->setExtraProperties(extraProps.dump());
    }

    template <typename T>
    void setVesselSetProjectionType(T & inData, ProjectionType projectionType)
    {
        using json = nlohmann::json;
        json extraProps = getExtraPropertiesJSON(inData);
        extraProps["idps"]["vesselset"]["projectionType"] = getVesselSetProjectionTypeString(projectionType);
        inData->setExtraProperties(extraProps.dump());
    }

    template <typename T>
    void setVesselSetTimeWindow(T & inData, uint64_t timeWindow)
    {
        using json = nlohmann::json;
        json extraProps = getExtraPropertiesJSON(inData);
        extraProps["idps"]["vesselset"]["timeWindow"] = timeWindow;
        inData->setExtraProperties(extraProps.dump());
    }

    template <typename T>
    void setVesselSetTimeIncrement(T & inData, uint64_t timeIncrement)
    {
        using json = nlohmann::json;
        json extraProps = getExtraPropertiesJSON(inData);
        extraProps["idps"]["vesselset"]["timeIncrement"] = timeIncrement;
        inData->setExtraProperties(extraProps.dump());
    }

    template <typename T>
    void setVesselSetMetadata(T & inData, VesselSetMetadata vesselSetMetadata)
    {
        setVesselSetType(inData, vesselSetMetadata.m_type);
        setVesselSetUnits(inData, vesselSetMetadata.m_units);
        setVesselSetProjectionType(inData, vesselSetMetadata.m_projectionType);
        setVesselSetTimeWindow(inData, vesselSetMetadata.m_timeWindow);
        setVesselSetTimeIncrement(inData, vesselSetMetadata.m_timeIncrement);
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

        extraProps["integratedBasePlate"] = integratedBasePlateString;

        // prevents the addition of a null idps tag ("idps": null)
        // which would result in an error when parsing the file metadata
        if (extraProps["idps"].is_null())
        {
            extraProps.erase("idps");
        }

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
    std::string addVesselSetMetadata(T & inData, VesselSetMetadata vesselSetMetadata)
    {
        using json = nlohmann::json;
        json extraProps = getExtraPropertiesJSON(inData);
        extraProps["idps"]["vesselset"]["type"] = getVesselSetTypeString(vesselSetMetadata.m_type);
        extraProps["idps"]["vesselset"]["units"] = getVesselSetUnitsString(vesselSetMetadata.m_units);
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

    template <typename T>
    bool hasPreMotionCorrMetadata(const T & inData)
    {
        using json = nlohmann::json;
        json extraProps = getExtraPropertiesJSON(inData);
        return !extraProps["idps"]["pre_mc"].is_null();
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

} // namespace isx

#endif // ISX_METADATA_H
