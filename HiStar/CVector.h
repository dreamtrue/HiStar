/*
A multithread_safe vector
TB  线程安全的vector,效率会降低很多
*/
#pragma once
#include <afxmt.h>
#include <vector>
using namespace std;
template<typename T>
class CVector
{
private:
	std::vector<T> m_vecCon;
	CCriticalSection m_syncFlag;
public:
	typedef typename std::vector<T>::size_type size_type;
	//typedef T value_type;
	typedef typename std::vector<T>::value_type value_type;
	typedef value_type& reference;
	//typedef const value_type& const_reference;
	typedef typename std::vector<T>::const_reference const_reference;
	//typedef value_type* iterator;
	typedef typename std::vector<T>::iterator iterator;
	//typedef const value_type* const_iterator;
	typedef typename std::vector<T>::const_iterator const_iterator;
	CVector(){}
	CVector(const CVector& cc) {*this = cc;}
	virtual ~CVector(void) {m_vecCon.clear();}
	CVector& operator=(const CVector& con)
	{
		m_syncFlag.Lock();
		m_vecCon = con.m_vecCon;
		m_syncFlag.Unlock();
		return *this;
	}
	CVector& operator=(const std::vector<T>& item)
	{
		m_syncFlag.Lock();
		m_vecCon = item;
		m_syncFlag.Unlock();
		return *this;
	}
	reference operator[](size_type n)
	{
		m_syncFlag.Lock();
		reference ref = m_vecCon[n];
		m_syncFlag.Unlock();
		return ref;
	}
	const_reference operator[](size_type n)const
	{
		m_syncFlag.Lock();
		const_reference ref = m_vecCon[n];
		m_syncFlag.Unlock();
		return ref;
	}
	operator std::vector<T>() const {return m_vecCon;}
public:
	void push_back(const T& x)
	{
		m_syncFlag.Lock();
		m_vecCon.push_back(x);
		m_syncFlag.Unlock();
	}
	iterator insert(iterator pos, const T& x)
	{
		m_syncFlag.Lock();
		m_vecCon.insert(pos, x);
		m_syncFlag.Unlock();
	}
	bool empty() const {return m_vecCon.empty();}
	void reserve(size_type n)
	{
		m_syncFlag.Lock();
		m_vecCon.reserve(n);
		m_syncFlag.Unlock();
	}
	void pop_back()
	{
		m_syncFlag.Lock();
		m_vecCon.pop_back();
		m_syncFlag.Unlock();
	}
	void swap(CVector& cc)
	{
		m_syncFlag.Lock();
		m_vecCon.swap(cc.m_vecCon);
		m_syncFlag.Unlock();
	}
	void swap(std::vector<T>& item)
	{
		m_syncFlag.Lock();
		m_vecCon.swap(item);
		m_syncFlag.Unlock();
	}
	reference at(size_type _pos)
	{
		m_syncFlag.Lock();
		reference ref = m_vecCon.at(_pos);
		m_syncFlag.Unlock();
		return ref;
	}
	const_reference at(size_type _pos) const
	{
		m_syncFlag.Lock();
		const_reference ref = m_vecCon.at(_pos);
		m_syncFlag.Unlock();
		return ref;
	}
	iterator erase(const_iterator _pos)
	{
		m_syncFlag.Lock();
		iterator it = m_vecCon.erase(_pos);
		m_syncFlag.Unlock();
		return it;
	}
	iterator erase(size_type _pos)
	{
		m_syncFlag.Lock();
		iterator it = m_vecCon.erase(m_vecCon.begin()+_pos);
		m_syncFlag.Unlock();
		return it;
	}
	iterator erase(const_iterator _first, const_iterator _last)
	{
		m_syncFlag.Lock();
		iterator it = m_vecCon.erase(_first, _last);
		m_syncFlag.Unlock();
		return it;
	}
	void assign(size_type _n, const T& _val)
	{
		m_syncFlag.Lock();
		m_vecCon.assign(_n, _val);
		m_syncFlag.Unlock();
	}
	void assign(iterator _first, iterator _last)
	{
		m_syncFlag.Lock();
		m_vecCon.assign(_first, _last);
		m_syncFlag.Unlock();
	}
	void clear()
	{
		m_syncFlag.Lock();
		m_vecCon.clear();
		m_syncFlag.Unlock();
	}
	reference front()
	{
		m_syncFlag.Lock();
		reference ref = *m_vecCon.begin();
		m_syncFlag.Unlock();
		return ref;
	}
	const_reference front() const
	{
		m_syncFlag.Lock();
		const_reference ref = m_vecCon.front();
		m_syncFlag.Unlock();
		return ref;
	}
	reference back()
	{
		m_syncFlag.Lock();
		reference ref = m_vecCon.back();
		m_syncFlag.Unlock();
		return ref;
	}
	const_reference back() const
	{
		m_syncFlag.Lock();
		const_reference ref = m_vecCon.back();
		m_syncFlag.Unlock();
		return ref;
	}
	//iterator begin() {return m_vecCon.empty() ? NULL : &*m_vecCon.begin();}
	iterator begin() {return m_vecCon.begin();}
	const_iterator begin() const {return m_vecCon.begin();}
	iterator end() {return m_vecCon.end();}
	const_iterator end() const {return m_vecCon.end();}
	size_type size() const {return m_vecCon.size();}
	size_type capacity() const {return m_vecCon.capacity();}
};
