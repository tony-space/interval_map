#include <algorithm>
#include <iostream>
#include <cassert>
#include <random>
#include <vector>
#include <memory>

#include "interval_map.hpp"

class IntegerKey
{
public:
	IntegerKey(int k) : m_key(k)
	{
	}
	IntegerKey(const IntegerKey& o)
	{
		m_key = o.m_key;
	}
	IntegerKey(IntegerKey&& o) noexcept
	{
		m_key = std::move(o.m_key);

		// simulating move semantics side-effect inside original object
		o.m_key = 0;
	}

	IntegerKey& operator=(const IntegerKey& o)
	{
		if (&o != this)
		{
			m_key = o.m_key;
		}
		return *this;
	}
	IntegerKey& operator=(IntegerKey&& o) noexcept
	{
		if (&o != this)
		{
			m_key = std::move(o.m_key);
			// simulating move semantics side-effect inside original object
			o.m_key = 0;
		}
		return *this;
	}

	friend bool operator< (const IntegerKey& x, const IntegerKey& y);
private:
	int m_key;
};

bool operator< (const IntegerKey& x, const IntegerKey& y)
{
	return x.m_key < y.m_key;
}

class CharValue
{
public:
	CharValue(char v) : m_value(v)
	{
	}
	CharValue(const CharValue& o)
	{
		m_value = o.m_value;
	}
	CharValue(CharValue&& o) noexcept
	{
		m_value = std::move(o.m_value);
		// simulating move semantics side-effect inside original object
		o.m_value = '\0';
	}

	CharValue& operator=(const CharValue& o)
	{
		if (&o != this)
		{
			m_value = o.m_value;
		}
		return *this;
	}
	CharValue& operator=(CharValue&& o) noexcept
	{
		if (&o != this)
		{
			m_value = std::move(o.m_value);
			// simulating move semantics side-effect inside original object
			o.m_value = '\0';
		}
		return *this;
	}

	friend bool operator== (const CharValue& x, const CharValue& y);
private:
	char m_value;
};

bool operator== (const CharValue& x, const CharValue& y)
{
	return x.m_value == y.m_value;
}


template<typename K, typename V>
bool isCanonical(const V& valBegin, const std::map<K, V>& map)
{
	if (map.empty())
	{
		return true;
	}

	if (valBegin == map.begin()->second)
	{
		return false;
	}

	for (auto it = (++map.begin()); it != map.end(); it++)
	{
		auto cur = it;
		auto prev = it;
		prev--;

		if (cur->second == prev->second)
		{
			return false;
		}
	}

	return true;
}

void IntervalMapTest()
{
	constexpr static auto canonical = [](const auto& map)
	{
		return isCanonical(map.m_valBegin, map.m_map);
	};

	static constexpr int kRangeRadius = 200;
	static constexpr int kInsertions = 100;

	{
		interval_map<IntegerKey, CharValue> imap(CharValue('X'));
		imap.assign(0, 10, CharValue('X'));
		assert(canonical(imap));
	}

	std::mt19937 randomDevice(10);
	for (int rangeMin = 0; rangeMin >= -kRangeRadius; --rangeMin)
	{
		for (int rangeMax = 1; rangeMax <= kRangeRadius; ++rangeMax)
		{
			std::cout << "Range [" << rangeMin << ", " << rangeMax << "]\n";
			std::uniform_int_distribution<int> keyDestr(rangeMin, rangeMax);
			std::uniform_int_distribution<int> valueDestr('A', 'Z');

			for (int extra = 0; extra <= 1; ++extra)
			{
				const CharValue backgroundValue = valueDestr(randomDevice);
				interval_map<IntegerKey, CharValue> imap(backgroundValue);
				std::vector<CharValue> controlStructure(1 + rangeMax - rangeMin, backgroundValue);

				CharValue extraValue = valueDestr(randomDevice);

				// non-mandatory extra case just to double-check the map is canonical
				// the idea is to add one point to an empty map to split it into half infinite ranges
				if (extra)
				{
					int from = keyDestr(randomDevice);
					while (extraValue == backgroundValue)
					{
						extraValue = valueDestr(randomDevice);
					}

					for (int i = from; i <= rangeMax; ++i)
					{
						controlStructure[i - rangeMin] = extraValue;
					}

					imap.m_map.emplace(from, extraValue);
				}

				for (int insertion = 0; insertion < kInsertions; ++insertion)
				{
					int from = keyDestr(randomDevice);
					int to = keyDestr(randomDevice);
					CharValue what = valueDestr(randomDevice);


					imap.assign(from, to, what);
					assert(canonical(imap));

					for (int i = from; i < to; ++i)
					{
						controlStructure[i - rangeMin] = what;
					}

					for (int i = rangeMin; i <= rangeMax; ++i)
					{
						assert(controlStructure[i - rangeMin] == imap[i]);
					}
					assert(imap[rangeMin - 1] == backgroundValue);
					if (!extra)
					{
						assert(imap[rangeMax + 1] == backgroundValue);
					}
					else
					{
						assert(imap[rangeMax + 1] == extraValue);
					}
				}

				imap.assign(rangeMin, rangeMax, backgroundValue);
				assert(canonical(imap));
				assert(imap[rangeMin - 1] == backgroundValue);
				assert(imap[rangeMin] == backgroundValue);

				if (!extra)
				{
					assert(imap[rangeMax + 1] == backgroundValue);
				}
				else
				{
					assert(imap[rangeMax + 1] == extraValue);
				}
			}
		}
	}
}

int main()
{
	IntervalMapTest();
}
