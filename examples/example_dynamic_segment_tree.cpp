//
// Created by lukas on 26.09.17.
//

#include "../src/ygg.hpp"

using namespace ygg;

/* First, we define which combiners we want to use. For this simple example, we
 * use a simple provided RangedMaxCombiner. See TODO for other combiners.
 */
using MCombiner = RangedMaxCombiner<double, int>;
using Combiners = CombinerPack<double, int, MCombiner>;

/* You can select whether to use a red-black tree or a zip tree as basis for
 * the dynamic segment tree. By default, a red-black tree is used. However, in
 * some situations, a zip tree is faster. Let's use the zip tree!
 */
using TreeSelector = UseDefaultZipTree;

/* The class representing our intervals and associated values.
 *
 * Our key space (i.e., the borders of the intervals) will be double.
 *
 * The while the values will be ints. The DynamicSegmentTree aggregates values
 * by using operator+, thus our aggregation will be simply the sum of the
 * aggregated values. We need to specify the type of the aggregate values, too.
 *
 *
 */
class Interval
    : public DynSegTreeNodeBase<double, int, int, Combiners, TreeSelector> {
public:
	double lower; // lower interval border
	double upper; // upper interval border

	int value; // value associated with this interval.

	Interval(double lower_in, double upper_in, int value_in)
	    : lower(lower_in), upper(upper_in), value(value_in){};
};

/* We need to define traits on our interval class, derived from
 * DynSegTreeNodeTraits<Interval>. This class tells the DynamicSegmentTree how
 * to get the interval borders and the value from our Interval class.
 */
class IntervalTraits : public DynSegTreeNodeTraits<Interval> {
public:
	using key_type = double;
	using value_type = int;

	static key_type
	get_lower(const Interval & n)
	{
		return n.lower;
	}

	static key_type
	get_upper(const Interval & n)
	{
		return n.upper;
	}

	static value_type
	get_value(const Interval & n)
	{
		return n.value;
	}

	/* All our intervals are right-open. The DynamicSegmentTree can handle every
	 * combination of openness / closedness, and in fact can contain intervals of
	 * different types.
	 */
	static bool
	is_lower_closed(const Interval & n)
	{
		(void)n;
		return true; // closed on the lower border
	};
	static bool
	is_upper_closed(const Interval & n)
	{
		(void)n;
		return false; // open on the upper border
	};
};

/* This is our DynamicSegmentTree
 */
using MyTree = DynamicSegmentTree<Interval, IntervalTraits, Combiners,
                                  DefaultOptions, TreeSelector>;

int
main(int argc, char ** argv)
{
	(void)argc;
	(void)argv;

	MyTree t;

	// Storage for the actual intervals.
	// WARNING: using STL containers here can backfire badly. See TODO.
	std::vector<Interval> intervals;

	/* We add the following interval / value combinations:
	 *
	 *  [0,   10)   -> 1
	 *  [0.5, 10)   -> 2
	 *  [10,  15)   -> 3
	 *  [12,  20)   -> 8
	 */
	intervals.emplace_back(0.0, 10.0, 1);
	intervals.emplace_back(0.5, 10.0, 2);
	intervals.emplace_back(10.0, 15.0, 3);
	intervals.emplace_back(12.0, 20.0, 8);
	for (auto & interval : intervals) {
		t.insert(interval);
	}

	/* Let's see what the aggregate value at the points 0, 0.5, 5, 10, 14 and 15
	 * is:
	 */
	for (auto point : std::vector<double>{0, 0.5, 5, 10, 14, 15}) {
		std::cout << "Point: " << point << "\t| Aggregate Value: " << t.query(point)
		          << "\n";
	}

	/* Should output:
	 * Point: 0     | Aggregate Value: 1
	 * Point: 0.5	  | Aggregate Value: 3
	 * Point: 5     | Aggregate Value: 3
	 * Point: 10    | Aggregate Value: 3
	 * Point: 14	  | Aggregate Value: 11
	 * Point: 15	  | Aggregate Value: 8
	 * */

	std::cout << "\n==============================\n\n";

	/* We can also iterate all start and end events. */
	for (auto event : t) {
		std::cout << "At point " << event.get_point();
		if (event.is_closed()) {
			std::cout << " (inclusive) ";
		} else {
			std::cout << " (exclusive) ";
		}

		std::cout << " there ";
		if (event.is_start()) {
			std::cout << " starts ";
		} else {
			std::cout << " ends ";
		}

		// We need to up-cast the pointer we get to our interval class
		auto interval = static_cast<const Interval *>(event.get_interval());
		std::cout << "the interval [" << interval->lower << "," << interval->upper
		          << ")\n";
	}

	/* Should output:
	 * At point 0 (inclusive)  there  starts the interval [0,10)
	 * At point 0.5 (inclusive)  there  starts the interval [0.5,10)
	 * At point 10 (exclusive)  there  ends the interval [0.5,10)
	 * At point 10 (exclusive)  there  ends the interval [0,10)
	 * At point 10 (inclusive)  there  starts the interval [10,15)
	 * At point 12 (inclusive)  there  starts the interval [12,20)
	 * At point 15 (exclusive)  there  ends the interval [10,15)
	 * At point 20 (exclusive)  there  ends the interval [12,20)
	 */

	std::cout << "\n==============================\n\n";

	/* Now, combiners. The ranged max combiner first allows us to query (in O(1))
	 * what the total maximum value is: */
	std::cout << "Maximum Value: " << t.get_combined<MCombiner>() << "\n";
	/* Should output:
	 * Maximum Value: 11
	 */

	/* Also, in contrast to its slightly faster, non-ranged brother, the
	 * MaxCombiner, it will also tell us over which interval the maximum occurs:
	 */
	auto combiner = t.get_combiner<MCombiner>();
	std::cout << "Maximum occurs between " << combiner.get_left_border()
	          << " and " << combiner.get_right_border() << "\n";
	/* This should output:
	 * Maximum occurs between 12 and 15 */

	/* But, we can also query (in O(log n)) the maximum over sub-ranges:*/
	std::cout << "Maximum over [0, 10): " << t.get_combined<MCombiner>(0, 10)
	          << "\n";
	std::cout << "Maximum over [10, 12): " << t.get_combined<MCombiner>(10, 12)
	          << "\n";

	/* Again, we can get the actual combiner to tell us the maximum interval. Note
	 * that the reported borders might be too large and must be clipped to your
	 * query range. */
	combiner = t.get_combiner<MCombiner>(0, 10);
	std::cout << "Maximum interval in the range [0,10) is between "
	          << combiner.get_left_border() << " and "
	          << combiner.get_right_border() << "\n";

	// We also can specify to query an interval closed on both sides:
	std::cout << "Maximum over [10, 12]: "
	          << t.get_combined<MCombiner>(10, 12, true, true) << "\n";

	/* Overall output should be:
	 * Maximum over [0, 10): 3
	 * Maximum over [10, 12): 3
	 * Maximum interval in the range [0, 10) is between 0.5 and 10
	 * Maximum over [10, 12]: 11
	 */

	return 0;
}
