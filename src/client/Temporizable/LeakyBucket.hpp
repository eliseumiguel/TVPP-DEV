#ifndef LEAKY_BUCKET_H
#define LEAKY_BUCKET_H

#include <iostream>
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>

#include "../../common/Defines.hpp"
#include "../../common/Temporizable.hpp"

class LeakyBucket: public Temporizable
{
    private:
	    uint32_t bucketSize, tokens;
		boost::mutex leakyBucketMutex;
        boost::condition_variable lowLeakyBucket;
        boost::mutex lowLeakyBucketMutex;

	public:
	    LeakyBucket(uint32_t bucketSize, uint64_t timerPeriod = 100000000);
        void IncToken(uint32_t tokensReplaced);
        bool DecToken(uint32_t tokensRequested);
		uint32_t GetTokens();
		uint32_t GetBucketSize();
		void Replenish(float percentageToFill);
		void TimerAlarm(uint64_t timerPeriod, string timerName);
};

#endif
