#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP

//Configuration file constants/global-parameters. 
const std::string config_path = "rule.cfg"; //Path to configuration file. 
const char comment_head = '#'; //Character denoting a comment in the rules file. 
const size_t rules_offset = 13; //Number of non-rule parameters at top of file. 
const std::string low_level_indicator = "LOW:", high_level_indicator = "HIGH:"; //Indicators that a rule is low/high level. 
std::vector<std::string> loaded_config; //Rules governing simulation. 

//Initialisation/drawing constants. 
const double grx = 150.0, gry = 110.0; //Half-dimensions of bounding rectangle. 
const double cx = width/2.0, cy = height/2.0, s = 5.0; //Drawing parameters. 
const double wr = 5.0; //Distance from walls that they are effective. 

//Agent constants. 
//Miscelaneous. 
std::vector<std::string> colours; //Colours to exist in this simulation. 
std::vector<std::string> goal_colours; //Colours that require a goal to be spawned. 
double clust_density = 1.0; //Local density required to be regarded as "clustered" (minimum group size). 
double lowagent_size = 2.5; //Size of a single low-level agent. 
double highagent_size = lowagent_size * 9; //Size of a single high-level agent (approx. 1 cluster). 
unsigned winstate_tickdelay = 10; //Every this many ticks, check if the goal-seekers have won. 

//Struct for storing information about a certain type of agent. 
struct agent_cluster_info {
	sf::Color color; 
	unsigned wandering; 
	unsigned killed; 
	unsigned ingoal; 
}; 
std::vector<agent_cluster_info> cluster_info; 

//CONSTANTS BELOW THIS POINT ARE LOADED IN FROM A FILE. 

//Main control constants. 
//Modes are: 0 - Start unclustered w/ goals, 1 - Start unclustered w/o goals (appear when clustered), 2 - Start clustered w/ goals. 
int mode; //Operating mode, has impact in 'reset()' and 'tick()' in "main.cpp". 
unsigned particle_count; //Number of particles to create. 
//Miscelaneous. 
unsigned tick_limit; //Maximum number of ticks one simulation can take. 
double guard_dist; //Guard-behaviour compels agents to hover at this distance from the goal. 
unsigned predator_stranger_tick_limit; //Number of ticks after which predators disengage from predators of a different color if no prey has been seen. 
//Radii. 
double lowagent_radius; //Viewing radius of low-level agents. 
double highagent_radius; //Viewing radius of high-level agents. 
double flee_radius_scale; //Prey does not flee from predators until the predators are within this fraction of radial distance. 
double goal_radius; //Radius at which the goal is effective. 
//Speed caps. 
double cap; //Final-total-summed speed cap for agents (After summing). 
double const_cap; //Speed cap for conserved velocity of agents (Before summing). 
double goal_cap; //Speed cap for goal-seeking velocity (Before summing). 
double goal_cap_fleeing; //Speed cap for goal-seeking velocity while fleeing (Before summing). 
double antigoal_cap; //Speed cap for goal-fleeing velocity (Before summing). 
double antigoal_cap_fleeing; //Speed cap for goal-fleeing velocity while fleeing (Before summing). 
double guard_cap; //Speed cap for guard velocity. 
//Scalars for tickwise velocity-components. 
double jitter; //Scalar for jittering velocity. 
double goaljitter; //Scalar for jittering while in goal. 
double clust; //Scalar for clustering velocity. 
double chase; //Scalar for chasing velocity. 
double flee; //Scalar for fleeing velocity. 
double goal; //Scalar for goal-seeking velocity. 
double antigoal; //Scalar for antigoal-fleeing velocity. 
double rotate; //Scalar for rotation velocity. 
double guard; //Scalar for guard velocity. 
//Acceleration constraints. 
double wall; //Acceleration away from walls. 

///////////////////////////////////////////////////////////////

//Parse string-named colours into true colours. 
sf::Color parse_color(std::string c) {
	if(c == "RED") {
		return sf::Color::Red; 
	} else if(c == "GREEN") {
		return sf::Color::Green; 
	} else if(c == "BLUE") {
		return sf::Color::Blue; 
	} else if(c == "CYAN") {
		return sf::Color::Cyan; 
	} else if(c == "MAGENTA") {
		return sf::Color::Magenta; 	
	} else if(c == "YELLOW") {
		return sf::Color::Yellow; 
	} else {
		return sf::Color::White; 
	}
}

//Find the particular set of rules that correspond to a particular colour. 
std::vector<std::string> find_rules_for_colour(std::string c) {
	std::vector<std::string> extracted; 
	bool write = false; 
	for(size_t i = 0; i < loaded_config.size(); i++) {
		if(loaded_config[i][0] == '*') {
			std::string tmp = loaded_config[i]; 
			tmp.erase(tmp.begin()); 
			if(find(tmp, c) == 0) {
				write = true; 
				extracted.push_back(loaded_config[i]); 
			} else {
				write = false; 
			}
		} else if(write) {
			extracted.push_back(loaded_config[i]); 
		}
	}
	return extracted; 
}

//Ensure the magnitude of a vector never exceeds a cap. 
std::vector<double> veccap(std::vector<double> v, double l) {
	if(ktw::norm(v) > l) {
		v = ktw::hat(v); 
		v = ktw::scale(l, v); 
	}
	return v; 
}

#endif