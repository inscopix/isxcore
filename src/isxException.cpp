#include "isxException.h"


const std::vector<std::string> isx::Exception::ExceptionTypeNames({ "FileIO", "DataIO", "UserInput" });

const std::string isx::Exception::DEFAULT_MSG = "No detailed information provided";