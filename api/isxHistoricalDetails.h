#ifndef ISX_HISTORICAL_DETAILS_H
#define ISX_HISTORICAL_DETAILS_H

#include <string>

namespace isx
{

/// A class encapsulating a the history of a dataset
/// It contains the details of what operation was applied and 
/// what parameters were used in that operation
class HistoricalDetails
{
public:
    /// Constructor 
    /// 
    HistoricalDetails();

    /// Constructor
    /// \param inOperationName the operation name
    /// \param inParams a JSON-formatted string with the input parameters
    HistoricalDetails(const std::string & inOperationName, const std::string & inParams);

    /// Destructor
    ///
    ~HistoricalDetails();

    /// \return a string containing the name of the 
    /// applied operation
    const std::string &  
    getOperation() const;

    /// \return a JSON string containing the details
    /// of the parameters used for the operation
    const std::string & 
    getInputParameters() const;    

    /// Equality operator
    /// \return true if this item equals the other
    bool operator ==(const HistoricalDetails & other) const;

    /// Inequality operator
    /// \return true if this item is different than the other
    bool operator !=(const HistoricalDetails & other) const;
    
private:
    std::string         m_operationName;        ///< The operation name
    std::string         m_inputParameters;      ///< The input parameters (JSON string)
    

}; // class HistoricalDetails


} // namespace isx

#endif // ISX_HISTORICAL_DETAILS_H