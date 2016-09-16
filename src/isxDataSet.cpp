#include "isxDataSet.h"
#include "isxException.h"
#include "isxGroup.h"
#include "isxJsonUtils.h"
#include "isxVideoFrame.h"
#include "isxMovieFactory.h"

#include <fstream>
#include <limits>

namespace isx
{
    
const std::string DataSet::PROP_DATA_MIN = "dmin";
const std::string DataSet::PROP_DATA_MAX = "dmax";
const std::string DataSet::PROP_VIS_MIN  = "vmin";
const std::string DataSet::PROP_VIS_MAX  = "vmax";


DataSet::DataSet()
    : m_valid(false)
    , m_parent(nullptr)
{
}

DataSet::DataSet(
        const std::string & inName,
        Type inType,
        const std::string & inFileName,
        const std::map<std::string, float> & inProperties)
    : m_valid(true)
    , m_name(inName)
    , m_type(inType)
    , m_fileName(inFileName)
    , m_parent(nullptr)
    , m_properties(inProperties)
{
}

bool
DataSet::isValid() const
{
    return m_valid;
}

DataSet::Type
DataSet::getType() const
{
    return m_type;
}

std::string
DataSet::getName() const
{
    return m_name;
}

std::string
DataSet::getFileName() const
{
    return m_fileName;
}

Group *
DataSet::getParent() const
{
    return m_parent;
}

void
DataSet::setParent(Group * inParent)
{
    m_parent = inParent;
}

std::string
DataSet::getPath() const
{
    if (m_parent)
    {
        if (m_parent->getName() == "/")
        {
            return m_parent->getName() + m_name;
        }
        else
        {
            return m_parent->getPath() + "/" + m_name;
        }
    }
    return m_name;
}

bool
DataSet::operator ==(const DataSet & inOther) const
{
    bool sameParent = true;
    if (m_parent)
    {
        if (!inOther.m_parent)
        {
            return false;
        }
        sameParent &= (*m_parent == *(inOther.m_parent));
    }
    return sameParent &&
        (m_name == inOther.m_name) &&
        (m_type == inOther.m_type) &&
        (m_fileName == inOther.m_fileName);
}

void
DataSet::serialize(std::ostream & strm) const
{
    strm << "DataSet(" <<
        "path = " << getPath() << ", " <<
        "type = " << int(m_type) << ", " <<
        "fileName = " << m_fileName << ")";
}

const std::map<std::string, float> & 
DataSet::getProperties() const
{
    return m_properties;
}

bool 
DataSet::getPropertyValue(const std::string & inPropertyName, float & outValue)
{   
    bool found = false;
    if (m_properties.find(inPropertyName) != m_properties.end()) 
    {        
        found = true;
        outValue = m_properties[inPropertyName];
    }
    return found;
}

void 
DataSet::setProperty(const std::string & inPropertyName, float inValue)
{
    m_properties[inPropertyName] = inValue;
}

DataSet::Type
readDataSetType(const std::string & inFileName)
{
    std::ifstream file(inFileName, std::ios::binary);
    json j = readJsonHeader(file);

    try
    {
        return DataSet::Type(int(j["type"]));
    }
    catch (const std::exception & error)
    {
        ISX_THROW(isx::ExceptionDataIO, "Error parsing data file header: ", error.what());
    }
    catch (...)
    {
        ISX_THROW(isx::ExceptionDataIO, "Unknown error while parsing data file header.");
    }
}

void 
getDataRange(const std::string & inFilePath, float & outMin, float & outMax)
{
    SpMovie_t m = readMovie(inFilePath);
    SpVideoFrame_t vf = m->getFrame(0);                

    DataType dt = vf->getDataType();
    isize_t numPixels = vf->getImage().getSpacingInfo().getTotalNumPixels();
    switch(dt)
    {
        case DataType::U16:
        {
            outMin = (float) std::numeric_limits<uint16_t>::max();
            outMax = (float) std::numeric_limits<uint16_t>::min();
            uint16_t * pixels = vf->getPixelsAsU16();
            for (isize_t i(0); i < numPixels; ++i)
            {
                if(pixels[i] > (uint16_t)outMax)
                {
                    outMax = (float)pixels[i];
                }
                else if(pixels[i] < (uint16_t)outMin)
                {
                    outMin = (float)pixels[i];
                }
            }

        }
        break;
        case DataType::F32:
        {
            outMin = std::numeric_limits<float>::max();
            outMax = std::numeric_limits<float>::min();
            float * pixels = vf->getPixelsAsF32();
            for (isize_t i(0); i < numPixels; ++i)
            {
                if(pixels[i] > outMax)
                {
                    outMax = pixels[i];
                }
                else if(pixels[i] < outMin)
                {
                    outMin = pixels[i];
                }
            }

        }
        break;
        default:
        {
            ISX_THROW(isx::ExceptionFileIO, "Unsupported datatype: ", dt);
        }
        break;
    }
}

} // namespace isx
