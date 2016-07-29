#include "isxMosaicMovie.h"
#include "isxProject.h"
#include "isxRecording.h"
#include "isxInputFileParser.h"
#include "isxException.h"

#include <fstream>

namespace isx
{

Project::Project()
    : m_valid(false)
{
}

Project::Project(const std::string & inFileName)
    : m_valid(false)
{
    m_file = std::unique_ptr<ProjectFile>(new ProjectFile(inFileName));
    m_valid = true;
}

bool
Project::isValid()
{
    return m_valid;
}

SpWritableMovie_t
Project::createMosaicMovie(
    isize_t collectionIndex,
    std::string & outFileName,
    const TimingInfo & inTimingInfo,
    const SpacingInfo & inSpacingInfo)
{
    // TODO sweet : this is some throwaway code to make sure we get a unique
    // name. This should cleaned up and put somewhere more central.
    std::string extension = isx::InputFileParser::getExtension(outFileName);
    std::string outFileBase = outFileName.substr(0, outFileName.size() - (extension.size() + 1));
    outFileName = outFileBase + ".isxd";

    std::ifstream file;
    file.open(outFileName);
    isize_t index;
    for (index = 0; index < 100 && file.good(); ++index)
    {
        outFileName = outFileBase + "-" + std::to_string(index) + ".isxd";
        file.open(outFileName);
    }
    if (index == 99 && file.good())
    {
        ISX_THROW(isx::ExceptionFileIO, "Gave up finding a unique name: ", outFileName);
    }

    SpWritableMovie_t movie = std::make_shared<MosaicMovie>(
        outFileName, inTimingInfo, inSpacingInfo);
    ProjectFile::DataFileDescriptor dfDesc(
        ProjectFile::DataFileType::DATAFILETYPE_MOVIE,
        outFileName);
    m_file->addFileToDataCollection(dfDesc, collectionIndex);
    return movie;
}

SpMovie_t
Project::getMovie(
    const ProjectFile::DataCollection & inDc,
    isize_t inIndex)
{
    // TODO sweet : check that this is a movie type
    std::string fileName = inDc.files[inIndex].filename;

    SpMovie_t movie;
    std::string extension = isx::InputFileParser::getExtension(fileName);
    if ((extension == "hdf5") || (extension == "xml"))
    {
        auto recording = std::make_shared<Recording>(fileName);
        movie = recording->getMovie();
    }
    else
    {
        // Assume the file is a mosaic movie file (.isxd)
        movie = std::make_shared<MosaicMovie>(fileName);
    }

    return movie;
}

ProjectFile::DataCollection
Project::getDataCollection(isize_t inIndex)
{
    return m_file->getDataCollection(inIndex);
}

void
Project::addDataCollection(ProjectFile::DataCollection & inData)
{
    return m_file->addDataCollection(inData);
}

isize_t
Project::getNumDataCollections()
{
    return m_file->getNumDataCollections();
}

const std::vector<std::string> &
Project::getOriginalNames()
{
    return m_file->getOriginalNames();
}

std::string
Project::getName()
{
    return m_file->getName();
}

} // namespace isx

