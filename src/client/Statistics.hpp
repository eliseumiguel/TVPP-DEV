#ifndef STATISTICS_H
#define STATISTICS_H

#include <string>
#include <list>

using namespace std;

class Statistics {
    public:
        static Statistics* Instance();
        int GetEstimatedChunkRate();
        void SetEstimatedChunkRate(int estimatedChunkRate);
        float GetLatency();
        void AddLatencySample(int sampleLatency);

    private:
        Statistics(){estimatedChunkRate=0;};  // Private so that it can  not be called
        Statistics(Statistics const&){};             // copy constructor is private
        void operator=(Statistics const&){};  // assignment operator is private
        static Statistics* m_pInstance;
        
        int estimatedChunkRate;
        list<int> latencyVector; 
};

#endif