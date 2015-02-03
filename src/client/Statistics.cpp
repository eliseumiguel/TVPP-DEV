#include "Statistics.hpp"

// Global static pointer used to ensure a single instance of the class.
Statistics* Statistics::m_pInstance = NULL;  

/** This function is called to create an instance of the class. 
    Calling the constructor publicly is not allowed. The constructor 
    is private and is only called by this Instance function.
*/
  
Statistics* Statistics::Instance()
{
   if (!m_pInstance)   // Only allow one instance of class to be generated.
      m_pInstance = new Statistics;

   return m_pInstance;
}

int Statistics::GetEstimatedChunkRate()
{
    return estimatedChunkRate;
}

void Statistics::SetEstimatedChunkRate(int ecr)
{
    estimatedChunkRate = ecr;
}

float Statistics::GetLatency()
{
    float latency = 0;
    int count = 0;
    for (list<int>::iterator it=latencyVector.begin(); it != latencyVector.end(); ++it)
    {
        latency += *it;
        count++;
    }
    
    return latency/count;
}

void Statistics::AddLatencySample(int sampleLatency)
{
    latencyVector.push_back(sampleLatency);
    if (latencyVector.size() > 30)
    {
        latencyVector.pop_front();
    }
}


/*if (estimatedChunkRate < 0)
+                estimatedChunkRate = chunksRecv;
+            else
+            {
+                float alpha = 0.125, beta = 0.25;
+                estimatedChunkRate = (1-alpha)*estimatedChunkRate + alpha*chunksRecv;
+                devEstimatedChunkRate = (1-beta)*devEstimatedChunkRate + beta*abs(chunksRecv-estimatedChunkRate);
+            }
*/