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

    struct CellSetMetadata
    {
        CellSetMetadata()
        {
        }

        CellSetMetadata(
            const CellSetMethod_t method,
            const CellSetType_t type)
        : m_method(method)
        , m_type(type)
        {
        }

        CellSetMethod_t  m_method = CellSetMethod_t::UNAVAILABLE;
        CellSetType_t  m_type = CellSetType_t::UNAVAILABLE;
    };


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
        }
    }


    // Getters
    template <class T>
    CellSetMethod_t getCellSetMethod(T & inData)
    {
        using json = nlohmann::json;
        json extraProps = json::parse(inData->getExtraProperties());
        if (extraProps["idps"] != "" && !extraProps["idps"]["cellset"]["method"].is_null())
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


} // namespace isx

#endif // ISX_METADATA_H
