#ifndef ISX_PLAYER_H
#define ISX_PLAYER_H

#include <QBoxLayout>
#include <memory>


namespace isx
{

///
/// A class implementing a player.
///
class Player
{
public:
    /// Constructor.
    /// \param inParent BoxLayout to which this player will at its window into which it draw its frames
    ///
    Player(const std::shared_ptr<QBoxLayout> & inParent);

    /// Destructor.
    ///
    ~Player();

    /// Start playback.
    ///
    void
    start();

    /// Stop playback.
    ///
    void
    stop();

    /// Query playback state
    /// \return true means is this player is in playing state
    ///
    bool
    isPlaying();

private:
    class MyOpenGLWidget;
    std::shared_ptr<MyOpenGLWidget> window_;

    bool isPlaying_ = false;

};

} // namespace isx

#endif // ISX_PLAYER_H
