#include "EffectProcessor.h"

juce::AudioProcessor* JUCE_CALLTYPE createEffectFilter() {
    return new EffectProcessor();
}
