/*
Summer Swarm Research. 

AUTHOR: Kyle T. Wylie
EST: 16 May 2020
*/

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <SFML/Window.hpp>
#include <SFML/System.hpp>

#include <array>
#include <iostream>
#include <string>
#include <cmath>
#include <vector>
#include <iomanip>
#include <functional>
#include <complex>
#include <chrono>
#include <ctime>
#include <regex>

#include "C:\Users\kylewylie\Data\Corporate\Programming\c++\ktw-lib\ktwgen.hpp"
#include "C:\Users\kylewylie\Data\Corporate\Programming\c++\ktw-lib\ktwmath.hpp"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//Pseudo-random number generator & initial seed. 
const int rng_seed = (uint64_t)time(0)+((uint64_t)time(0)<<32); 
ktw::llcaprng2 rng(rng_seed); 
const std::clock_t t0 = std::clock(); //Program start timestamp. 

//Utilty functions. 
#include "utils.hpp"

//Scores: How many cycles each team has "won". 
//Win for prey: All prey makes it to the goal. 
//Win for predator: Tick-limit is reached with prey not in goal. 
unsigned long long goal_points = 0; 
unsigned long long prevent_points = 0; 
unsigned long long neither_points = 0; 

bool draw_extras = false; //Draw extra elements to the screen. 
bool paused = false; //Is the simulation paused? 
bool showcontrols = false; //Should we draw the controls to the screen? 
bool abide_autoreset = true; //If true, the simulation will automatically reset once the ticklimit is exceeded. 
unsigned cycles = 0; //How many times has the simulation been reset? 

#include "constants.hpp" //Simulation constants. 
#include "entities/agent_super.hpp" //Include for the agents used by this file. 
#include "entities/agent_goal.hpp" //Ditto. 
//Possible cause of prey not fleeing directly away from predators sometimes is that if predator is too close, flee desire would be lower than other components. 
#include "entities/agent_cluster.hpp" //Ditto. 

std::vector<agent*> particles; //List of particles. 

//To be written to the CSV output. 
std::vector<unsigned> cycle_ticks; //How long each cycle that has so far transpired took to finish. 
std::vector<unsigned> win_type; //Manner of each cycle's win-state (See "ActionItems7_24_2020.docx"). 

#include "file_io/loading.hpp" //Functions related to loading in files. 

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//Reset the simulation. 
//AN: This could use some streamlining. 
void reset() {
	cycles++; //Tally this cycle. 
	t = 0; //Reset ticks. 
	particles.clear(); //Clear particles. 
	cluster_info.clear(); 
	for(size_t i = 0; i < goal_colours.size(); i++) cluster_info.push_back(agent_cluster_info{parse_color(colours[i]), 0, 0, 0}); //Reset (prey) cluster info. 

	/*
	if(mode == 0) {

	} else if(mode == 1) {

	} else if(mode == 2) {

	} else if(mode == 3) {

	} else {

	}
	*/

	//Generate points to center clusters at (mode 2). 
	std::vector<ktw::point2> cluster_centers; 
	double cluster_dist = 15; //Initial density of clusters. 
	for(size_t i = 0; i < colours.size(); i++) {
		cluster_centers.push_back(ktw::point2{rng.next<double>()*(grx), rng.next<double>()*(gry)}); 
	}
	//Add agents. 
	if(mode == 3) particle_count = colours.size(); 
	for(size_t i = 0; i < particle_count; i++) {
		std::vector<std::string> lcc; 
		double genx, geny; 
		for(size_t j = 0; j < colours.size(); j++) { //Generate each colour in equal proportion. 
			if(i % colours.size() == j) {
				lcc = find_rules_for_colour(colours[j]); 

				if(mode == 2) {
					genx = cluster_centers[j].x; 
					geny = cluster_centers[j].y; 
				}

				break; 
			}
		}
		if(mode == 0 || mode == 1 || mode == 3) {
			genx = rng.next<double>()*(1.0*grx); 
			geny = rng.next<double>()*(1.0*gry); 
		} else if(mode == 2) {
			genx += rng.next<double>() * cluster_dist; 
			geny += rng.next<double>() * cluster_dist; 
		}
		if(mode == 3) {
			particles.push_back(new highlevel_agent(genx, geny, 1.0*rng.next<double>(), 1.0*rng.next<double>(), highagent_radius, lcc)); 
		} else {
			particles.push_back(new cluster_agent(genx, geny, 1.0*rng.next<double>(), 1.0*rng.next<double>(), lowagent_radius, lcc)); 
			if(mode == 2) {
				particles[particles.size() - 1]->const_accel(grx*rng.next<double>(), gry*rng.next<double>(), 1.0); //Get "start clustered" low-level agents (mode 2) moving around more. 
			}
		}
	}
	//Add goals/antigoals. 
	if(mode == 1) return; 
	for(size_t i = 0; i < goal_colours.size(); i++) {
		double goal_x = rng.next<double>()*(0.8*grx); 
		double goal_y = rng.next<double>()*(0.8*gry); 
		double antigoal_x = rng.next<double>()*(0.8*grx); 
		double antigoal_y = rng.next<double>()*(0.8*gry); 
		while(ktw::distance(goal_x, goal_y, antigoal_x, antigoal_y) < 2*goal_radius) { //Ensure goals/antigoals don't overlap. 
			goal_x = rng.next<double>()*(0.8*grx); 
			goal_y = rng.next<double>()*(0.8*gry); 
			antigoal_x = rng.next<double>()*(0.8*grx); 
			antigoal_y = rng.next<double>()*(0.8*gry); 
		}
		particles.push_back(new goal_agent(goal_x, goal_y, goal_radius, parse_color(goal_colours[i]))); 
		particles.push_back(new antigoal_agent(antigoal_x, antigoal_y, goal_radius, parse_color(goal_colours[i]))); 
	}
}

//Prints/writes statistics once the end state has been detected. 
void check_end_state() {
	//Assess how many prey are in the goal (prey that have been killed are already tallied)
	const unsigned prey_clusters = 2; 
	unsigned prey_clusters_ingoal = 0, prey_clusters_wandering = 0, prey_clusters_killed = 0; 
	for(size_t i = 0; i < cluster_info.size(); i++) {
		unsigned total = cluster_info[i].killed + cluster_info[i].wandering + cluster_info[i].ingoal; 
		//std::cout << i << "; killed " << cluster_info[i].killed << ", wander " << cluster_info[i].wandering << ", ingoal " << cluster_info[i].ingoal << std::endl; 
		if(cluster_info[i].killed > total/2) {
			prey_clusters_killed++; 
		} else if(cluster_info[i].ingoal > total/2) {
			prey_clusters_ingoal++; 
		} else if(cluster_info[i].wandering > total/2) {
			prey_clusters_wandering++; 
		}
	}

	//std::cout << "ingoal: " << prey_clusters_ingoal << ", killed: " << prey_clusters_killed << ", wandering: " << prey_clusters_wandering << std::endl; 

	/* Win states that were originally here (extra predator time). 
	if(prey_clusters_ingoal == 2) { //1: Both prey reach goal 
		cycle_ticks.push_back(t); 
		win_type.push_back(1); 
	} else if(prey_clusters_killed == 2) { //5: Both prey eliminated
		cycle_ticks.push_back(t); 
		win_type.push_back(6); 
	} else if(t > tick_limit) { //Tick limit exceeded...
		if(prey_clusters_ingoal == 1 && prey_clusters_wandering == 1) { //2: One prey reaches goal, other wandering 
			cycle_ticks.push_back(t); 
			win_type.push_back(2); 
		} else if(prey_clusters_ingoal == 1 && prey_clusters_killed == 1) { //3a: One prey reaches goal, other eliminated
			cycle_ticks.push_back(t); 
			win_type.push_back(3); 
		} else if(prey_clusters_wandering == 2) { //3b: No prey reach goal, none eliminated
			cycle_ticks.push_back(t); 
			win_type.push_back(4); 
		} else if(prey_clusters_killed == 1 && prey_clusters_wandering == 1) { //4: One prey eliminated, other wandering 
			cycle_ticks.push_back(t); 
			win_type.push_back(5); 
		} else {
			return; 
		}
	} else {
		return; 
	}
	//*/

	//* Modern win conditions with "early prey victories". 
	if(prey_clusters_ingoal == 2) { //1: Both prey reach goal 
		cycle_ticks.push_back(t); 
		win_type.push_back(1); 
	} else if(prey_clusters_killed == 2) { //5: Both prey eliminated
		cycle_ticks.push_back(t); 
		win_type.push_back(6); 
	} else if(prey_clusters_ingoal == 1 && prey_clusters_killed == 1) { //3a: One prey reaches goal, other eliminated MOVED TO HAPPEN INSTANTLY ONLY TWEAK FROM OTHER DEBUGGING DATA
		cycle_ticks.push_back(t); 
		win_type.push_back(3); 
	} else if(t > tick_limit) { //Tick limit exceeded...
		if(prey_clusters_ingoal == 1 && prey_clusters_wandering == 1) { //2: One prey reaches goal, other wandering 
			cycle_ticks.push_back(t); 
			win_type.push_back(2); 
		} else if(prey_clusters_wandering == 2) { //3b: No prey reach goal, none eliminated
			cycle_ticks.push_back(t); 
			win_type.push_back(4); 
		} else if(prey_clusters_killed == 1 && prey_clusters_wandering == 1) { //4: One prey eliminated, other wandering 
			cycle_ticks.push_back(t); 
			win_type.push_back(5); 
		} else {
			return; 
		}
	} else {
		return; 
	}
	//*/

	std::cout << "END-STATE-" << cycles << "-DETECTED (~" << t << " ticks, " << ktw::str(ktw::dur(t0)) << " elapsed)" << std::endl; 
	reset(); 
	
	/*
	//Determine if any goal-seekers are not in the goal. 
	bool all_seekers_in_goal = true; 
	bool goals_exist = false; 
	bool prey_exists = false; 
	std::vector<sf::Color> goal_colours_real; 
	for(size_t i = 0; i < goal_colours.size(); i++) goal_colours_real.push_back(parse_color(goal_colours[i])); 
	for(size_t i = 0; i < particles.size(); i++) {
		if(particles[i]->isgoal()) goals_exist = true; 
		if(particles[i]->isprey()) prey_exists = true; 
		if(ktw::someof(goal_colours_real, particles[i]->getcolor()) && !particles[i]->ingoal()) all_seekers_in_goal = false; 
	}
	//If there are no goals, do not continue. 
	if(!goals_exist) return; 
	//At this point all the winning conditions have been determined. 
	if(!prey_exists) { //Predators win if they eliminated all prey. 
		prevent_points++; 
	} else if(all_seekers_in_goal) { //Prey wins if they all make it to the goal. 
		goal_points++; 
	} else if(t > tick_limit) { //Neither win if the time runs out. 
		neither_points++; 
	} else { //If no end-state has been reached, continue with the simulation. 
		return; 
	}
	std::cout << "END-STATE-" << cycles << "-DETECTED (~" << t << " ticks, " << ktw::str(ktw::dur(t0)) << " elapsed)" << std::endl; 
	reset(); 
	*/
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//Setup, run once at start of program. 
void init() {
	std::cout << "INITIALISING..." << std::endl; 
	ktw::nl(); 

	srand(time(NULL)); 
	//Summon rules from file. 
	parse_rules(); 
	//Reset. 
	reset(); 
	
	ktw::nl(); 
	std::cout << "INITIALISATION COMPLETE. " << std::endl; 
	ktw::cl(); 
}

//Tick a sub-section of the list of particles for multithreading. 
//AN: Modify this to guarantee that even if particle-count is not divisible by thread count that all particles get ticked. 
template <unsigned chunks, unsigned which> void particle_tick() {
	size_t chunk_size = particles.size() / chunks; 
	size_t start_i = chunk_size * which; 
	size_t end_i = start_i + chunk_size; 
	for(size_t i = start_i; i < end_i; i++) {
		//std::cout << "<" << chunks << "," << which << "> " << start_i << " -> " << i << " -> " << start_i + chunk_size << std::endl; 
		particles[i]->tick(particles, i); 
	}
}
sf::Thread q1(&particle_tick<8, 0>); 
sf::Thread q2(&particle_tick<8, 1>); 
sf::Thread q3(&particle_tick<8, 2>); 
sf::Thread q4(&particle_tick<8, 3>); 
sf::Thread q5(&particle_tick<8, 4>); 
sf::Thread q6(&particle_tick<8, 5>); 
sf::Thread q7(&particle_tick<8, 6>); 
sf::Thread q8(&particle_tick<8, 7>); 

//Perform these actions each tick. 
void tick(sf::RenderWindow* w) {
	if(!paused) {
		//Tick all particles. 

		//Single-threaded solution. 
		
		//Tick all particles. 
		for(size_t i = 0; i < particles.size(); i++) particles[i]->tick(particles, i); 
		
		//Multi-threaded solution (?). 
		
		//Tick all particles. 
		/*
		q1.launch(); 
		q2.launch(); 
		q3.launch(); 
		q4.launch(); 
		q5.launch(); 
		q6.launch(); 
		q7.launch(); 
		q8.launch(); 
		q1.wait(); 
		q2.wait(); 
		q3.wait(); 
		q4.wait(); 
		q5.wait(); 
		q6.wait(); 
		q7.wait(); 
		q8.wait(); 
		*/
		
		//Reset values of cluster_info that can fluctuate (e.g. not killed, since it's absolute). 
		for(size_t i = 0; i < cluster_info.size(); i++) {
			cluster_info[i].wandering = 0; 
			cluster_info[i].ingoal = 0; 
		}
		//Then perform removals/tallies/motion. 
		for(size_t i = 0; i < particles.size(); i++) {
			if(particles[i]->isflaggedforremoval()) {
				//std::cout << "Killed " << search_cluster_info(particles[i]->getcolor(), 0) << " " << cluster_info[search_cluster_info(particles[i]->getcolor(), 0)].killed << std::endl; 
				cluster_info[search_cluster_info(particles[i]->getcolor(), 1)].killed += 1; //Tally that this one is killed. 
				//Remove those that must be. 
				delete particles[i]; 
				particles.erase(particles.begin() + i); //Why does this sometimes erase the wrong particle? FIXED
				i--; 
			} else {
				//Determine if this non-killed (prey) agent is in the goal or wandering. 
				if(particles[i]->isprey()) {
					if(particles[i]->ingoal()) {
						cluster_info[search_cluster_info(particles[i]->getcolor(), 2)].ingoal += 1; //Tally that this one is in the goal 
					} else {
						cluster_info[search_cluster_info(particles[i]->getcolor(), 3)].wandering += 1; //Tally that this one is wandering. 
					}
				}
				//Move all agents regardless. 
				particles[i]->move(); 
			}
		}
		//Perform any mode-specific actions. 
		if(mode == 1) { //Check if all are clustered and if they are, add the goals. 
			bool all_clust = true;
			for(size_t i = 0; i < particles.size(); i++) {
				if(!particles[i]->isclustered()) {
					all_clust = false; 
					break; 
				}
			}
			if(all_clust) {
				for(size_t i = 0; i < goal_colours.size(); i++) {
					particles.push_back(new goal_agent(rng.next<double>()*(0.8*grx), rng.next<double>()*(0.8*gry), goal_radius, parse_color(goal_colours[i]))); 
				}
				std::cout << "CLUSTERED-STATE-DETECTED (~" << t << " ticks, " << ktw::str(ktw::dur(t0)) << " elapsed)" << std::endl; 
			}
		}
		//Every so often, check if the goal-seekers have won. 
		if(t % winstate_tickdelay == 0 && abide_autoreset) {
			check_end_state(); 
		}
	}
}

//Draw a single frame. 
void frame(sf::RenderWindow* w) {
	for(size_t i = 0; i < particles.size(); i++) particles[i]->draw_1(w, cx, cy, s); //Draw grey radial circles. 
	for(size_t i = 0; i < particles.size(); i++) particles[i]->draw_2(w, cx, cy, s); //...then draw the agents themselves. 
	//Then draw the bounding rectangle. 
	draw_line(s * -grx + cx, s * -gry + cy, s * grx + cx, s * -gry + cy, sf::Color::White); 
	draw_line(s * -grx + cx, s * gry + cy, s * grx + cx, s * gry + cy, sf::Color::White); 
	draw_line(s * -grx + cx, s * -gry + cy, s * -grx + cx, s * gry + cy, sf::Color::White); 
	draw_line(s * grx + cx, s * -gry + cy, s * grx + cx, s * gry + cy, sf::Color::White); 
	draw_rect(0, 0, s * -grx + cx - 1, height, sf::Color::Black); 
	draw_rect(0, 0, width, s * -gry + cy, sf::Color::Black); 
	draw_rect(s * grx + cx, 0, width, height, sf::Color::Black); 
	draw_rect(s * -grx + cx, s * gry + cy + 1, width, height, sf::Color::Black); 
	//Draw point counts. 
	draw_string("Predator points: " + ktw::str(prevent_points) + "    Prey points: " + ktw::str(goal_points) + "    Neither points: " + ktw::str(neither_points), 10, 10, sf::Color::White); 
	//Report if paused. 
	if(paused) {
		draw_string("Paused", width - 70, 10, sf::Color::Green); 
	} else {
		draw_string("Paused", width - 70, 10, sf::Color::Red); 
	}
	//Report if auto-reset is to be abided. 
	if(abide_autoreset) {
		draw_string("Abide autoreset", width - 230, 10, sf::Color::Green); 
	} else {
		draw_string("Abide autoreset", width - 230, 10, sf::Color::Red); 
	}
	//Draw controls. 
	if(showcontrols) {
		draw_string("Controls: R - Reset, P - Pause, E - Extra graphics, D - Dump logs, L - Reload rules, A - Abide auto-reset", width/2 - 200, 10, sf::Color::White); 
	} else {
		draw_string("Press \'C\' on keyboard to show controls.", width/2 - 200, 10, sf::Color::White); 
	}

	//draw_line(s*0 + cx, s*0 + cy, s*(agent_radius) + cx, s*(0) + cy, sf::Color::Magenta); 
	//draw_line(s*(agent_radius) + cx, s*0 + cy, s*(agent_radius + goal_radius) + cx, s*(0) + cy, sf::Color::Green); 
}

//Termination, run once at end of program. 
void cleanup() {
	for(size_t i = 0; i < particles.size(); i++) { //Clean up allocated memory. 
		delete particles[i]; 
		particles[i] = 0; 
	}
	//Write output log. 
	if(cycle_ticks.size() > 0) dump_data(); 
}

//Event handling function.
void eventhandle(sf::RenderWindow* w, sf::Event event) {
	while(w->pollEvent(event)) {
		switch(event.type) {
			case sf::Event::Closed:
				cleanup(); 
				w->close();
				break;
			case sf::Event::KeyPressed:
				if(event.key.code == sf::Keyboard::R) { //Reset simulation, same rules. 
					reset(); 
				} else if(event.key.code == sf::Keyboard::P) { //Toggle pause. 
					paused = !paused; 
				} else if(event.key.code == sf::Keyboard::E) { //Toggle extras. 
					draw_extras = !draw_extras; 
				} else if(event.key.code == sf::Keyboard::D) { //Dump current logs to file. 
					dump_data(); 
				} else if(event.key.code == sf::Keyboard::L) { //Reload rules, requires reset to take effect. 
					ktw::nl(); 
					parse_rules(); 
					ktw::cl(); 
				} else if(event.key.code == sf::Keyboard::A) { //Abide auto-reset or not. 
					abide_autoreset = !abide_autoreset; 
				} else if(event.key.code == sf::Keyboard::C) { //Show controls. 
					showcontrols = !showcontrols; 
				}
				break; 
			case sf::Event::MouseButtonPressed:
				if(event.mouseButton.button == sf::Mouse::Left) {
					//...
				}
				break; 
			default:
				break;
		}; 
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//Graphical rendering thread.
void renderthread(sf::RenderWindow* w) {
	font.loadFromFile("tnr.ttf");
	text.setFont(font);
	text.setCharacterSize(fontsize);
	while(w->isOpen()) {
		w->clear(sf::Color::Black); //Clear. 
		
		//Draw all buttons. 
		for(size_t i = 0; i < buttons.size(); i++) buttons[i].draw(w); 

		//Draw frame. 
		frame(w); 

		//Draw FPS. 
		draw_string(ktw::str((int) fps) + " fps, " + ktw::str((int) tps) + " tps, " + ktw::str((int) t) + " ticks", 10, height - fontsize - 10, sf::Color::White, w); 
		//Initiate frame-draw. 
		w->display(); 
		//Wait some time per frame.
		ktw::wait(framedelay);
		frames_since_last++; 
		f++; 
	}
}

//Main program entry point.
int main() {
	srand(time(NULL));
	sf::RenderWindow w(sf::VideoMode(width, height), "Summer Swarm 2020", sf::Style::Default);
	mw = &w; 
	w.setActive(false);

	init(); //Run any initial setup that must be done. 

	sf::Thread rt(&renderthread, &w);
	rt.launch();
	std::clock_t fps_t0 = std::clock();  //Timestamp for FPS. 
	while(w.isOpen()) {
		//Handle misc events.
		sf::Event event;
		eventhandle(&w, event);
		//Check if any buttons are being clicked. 
		for(size_t i = 0; i < buttons.size(); i++) buttons[i].click(&w); 

		//Game events. 
		tick(&w); 

		//Calculate frames-per-second & ticks-per-second. 
		if(ktw::dur(fps_t0) >= fps_calc_delay) { //Every so often. 
			fps = (long double) frames_since_last / ktw::dur(fps_t0); //Compute FPS. 
			tps = (long double) ticks_since_last / ktw::dur(fps_t0); //Compute TPS. 
			frames_since_last = 0; //Reset frames-since-last check. 
			ticks_since_last = 0; 
			fps_t0 = std::clock(); //Reset timer. 
		}

		//Wait some time per tick. 
		ktw::wait(tickdelay); 
		ticks_since_last++; 
		
		if(!paused) t++; 
	}

	//Clean up and report normal exit. 
	return 0;
}