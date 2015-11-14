#!/usr/bin/env ruby

require 'set'

# A simple script to generate connections for a graph
#
# Usage:
#   PROGRAM_NAME <num vertices> <num edges>

NUM_VERTICES = ARGV[0].nil? ? 10 : ARGV[0].to_i
NUM_EDGES = ARGV[1].nil? ? 20 : ARGV[1].to_i

edges = Set.new
while edges.size < NUM_EDGES do
  a = rand(NUM_VERTICES)
  b = rand(NUM_VERTICES)
  next if a == b
  edges.add(Set.new([a, b]))
end

arr = edges.map do |e|
  "#{e.to_a.join(' ')}"
end

puts arr.sort
