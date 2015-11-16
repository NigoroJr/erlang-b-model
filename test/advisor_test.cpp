#define BOOST_TEST_MODULE AdvisorTest
#include <boost/test/unit_test.hpp>

#include "Advisor.h"
#include "Event.h"
#include "Link.h"

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/breadth_first_search.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/visitors.hpp>

#include <vector>

using Graph = Advisor::Graph;
using vertex_t = Advisor::vertex_t;
using edge_t = Advisor::edge_t;

/*      0
 *      |
 * 1 -- 2 -- 3
 *      |
 *      4
 */
Graph
make_crossing_graph(const unsigned num_links) {
    Graph g;
    boost::add_edge(0, 2, Link(num_links), g);
    boost::add_edge(1, 2, Link(num_links), g);
    boost::add_edge(2, 3, Link(num_links), g);
    boost::add_edge(2, 4, Link(num_links), g);

    return g;
}

/* With diagonal edges
 *
 *    - 0 -
 *   /  |  \
 *  /   |   \
 * 1 -- 2 -- 3
 *  \   |   /
 *   \  |  /
 *    - 4 -
 */
Graph
make_diamond_graph(const unsigned num_links) {
    auto g = make_crossing_graph(num_links);
    boost::add_edge(0, 1, Link(num_links), g);
    boost::add_edge(0, 3, Link(num_links), g);
    boost::add_edge(1, 4, Link(num_links), g);
    boost::add_edge(3, 4, Link(num_links), g);

    return g;
}

Graph
make_graph(const unsigned num_links) {
    Graph g;
    std::vector<std::pair<int, int>> connections = {
        {0, 1},
        {0, 3},
        {0, 8},
        {0, 9},
        {1, 8},
        {1, 9},
        {2, 6},
        {3, 2},
        {3, 6},
        {3, 7},
        {4, 0},
        {4, 1},
        {4, 2},
        {4, 6},
        {4, 7},
        {6, 1},
        {6, 5},
        {6, 8},
        {7, 2},
        {9, 5},
    };

    for (auto&& p : connections) {
        boost::add_edge(p.first, p.second, Link(num_links), g);
    }

    return g;
}

BOOST_AUTO_TEST_CASE(advisor_path_between_test) {
    auto nodes = make_graph(5);

    const Event::event_t lambda = 5;
    const Event::event_t duration_mean = 1;
    Advisor advisor{nodes, lambda, duration_mean};

    // Adjacent
    // 0 -- 8
    // Only one edge
    auto path = advisor.path_between(0, 8).first;
    BOOST_CHECK_EQUAL(path.size(), 1);

    // Multiple hops
    // 0 -- 9 -- 5
    // Two edges
    path = advisor.path_between(0, 5).first;
    BOOST_CHECK_EQUAL(path.size(), 2);

    auto g = make_crossing_graph(1);
    Advisor a2{g, lambda, duration_mean};
    Link::wavelength_t wl;
    std::tie(path, wl) = a2.path_between(0, 4);
    // 0 -- 2 -- 4
    // Two edges
    BOOST_REQUIRE_EQUAL(path.size(), 2);

    // Lock the path
    a2.make_connection(0, 4);

    std::tie(path, wl) = a2.path_between(1, 3);
    BOOST_CHECK_NE(wl, Link::NONE);

    auto g2 = make_diamond_graph(1);
    a2 = Advisor{g2, lambda, duration_mean};
    /* This type of connection
     *
     *      0 -
     *         \
     *          \
     * 1 -- 2 -- 3
     *  \
     *   \
     *    - 4
     */
    a2.make_connection(0, 1);
    a2.make_connection(0, 2);
    a2.make_connection(2, 4);
    a2.make_connection(3, 4);

    std::tie(path, std::ignore) = a2.path_between(0, 4);
    BOOST_CHECK_EQUAL(path.size(), 4);
}

BOOST_AUTO_TEST_CASE(advisor_has_path_between_test) {
    auto nodes = make_graph(5);

    const Event::event_t lambda = 5;
    const Event::event_t duration_mean = 1;
    Advisor advisor{nodes, lambda, duration_mean};

    // Adjacent
    BOOST_CHECK(advisor.has_path_between(0, 8));
    BOOST_CHECK(advisor.has_path_between(8, 0));

    // Multiple hops
    BOOST_CHECK(advisor.has_path_between(0, 5));
    BOOST_CHECK(advisor.has_path_between(5, 0));

    // With multiple paths and links
    advisor = Advisor{make_diamond_graph(2), lambda, duration_mean};
    advisor.make_connection(0, 4);
    advisor.make_connection(0, 4);
    BOOST_CHECK(advisor.has_path_between(0, 4));
    advisor.make_connection(0, 4);
    advisor.make_connection(0, 4);
    BOOST_CHECK(advisor.has_path_between(4, 0));
    advisor.make_connection(0, 4);
    advisor.make_connection(0, 4);
    // Now there should be no more path
    BOOST_CHECK(!advisor.has_path_between(4, 0));
}

BOOST_AUTO_TEST_CASE(advisor_make_connection_test) {
    auto nodes = make_crossing_graph(1);

    const Event::event_t lambda = 5;
    const Event::event_t duration_mean = 1;
    Advisor advisor{nodes, lambda, duration_mean};

    BOOST_REQUIRE(advisor.has_path_between(0, 4));

    advisor.make_connection(0, 4);

    BOOST_CHECK(!advisor.has_path_between(0, 3));
    BOOST_CHECK(!advisor.has_path_between(0, 4));
    // 1 -- 2 -- 3
    // Links are not used
    BOOST_CHECK(advisor.has_path_between(1, 3));

    // Blocking with a detour
    nodes = make_diamond_graph(1);
    advisor = Advisor{nodes, lambda, duration_mean};
    advisor.make_connection(2, 4);

    std::vector<edge_t> path;
    std::tie(path, std::ignore) = advisor.make_connection(1, 3);
    // 1 -- 0 -- 3
    BOOST_CHECK_EQUAL(path.size(), 2);

    // Only two nodes with two links each
    nodes = Graph();
    boost::add_edge(0, 1, Link(2), nodes);
    advisor = Advisor{nodes, lambda, duration_mean};
    BOOST_CHECK(advisor.has_path_between(0, 1));
    advisor.make_connection(0, 1);
    BOOST_CHECK(advisor.has_path_between(0, 1));
    advisor.make_connection(0, 1);
    BOOST_CHECK(!advisor.has_path_between(0, 1));
}

BOOST_AUTO_TEST_CASE(advisor_remove_connection_test) {
    auto nodes = make_crossing_graph(1);

    const Event::event_t lambda = 5;
    const Event::event_t duration_mean = 1;
    Advisor advisor{nodes, lambda, duration_mean};

    std::vector<edge_t> path;
    Link::wavelength_t wl;

    // Normal blocking
    std::tie(path, wl) = advisor.make_connection(0, 4);
    BOOST_REQUIRE(!advisor.has_path_between(0, 4));
    advisor.remove_connection(path, wl);
    BOOST_CHECK(advisor.has_path_between(0, 4));

    std::tie(path, wl) = advisor.make_connection(0, 4);
    BOOST_REQUIRE(!advisor.has_path_between(0, 4));
    advisor.remove_connection(path, wl);
    BOOST_CHECK(advisor.has_path_between(0, 4));

    // Only two nodes with two links each
    nodes = Graph();
    boost::add_edge(0, 1, Link(2), nodes);
    advisor = Advisor{nodes, lambda, duration_mean};
    advisor.make_connection(0, 1);
    std::tie(path, wl) = advisor.make_connection(0, 1);
    BOOST_REQUIRE(!advisor.has_path_between(0, 1));
    // Removing
    advisor.remove_connection(path, wl);
    BOOST_CHECK(advisor.has_path_between(0, 1));
}
