#ifndef ISX_PROJECT_ITEM_H
#define ISX_PROJECT_ITEM_H

#include "isxCore.h"

#include <vector>
#include <memory>
#include <string>

namespace isx
{

/// An interface for an item in a project, which can be a group, series or data set.
///
class ProjectItem
{
public:

    /// The type of an item.
    ///
    enum class Type
    {
        GROUP,          /// Organizes other project items like a directory.
        DATASET,        /// Represents a movie, cell set, etc. backed by one or more files.
        SERIES,         /// Represents a series of data sets.
    };

    virtual ~ProjectItem();

    /// \return The type of this item.
    ///
    virtual Type getItemType() const = 0;

    /// \return True if this is valid.
    ///
    virtual bool isValid() const = 0;

    /// \return The name of this data set.
    ///
    virtual std::string getName() const = 0;

    /// \param  inName  The new name of the item.
    ///
    virtual void setName(const std::string & inName) = 0;


    /// \return The non-historical descendant of an item
    /// 
    virtual ProjectItem * getMostRecent() const = 0;

    /// \return The previous item (this items's ancestor)
    ///
    virtual ProjectItem * getPrevious() const = 0;
    
    /// \return The parent of this item.
    ///
    virtual ProjectItem * getParent() const = 0;

    /// Set the parent of this item.
    ///
    /// Only use this if you're sure that you need to.
    /// This simply updates the parent of this item and does not move it
    /// into the new parent.
    ///
    /// \param  inParent    The new parent of this data set.
    virtual void setParent(ProjectItem * inParent) = 0;

    /// \return The children of this item.
    ///
    /// TODO sweet : We may want this and other related methods here and
    /// elsewhere to return shared_ptrs. As of now, we don't really need
    /// shared_ptrs, but they may useful/required in the future.
    virtual std::vector<ProjectItem *> getChildren() const = 0;

    /// \return The number of children this item has.
    ///
    virtual size_t getNumChildren() const = 0;

    /// Insert a child item.
    ///
    /// The index may be ignored in some cases (e.g. Series, DataSet).
    ///
    /// \param  inItem  The child item to insert.
    /// \param  inIndex The index where the child should be inserted.
    ///
    /// \throw  ExceptionDataIO If the child cannot be added to this due its type or name.
    /// \throw  ExceptionSeries If the child cannot be added to this due to series constraints.
    virtual void insertChild(std::shared_ptr<ProjectItem> inItem, const int inIndex = -1) = 0;

    /// Remove a child by name.
    ///
    /// \param  inName  The name of the child to remove.
    ///
    /// \throw  ExceptionDataIO If a child with the given name cannot be found.
    virtual std::shared_ptr<ProjectItem> removeChild(const std::string & inName) = 0;

    /// \return True if the item has been modified since last save, false otherwise.
    ///
    virtual bool isModified() const = 0;

    /// Sets the item flag indicating whether there are unsaved changes to false.
    ///
    virtual void setUnmodified() = 0;

    /// \param  inPretty    If true make the serialization pretty, ugly otherwise.
    /// \return             The serialized JSON string of the item.
    virtual std::string toJsonString(const bool inPretty = false) const = 0;

    /// Exact comparison.
    ///
    /// \param  inOther     The items with which to compare.
    /// \return             True if the items are exactly equal.
    virtual bool operator==(const ProjectItem & inOther) const = 0;

    /// Return whether the item has history
    /// \return true if there are ancestors items, false otherwise
    virtual bool hasHistory() const = 0;

    /// \return the number of historical data sets
    virtual isize_t getNumHistoricalItems() const = 0;

    /// \return whether this is an historical item or not
    virtual bool isHistorical() const = 0;

    /// \return a JSON-formatted string containing the Historical Details 
    virtual std::string getHistoricalDetails() const = 0;

    /// Get a child of this item by name.
    ///
    /// \param  inName  The name of the child.
    /// \return         The child item.
    ///
    /// \throw  ExceptionDataIO    If a child with the given name cannot be found.
    ProjectItem * getChild(const std::string & inName) const;

    /// Get a child of this item by index.
    ///
    /// \param  inIndex The index of the child.
    /// \return         The child item.
    ///
    /// \throw  ExceptionDataIO    If the index is greater than or equal to the number of children.
    ProjectItem * getChild(const isize_t inIndex) const;

    /// Check if this item has a child with a given name.
    ///
    /// \param  inName  The name of the child to check.
    /// \return         True if this has a child with the given name.
    bool isChild(const std::string & inName) const;

    /// \return The path of this item from its oldest ancestor.
    ///
    std::string getPath() const;

    /// \return The index of this in its parent, or -1 if it has no parent.
    ///
    int getIndex() const;

    /// Create a project item from a serialized JSON string.
    ///
    /// \param  inString    The serialized JSON string.
    /// \return             The deserialized project item.
    ///
    /// \throw  ExceptionDataIO If the string cannot be parsed.
    static std::shared_ptr<ProjectItem> fromJsonString(const std::string & inString);

protected:

    /// \throw ExceptionDataIO  If an item cannot be inserted into this due to self/ancestor constraints.
    ///
    void validateItemToBeInserted(const ProjectItem * inItem) const;

    /// \return     The ancestors of this (i.e. recursively get parents).
    ///
    std::vector<ProjectItem *> getAncestors() const;

    /// \return     True if the children of this item are modified.
    ///
    bool areChildrenModified() const;

    /// Set the children of this item to be unmodified.
    ///
    void setChildrenUnmodified();

    /// Convert a signed index to an unsigned one.
    ///
    /// \param  inIndex The signed index.
    /// \param  inSize  The size of the container.
    /// \return         The unsigned version of the index w.r.t. the size.
    ///                 If the signed index is negative, this will equal
    ///                 the size of the container.
    static size_t convertIndex(const int inIndex, const size_t inSize);
};

/// STL overload for print to a stream.
///
/// \param   inStream   The output stream to which to print.
/// \param   inItem     The item to print.
/// \return             The modified stream.
::std::ostream & operator<<(::std::ostream & inStream, const ProjectItem & inItem);

} // namespace isx

#endif // ISX_PROJECT_ITEM_H
