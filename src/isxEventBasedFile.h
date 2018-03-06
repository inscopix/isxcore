#ifndef ISX_EVENT_BASED_FILE_H
#define ISX_EVENT_BASED_FILE_H

#include <string>
#include <vector>
#include "isxCoreFwd.h"
#include "isxTimingInfo.h"
#include "isxTrace.h"

namespace isx
{

    enum class SignalType : uint8_t
    {
        SPARSE = 0,     ///< Signals containing event-triggered data 
        DENSE = 1       ///< Signals containing data that's been sampled at a fixed sampling rate
    };

class EventBasedFile 
    {
    public: 
        virtual 
        bool 
        isValid() const = 0; 

        virtual 
        const std::string & 
        getFileName() const = 0;

        virtual
        const std::vector<std::string> 
        getChannelList() const = 0;

        virtual
        SpLogicalTrace_t 
        getLogicalData(const std::string & inChannelName) = 0;

        virtual
        SpFTrace_t 
        getAnalogData(const std::string & inChannelName) = 0;

        virtual
        const isx::TimingInfo  
        getTimingInfo() const = 0;

    };
}

#endif /// ISX_EVENT_BASED_FILE_H