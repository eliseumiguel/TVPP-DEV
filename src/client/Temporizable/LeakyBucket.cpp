#include "LeakyBucket.hpp"

LeakyBucket::LeakyBucket(uint32_t bucketSize, uint64_t timerPeriod) : Temporizable(timerPeriod)
{
    this->bucketSize = bucketSize;
	this->tokens = bucketSize;
}

uint32_t LeakyBucket::GetTokens()
{
	return tokens;
}

uint32_t LeakyBucket::GetBucketSize()
{
	return bucketSize;
}

void LeakyBucket::IncToken(uint32_t tokensReplaced)
{
	boost::mutex::scoped_lock leakyBucketLock(leakyBucketMutex);
    tokens += tokensReplaced;
    if (tokens > bucketSize)
	    tokens = bucketSize;
    lowLeakyBucket.notify_one();
	leakyBucketLock.unlock();
}

bool LeakyBucket::DecToken(uint32_t tokensRequested)
{
    boost::unique_lock<boost::mutex> lowLock(lowLeakyBucketMutex);
	boost::mutex::scoped_lock leakyBucketLock(leakyBucketMutex);
    if ((tokens >= tokensRequested))
	{
	    tokens -= tokensRequested;
		leakyBucketLock.unlock();
		return true;
	}
	leakyBucketLock.unlock();
    lowLeakyBucket.wait(lowLock);
	return false;
}

void LeakyBucket::Replenish(float percentageToFill)
{
	IncToken(GetBucketSize()*percentageToFill);
}

void LeakyBucket::TimerAlarm(uint64_t timerPeriod, string timerName)
{
	Replenish((float)timerPeriod/SECONDS);
}