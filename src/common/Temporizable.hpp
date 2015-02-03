#ifndef TEMPORIZABLE_H
#define TEMPORIZABLE_H

#include <vector>

using namespace std;

class Temporizable
{
	private:
		vector<string> timerName;
		vector<uint64_t> timer;
		vector<uint64_t> period;

	public:
		Temporizable(vector<string> timerName, vector<uint64_t> period)
		{
			this->timerName = timerName;
			this->period = period;
			this->timer = period;
		}

		Temporizable(uint64_t period)
		{
			this->period.push_back(period);
			this->timer.push_back(period);
		}

		virtual void UpdateTimer(uint64_t diffTime)
		{
			for (unsigned int i=0; i < period.size(); i++)
			{
				if (timer[i] <= diffTime)
				{
					if (!timerName.empty())
						TimerAlarm(period[i], timerName.at(i));
					else
						TimerAlarm(period[i]);
					timer[i] = period[i];
				} else timer[i] -= diffTime;
			}
		}

		virtual void TimerAlarm(uint64_t timerPeriod, string timerName = "") = 0;
};

#endif
