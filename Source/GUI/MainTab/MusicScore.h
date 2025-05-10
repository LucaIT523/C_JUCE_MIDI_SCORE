/*
  ==============================================================================

    MusicScore.h
    Created: 10 Nov 2023 5:10:57pm
    Author:  Marko Des

  ==============================================================================
*/
#ifndef MUSICSCORE_H_INCLUDED
#define MUSICSCORE_H_INCLUDED

//#include "../Device/GuidoComponent.h"
#include "JuceHeader.h"
#include "../../AppData.h"
#include "../../Midi/MidiSequence.h"
#include "../../Midi/MidiHelpers.h"
#include "NoteButton.h"

class MusicScore : public Component
{
public:
    MusicScore(MidiHelpers& midiHelperIn);

    ~MusicScore();

    //==============================================================================
    void paint(Graphics& g) override;
    void resized() override;
    void mouseDown(const MouseEvent& event) override;

    /** Scale width of this component. Afterwards resize and redraw.
    */
    void scale(double scaleFactor);

    /** Indicates that a new file is laoded and we have to change the piano roll
    */
    void newFileLoaded();
    /** Calculate X-Offset in pixels from the relative midi time in seconds.
        Used in the viewport to set the correct viewing position.
    */
    int getXOffsetFromMidiTime(double time);
    /** Chekc if the track is enabled.
    */
    bool isEnabled(const int track) const;
    /** Set the focus to the track selected.
    */
    void setFocusTrack(const int track);
    /** Enable the track
    */
    void enableTrack(const int track);
    /** Enable the track
    */
    void disableTrack(const int track);
    /** Set the state of the track (i.e. selected) and bring it to front, if needed.
    */
    void setTrackState(const int track, const NoteButton::NoteButtonState state, const bool bringToFront = false);
    /** Highlight the notes, which are currently "pressed down" in the Midi file.
    */
    void highlightNotes();
    /** Sets the midi time of the midi player to the given value
    */
    void setMidiTime(const double time);
    /** Reset object data.
    */
    void reset();


private:

    /** Collection of common midi functions*/
    MidiHelpers& midiHelper;
    int xOffset;
    /** The track, which is currently in focus. */
    int focusTrack;
    /** Array with all notes as buttons */
    OwnedArray<NoteButton> noteButtons;
    /** Start of the noteButtons for each midi track */
    Array<int> noteButtonStart;
    /** End of the noteButtons for each midi track */
    Array<int> noteButtonEnd;
    /** Indices of currently higlighted notes.
    */
    Array<int> higlightedNotes;
    /** X-Coordinate of the klicked marker
    */
    int markerLineX;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MusicScore);
};

class MusicScoreViewport : public Viewport
{
public:
    MusicScoreViewport(const String& name = String::empty);
    void setViewedComponent(MusicScore* newMusicScore, bool deleteComponentWhenNoLongerNeeded = true);
    void paint(Graphics& g) override;
    void setBounds(int x, int y, int width, int height);
    void setBoundsForMusicScore();
    void newFileLoaded();
    int musicScoreHeight();
private:
    MusicScore* musicScoreRef;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MusicScoreViewport);
};
#endif