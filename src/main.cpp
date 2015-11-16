#include "Advisor.h"
#include "Event.h"
#include "Link.h"

#include "cxxopts.hpp"

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_traits.hpp>

#include <algorithm>
#include <fstream>
#include <functional>
#include <iostream>
#include <queue>
#include <utility>

Advisor::Graph
make_graph_from_file(std::istream& is,
                     const unsigned num_links,
                     const bool has_converter) {
    unsigned num_vertices, num_edges;
    is >> num_vertices >> num_edges;

    Advisor::Graph g;

    for (unsigned i = 0; i < num_vertices; i++) {
        boost::add_vertex(Link(i, num_links, has_converter), g);
    }

    unsigned a, b;
    for (unsigned i = 0; i < num_edges; i++) {
        is >> a >> b;
        boost::add_edge(a, b, g);
    }

    return g;
}

void
output_network(std::ostream& os, const Advisor::Graph& nodes) {
    boost::graph_traits<Advisor::Graph>::edge_iterator e_b, e_e;
    std::tie(e_b, e_e) = boost::edges(nodes);

    os << "graph {" << std::endl;
    for (auto it = e_b; it != e_e; it++) {
        auto src = boost::source(*it, nodes);
        auto tgt = boost::target(*it, nodes);
        os << "    " << src << " -- " << tgt << ";" << std::endl;
    }
    os << "}" << std::endl;
}

/**
 * \param[in] advisor the Advisor object that handles random number
 *                    generation, keeping track of the network state, and
 *                    finding paths.
 *
 * \param[in] limit the total number of packets to observe.
 *
 * \param[in] ignore_first the number of packets to ignore. Default to 10% of limit.
 */
float
simulate(Advisor& advisor,
         const unsigned limit,
         const unsigned ignore_first = 0) {
    bool ignored = false;
    // 10% of the limit by default
    unsigned to_ignore = ignore_first == 0 ? limit * 0.1 : ignore_first;
    unsigned connection_count = 0;
    unsigned success_count = 0;
    unsigned block_count = 0;
    Advisor::event_t now = 0;

    std::priority_queue<Event, std::vector<Event>, std::greater<Event>> pq;
    pq.push(Event(advisor.get_nodes(),
                  Event::START,
                  advisor.get_arrival()));
    connection_count++;

    while (true) {
        if (connection_count > limit) {
            break;
        }
        if (connection_count == to_ignore && !ignored) {
            connection_count = 0;
            success_count = 0;
            block_count = 0;
            ignored = true;
        }

        Event event = pq.top();
        pq.pop();
        now = event.time;

        Advisor::vertex_t src = event.src;
        Advisor::vertex_t dst = event.dst;

        switch (event.type) {
            case Event::START:
                {
                    std::vector<Advisor::vertex_t> path;
                    Link::wavelength_t wl;
                    std::tie(path, wl) = advisor.make_connection(src, dst);
                    // Wavelength is Link::NONE on failure
                    if (wl != Link::NONE) {
                        // Schedule finishing of connection
                        auto e = Event{event.src, event.dst, Event::END,
                            now + advisor.get_duration(), path, wl};
                        pq.push(e);
                    }
                    else {
                        pq.push(Event(Event::BLOCK, now));
                    }

                    // Schedule connection between two random nodes
                    pq.push(Event(advisor.get_nodes(),
                                  Event::START,
                                  now + advisor.get_arrival()));
                    connection_count++;
                    break;
                }
            case Event::END:
                advisor.remove_connection(event.path, event.wavelength);
                success_count++;
                break;
            case Event::BLOCK:
                block_count++;
                break;
            default:
                break;
        }
    }

    return static_cast<float>(block_count) / connection_count;
}

int
main(int argc, char* argv[]) {
    Advisor::event_t lambda = 5;
    Advisor::event_t duration_mean = 1;
    unsigned total = 8000;
    bool converter = false;
    std::string dot_file = "graph.dot";

    bool help = false;

    cxxopts::Options options{argv[0], " <graph filename> <num links>"};
    options.add_options()
        ("l,lambda", "Mean arrival rate in packets per second",
         cxxopts::value(lambda))
        ("d,duration", "Mean duration of connections in seconds",
         cxxopts::value(duration_mean))
        ("t,total", "Total number of connections to observe",
         cxxopts::value(total))
        ("c,converter", "Whether nodes have converters",
         cxxopts::value(converter))
        ("o,output", "Name of the output file for visualizing graph",
         cxxopts::value(dot_file))
        ("h,help", "Show this help",
         cxxopts::value(help));
    options.parse(argc, argv);

    if (help) {
        std::cout << options.help() << std::endl;
        return 0;
    }

    if (argc < 3) {
        std::cerr << options.help() << std::endl;
        return 1;
    }

    std::string filename{argv[1]};
    unsigned num_links = std::atoi(argv[2]);

    // Make nodes
    std::ifstream ifs{filename, std::ios::in};

    if (!ifs.good()) {
        std::cerr << "Error opening file" << std::endl;
        return 1;
    }

    // Output dot file to visualize network
    std::ofstream ofs{dot_file, std::ios::out};
    Advisor::Graph nodes = make_graph_from_file(ifs, num_links, converter);
    output_network(ofs, nodes);
    auto advisor = Advisor{nodes, lambda, duration_mean};

    auto pb = simulate(advisor, total);
    std::cout << pb * 100  << " %" << std::endl;

    return 0;
}
