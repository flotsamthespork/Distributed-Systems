#ifndef __DB_H__
#define __DB_H__

#include <map>
#include <vector>
#include <string>

template<class T>
class ArgNode {
private:
	std::map<int, ArgNode<T> > m_next_arg;
	std::vector<T> m_values;
	int m_valueidx;
public:
	ArgNode() {
		m_valueidx = 0;
	}

	int add_value(T value)
	{
		m_values.push_back(value);
		return m_values.size();
	}

	ArgNode<T> *get_node(int *args, const bool create) {
		if (!*args)
			return this;
		int key = args[0] & 0xFFFF0000;
		typename std::map<int, ArgNode<T> >::iterator idx = m_next_arg.find(key);
		if (idx != m_next_arg.end())
			return (*idx).second.get_node(args+1, create);
		else
		{
			if (create)
			{
				ArgNode<T> tmp;
				m_next_arg[key] = tmp;
				return m_next_arg[key].get_node(args+1, create);
			}
			return 0;
		}
	}

	T *get_value()
	{
		const int n = m_values.size();
		if (n == 0)
			return 0;
		m_valueidx = (m_valueidx+1)%n;
		return &m_values[m_valueidx];
	}
};

template<class T>
class Database {
private:
	std::map<std::string, ArgNode<T> > m_trees;
public:

	ArgNode<T> *get_node(const char *fn_name, int *args, const bool create)
	{
		std::string name = fn_name;
		typename std::map<std::string, ArgNode<T> >::iterator idx = m_trees.find(name);
		if (idx != m_trees.end())
			return (*idx).second.get_node(args, create);

		if (create)
		{
			ArgNode<T> tmp;
			m_trees[name] = tmp;
			return m_trees[name].get_node(args, create);
		}
		return 0;
	}
};

#endif
