#include "isxReportUtils.h"
#include "isxProject.h"
#include "isxProjectItem.h"
#include "isxDataSet.h"
#include "isxSeries.h"
#include "isxLog.h"
#include <QSysInfo>
#include <QString>
#include <thread>

#if ISX_OS_MACOS
#include <sys/types.h>
#include <sys/sysctl.h>
#elif ISX_OS_LINUX
#include <unistd.h>
#include <sys/sysinfo.h>
#elif ISX_OS_WIN32
#include <windows.h>
#endif

namespace 
{
    void serializeSeries(isx::Series * inSeries, std::stringstream & ss)
    {
        bool unitary = inSeries->isUnitary();
        if(!unitary)
        {
            ss << "- Series: " << inSeries->getName() << "\n";
        }

        for (const auto & dataSet : inSeries->getDataSets())
        {
            if(!unitary)
            {
                ss << "  ";
            }                    
            
            ss << "- Data set: \n";

            isx::DataSet::Metadata md = dataSet->getMetadata();

            for(auto & pair : md)
            {
                if(!unitary)
                {
                    ss << "  ";
                }
                ss << "  - " << pair.first << ": " << pair.second << "\n";
            }                    
        }
    }
}


namespace isx
{
    void reportSessionStart()
    {
        ISX_LOG_INFO_NO_PRINT("Initialized session ", CoreVersionString());
    }

    void reportSessionEnd()
    {
        ISX_LOG_INFO_NO_PRINT("Ended session");
    }

    void reportSystemInfo()
    {
        std::string osType = "OS type: ";
        std::string osVersion;
        std::string cpuArchitecture = "CPU architecture: " + QSysInfo::currentCpuArchitecture().toStdString();
        std::string numCoresStr = "Num Cores: ";
        isize_t numCores = std::thread::hardware_concurrency();
        std::string memDetails;

        #if ISX_OS_MACOS
            osType += "Mac";
            
            std::stringstream ss;
            ss << int(QSysInfo::MacintoshVersion);
            osVersion = ss.str();
            ss.str("");

            isize_t numCpus;
            size_t len = sizeof(numCpus);
            sysctlbyname("hw.physicalcpu", &numCpus, &len, NULL, 0);           

            ss << numCores << ", Num CPUs: " << numCpus;
            numCoresStr += ss.str();
            ss.str("");

            int mib[2];
            mib[0] = CTL_HW;
            mib[1] = HW_MEMSIZE;
            uint64_t memSize;
            len = sizeof(memSize);
            sysctl(mib, 2, &memSize, &len, NULL, 0);            
            const unsigned int div = 1024;
            ss << "Total Physical Memory: " << memSize / div << " KB";
            memDetails = ss.str();
            
            
        #elif ISX_OS_WIN32
            osType += "Windows";
            
            std::stringstream ss;
            ss << int(QSysInfo::WindowsVersion);
            osVersion = ss.str();
            ss.str("");
            
            if(numCores == 0)
            {
                // The number of cores could not be computed through the hardware_concurrency()
                SYSTEM_INFO sysinfo;
                GetSystemInfo(&sysinfo);
                numCores = isize_t(sysinfo.dwNumberOfProcessors);
            }

            ss << numCores;
            numCoresStr += ss.str();
            ss.str("");

            const int div = 1024; // to convert to KB
            MEMORYSTATUSEX statex;
            statex.dwLength = sizeof(statex);
            GlobalMemoryStatusEx(&statex);
            ss << "Total Physical Memory: " << statex.ullTotalPhys/div << " KB\n";
            ss << "Total Virtual Memory: " << statex.ullTotalVirtual/div << " KB\n";
            ss << "Total Extended Memory: " << statex.ullAvailExtendedVirtual/div << " KB";
            memDetails = ss.str();
            
        #elif ISX_OS_LINUX
            osType += "Linux";
            
            if(numCores == 0)
            {
                // The number of cores could not be computed through the hardware_concurrency()
                numCores = isize_t(sysconf(_SC_NPROCESSORS_ONLN));
            }

            std::stringstream ss;
            ss << numCores;
            numCoresStr += ss.str();
            ss.str("");

            struct sysinfo si;
            if(!sysinfo(&si))
            {
                const int div = 1024;                
                ss << "Total Physical Memory: " << si.totalram*si.mem_unit / div << " KB";
                memDetails = ss.str();
            }

        #endif

        ISX_LOG_INFO_NO_PRINT(osType);
        if (!osVersion.empty())
        {
            ISX_LOG_INFO_NO_PRINT("OS Version: ", osVersion);
        }
        
        ISX_LOG_INFO_NO_PRINT(cpuArchitecture);
        ISX_LOG_INFO_NO_PRINT(numCoresStr);
        ISX_LOG_INFO_NO_PRINT(memDetails);

    }

    void reportAlgoResults(const std::string & inAlgoName, AsyncTaskStatus inStatus, const double inDurationInSeconds)
    {
        ISX_LOG_INFO_NO_PRINT(inAlgoName, " ran for ", inDurationInSeconds, " seconds and finished with status ", int(inStatus));
    }

    void reportAlgoParams(const std::string & inAlgoName, const std::vector<std::string> & inFileNames, const std::string & inParams, const std::vector<std::string> & inOutputFileNames)
    {
        std::stringstream ss;
        ss << "Running " << inAlgoName << " : \n";

        ss << "Parameters:\n";
        ss << inParams << "\n";

        ss << "Input files:\n";
        for (const auto & fn : inFileNames)
        {
            ss << fn << "\n";
        }

        ss << "Output files:\n";
        for (const auto & fn : inOutputFileNames)
        {
            ss << fn << "\n";
        }

        ISX_LOG_INFO_NO_PRINT(ss.str());
    }

    void reportOpenProject(const isx::SpProject_t inProject)
    {   
        ISX_ASSERT(inProject);

        std::stringstream ss;
        ss << "Opened project: " << inProject->getFileName() << "\n";
        ss << "containing\n";

        Group * root = inProject->getRootGroup();
        ISX_ASSERT(root);

        std::vector<ProjectItem *> members = root->getGroupMembers();

        for (auto m : members)
        {
            const isx::ProjectItem::Type itemType = m->getItemType();
            if (itemType == isx::ProjectItem::Type::SERIES)
            {
                auto series = static_cast<isx::Series *>(m);
                serializeSeries(series, ss);
            }
        }

        ISX_LOG_INFO_NO_PRINT(ss.str());

    }

    void reportImportData(DataSet * inDataSet)
    {
        std::stringstream ss;
        ss << "Imported file: \n";

        DataSet::Metadata md = inDataSet->getMetadata();

        for(auto & pair : md)
        {
            ss << "  - " << pair.first << ": " << pair.second << "\n";
        }
        ISX_LOG_INFO_NO_PRINT(ss.str());
    }

    void reportCreationOfSeries(Series * inSeries)
    {
        ISX_LOG_INFO_NO_PRINT("Create Series ", inSeries->getName());
    }

    void reportAddDataSetToSeries(const std::string & seriesName, DataSet * inDataSet)
    {
        std::stringstream ss;
        ss << "Added data set to series: " << seriesName << "\n";
        ss << "  - Data set: " << inDataSet->getFileName() << "\n";
        DataSet::Metadata md = inDataSet->getMetadata();

        for(auto & pair : md)
        {
            ss << "    - " << pair.first << ": " << pair.second << "\n";
        }
        ISX_LOG_INFO_NO_PRINT(ss.str());
    }

    void reportVisualizerLayoutChange(int oldLayout, int newLayout)
    {
        ISX_LOG_INFO_NO_PRINT("Visualizer layout changed from ", oldLayout, " to ", newLayout);
    }

    void reportDeleteDataFile(const std::string & inFileName)
    {
        ISX_LOG_INFO_NO_PRINT("Deleting data file ", inFileName);
    }
}
