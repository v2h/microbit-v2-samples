/*
 * Music Box Program
 * 
 * This program implements a micro:bit music box that plays a melody when light
 * is detected (lid opens) and stops when darkness is detected (lid closes).
 * It behaves like a real music box, looping the melody while light is present.
 */

#include "MicroBit.h"
#include "Tests.h"
#include <cmath>

// Music box state
static bool shouldPlay = false;
static bool isPlaying = false;

// Copied from OOB_v3.cpp
// Format: NOTE[octave][:duration]
static const int DEFAULT_TEMPO_BPM = 120;
static const int MS_PER_BPM = (60000 / DEFAULT_TEMPO_BPM) / 4;

#define NOTE_LEN 6

// Harry Potter theme
static const char MUSIC_BOX_MELODY[][NOTE_LEN] = {
    "E5:1","G5:1","A5:2",
    "G5:1","F#5:1","E5:2",
    "D5:1","C#5:1","B4:2",
    "C#5:1","D5:1","E5:4",

    "B4:1","C#5:1","D5:2",
    "E5:1","G5:1","A5:2",
    "G5:1","F#5:1","E5:2",
    "D5:1","C#5:1","B4:4",

    "E5:1","G5:1","A5:2",
    "G5:1","F#5:1","E5:2",
    "D5:1","C#5:1","B4:2",
    "C#5:1","D5:1","E5:4",

    "A4:1","E5:1","G5:2",
    "F#5:1","E5:1","D5:2",
    "C#5:1","B4:1","A4:2",
    "B4:1","C#5:1","D5:4",

    "R:2"
};

#define MELODY_LEN (sizeof(MUSIC_BOX_MELODY) / sizeof(MUSIC_BOX_MELODY[0]))

/**
 * Play a single note or rest.
 * Shamelessly copied from OOB_v3.cpp
 */
static void playMelody(const char note[NOTE_LEN]) {
    const char *note_char = &note[0];
    int distanceFromA = 0;
    int frequency = 0;
    bool rest = false;
    int octave = 4;
    int durationMs = 4 * MS_PER_BPM;

    // First process the note, as its distance from A
    switch (*note_char) {
        case 'A': distanceFromA = 0; break;
        case 'B': distanceFromA = 2; break;
        case 'C': distanceFromA = -9; break;
        case 'D': distanceFromA = -7; break;
        case 'E': distanceFromA = -5; break;
        case 'F': distanceFromA = -4; break;
        case 'G': distanceFromA = -2; break;
        case 'R': rest = true; break;
        default: return;
    }

    // Then process the optional #/b modifiers and/or scale
    note_char++;
    while (*note_char != ':' && *note_char != '\0') {
        if (*note_char == '#') {
            distanceFromA++;
        } else if (*note_char == 'b') {
            distanceFromA--;
        } else if ((*note_char >= '0') && (*note_char <= '9')) {
            octave = (*note_char - '0');
        }
        note_char++;
    }

    // If an optional duration is present, calculate the delay in ms
    if (*note_char == ':') {
        note_char++;
        if ((*note_char >= '0') && (*note_char <= '9')) {
            durationMs = atoi((const char*)note_char) * MS_PER_BPM;
        }
    }

    // Calculate note frequency, or keep it as zero for a rest
    if (!rest) {
        float distanceFromA4 = (octave - 4) * 12 + distanceFromA;
        frequency = (int)(440.0 * pow(2, distanceFromA4 / 12.0));
    }

    // Play the tone/rest for the calculated duration
    if (frequency > 0 && shouldPlay) {
        uBit.audio.virtualOutputPin.setAnalogPeriodUs(1000000 / frequency);
        uBit.audio.virtualOutputPin.setAnalogValue(127);
    } else {
        uBit.audio.virtualOutputPin.setAnalogValue(0);
    }
    
    uBit.sleep(durationMs);
    
    // Small break between notes for clarity
    uBit.audio.virtualOutputPin.setAnalogValue(0);
    uBit.sleep(10);
}

/**
 * Music box playback loop - plays the melody continuously while shouldPlay is true
 */
static void musicBoxPlaybackLoop()
{
    // Gotta keep track of the current note so that
    // when the box is opened again,
    // the melody continues where it's left off
    static size_t currentNoteIndex = 0;

    while (1) {
        if (shouldPlay && !isPlaying) {
            isPlaying = true;

            // Play from where we left off; stop if shouldPlay becomes false.
            for (; currentNoteIndex < MELODY_LEN && shouldPlay; ++currentNoteIndex) {
                playMelody(MUSIC_BOX_MELODY[currentNoteIndex]);
            }

            // If we reached the end of the melody, loop back to the start.
            if (currentNoteIndex >= MELODY_LEN) {
                currentNoteIndex = 0;
            }

            // Small pause between full loops (like a real music box)
            if (shouldPlay) {
                uBit.sleep(500);
            }

            isPlaying = false;
        } else {
            // Wait a bit before checking again
            uBit.sleep(100);
        }
    }
}

/**
 * Event handler for when light is detected (lid opens)
 */
static void onLight(MicroBitEvent) {
    DMESG("Light detected - starting music box");
    shouldPlay = true;

    const char * const indicator_box_open = 
        "000,255,255,255,000\n"
        "255,000,000,000,255\n"
        "255,000,000,000,255\n"
        "255,000,000,000,255\n"
        "000,255,255,255,000\n";
    
    // Show a visual indicator that the music box is active
    MicroBitImage musicNote(indicator_box_open);
    uBit.display.print(musicNote, 0, 0, 0, 500);
    uBit.display.clear();
    
    // Re-enable light sense mode after showing indicator
    uBit.display.setDisplayMode(DisplayMode::DISPLAY_MODE_BLACK_AND_WHITE_LIGHT_SENSE);
}

/**
 * Event handler for when darkness is detected (lid closes)
 */
static void onDark(MicroBitEvent) {
    DMESG("Darkness detected - stopping music box");
    shouldPlay = false;
    
    // Stop any currently playing note
    uBit.audio.virtualOutputPin.setAnalogValue(0);

    const char * const indicator_box_closed = 
        "255,000,000,000,255\n"
        "000,255,000,255,255\n"
        "000,000,255,000,255\n"
        "000,255,000,255,255\n"
        "255,000,000,000,255\n";
    
    // Show a visual indicator that the music box stopped
    MicroBitImage closedBox(indicator_box_closed);
    uBit.display.print(closedBox, 0, 0, 0, 500);
    uBit.display.clear();
    
    // Re-enable light sense mode after showing indicator
    uBit.display.setDisplayMode(DisplayMode::DISPLAY_MODE_BLACK_AND_WHITE_LIGHT_SENSE);
}

/**
 * Main music box program
 * Sets up event handlers and starts the playback loop
 */
void music_box()
{
    DMESG("Music Box Program Starting");
    
    // Enable light sensing on the display
    // This enables the events to be generated by putting the display in light sense mode
    // We explicitly set the mode to ensure light sensing stays enabled
    uBit.display.setDisplayMode(DisplayMode::DISPLAY_MODE_BLACK_AND_WHITE_LIGHT_SENSE);
    
    // Clear the display - we don't need to show anything, just sense light
    uBit.display.clear();
    
    // Set up event handlers for light/dark events
    uBit.messageBus.listen(MICROBIT_ID_DISPLAY, MICROBIT_DISPLAY_EVT_LIGHT_SENSE_LIGHT, onLight);
    uBit.messageBus.listen(MICROBIT_ID_DISPLAY, MICROBIT_DISPLAY_EVT_LIGHT_SENSE_DARK, onDark);
    
    // Initialize audio
    uBit.audio.setVolume(255);
    uBit.audio.requestActivation();

    const char * const m_shape = 
        "255,000,000,000,255\n"
        "255,255,000,255,255\n"
        "255,000,255,000,255\n"
        "255,000,000,000,255\n"
        "255,000,000,000,255\n";
    
    // Show a startup indicator (briefly, then clear for light sensing)
    MicroBitImage m_image(m_shape);
    uBit.display.print(m_image, 0, 0, 0, 1000);
    uBit.display.clear();
    
    // Re-enable light sense mode after printing (in case print changed it)
    uBit.display.setDisplayMode(DisplayMode::DISPLAY_MODE_BLACK_AND_WHITE_LIGHT_SENSE);
    
    // Start the playback loop in a fiber
    create_fiber(musicBoxPlaybackLoop);
    
    // Main loop - just wait for events
    // The display will continue to sense light and emit events in the background
    while (true) {
        uBit.sleep(1000);
    }
}

