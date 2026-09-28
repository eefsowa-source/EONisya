#include "PluginProcessor.h"

#include "UI/HeaderStrip.h"
#include "UI/LCDPanel.h"
#include "UI/MixingPage.h"
#include "UI/PerformPage.h"
#include "UI/VoiceEditPage.h"

namespace
{
    constexpr int headerHeight = 64;
    constexpr int pageHeight = 470;
    constexpr int editorWidth = 980;

    const juce::StringArray modeNames { "PERFORM", "VOICE", "MIXING", "SONG", "PATTERN" };
}

class EONisyaAudioProcessorEditor : public juce::AudioProcessorEditor,
                                    private juce::Timer
{
public:
    explicit EONisyaAudioProcessorEditor (EONisyaAudioProcessor& p)
        : juce::AudioProcessorEditor (&p), processor (p)
    {
        header = std::make_unique<UI::HeaderStrip> (
            modeNames[0], [this] (int mode) { setMode (mode); }, 0);
        addAndMakeVisible (*header);

        performPage   = std::make_unique<UI::PerformPage>   (processor.getAPVTS());
        voiceEditPage = std::make_unique<UI::VoiceEditPage> (processor.getAPVTS());
        mixingPage    = std::make_unique<UI::MixingPage>    (processor.getAPVTS());
        placeholderLcd = std::make_unique<UI::LCDPanel>();

        addChildComponent (*performPage);
        addChildComponent (*voiceEditPage);
        addChildComponent (*mixingPage);
        addChildComponent (*placeholderLcd);

        setSize (editorWidth, headerHeight + pageHeight);
        setMode (static_cast<int> (processor.getAPVTS()
                                     .getRawParameterValue (ParamIDs::page)->load()));
        startTimerHz (10);
    }

    ~EONisyaAudioProcessorEditor() override { stopTimer(); }

    void paint (juce::Graphics& g) override
    {
        g.fillAll (Theme::panelBg);
    }

    void resized() override
    {
        auto b = getLocalBounds();
        header->setBounds (b.removeFromTop (headerHeight));
        for (auto* page : { static_cast<juce::Component*> (performPage.get()),
                            static_cast<juce::Component*> (voiceEditPage.get()),
                            static_cast<juce::Component*> (mixingPage.get()),
                            static_cast<juce::Component*> (placeholderLcd.get()) })
            page->setBounds (b);
    }

private:
    void setMode (int mode)
    {
        mode = juce::jlimit (0, modeNames.size() - 1, mode);
        currentMode = mode;
        header->setMode (modeNames[mode], mode);

        if (auto* p = processor.getAPVTS().getParameter (ParamIDs::page))
            if (mode <= 2)
                p->setValueNotifyingHost (p->getNormalisableRange().convertTo0to1 (mode));

        performPage->setVisible (mode == 0);
        voiceEditPage->setVisible (mode == 1);
        mixingPage->setVisible (mode == 2);

        placeholderLcd->setVisible (mode >= 3);
        if (mode >= 3)
            placeholderLcd->setTitle ("EON MOTIF6", modeNames[mode],
                                      "MODE", modeNames[mode],
                                      "STATUS", "NOT IMPLEMENTED");
    }

    void timerCallback() override
    {
        auto& apvts = processor.getAPVTS();
        header->setValues (
            juce::String (apvts.getRawParameterValue (ParamIDs::tempo)->load(), 1),
            juce::String (static_cast<int> (apvts.getRawParameterValue (ParamIDs::masterLevel)->load())),
            juce::String (processor.getActiveVoiceCount()));
    }

    EONisyaAudioProcessor& processor;
    int currentMode = 0;

    std::unique_ptr<UI::HeaderStrip> header;
    std::unique_ptr<UI::PerformPage> performPage;
    std::unique_ptr<UI::VoiceEditPage> voiceEditPage;
    std::unique_ptr<UI::MixingPage> mixingPage;
    std::unique_ptr<UI::LCDPanel> placeholderLcd;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EONisyaAudioProcessorEditor)
};

bool EONisyaAudioProcessor::hasEditor() const { return true; }

juce::AudioProcessorEditor* EONisyaAudioProcessor::createEditor()
{
    return new EONisyaAudioProcessorEditor (*this);
}
