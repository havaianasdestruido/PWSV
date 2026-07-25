#include "GeneratorProcessor.h"

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
    return new GeneratorProcessor();
}
