// Music 256a / CS 476a | fall 2016
// CCRMA, Stanford University
//
// Author: Romain Michon (rmichonATccrmaDOTstanfordDOTedu)
// Description: Simple JUCE sine wave synthesizer

#ifndef MAINCOMPONENT_H_INCLUDED
#define MAINCOMPONENT_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"
#include "Sine.h"
#include "Smooth.h"
#include "FaustReverb.h"
#include "stk/ADSR.h"
#include "stk/BlitSaw.h" 
#define oscNum 8 //number of the oscs that you're using

using namespace stk;

class MainContentComponent :
    public AudioAppComponent,
    private Slider::Listener,
    private ToggleButton::Listener
{
public:
	MainContentComponent() : onOff(0), samplingRate(0.0), onSaw(0)
    {
		//connect key to the keyboard
		keys[0] = KeyPress('A');
		keys[1] = KeyPress('S');
		keys[2] = KeyPress('D');
		keys[3] = KeyPress('F');
		keys[4] = KeyPress('G');
		keys[5] = KeyPress('H');
		keys[6] = KeyPress('J');
		keys[7] = KeyPress('K');

        //use for loop to configure multiple oscs
		for (int i = 0; i < oscNum-1; i++) {
		    // configuring frequency slider and adding it to the main window
			addAndMakeVisible(frequencySlider[i]);
			frequencySlider[i].setRange(50.0, 5000.0);
			frequencySlider[i].setSkewFactorFromMidPoint(500.0);
			//frequencySlider[i].setSliderStyle(Slider::Rotary);
			frequencySlider[i].setValue(normalKey[i]); // will also set the default frequency of the sine osc
			frequencySlider[i].addListener(this);

			// configuring frequency label box and adding it to the main window
			addAndMakeVisible(frequencyLabel[i]);
			frequencyLabel[i].setText("Frequency", dontSendNotification);
			frequencyLabel[i].attachToComponent(&frequencySlider[i], true);


			// configuring gain slider and adding it to the main window
			addAndMakeVisible(gainSlider[i]);
			gainSlider[i].setRange(0.0, 1.0);
			gain[i] = 0.0; // initialize gain value
			gainSlider[i].setValue(0.5); // will alsi set the default gain of the sine osc
			gainSlider[i].addListener(this);


			// configuring gain label and adding it to the main window
			addAndMakeVisible(gainLabel[i]);
			gainLabel[i].setText("Gain", dontSendNotification);
			gainLabel[i].attachToComponent(&gainSlider[i], true);

			//configuring mute button and add it to the main window
			//addAndMakeVisible(muteButton[i]);
			onMute[i] = 1;
			//muteButton[i].addListener(this);

			////configuring mute label and add it to the main window
			//addAndMakeVisible(muteLabel[i]);
			//muteLabel[i].setText("Mute", dontSendNotification);
			//muteLabel[i].attachToComponent(&muteButton[i], true);
			
			//configuring key button and add it to the main window
			addAndMakeVisible(keyButton[i]);
			keyButton[i].addListener(this);

			//addAndMakeVisible(pianos[i]);
			
		}
		//configuring button for normal C4-B4 and add it to the main window
		addAndMakeVisible(normalButt);
		normalButt.setButtonText("Normal");
		normalButt.addListener(this);

		//configuring button for poker face keys add it to the main window
		addAndMakeVisible(gagaButt);
		gagaButt.setButtonText("GaGa");
		gagaButt.addListener(this);
        
        // configuring on/off button and adding it to the main window
        addAndMakeVisible(onOffButton);
        onOffButton.addListener(this);
        
        
        // configuring on/off label and adding it to the main window
        addAndMakeVisible(onOffLabel);
        onOffLabel.setText ("On/Off", dontSendNotification);
        onOffLabel.attachToComponent (&onOffButton, true);

		//set adsr value
		carrierADSR.setAllTimes(0.476/1000, 2, 0.2, 0.1);
		//i tried to lower the click and pop sound using adsr

        setSize (600, (oscNum-1)*80 + 56 + 40);
        nChans = 2;
        setAudioChannels (0, nChans); // no inputs, one output
		audioBuffer = new float*[nChans];
    }
    
    ~MainContentComponent()
    {
        shutdownAudio();
    }
	void LookAndFeel() {
		//don't have time for this. maybe next time
	}

	//change background color
	void paint(Graphics& g) override
	{
		g.fillAll(Colour::greyLevel(0.4f));
	}

    void resized() override
    {
        // placing the UI elements in the main window
        // getWidth has to be used in case the window is resized by the user
        const int sliderLeft = 80;
		for (int i = 0; i < oscNum-1; i++) {
			frequencySlider[i].setBounds(sliderLeft, 10 + i*80 , getWidth() - sliderLeft - 20, 20);
			gainSlider[i].setBounds(sliderLeft, 40 + i*80, getWidth() - sliderLeft - 20, 20);

		}
		onOffButton.setBounds(sliderLeft, 26 + (oscNum-1)*80, getWidth() - sliderLeft - 20, 20);
		normalButt.setBounds(sliderLeft , 26 + (oscNum-1) * 80 + 20, 40, 20);
		gagaButt.setBounds(sliderLeft , 26 + (oscNum-1) * 80 + 40, 40, 20);
    }
    
    void sliderValueChanged (Slider* slider) override
    {
        if (samplingRate > 0.0){
			for (int i = 0; i < oscNum-1; i++) {
				if (slider == &frequencySlider[i]) {
					sine[i].setFrequency(frequencySlider[i].getValue());
					saw[i].setFrequency(frequencySlider[i].getValue());
				}
				else if (slider == &gainSlider[i]) {
					gain[i] = gainSlider[i].getValue();
				}
			}

        }
    }
    

    void buttonClicked (Button* button) override
    {
        // turns audio on or off
        if(button == &onOffButton && onOffButton.getToggleState()){
            onOff = 1;
        }
        else if(button == &onOffButton && !onOffButton.getToggleState() ){
            onOff = 0;
		}
		//set slider value for a normal c4-b4 key set
		else if (button == &normalButt ) {
			onSaw = 0;
			for (int i = 0; i < oscNum-1; i++) {
				frequencySlider[i].setValue(normalKey[i]);
			}
		}//for poker face key set
		else if (button == &gagaButt ) {
			onSaw = 1;
			for (int i = 0; i < oscNum-1; i++) {
				frequencySlider[i].setValue(gagaKey[i]);
			}
		}


    }

	//for keyboard control
	bool keyStateChanged(bool isKeyDown) override
	{
		for (int i = 0; i < oscNum; i++) {//why is here always missing the last key? I have to save oscNum to 8 to avoid the last null key
			if (!KeyPress::isKeyCurrentlyDown( keys[i].getKeyCode() )) {
				carrierADSR.keyOn();
				onMute[i] = 1;
			}
			else if (KeyPress::isKeyCurrentlyDown( keys[i].getKeyCode() )) {
				carrierADSR.keyOff();
				onMute[i] = 0;
			}

		}

		return true;
	}


    void prepareToPlay (int /*samplesPerBlockExpected*/, double sampleRate) override
    {
		for (int i = 0; i < oscNum-1; i++) {
			samplingRate = sampleRate;
			sine[i].setSamplingRate(sampleRate);
			smooth[i].setSmooth(0.999);
		}
		reverb.init(sampleRate);
		reverb.buildUserInterface(&reverbControl);
    }
    
    void releaseResources() override
    {
    }
    
    void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override
    {
        // getting the audio output buffer to be filled
		for (int i = 0; i<nChans; i++) {
			audioBuffer[i] = bufferToFill.buffer->getWritePointer(i, bufferToFill.startSample);
		}
        
        // computing one block
        for (int sample = 0; sample < bufferToFill.numSamples; ++sample)
        {
			if (onOff == 1) {
				if (onSaw == 0) {
					for (int i = 0; i < oscNum; i++) {
						if (onMute[i] == 0) {
							audioBuffer[0][sample] += sine[i].tick() * smooth[i].tick(gain[i]) *smooth[i].tick(onOff)*smooth[i].tick(onMute[i])* carrierADSR.tick();
						}
					}
				}
				else if (onSaw == 1) {
					for (int i = 0; i < oscNum; i++) {
						if (onMute[i] == 0) {
							audioBuffer[0][sample] += saw[i].tick() * smooth[i].tick(gain[i]) *smooth[i].tick(onOff)*smooth[i].tick(onMute[i])* carrierADSR.tick();
						}
					}
				}

				audioBuffer[1][sample] = audioBuffer[0][sample];
			}
			else {
				audioBuffer[0][sample] = 0;
				audioBuffer[1][sample] = 0;
				
			}
        }
		reverb.compute(bufferToFill.numSamples, audioBuffer, audioBuffer);
    }
    
    
private:
    // UI Elements
    Slider frequencySlider[oscNum];
    Slider gainSlider[oscNum];
    ToggleButton onOffButton;
	LookAndFeel_V1 look;
	//ToggleButton muteButton[oscNum];
	TextButton keyButton[oscNum];
	TextButton normalButt;
	TextButton gagaButt;
    Label frequencyLabel[oscNum], gainLabel[oscNum], onOffLabel, muteLabel[oscNum];
    
    Sine sine[oscNum]; // the sine wave oscillator
	BlitSaw saw[oscNum]; // sawtooth osc
	ADSR carrierADSR;

    //key parameters
	KeyPress keys[oscNum];
	double gagaKey[8] = { 220, 110, 164.8, 174.6, 132.8, 196, 98 ,110};
	double normalKey[8] = { 261.3, 293.66, 329.63, 349.23, 392, 440, 493.88,110 };
	
	FaustReverb reverb;
	MapUI reverbControl;
	float** audioBuffer;

    // Global Variables
    float gain[oscNum];
    int onOff,onSaw, samplingRate, nChans;
	int onMute[oscNum];
    
	Smooth smooth[oscNum]; // to prevent cliking of some of the parameters

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainContentComponent)
};

Component* createMainContentComponent()     { return new MainContentComponent(); }


#endif  // MAINCOMPONENT_H_INCLUDED
