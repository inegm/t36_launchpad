#ifndef LAUNCHPAD_H
#define LAUNCHPAD_H

#include <cstdint>
using std::uint8_t;
#include <USBHost_t36.h>


const uint8_t AUTOMAP_BASE_VAL = 104;
const uint8_t CELLS_PER_ROW = 8;
const uint8_t CELL_BASE_KEY = 16;
const uint8_t CELL_BASE_VELOCITY = 12;
const uint8_t FLASH_FLAG = 8;
const uint8_t META_CC_CONTROL = 0;
const uint8_t RESET_CC_VAL = 0;
const uint8_t TEST_CC_VAL = 124;
const uint8_t VELOCITY_FACTOR = 16;

enum class Color {RED, ORANGE, GREEN};
enum class Brightness {OFF, SOFT, MEDIUM, BRIGHT};
enum class ButtonCategory {GRID_CELL, PARAMETER, AUTOMAP};

struct Button
{
    ButtonCategory category;
    uint8_t key;
    uint8_t index;
};


class Launchpad : public MIDIDevice
{
public:
    uint8_t midi_channel {1};
    Launchpad(USBHost host, uint8_t channel=1) : MIDIDevice{host} {
        usb_host = host;
        midi_channel = channel;
    };

    USBHost getUSBHost() { return usb_host; }

    void begin() { 
        usb_host.begin();
        resetLaunchpad();
    }

    // Set the color and brightness of one of the 64 square cells.
    void setCellButton(
        uint8_t cell_index,
        Color color,
        Brightness brightness,
        bool flash=false
    ) {
        // uint8_t key = cellKeyFromIndex(cell_index);
        // uint8_t velocity = getVelocityValue(color, brightness, flash);
        sendNoteOn(
            cellKeyFromIndex(cell_index),
            getVelocityValue(color, brightness, flash),
            midi_channel
        );
    }

    // Set the color and brightness of one of the 8 automap buttons (top).
    void setAutomapButton(
        uint8_t button_index,
        Color color,
        Brightness brightness,
        bool flash=false
    ) {
        // uint8_t velocity = getVelocityValue(color, brightness, flash);
        sendControlChange(
            button_index + AUTOMAP_BASE_VAL,
            getVelocityValue(color, brightness, flash),
            midi_channel
        );
    }

    // Set the color and brightness of one of the 8 parameter buttons (right)
    void setParameterButton(
        uint8_t button_index,
        Color color,
        Brightness brightness,
        bool flash=false
    ) {
        // uint8_t velocity = getVelocityValue(color, brightness, flash);
        // uint8_t key = CELL_BASE_KEY * button_index + CELLS_PER_ROW;
        sendNoteOn(
            CELL_BASE_KEY * button_index + CELLS_PER_ROW,
            getVelocityValue(color, brightness, flash),
            midi_channel
        );
    }

    // Parse a midi key to identify its button's type and index.
    Button parseNote(uint8_t key)
    {
        uint8_t col = key % CELL_BASE_KEY;
        uint8_t row = (key - col) / CELL_BASE_KEY;
        Button button;
        if (col == 8) 
        { 
            button.category = ButtonCategory::PARAMETER;
            button.index = row;
        } else 
        {
            button.category = ButtonCategory::GRID_CELL;
            button.index = (row * CELLS_PER_ROW) + col;
        }
        button.key = key;
        return button;
    }

    Button parseCC(uint8_t value)
    {
        Button button;
        button.category = ButtonCategory::AUTOMAP;
        button.index = value - AUTOMAP_BASE_VAL;
        button.key = value;
        return button;
    }

    void resetLaunchpad() { 
        sendControlChange(
            META_CC_CONTROL,
            RESET_CC_VAL,
            midi_channel
        ); 
    }

    void testBrightness(Brightness brightness)
    {
        if (brightness == Brightness::OFF) 
        {
            resetLaunchpad();
        } else 
        {
            sendControlChange(
                META_CC_CONTROL,
                TEST_CC_VAL + static_cast<uint8_t>(brightness),
                midi_channel
            );
        }
    }

private:
    USBHost usb_host {};

    // Get the MIDI key number for a cell from its index.
    uint8_t cellKeyFromIndex(uint8_t cell_index)
    {
        return (
            CELL_BASE_KEY * (cell_index / CELLS_PER_ROW) +
            cell_index % CELLS_PER_ROW
        );
    }

    // Get the velocity value required to set for a given color and brightness.
    uint8_t getVelocityValue(
        Color color,
        Brightness brightness,
        bool flash=false
    ) {
        uint8_t velocity {CELL_BASE_VELOCITY};
        switch(color)
        {
        case Color::RED:
            velocity += static_cast<uint8_t>(brightness);
            break;
        case Color::ORANGE:
            velocity += VELOCITY_FACTOR * static_cast<uint8_t>(brightness);
            velocity += static_cast<uint8_t>(brightness);
            break;
        case Color::GREEN:
            velocity += VELOCITY_FACTOR * static_cast<uint8_t>(brightness);
            break;
        }
        if (flash) { velocity += FLASH_FLAG; }
        return velocity;
    }
};

#endif
