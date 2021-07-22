/*
  Yucong Jiang, June 2021
*/


#include "AudioToScoreAligner.h"
#include "Templates.h"
#include "SimpleHMM.h"

#include <cmath>
#include <vector>

AudioToScoreAligner::AudioToScoreAligner(float inputSampleRate, int hopSize) :
    m_inputSampleRate{inputSampleRate} , m_hopSize{hopSize}
{
}

AudioToScoreAligner::~AudioToScoreAligner()
{
}

bool AudioToScoreAligner::loadAScore(int blockSize)
{
    //string testScorePath = "/Users/yjiang3/Desktop/Pilot/testingScores/Barcarolle.solo";
    string testScorePath = "/Users/yjiang3/Desktop/Pilot/BothHandsC/BothHandsC.solo";
    bool success = m_score.initialize(testScorePath);
    NoteTemplates t =
        CreateNoteTemplates::getNoteTemplates(m_inputSampleRate, blockSize);
    m_score.setEventTemplates(t);
    return success;
}

void AudioToScoreAligner::supplyFeature(DataSpectrum s)
{
    m_dataFeatures.push_back(s);
}

void AudioToScoreAligner::initializeLikelihoods()
{
    int frames = m_dataFeatures.size();
    int events = m_score.getMusicalEvents().size();
    if (frames == 0) {
        std::cerr << "AudioToScoreAligner::initializeLikelihoods:\
        features are not supplied." << '\n';
    }
    for (int frame = 0; frame < frames; frame++) {
        vector<Likelihood> l;
        for (int event = 0; event < events; event++) {
            l.push_back(Likelihood(0, false));
        }
        m_likelihoods.push_back(l);
    }
}


double AudioToScoreAligner::getLikelihood(int frame, int event)
{
    if (m_dataFeatures.size() == 0) {
        std::cerr << "AudioToScoreAligner::getLikelihood:\
        features are not supplied." << '\n';
    }
    // TODO: check the range for frame and event
    if (!m_likelihoods[frame][event].calculated) {
        const Score::MusicalEventList& eventList = m_score.getMusicalEvents();
        double score = 0;

        for (int bin = 0; bin < m_dataFeatures[frame].size(); bin++) {
            score += m_dataFeatures[frame][bin]*log(eventList[event].eventTemplate[bin]);
        }
        m_likelihoods[frame][event].likelihood = exp(score);
    }

    return m_likelihoods[frame][event].likelihood;
}

AudioToScoreAligner::AlignmentResults AudioToScoreAligner::align()
{
    initializeLikelihoods(); // all zeros
    AlignmentResults results;

    SimpleHMM hmm = SimpleHMM(*this); // build state graph
    results = hmm.getAlignmentResults();

    return results;

/*
    // Maximum Likelihood Method:
    int frame = 0;
    for (const auto& feature: m_dataFeatures) {
        int maxEvent = 0;
        double maxL = 0;
        for (int event = 0; event < m_score.getMusicalEvents().size(); event++) {
            double l = getLikelihood(frame, event);
            if (l > maxL) {
                maxL = l;
                maxEvent = event;
            }
        }
        results.push_back(maxEvent);
        frame++;
    }

    return results;
*/
}

float AudioToScoreAligner::getSampleRate() const
{
    return m_inputSampleRate;
}

float AudioToScoreAligner::getHopSize() const
{
    return m_hopSize;
}

Score AudioToScoreAligner::getScore() const
{
    return m_score;
}

AudioToScoreAligner::DataFeatures AudioToScoreAligner::getDataFeatures() const
{
    return m_dataFeatures;
}