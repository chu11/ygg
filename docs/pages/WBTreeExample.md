@page wbtreeexample Weight Balanced Tree Example

This page provides an example of how to use the weight balanced tree. For information on what a weight balanced
 tree is and how it behaves, please see @ref datastructuresoverview.

The code can also be found in the examples directory.

~~~~~~~~~~~~~{.cpp}
#include "../src/ygg.hpp"

using namespace ygg;

/* The tree options
 * We allow multiple nodes with the same key to be inserted into the tree.
 *
 * Also, we want the balance parameters to be chosen as (3,2), i.e., (3/1, 2/1).
 */
using MyTreeOptions =
    TreeOptions<TreeFlags::MULTIPLE, TreeFlags::WBT_DELTA_NUMERATOR<3>,
                TreeFlags::WBT_DELTA_DENOMINATOR<1>,
                TreeFlags::WBT_GAMMA_NUMERATOR<2>,
                TreeFlags::WBT_GAMMA_DENOMINATOR<1>>;

/* The node class
 * Provides a simple key -> value mapping, where the key is an integer and the
 * value is a string.
 */
class Node : public WBTreeNodeBase<Node, MyTreeOptions> {
public:
	int key;
	std::string value;

	// need to implement this s.t. we can use the default
	// ygg::utilities::flexible_less as comparator
	bool
	operator<(const Node & other) const
	{
		return this->key < other.key;
	}
};

// Configure the RBTree based on Node and the default NodeTraits
using MyTree = WBTree<Node, WBDefaultNodeTraits, MyTreeOptions>;

// We need this s.t. we can query by key value (i.e, an int) directly
bool
operator<(const Node & lhs, const int rhs)
{
	return lhs.key < rhs;
}
bool
operator<(const int lhs, const Node & rhs)
{
	return lhs < rhs.key;
}

int
main(int argc, char ** argv)
{
	(void)argc;
	(void)argv;

	MyTree t;

	// Storage for the actual nodes.
	// WARNING: using STL containers here can backfire badly. See TODO.
	Node nodes[5];

	// Initialize the nodes with some values
	for (int i = 0; i < 5; ++i) {
		nodes[i].key = i;
		nodes[i].value = std::string("The key is ") + std::to_string(i);
	}

	// Insert them
	for (size_t i = 0; i < 5; ++i) {
		t.insert(nodes[i]);
	}

	// What was the string for i = 3 again?
	auto it = t.find(3); // Note we're using a int to query here, not a Node
	assert(it != t.end());
	std::string retrieved_value = it->value; // *it is the Node

	// Okay, we don't need that Node anymore.
	t.remove(*it);

	return 0;
}
~~~~~~~~~~~~~
