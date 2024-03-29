#ifndef BP_SCENE_RESOURCE_LIST_H
#define BP_SCENE_RESOURCE_LIST_H

#include <vector>
#include <queue>
#include <memory>

namespace bpScene
{

template<typename T>
struct ResourceListIterator
{
	typename std::vector<std::unique_ptr<T>>::iterator elementIterator;
	typename std::vector<std::unique_ptr<T>>::iterator end;

	T& operator*() { return **elementIterator; }

	ResourceListIterator<T>& operator++()
	{
		do { elementIterator++; }
		while (elementIterator != end && !*elementIterator);
		return *this;
	}

	ResourceListIterator<T> operator++(int)
	{
		ResourceListIterator<T> prev{elementIterator, end};
		do { elementIterator++; }
		while (elementIterator != end && !*elementIterator);
		return prev;
	}

	bool operator==(const ResourceListIterator<T>& other)
	{
		return elementIterator == other.elementIterator;
	}

	bool operator!=(const ResourceListIterator<T>& other)
	{
		return elementIterator != other.elementIterator;
	}
};

template<typename T>
class ResourceList
{
public:
	using iterator = ResourceListIterator<T>;

	template<typename... ConstructParams>
	unsigned createResource(ConstructParams... args)
	{
		T* element = new T(args...);
		if (freeIndices.empty())
		{
			unsigned index = static_cast<unsigned>(elements.size());
			elements.emplace_back(element);
			return index;
		} else
		{
			unsigned index = freeIndices.front();
			freeIndices.pop();
			elements[index].reset(element);
			return index;
		}
	}

	void freeResource(unsigned index)
	{
		elements[index].reset(nullptr);
		freeIndices.push(index);
	}

	unsigned size() 
	{
		return (unsigned)elements.size();
	}
	bool hasElementAt(unsigned _index) { return _index < size() && elements[_index]; }
	void clear()
	{
		for (int i = 0; i < elements.size(); i++)
		{
			if (elements[i]) freeResource(i);
		}
	}

	T& getResource(unsigned index) { return *elements[index]; }
	T& operator[](unsigned index) { return *elements[index]; }

	iterator begin() { return {elements.begin(), elements.end()}; }
	iterator end() { return {elements.end(), elements.end()}; }

	ResourceList() : elements(), freeIndices() {}
private:
	std::vector<std::unique_ptr<T>> elements;
	std::queue<unsigned> freeIndices;
};

}

#endif
