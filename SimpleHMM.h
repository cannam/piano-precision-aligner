/*
  Yucong Jiang, July 2021
*/

#ifndef SIMPLE_HMM_H
#define SIMPLE_HMM_H

#include "AudioToScoreAligner.h"

#include <vector>
#include <map>
#include <sstream> // for printing probs with high precision

using std::vector;


class SimpleHMM
{
public:
    SimpleHMM(AudioToScoreAligner& aligner); // not const bc of getLikelihood()
    ~SimpleHMM();

    struct State {
        int eventIndex; // -1 means before first event; -2 means after last event
        int microIndex; // index of this microstate within the event
        //unordered_map<State*, double> nextStates; // value is the transition prob
        //unordered_map<State*, double> prevStates;
        // careful about using pointers as keys.
        // unordered_map<pair<event, micro>, double> nextStates;

        State(int e, int m) : eventIndex{e}, microIndex{m} { }
        State(const State& s) : eventIndex{s.eventIndex}, microIndex{s.microIndex} { }

        bool operator==(const State &other) const {
            if (eventIndex == other.eventIndex && microIndex == other.microIndex)
                return true;
            return false;
        }

        bool operator<(const State& other) const {
            if (eventIndex < other.eventIndex)  return true;
            else if (eventIndex > other.eventIndex) return false;
            // equal eventIndex:
            if (microIndex < other.microIndex)  return true;
            else if (microIndex > other.microIndex) return false;
            // equal eventIndex and equal microIndex:
            return false;
        }

        bool operator>(const State& other) const {
            return !( (*this < other) || (*this == other) );
        }

        static string toString(const State& s) {
            string ss = to_string(s.eventIndex) + "\t" + to_string(s.microIndex);
            /*
            for (auto& p : s.nextStates) {
                ss += "\t next: " + to_string(p.first->eventIndex) +
                 " " + to_string(p.first->microIndex) + " " + to_string(p.second);
            }
            for (auto& p : s.prevStates) {
                ss += "\t prev: " + to_string(p.first->eventIndex) +
                 " " + to_string(p.first->microIndex) + " " + to_string(p.second);
            }
            */
            return ss;
        }
    };

    //typedef State StateGraph; // Starting State (eventIndex = -1)

    struct Hypothesis {
        State state;
        double prob; // might be in log
        Hypothesis(const State& s, double p) : state{s}, prob{p} { }
        Hypothesis(const Hypothesis& h) : state{h.state}, prob{h.prob} { }
        // Design assignment operator

        Hypothesis& operator=(const Hypothesis& other) {
            state = other.state;
            prob = other.prob;
            return *this;
        }

        bool operator==(const Hypothesis &other) const {
            if (state == other.state && prob == other.prob)
                return true;
            return false;
        }

        // Compare prob first; if equal, compare state.
        bool operator<(const Hypothesis& other) const {
            if (prob < other.prob)  return true;
            else if (prob > other.prob) return false;
            return state < other.state;
        }

        bool operator>(const Hypothesis& other) const {
            return !( (*this < other) || (*this == other) );
        }

        static string toString(const Hypothesis& h) {
            std::ostringstream out;
            out.precision(38);
            out << std::fixed << h.prob;
            return out.str() + ":\t" + State::toString(h.state);
            // string ss = to_string(h.prob);
            //return ss + ":\t" + State::toString(h.state);
        }
    };

    AudioToScoreAligner::AlignmentResults getAlignmentResults();
    const map<State, map<State, double>>& getNextStates() const;

private:
    AudioToScoreAligner m_aligner;
    map<State, map<State, double>> m_nextStates; // value is <next state, trans prob>
    map<State, map<State, double>> m_prevStates; // value is <prev state, trans prob>
};

#endif
