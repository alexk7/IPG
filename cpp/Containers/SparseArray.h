/*
 * SparseArray.h
 *
 *  Created on: 29-03-2009
 *      Author: alexk7
 */

#ifndef SPARSEARRAY_H_
#define SPARSEARRAY_H_

#include <bitset>
#include <vector>
#include <cassert>

template <class T, int N>
class SparseArray
{
public:
	T& operator[](int _index)
	{
        return Get(_index);
	}

	const T& operator[](int _index) const
    {
        return Get(_index);
    }

	void clear()
	{
		mBitset.reset();
		mVector.clear();
	}

private:
	T& Get(int _index) const
    {
		size_t pos = (mBitset << (N - _index)).count();
		if (!mBitset.test(_index))
        {
            mBitset.set(_index);
            mVector.insert(mVector.begin() + pos, T());
        }
        assert(pos < mVector.size());
        assert(mBitset.count() == mVector.size());
        return mVector[pos];
    }

	mutable std::bitset<N> mBitset;
	mutable std::vector<T> mVector;
};

#endif /* SPARSEARRAY_H_ */
