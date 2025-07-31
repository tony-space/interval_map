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
		//
		// if sequence is empty
		if (!(keyBegin < keyEnd))
		{
			return;
		}

		typename std::map<K, V>::iterator it = m_map.end();

		if (val == m_valBegin)
		{
			if (m_map.empty())
			{
				return;
			}

			it = m_map.lower_bound(keyBegin);
			if (it == m_map.begin())
			{
				typename std::map<K, V>::node_type node;

				while (it != m_map.end() && (it->first < keyEnd || it->second == m_valBegin))
				{
					if (keyEnd < it->first && node)
					{
						node.key() = keyEnd;
						m_map.insert(it, std::move(node));
						return;
					}

					auto toExtract = it++;
					node = m_map.extract(toExtract);
				}

				if (node && !(node.mapped() == m_valBegin))
				{
					node.key() = keyEnd;
					m_map.insert(it, std::move(node));
				}

				return;
			}

			if (it == m_map.end())
			{
				auto prevElement = --m_map.end();
				if (!(prevElement->second == val))
				{
					m_map.emplace_hint(m_map.end(), keyBegin, std::forward<V_forward>(val));
					m_map.emplace_hint(m_map.end(), keyEnd, prevElement->second);
				}

				return;
			}
		}
		else
		{
			//
			// if map is empty or insertion right in front of the map
			// i.e. keyEnd <= map.begin()->frist
			if (m_map.empty() || !(m_map.begin()->first < keyEnd))
			{
				if (m_map.empty() || keyEnd < m_map.begin()->first)
				{
					m_map.emplace_hint(m_map.begin(), keyEnd, m_valBegin);
				}

				if (!m_map.empty() && m_map.begin()->second == val)
				{
					auto node = m_map.extract(m_map.begin());
					node.key() = keyBegin;
					m_map.insert(m_map.begin(), std::move(node));
				}
				else
				{
					m_map.emplace_hint(m_map.begin(), keyBegin, std::forward<V_forward>(val));
				}
				return;
			}

			auto lastElement = m_map.end();
			// if insertion after non-empty map value
			if (!m_map.empty() && (--lastElement)->first < keyBegin)
			{
				if (!(lastElement->second == val))
				{
					m_map.emplace_hint(m_map.end(), keyBegin, std::forward<V_forward>(val));
					m_map.emplace_hint(m_map.end(), keyEnd, lastElement->second);
				}
				return;
			}

			it = m_map.lower_bound(keyBegin);

		}
		typename std::map<K, V>::node_type node;
		typename std::map<K, V>::iterator prevElement = m_map.end();
		typename std::map<K, V>::iterator inserted = m_map.end();

		if (it != m_map.begin())
		{
			prevElement = it;
			--prevElement;
		}

		//
		// if value overwrite required
		if (!(it->first < keyBegin || keyBegin < it->first))
		{
			const auto curElement = it++;

			node = m_map.extract(curElement);
			if (prevElement == m_map.end() || !(prevElement->second == val))
			{
				inserted = m_map.emplace_hint(it, keyBegin, std::forward<V_forward>(val));
			}
		}
		//
		// if key a overwrite required
		else if (it->second == val && !(keyEnd < it->first))
		{
			auto curElement = it++;
			node = m_map.extract(curElement);
			node.key() = keyBegin;
			inserted = m_map.insert(it, std::move(node));
			node = typename std::map<K, V>::node_type{};
			prevElement = m_map.end();
		}
		//
		// new insertion?
		else if (prevElement == m_map.end() || !(prevElement->second == val))
		{
			inserted = m_map.emplace_hint(it, keyBegin, std::forward<V_forward>(val));
		}

		//
		// common case
		while (it != m_map.end() && it->first < keyEnd)
		{
			auto toExtract = it++;
			node = m_map.extract(toExtract);
		}

		if (it == m_map.end() || keyEnd < it->first)
		{
			if (node)
			{
				if ((inserted != m_map.end() && !(node.mapped() == inserted->second))
					||
					(inserted == m_map.end() && prevElement != m_map.end() && !(node.mapped() == prevElement->second)))
				{
					node.key() = keyEnd;
					m_map.insert(it, std::move(node));
				}
			}
			else if (it != m_map.end() && inserted != m_map.end() && prevElement != m_map.end())
			{
				m_map.emplace_hint(it, keyEnd, prevElement->second);
			}
		}
		else if ((inserted != m_map.end() && it->second == inserted->second) ||
			(inserted == m_map.end() && prevElement != m_map.end() && it->second == prevElement->second))
		{
			m_map.erase(it);
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
