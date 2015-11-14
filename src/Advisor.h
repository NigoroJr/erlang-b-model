#ifndef ADVISOR_H_
#define ADVISOR_H_

#include "Node.h"

#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/breadth_first_search.hpp>
#include <boost/graph/visitors.hpp>
#include <boost/graph/filtered_graph.hpp>

#include <random>
#include <vector>

/**
 * Only considers the edges with wavelengths available.
 */
template<typename Graph>
struct EdgeFilter {
    EdgeFilter() { }
    EdgeFilter(const Graph& g, const Node::wavelength_t wl)
        : g{g}
        , wl{wl}
    { }

    template<typename Edge>
    bool operator()(const Edge& e) const {
        const Node& src = g[boost::source(e, g)];
        const Node& tgt = g[boost::target(e, g)];
        return src.can_use(wl) && tgt.can_use(wl);
    }

    Graph g;
    Node::wavelength_t wl;
};

class Advisor {
public:
    using event_t = double;

    using Graph = boost::adjacency_list<boost::vecS, boost::vecS,
          boost::undirectedS, Node, boost::no_property>;
    using vertex_t = boost::graph_traits<Graph>::vertex_descriptor;
    using edge_t = boost::graph_traits<Graph>::edge_descriptor;

    /* Constructors, Destructor, and Assignment operators {{{ */
    // Default constructor
    Advisor();

    Advisor(const Graph& nodes,
            const Advisor::event_t lambda,
            const Advisor::event_t duration_mean);

    // Copy constructor
    Advisor(const Advisor& other);

    // Move constructor
    Advisor(Advisor&& other);

    // Destructor
    ~Advisor();

    // Assignment operator
    Advisor&
    operator=(const Advisor& other);

    // Move assignment operator
    Advisor&
    operator=(Advisor&& other);
    /* }}} */

    std::pair<Node::id_t, Node::id_t>
    get_nodes();

    /**
     * The time until the start of the next connection.
     * The duration is distributed exponentially with parameter `lambda'.
     *
     * \return the time until the next connection.
     */
    Advisor::event_t
    get_arrival();

    /**
     * The time until the end of a connection.
     * The duration is distributed exponentially with parameter
     * `duration_mean'.
     *
     * \return the time until the end of connection.
     */
    Advisor::event_t
    get_duration();

    /**
     * \return the path and the wavelength available between a and b.
     *         Wavelength is Node::NONE if no path is available.
     */
    std::pair<std::vector<vertex_t>, Node::wavelength_t>
    path_between(Node::id_t a, Node::id_t b);

    /**
     * \return true if there is a path between nodes a and b, false otherwise.
     */
    bool
    has_path_between(Node::id_t a, Node::id_t b);

    /**
     * Initiates a connection between nodes a and b.
     *
     * \return the path and the wavelength that the two nodes are using.
     */
    std::pair<std::vector<vertex_t>, Node::wavelength_t>
    make_connection(Node::id_t a, Node::id_t b);

    /**
     * Finishes the connection between nodes a and b using the given
     * wavelength.
     */
    void
    remove_connection(const std::vector<vertex_t>& path,
                      const Node::wavelength_t wl);

private:
    Advisor::event_t lambda;
    Advisor::event_t duration_mean;
    Graph nodes;
    std::random_device rd;
    std::mt19937 rgen{rd()};
    std::uniform_int_distribution<Node::id_t> u_dist;
    std::exponential_distribution<Advisor::event_t> arrival_dist;
    std::exponential_distribution<Advisor::event_t> duration_dist;
};

#endif /* end of include guard */
