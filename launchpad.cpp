/* Novation Launchpad mini examples */

#include <arduino.h>
#include "launchpad.h"

USBHost host;
Launchpad pad(host);
bool automaps[8];

void noteOnHandler(byte channel, byte note, byte velocity);
void noteOffHandler(byte channel, byte note, byte velocity);
void controlChangeHandler(byte channel, byte note, byte velocity);


void setup()
{
    Serial.begin(115200);
    delay(1500);
    pad.begin();
    delay(10);
    pad.setHandleNoteOn(noteOnHandler);
    pad.setHandleNoteOff(noteOffHandler);
    pad.setHandleControlChange(controlChangeHandler);
}

void loop()
{
    host.Task();
    pad.read();
}

void noteOnHandler(byte channel, byte note, byte velocity)
{
    Serial.print("Note ON, ch=");
    Serial.print(channel, DEC);
    Serial.print(", note=");
    Serial.println(note, DEC);
}

void noteOffHandler(byte channel, byte note, byte velocity)
{
    Serial.print("Note OFF, ch=");
    Serial.print(channel, DEC);
    Serial.print(", note=");
    Serial.println(note, DEC);
}

void controlChangeHandler(byte channel, byte value, byte velocity)
{
    if (velocity)
    {
        Serial.print("CC, ch=");
        Serial.print(channel, DEC);
        Serial.print(", value=");
        Serial.println(value, DEC);
    } else
    {
        Button button = pad.parseCC(value);
        if (automaps[button.index])
        {
            pad.setAutomapButton(button.index, Color::GREEN, Brightness::OFF);
            automaps[button.index] = false;
        } else
        {
            pad.setAutomapButton(button.index, Color::GREEN, Brightness::BRIGHT);
            automaps[button.index] = true;
        }
    }
}
