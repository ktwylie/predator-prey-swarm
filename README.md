# Predator/prey "Swarm Levels Problem" research project simulation. 
(Note that this code is here for demonstration, but is unlikely to compile immediately without extensive tweaking and updating.)

## Points of interest
The source code files "main.cpp" where the main program loop is located, and "entities/agent_cluster.cpp" which defines most of the interesting agent behavior. 

## Summary
The objective is to establish a relationship between "low-level" rules governing clusters of agents and "high-level" rules governing more abstracted representations of those clusters. 

## Technical details
Rules are represented as sequential condition checks which when activated cause particular actions. 
The simulation consists of agents (predator and prey) and goals (prey tries to reach its goal, predators try to keep prey from their goal and push into the "antigoal" where they are destroyed). 

## Dependencies
Visuals are produced via the SFML library, otherwise this depends entirely on the C++ STL, and to a limited extent an old version of my own private library. 
