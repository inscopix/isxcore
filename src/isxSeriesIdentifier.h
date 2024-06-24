#ifndef ISX_SERIES_IDENTIFIER_H
#define ISX_SERIES_IDENTIFIER_H

#include "isxCore.h"
#include "isxCoreFwd.h"

#include <map>
#include <string>
#include <atomic>

namespace isx
{

/// Class to manage unique identifiers for Series
///
class SeriesIdentifier
{
public:
    using IdentifierType = std::string;                     ///< Identifer type
    using SeriesMap = std::map<IdentifierType, Series *>;   ///< Map type

    /// Constructor
    ///
    SeriesIdentifier(Series * inSeries);

    /// Destructor
    ///
    ~SeriesIdentifier();

    SeriesIdentifier(const SeriesIdentifier &) = delete;
    SeriesIdentifier & operator=(const SeriesIdentifier &) = delete;

    /// \return Id for this SeriesIdentifier
    ///
    IdentifierType
    getId() const;

    /// \return Series for a given Id (lookup)
    /// \param inId Id for which to retrieve the series
    static
    Series *
    getSeries(IdentifierType inId);

private:

    IdentifierType               m_id;
    static std::atomic<isize_t>  s_nextAvailableId;
    static SeriesMap             s_seriesMap;
};

}; // namespace

#endif // def ISX_SERIES_IDENTIFIER_H