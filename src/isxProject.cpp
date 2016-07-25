#include "isxMosaicMovie.h"
#include "isxProject.h"

namespace isx
{

Project::Project()
    : m_valid(false)
{
}

Project::Project(const std::string & inFileName)
{
    m_file = std::unique_ptr<ProjectFile>(new ProjectFile(inFileName));
    ProjectFile::DataCollection rootDc;
    rootDc.name = "root";
    m_file->addDataCollection(rootDc);
    m_valid = true;
}

bool
Project::isValid()
{
    return m_valid;
}

SpWritableMovie_t
Project::createMosaicMovie(
    const std::string & inFileName,
    const TimingInfo & inTimingInfo,
    const SpacingInfo & inSpacingInfo)
{
    SpWritableMovie_t movie = std::make_shared<MosaicMovie>(
        inFileName, inTimingInfo, inSpacingInfo);
    ProjectFile::DataFileDescriptor dfDesc(
        ProjectFile::DataFileType::DATAFILETYPE_MOVIE,
        inFileName);
    m_file->addFileToDataCollection(dfDesc, 0);
    return movie;
}

ProjectFile::DataCollection
Project::getDataCollection(isize_t inIndex)
{
    return m_file->getDataCollection(inIndex);
}

} // namespace isx

