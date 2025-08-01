#pragma once

#include <map>

template<typename K, typename V>
class interval_map
{
public:
	// constructor associates whole range of K with val
	template<typename V_forward>
	interval_map(V_forward&& val) :
		m_valBegin(std::forward<V_forward>(val))
	{
	}

	// Assign value val to interval [keyBegin, keyEnd).
	// Overwrite previous values in this interval.
	// Conforming to the C++ Standard Library conventions, the interval
	// includes keyBegin, but excludes keyEnd.
	// If !( keyBegin < keyEnd ), this designates an empty interval, and assign must do nothing.
	template<typename V_forward>
	void assign(const K& keyBegin, const K& keyEnd, V_forward&& val) requires (std::is_same<std::remove_cvref_t<V_forward>, V>::value)
	{
		if (!(keyBegin < keyEnd))
		{
			return;
		}

		// --- The Single O(log N) Operation ---
		auto it_begin = m_map.lower_bound(keyBegin);

		// --- Determine boundary values before modification ---
		auto it_before_begin = it_begin;
		if (it_before_begin != m_map.begin())
		{
			--it_before_begin;
		}
		else
		{
			it_before_begin = m_map.end(); // Use end() as a sentinel for m_valBegin
		}
		const V& value_before = (it_before_begin != m_map.end()) ? it_before_begin->second : m_valBegin;

		auto it_end = it_begin;
		while (it_end != m_map.end() && it_end->first < keyEnd)
		{
			++it_end;
		}

		V value_at_end = m_valBegin; // Default to m_valBegin
		if (it_end != m_map.begin())
		{
			value_at_end = std::prev(it_end)->second;
		}
		if (it_end != m_map.end() && !(it_end->first < keyEnd || keyEnd < it_end->first))
		{
			value_at_end = it_end->second;
		}


		// --- Perform modifications ---

		auto erase_until = it_end;
		if (it_end != m_map.end() && !(it_end->first < keyEnd || keyEnd < it_end->first) && it_end->second == val)
		{
			erase_until = std::next(it_end);
		}

		auto it = m_map.erase(it_begin, erase_until);

		if (!(val == value_before))
		{
			it = m_map.emplace_hint(it, keyBegin, val);
		}

		if (!(val == value_at_end))
		{
			m_map.emplace_hint(it, keyEnd, std::move(value_at_end));
		}
	}

	// look-up of the value associated with key
	const V& operator[](const K& key) const
	{
		auto it = m_map.upper_bound(key);
		if (it == m_map.begin())
		{
			return m_valBegin;
		}
		else
		{
			return (--it)->second;
		}
	}
private:
	friend void IntervalMapTest();
	std::map<K, V> m_map;
	const V m_valBegin;
};

// Many solutions we receive are incorrect. Consider using a randomized test
// to discover the cases that your implementation does not handle correctly.
// We recommend to implement a test function that tests the functionality of
// the interval_map, for example using a map of int intervals to char.
