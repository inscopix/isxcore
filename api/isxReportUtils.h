#ifndef ISX_REPORT_UTILS_H
#define ISX_REPORT_UTILS_H

#include "isxAsync.h"
#include "isxTime.h"
#include <string>


namespace isx
{
    class DataSet;
    class Series; 

    void reportSessionStart();
    void reportSessionEnd();
    void reportSystemInfo();
    void reportAlgoResults(const std::string & inAlgoName, AsyncTaskStatus inStatus, DurationInSeconds inProcessingTime);
    void reportAlgoParams(const std::string & inAlgoName, const std::vector<std::string> & inFileNames, const std::string & inParams, const std::vector<std::string> & inOutputFileNames);
    void reportOpenProject(const isx::SpProject_t inProject);
    void reportImportData(DataSet * inDataSet);
    void reportCreationOfSeries(Series * inSeries);
    void reportAddDataSetToSeries(const std::string & seriesName, DataSet * inDataSet);
    void reportVisualizerLayoutChange(int oldLayout, int newLayout);
    void reportDeleteDataFile(const std::string & inFileName);
}

#endif // ISX_REPORT_UTILS_H
