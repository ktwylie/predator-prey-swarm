#ifndef LOADING_HPP
#define LOADING_HPP

//Load in the value of a particular constant from the file. 
void handle_loading_constants(std::string c) {
	size_t first_space = c.find_first_of(' '); 
	std::string name = c.substr(0, first_space); 
	double val = ktw::static_parse_string<double>(c.substr(first_space + 3)); 
	//Hardcoded parameter list. 
	if(name == "CAP") { cap = val; return; }
	if(name == "CONST_CAP") { const_cap = val; return; }
	if(name == "GOAL_CAP") { goal_cap = val; return; }
	if(name == "JITTER") { jitter = val; return; }
	if(name == "CLUST") { clust = val; return; }
	if(name == "CHASE") { chase = val; return; }
	if(name == "FLEE") { flee = val; return; }
	if(name == "GOAL") { goal = val; return; }
	if(name == "WALL") { wall = val; return; }
	if(name == "PARTICLE_COUNT") { particle_count = val; return; }
	if(name == "GOAL_CAP_FLEEING") { goal_cap_fleeing = val; return; }
	if(name == "GUARD_DIST") { guard_dist = val; return; }
	if(name == "ROTATE") { rotate = val; return; }
	if(name == "TICK_LIMIT") { tick_limit = val; return; }
	if(name == "GUARD") { guard = val; return; }
	if(name == "GUARD_CAP") { guard_cap = val; return; }
	if(name == "MODE") { mode = val; return; }
	if(name == "ANTIGOAL") { antigoal = val; return; }
	if(name == "ANTIGOAL_CAP") { antigoal_cap = val; return; }
	if(name == "ANTIGOAL_CAP_FLEEING") { antigoal_cap_fleeing = val; return; }
	if(name == "GOALJITTER") { goaljitter = val; return; }
	if(name == "LOWAGENT_RADIUS") { lowagent_radius = val; return; }
	if(name == "HIGHAGENT_RADIUS") { highagent_radius = val; return; }
	if(name == "FLEE_RADIUS_SCALE") { flee_radius_scale = val; return; }
	if(name == "GOAL_RADIUS") { goal_radius = val; return; }
	if(name == "PREDATOR_STRANGER_TICK_LIMIT") { predator_stranger_tick_limit = val; return; }
}

//Load in the rules from the configuration file and make them available to the agents. 
void parse_rules() {
	//Load in rules file and parse into meaningful newline-separated strings. 
	std::vector<char> cfg_chars = ktw::read(config_path); 
	std::string cts(cfg_chars.begin(), cfg_chars.end()); 
	loaded_config = ktw::sv(cts, "\r\n"); 

	//First pass: Cleanup and initial information gathering. 
	for(size_t i = 0; i < loaded_config.size(); i++) {
		if(loaded_config[i] == "" || loaded_config[i][0] == comment_head) { //Omit blank-lines and comments. 
			loaded_config.erase(loaded_config.begin() + i); 
			i--; 
		}
	}

	//Constants + header omissions. 
	size_t number_of_constants = ktw::static_parse_string<size_t>(loaded_config[0]); 

	//Second pass: Handle/ingest constant parameters, tally up prioritised rules. 
	for(size_t i = 1; i < loaded_config.size(); i++) {
		if(i - 1 < number_of_constants) {
			std::cout << "LOADING CONSTANT: " << loaded_config[i] << std::endl; 
			handle_loading_constants(loaded_config[i]); //Load in recorded value. 
		}
	}
	//Omit constants. 
	loaded_config.erase(loaded_config.begin(), loaded_config.begin() + number_of_constants + 1); 

	//Third pass: Determine what colours shall exist. 
	goal_colours = {}; 
	colours = {}; 
	for(size_t i = 0; i < loaded_config.size(); i++) {
		if(loaded_config[i][0] == '*') {
			std::string tmp = loaded_config[i]; 
			tmp.erase(tmp.begin()); 
			std::vector<std::string> tmpl = ktw::sv(tmp, ","); 
			colours.push_back(tmpl[0]); 
			if(tmpl[1] == "PREY") { //Give each prey group a goal to reach. 
				goal_colours.push_back(tmpl[0]); 
			}
		}
	}
	ktw::nl(); 
	std::cout << "SIMULATION WITH " << colours.size() << " COLOURS: "; 
	ktw::pv(colours); 
	std::cout << "AND " << goal_colours.size() << " PREY COLOURS: "; 
	ktw::pv(goal_colours); 

	//At this point the rules should be extracted completely, and are ready for splitting up by colours. 
}

//Write the current state of the "cycle_ticks" to the output file. 
void dump_data() {
	std::string out_cycle_ticks; 
	for(size_t i = 0; i < cycle_ticks.size(); i++) {
		out_cycle_ticks += ktw::str(cycle_ticks[i]) + ((i == cycle_ticks.size() - 1) ? "" : ","); 
	}
	out_cycle_ticks += "\r\n"; 
	for(size_t i = 0; i < win_type.size(); i++) {
		out_cycle_ticks += ktw::str(win_type[i]) + ((i == win_type.size() - 1) ? "" : ","); 
	}
	//Generate/write new log file for this run. 
	auto end = std::chrono::system_clock::now(); 
	std::time_t end_time = std::chrono::system_clock::to_time_t(end);
	std::string end_time_string = ktw::str(std::ctime(&end_time)); 
	end_time_string.erase(end_time_string.size() - 1); 
	std::string logfilepath = "logs/mode" + ktw::str(mode) + "_log_" + end_time_string + ".csv"; 
	for(size_t i = 0; i < logfilepath.size(); i++) {
		if(logfilepath[i] == ' ') {
			logfilepath[i] = '-'; 
		} else if(logfilepath[i] == ':') {
			logfilepath.erase(logfilepath.begin() + i); 
			i--; 
		}
	}
	std::cout << "DUMPING LOGGED-DATA TO \"" + logfilepath + "\"." << std::endl; 
	ktw::write(std::vector<char>(out_cycle_ticks.begin(), out_cycle_ticks.end()), logfilepath); 
}

//Get the index of a particular colour of agent in 'cluster_info'. 
size_t search_cluster_info(sf::Color c, int occ) {

	//if(occ == 1) std::cout << occ << std::endl;

	//if(c == sf::Color::Green) std::cout << "GREEN" << std::endl; 
	//if(c == sf::Color::Blue) std::cout << "BLUE" << std::endl; 

	for(size_t i = 0; i < cluster_info.size(); i++) {
		if(cluster_info[i].color == c) return i; 
	}
	return cluster_info.size(); 
}

#endif