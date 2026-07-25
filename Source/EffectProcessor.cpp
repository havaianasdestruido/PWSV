#include "EffectProcessor.h"

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
    return new EffectProcessor();
}
