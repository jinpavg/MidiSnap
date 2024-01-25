#include "c74_min.h"

using namespace c74::min;


class avg_midisnap : public object<avg_midisnap> {
public:
    MIN_DESCRIPTION	{"Snap MIDI notes to provided scale tones."};
    MIN_TAGS		{"utilities"};
    MIN_AUTHOR		{"Alex Van Gils"};
    MIN_RELATED		{"print, jit.print, dict.print"};
    
    inlet<>  MidiIn	{ this, "(int) incoming MIDI note number", "int" };
    inlet<>  ScaleIn { this, "(list) scale tones to snap to", "list" };
    inlet<>  RootIn  { this, "(int) root note of the scale as an offest 0..11", "int" };
    outlet<> output	{ this, "(int) outgoing MIDI note number", "int" };

    // pass the incoming MIDI note number to the outlet
    message<> midiin { this, "int",
        MIN_FUNCTION {

            int midi_output;

            if (inlet == 2) 
                {
                    root_note_in = args[0];
                }
            else if (inlet == 0) 
                {
                    midi_note_in = args[0];
                    
                    midi_output = QuantizeMidiNote(midi_note_in, scale_tones_in, root_note_in);
                    output.send(midi_output);
                } 
            
            return {};
        }
    };

    int QuantizeMidiNote(int midi_note, atoms scale_tones, int root_note)
    {
        int note_root_delta = midi_note - root_note;
        int semitone_range = 12; // for now, we'll assume a 12-semitone scale range
        int clamped_semitone_range = std::max(semitone_range, 1);
        int note_octave = floor(note_root_delta / clamped_semitone_range);
        int value_to_quantize = note_root_delta - (semitone_range * note_octave);

        int quantized_value = QuantizeValueToScale(value_to_quantize, scale_tones);

        // return the quantized value, plus the root note and octave
        return (semitone_range * note_octave + quantized_value) + root_note;
    }


    int QuantizeValueToScale(int value, atoms scale_tones)
    {
        // the scale_tones list should contain at least 2 values
        if (scale_tones.size() < 2)
        {
            cout << scale_tones.size() << " is not enough scale tones to quantize" << endl;
            for (auto& item : scale_tones)
            {
                cout << item << " ";
            }
            cout << " scale" << endl;
            return value;
        }

        // check the min delta against the scale range, which for now is 12
        int scale_range = 12;
        int quantized_value = scale_range;
        int current_min_delta = abs(value - scale_range);

        // check the scale tone list for the closest value
        for (number tone : scale_tones)
        {
            int delta = abs(value - tone);
            if (delta < current_min_delta)
            {
                current_min_delta = delta;
                quantized_value = tone;
            }
        }

        return quantized_value;

    }


    // set the scale tones list
    message<> list { this, "list",
        MIN_FUNCTION {
            
            if (inlet == 1) 
            {
                scale_tones_in = args;
            } 
            return {};
        }
    };


    // post to max window == but only when the class is loaded the first time
    message<> maxclass_setup { this, "maxclass_setup",
        MIN_FUNCTION {
            cout << "midisnap from avg" << endl;
            return {};
        }
    };

private:
    int midi_note_in { 0 };
    atoms scale_tones_in { 0 };
    int root_note_in { 0 };

};

MIN_EXTERNAL(avg_midisnap);
