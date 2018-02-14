#include "isxCsvTraceImporter.h"
#include "isxTimeStampedDataFile.h"
#include "isxCore.h"
#include "isxJsonUtils.h"

#include <fstream>
#include <cmath>
#include <limits>
#include <cctype>

namespace isx
{

CsvTraceImporterParams::CsvTraceImporterParams()
    : m_timeUnit(1, 1)
{
}

std::string
CsvTraceImporterParams::getOpName()
{
    return "Import CSV Trace";
}

CsvTraceImporterParams 
CsvTraceImporterParams::fromString(const std::string & inStr)
{
    CsvTraceImporterParams params;
    json j = json::parse(inStr);
    params.m_startRow = j["startRow"];
    params.m_colsToImport = j["colsToImport"].get<std::set<size_t>>();
    params.m_titleRow = j["titleRow"];
    params.m_timeCol = j["timeCol"];
    params.m_startTime = convertJsonToTime(j["startTime"]);
    params.m_timeUnit = convertJsonToRatio(j["timeUnit"]);

    return params;
}

std::string CsvTraceImporterParams::toString() const
{
    json j; 
    j["startRow"] = m_startRow;
    j["colsToImport"] = m_colsToImport;
    j["titleRow"] = m_titleRow;
    j["timeCol"] = m_timeCol;
    j["startTime"] = convertTimeToJson(m_startTime);
    j["timeUnit"] = convertRatioToJson(m_timeUnit);
    return j.dump(4);    
}

bool
CsvTraceImporterParams::checkColsToImport(std::string & outMessage) const
{
    if (m_colsToImport.size() == 1 && *(m_colsToImport.begin()) == m_timeCol)
    {
        outMessage =
            "The only column being imported is the time column. "
            "If you are specifying a subset of columns, this must include columns other than the time column. ";
        return false;
    }
    return true;
}

AsyncTaskStatus
runCsvTraceImporter(
        CsvTraceImporterParams inParams,
        std::shared_ptr<CsvTraceImporterOutputParams> inOutputParams,
        AsyncCheckInCB_t inCheckInCB)
{
    bool cancelled = false;

    if (inParams.m_inputFile.empty())
    {
        ISX_THROW(ExceptionUserInput, "Input file path not specified.");
    }

    if (inParams.m_outputFile.empty())
    {
        ISX_THROW(ExceptionUserInput, "Output file path not specified.");
    }

    std::string outMessage;
    if (!inParams.checkColsToImport(outMessage))
    {
        ISX_THROW(ExceptionUserInput, outMessage);
    }

    // First parse the CSV file to grab the titles, so we can do some initialization.
    std::ifstream inputStream(inParams.m_inputFile);
    if (!inputStream.good())
    {
        ISX_THROW(ExceptionFileIO, "Error opening input CSV trace file ", inParams.m_inputFile);
    }

    std::vector<std::string> colNames;
    std::string line;
    for (size_t row = 0; isx::getLine(inputStream, line); ++row)
    {
        if (row == inParams.m_titleRow)
        {
            colNames = splitString(line, ',');
            break;
        }
    }

    // Check the number of columns and relevant column indices.
    const size_t numCols = colNames.size();
    if (numCols < 2)
    {
        ISX_THROW(ExceptionUserInput, "CSV file must contain at least 2 columns. Found ", numCols, ".");
    }

    if (inParams.m_timeCol >= numCols)
    {
        ISX_THROW(ExceptionUserInput, "Time column ", inParams.m_timeCol, " exceeds last column ", numCols - 1, ".");
    }

    // To simplify indexing, we always store all columns temporarily, but
    // will only later write the columns we want.
    // The order here is important because an empty set of indices means
    // that all columns should be captured.
    if (inParams.m_colsToImport.empty())
    {
        for (size_t c = 0; c < numCols; ++c)
        {
            inParams.m_colsToImport.insert(c);
        }
    }
    else
    {
        inParams.m_colsToImport.insert(inParams.m_timeCol);
    }

    for (auto it = inParams.m_colsToImport.begin(); it != inParams.m_colsToImport.end();)
    {
        const size_t c = *it;
        if (c >= numCols)
        {
            ISX_LOG_WARNING("Requested import of column ", c, " but last column is ", numCols - 1, ". Skipping.");
            it = inParams.m_colsToImport.erase(it);
        }
        else
        {
            ++it;
        }
    }

    // Go back to the beginning of the file and parse the data values.
    // Clearing the error state flags in case the title was the last line.
    inputStream.clear();
    inputStream.seekg(0, std::ios_base::beg);
    if (!inputStream.good())
    {
        ISX_THROW(ExceptionFileIO, "Failed to rewind to beginning of CSV file.");
    }

    std::vector<std::vector<double>> colValues(numCols);
    for (size_t row = 0; isx::getLine(inputStream, line); ++row)
    {
        if (row == inParams.m_titleRow)
        {
            continue;
        }

        if (row < inParams.m_startRow)
        {
            continue;
        }

        const std::vector<std::string> rowValues = splitString(line, ',');

        if (rowValues.size() != colNames.size())
        {
            ISX_THROW(ExceptionUserInput, "Number of columns in row ", row, " (", rowValues.size(), ")",
                    " does not match number of columns in title row (", colNames.size(), ").");
        }

        for (size_t col = 0; col < rowValues.size(); ++col)
        {
            double value = std::numeric_limits<double>::quiet_NaN();
            const std::string & valueStr = rowValues.at(col);
            try
            {
                value = std::stod(valueStr);
                // TODO : Remove this when TimeStampedDataFile can stored negative values.
                // But keep the check for positive time stamps.
                if (value < 0.0)
                {
                    if (col == inParams.m_timeCol)
                    {
                        ISX_THROW(ExceptionDataIO, "All timestamps must be non-negative. ",
                                  "Found a negative timestamp on row ", row, " in column ",
                                  col, " (", valueStr, "). ");
                    }
                    value = std::numeric_limits<double>::quiet_NaN();
                }
            }
            catch (const std::exception &)
            {
                if (col == inParams.m_timeCol)
                {
                    ISX_THROW(ExceptionDataIO, "All timestamps must be numeric. ",
                            "Found a non-numeric timestamp on row ", row, " in column ",
                            col, " (", valueStr, "). ");
                }
            }
            colValues.at(col).push_back(value);
        }
    }

    // Convert timestamps to microsecond precision
    std::vector<uint64_t> timeStampsUSecs;
    for (const auto timeValue : colValues.at(inParams.m_timeCol))
    {
        const DurationInSeconds offset = DurationInSeconds(Ratio::fromDouble(double(timeValue), 6)) * inParams.m_timeUnit;
        const Time timeStamp = inParams.m_startTime + offset;
        timeStampsUSecs.push_back(uint64_t(std::round(timeStamp.getSecsSinceEpoch().toDouble() * 1e6)));
    }
    const size_t numValues = timeStampsUSecs.size();

    TimeStampedDataFile outputFile(inParams.m_outputFile, TimeStampedDataFile::StoredData::GPIO, false);
    for (const auto c : inParams.m_colsToImport)
    {
        if (c != inParams.m_timeCol)
        {
            outputFile.writeChannelHeader(colNames.at(c), "", "", numValues);
            for (size_t r = 0; r < numValues; ++r)
            {
                // TODO : We only want to store the value, but DataPkt does not
                // return 0.f as the value when it's stored as 0.f unless the
                // state is false.
                // We are likely going to update the TimeStampedDataFile and
                // DataPkt, so this may change then.
                const float value = float(colValues.at(c).at(r));
                TimeStampedDataFile::DataPkt pkt(timeStampsUSecs.at(r), value != 0.f, value);
                outputFile.writeDataPkt(pkt);
            }
        }
    }

    // TODO : Verify this is the right thing to store for timing info.
    const Time firstTime(DurationInSeconds(timeStampsUSecs.front(), int64_t(1e6)));
    const double stepDurationUSecs = double(timeStampsUSecs.back() - timeStampsUSecs.front()) / double(numValues - 1);
    const DurationInSeconds stepDuration(Ratio::fromDouble(stepDurationUSecs / 1e6, 6));
    outputFile.setTimingInfo(TimingInfo(firstTime, stepDuration, numValues));

    outputFile.closeFileForWriting();

    if (cancelled)
    {
        std::remove(inParams.m_outputFile.c_str());

        return AsyncTaskStatus::CANCELLED;
    }

    inCheckInCB(1.f);
    return AsyncTaskStatus::COMPLETE;
}

size_t
convertExcelIndexToIndex(const std::string & inIndex)
{
    const std::string trimmed = trimString(inIndex);
    size_t outIndex = 0;
    size_t i = 0;
    for (auto it = trimmed.rbegin(); it != trimmed.rend(); ++it, ++i)
    {
        const size_t c = size_t(std::tolower(*it));
        if (c >= 49 && c <= 57)
        {
            outIndex += size_t(c - 48) * size_t(std::pow(10, i));
        }
        else if (c >= 97 && c <= 122)
        {
            outIndex += size_t(c - 96) * size_t(std::pow(26, i));
        }
        else
        {
            ISX_THROW(ExceptionUserInput, "Converting ", inIndex, " to an index failed. ",
                "Character ", trimmed.size() - i - 1, " (", c, ") is not in [1-9] or [a-z]. ");
        }
    }

    if (outIndex > 0)
    {
        --outIndex;
    }
    return outIndex;
}

std::set<size_t>
convertExcelIndicesToIndices(const std::string & inIndices)
{
    std::set<size_t> outIndices;
    for (const auto & range : splitString(inIndices, ','))
    {
        const std::vector<std::string> limits = splitString(range, '-');
        const std::string first = limits.front();
        const std::string last = limits.back();
        if (limits.size() == 1)
        {
            if (!limits.front().empty())
            {
                outIndices.insert(convertExcelIndexToIndex(first));
            }
        }
        else if (limits.size() == 2)
        {
            if (!(limits.front().empty() || limits.back().empty()))
            {
                const size_t lower = convertExcelIndexToIndex(limits.front());
                const size_t upper = convertExcelIndexToIndex(limits.back());
                for (size_t i = lower; i <= upper; ++i)
                {
                    outIndices.insert(i);
                }
            }
        }
        else
        {
            ISX_THROW(ExceptionUserInput,
                    "Each index should be a alphanumerical value like 1, A, or BC; "
                    "or a range of those indicated with '-' like 1-3, Z-BC."
            );
        }
    }
    return outIndices;
}

} // namespace isx
