#ifndef ADVISOR_H_
#define ADVISOR_H_

#include "Link.h"

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
    EdgeFilter(const Graph& g, const Link::wavelength_t wl)
        : g{g}
        , wl{wl}
    { }

    template<typename Edge>
    bool operator()(const Edge& e) const {
        const auto& src = boost::source(e, g);
        const auto& tgt = boost::target(e, g);
        typename boost::graph_traits<Graph>::edge_descriptor edge;
        bool found;
        std::tie(edge, found) = boost::edge(src, tgt, g);
        if (found) {
            const Link& link = g[edge];
            return link.can_use(wl);
        }
        return false;
    }

    Graph g;
    Link::wavelength_t wl;
};

class Advisor {
public:
    using event_t = double;

    using Graph = boost::adjacency_list<boost::vecS, boost::vecS,
          boost::undirectedS, boost::no_property, Link>;
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

    std::pair<vertex_t, vertex_t>
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
     *         Wavelength is Link::NONE if no path is available.
     */
    std::pair<std::vector<edge_t>, Link::wavelength_t>
    path_between(vertex_t a, vertex_t b);

    /**
     * \return true if there is a path between nodes a and b, false otherwise.
     */
    bool
    has_path_between(vertex_t a, vertex_t b);

    /**
     * Initiates a connection between nodes a and b.
     *
     * \return the path and the wavelength that the two nodes are using.
     */
    std::pair<std::vector<edge_t>, Link::wavelength_t>
    make_connection(vertex_t a, vertex_t b);

    /**
     * Finishes the connection between nodes a and b using the given
     * wavelength.
     */
    void
    remove_connection(const std::vector<edge_t>& path,
                      const Link::wavelength_t wl);

private:
    Advisor::event_t lambda;
    Advisor::event_t duration_mean;
    Graph nodes;
    std::random_device rd;
    std::mt19937 rgen{rd()};
    std::uniform_int_distribution<vertex_t> u_dist;
    std::exponential_distribution<Advisor::event_t> arrival_dist;
    std::exponential_distribution<Advisor::event_t> duration_dist;
};

#endif /* end of include guard */
