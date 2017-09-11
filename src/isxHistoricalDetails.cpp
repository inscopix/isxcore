#include "isxHistoricalDetails.h"
#include "isxException.h"
#include "isxJsonUtils.h"

namespace isx {

HistoricalDetails::HistoricalDetails()
{
    m_operationName = "Imported";
    m_inputParameters = json::object().dump();
}

HistoricalDetails::HistoricalDetails(const std::string & inOperationName, const std::string & inParams) 
    : m_operationName(inOperationName)
    , m_inputParameters(inParams)
{
    if(!inParams.empty())
    {
        /// Will throw an exception if it cannot be parsed.
        /// This ensures the string has the correct format.
        json j = json::parse(inParams);    
        if(j.is_string())
        {
            ISX_THROW(isx::ExceptionUserInput, "The input parameters need to describe a JSON object.");
        }     
    }
    else
    {   
        m_inputParameters = json::object().dump();
    }

}

HistoricalDetails::~HistoricalDetails()
{

}

const std::string &  
HistoricalDetails::getOperation() const
{
    return m_operationName;
}

const std::string & 
HistoricalDetails::getInputParameters() const
{
    return m_inputParameters;
}

std::vector<std::string>
HistoricalDetails::getInputParametersByStrings() const
{
	json j = json::parse(m_inputParameters);

	std::vector<std::string> vec_str;
	for (auto b = j.begin(); b != j.end(); b++)
	{
		std::string str_key = b.key();
		std::string str_val = b.value().dump();
		std::string str = str_key + ": <b>" + str_val + "</b>";
		vec_str.push_back(str);
	}

	return vec_str;
}

bool 
HistoricalDetails::operator ==(const HistoricalDetails & other) const
{
    // Convert to json to allow for string formatting differences
    json jThis = json::parse(m_inputParameters);
    json jOther = json::parse(other.m_inputParameters);
    return (m_operationName == other.m_operationName)
        && (jThis == jOther);
}

bool 
HistoricalDetails::operator !=(const HistoricalDetails & other) const
{
    return !(*this == other);
}

}