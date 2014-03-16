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

	void add_value(T value)
	{
		m_values.push_back(value);
	}

	ArgNode *get_node(int *args, const bool create) {
		if (!*args)
			return this;
		typename std::map<int, ArgNode<T> >::iterator idx = m_next_arg.find(args[0]);
		if (idx != m_next_arg.end())
			return (*idx).second.get_node(args+1, create);
		else
		{
			if (create)
			{
				ArgNode<T> tmp;
				m_next_arg[args[0]] = tmp;
				return m_next_arg[args[0]].get_node(args+1, create);
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
		return &m_next_arg[m_valueidx];
	}
};

template<class T>
class Database {
private:
	std::map<std::string, ArgNode<T> > m_trees;
public:

	ArgNode<T> *get_node(const char *fn_name, int *args, const bool create)
	{
		std::string balls = fn_name;
		typename std::map<std::string, ArgNode<T> >::iterator idx = m_trees.find(balls);
		if (idx != m_trees.end())
			return (*idx).second.get_node(args, create);

		if (create)
		{
			ArgNode<T> tmp;
			m_trees[balls] = tmp;
			return m_trees[balls].get_node(args, create);
		}
		return 0;
	}
};

#endif
