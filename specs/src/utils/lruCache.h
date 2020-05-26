#ifndef SPECS2016__UTILS__LRU_CACHE__H
#define SPECS2016__UTILS__LRU_CACHE__H

#include <iostream>
#include <map>
#include <vector>
#include <memory>
#include <algorithm> // for sort
#include "utils/ErrorReporting.h"

template <class S, class T>
class cacheElement {
public:
	cacheElement(S& key, std::shared_ptr<T> pElem, uint64_t& counter) {
		m_pElem = pElem;
		m_lru = ++counter;
		m_key = key;
	}
	~cacheElement() {
	}
	std::shared_ptr<T> get(uint64_t& counter) {
		m_lru = ++counter;
		return m_pElem;
	}
	uint64_t getLRU() { return m_lru; }
private:
	std::shared_ptr<T> m_pElem;
	uint64_t  m_lru;
	S         m_key;
};

/*
 * This defines an LRU cache for a map object mapping S objects to T objects.
 *
 * The cache has an upper bound on the number of objects it can hold. When more
 * objects are added, the least recently used ones are removed. The cache is initialized
 * with a function pointer that can generate a T object from an S object that is not
 * currently in the map.
 */
template <class S, class T>
class lruCache {
public:
	lruCache(size_t lowWM, size_t highWM) {
		m_highWM = highWM;
		m_lowWM = lowWM;
		m_counter = 0;
	}

	~lruCache() {
		m_map.clear();
	}

	std::shared_ptr<T> get(S& s) {
		auto it = m_map.find(s);
		if (it!=m_map.end()) {
			auto pRet = it->second->get(m_counter);
			return pRet;
		}

		return NULL;
	}
	void set(S& s, std::shared_ptr<T> pT) {
		auto it = m_map.find(s);
		MYASSERT(it==m_map.end());

		if (m_map.size() >= m_highWM) {
			clearLRU();
		}

		m_map[s] = std::shared_ptr<cacheElement<S,T>>(new cacheElement<S,T>(s, pT, m_counter));
	}

	void Debug() {
		std::cerr << "lruCache object @" << this << " : " << m_lowWM << "-" << m_highWM
				<< " elements; currently: " << m_map.size() << ":";
		for (auto p: m_map) {
			std::cerr << "\n" << p.first << " : " << p.second->getLRU();
		}
		std::cerr << std::endl;
	}
private:
	void clearLRU() {
		MYASSERT(m_map.size()==m_highWM);
		std::vector<std::shared_ptr<cacheElement<S,T>>> vec;
		for (auto p : m_map) {
			vec.insert(vec.end(), p.second);
		}
		std::sort(vec.begin(), vec.end(), [](std::shared_ptr<cacheElement<S,T>> p1, std::shared_ptr<cacheElement<S,T>> p2) {
			return p1->getLRU() > p2->getLRU();
		});

		uint64_t cutoff = vec[m_lowWM]->getLRU();

		for (auto it = m_map.begin() ; it != m_map.end() ; ) {
			if (it->second->getLRU() < cutoff) {
				it = m_map.erase(it);
			} else {
				++it;
			}
		}
	}

	size_t m_highWM;
	size_t m_lowWM;
	std::map<S,std::shared_ptr<cacheElement<S,T>>> m_map;
	uint64_t m_counter;
};

#endif
