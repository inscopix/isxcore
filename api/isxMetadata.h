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
            const CellSetType_t type)
        : m_method(method)
        , m_type(type)
        {
        }

        CellSetMethod_t  m_method = CellSetMethod_t::UNAVAILABLE;  ///< method used to generate the cell set
        CellSetType_t  m_type = CellSetType_t::UNAVAILABLE;        ///< type of footprints in the cell set
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
    void setCellSetMetadata(T & inData, CellSetMetadata cellSetMetadata)
    {
        setCellSetMethod(inData, cellSetMetadata.m_method);
        setCellSetType(inData, cellSetMetadata.m_type);
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

    // Helper function to deal with string representations of the extra properties
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
