#ifndef AGENT_CLUSTER_HPP
#define AGENT_CLUSTER_HPP

/*
	Low-level predator and/or prey agent. 
	
	AUTHOR: Kyle T. Wylie
	EST: 15 May 2020
*/

//Simulation constraints. 
const bool goalsensor_works = true; //If true, prey will "fail" to detect/remember their goal when they encounter it. 

class cluster_agent : public agent {
private: 
	bool first_tick = true; //Is this agent currently on it's first tick?
	double const_dx = dx, const_dy = dy; //Preserved velocity component for each tick. 
	sf::Color chase_color, flee_color; //Colours to chase and flee. 
	double last_goal_x = NAN, last_goal_y = NAN; //Last known position of goal region. 
	double last_antigoal_x = NAN, last_antigoal_y = NAN; //Last known position of antigoal region. 
	bool in_goal = false; //Is this agent currently in the goal region. 
	std::string current_rule_state = ""; //Which prioritised rule is currently active on this agent. 
	std::vector<std::string> loaded_config; //Prioritised rules for this agent. 
	std::vector<unsigned> tallies; //Tallies of rule-occurance for this agent alone. 

	std::string acting; //Which actions is this agent currently performing. 

	//Variables relating to centres of  mass of other particles. 
	double same_color_com_x = 0.0, same_color_com_y = 0.0; //Those to cluster with (same). 
	double same_color_count = 0.0; 
	double chase_color_com_x = 0.0, chase_color_com_y = 0.0; //Prey. 
	double chase_color_count = 0.0; 
	double flee_color_com_x = 0.0, flee_color_com_y = 0.0; //Predators. 
	double flee_color_count = 0.0; 

	//Misc. movement control booleans. 
	bool dochase, doflee, docluster; 
	bool shouldcluster = true; //AN: Shouldcluster allows agents to stop trying to cluster. 
	bool is_seen_pred_pushing; //Has this agent seen a predator who is pushing a prey? 
	bool approaching_goal, approaching_antigoal; //Is this agent getting closer to the goal (only calculable if position is known). 
	double last_x, last_y; //Coordinate position last tick (for computing approaching_goal, approaching_antigoal). 

	//For predators: how long has this predator been seeing predators of a different color? 
	unsigned ticks_seeing_stranger_predators = 0; 

	//Booleans which define if this agent is a predator or is prey (or both, or neither). 
	bool is_predator, is_prey; 

	//Compute average position of neighbours same, chase, and flee colors. 
	void compute_centres_of_mass(std::vector<agent*> a) {
		same_color_com_x = 0.0, same_color_com_y = 0.0; 
		same_color_count = 0.0; 
		chase_color_com_x = 0.0, chase_color_com_y = 0.0; 
		chase_color_count = 0.0; 
		flee_color_com_x = 0.0, flee_color_com_y = 0.0; 
		flee_color_count = 0.0; 
		in_goal = false; 
		is_seen_pred_pushing = false; 
		double dist; 
		bool saw_stranger_predators = false; //Were any "stranger" predators spotted this tick. 
		for(size_t i = 0; i < a.size(); i++) {
			if(a[i]->isgoal()) { //If a goal region is found, record it's position. 
				dist = sqrt((x - a[i]->getx())*(x - a[i]->getx()) + (y - a[i]->gety())*(y - a[i]->gety())); 
				if(is_prey && goalsensor_works) { //Prey only records it's own goal. 
					if(a[i]->getcolor() == color) { //This is done rather than '&&' to force all goals to be caught here. 
						last_goal_x = a[i]->getx(); //AN: Alter such that agents are in the goal if in the goal's radius. 
						last_goal_y = a[i]->gety(); 
						in_goal = dist < goal_radius; //Currently in the goal region. 
					}
				} else if(is_predator) { //Predators record the first goal they encounter. 
					last_goal_x = a[i]->getx(); 
					last_goal_y = a[i]->gety(); 
					in_goal = dist < goal_radius; //Currently in the goal region. 
				}
			} else if(a[i]->isantigoal()) { //If an antigoal region is found, record it's position. 
				if(is_prey) {
					if(a[i]->getcolor() == color) { //...iff it is the dangerous colour. 
						last_antigoal_x = a[i]->getx(); 
						last_antigoal_y = a[i]->gety(); 
					}
				} else if(is_predator) {
					last_antigoal_x = a[i]->getx(); //Predators remember all antigoals. 
					last_antigoal_y = a[i]->gety(); 
				}
			} else if(a[i]->getcolor() == color) { //Otherwise, compute centres of mass. 
				same_color_count += 1.0; 
				same_color_com_x += a[i]->getx(); 
				same_color_com_y += a[i]->gety(); 
			} else if(!a[i]->isclustered()) { //Otherwise, if not clustered, disregard. 
				continue; 
			} else if(a[i]->isprey()) {
				chase_color_count += 1.0; 
				chase_color_com_x += a[i]->getx(); 
				chase_color_com_y += a[i]->gety(); 
				if(this->ispredator()) { //If we are a predator, we have seen prey so do not disengage from friendly predators. 
					ticks_seeing_stranger_predators = 0; 
				}
			} else if(a[i]->ispredator()) {
				if(a[i]->ispushing()) is_seen_pred_pushing = true; 
				flee_color_count += 1.0; 
				flee_color_com_x += a[i]->getx(); 
				flee_color_com_y += a[i]->gety(); 
				if(this->ispredator()) { //If we are a predator, we are seeing foreign predators so count ticks having seen them. 
					saw_stranger_predators = true; 
				}
			}
		}
		if(saw_stranger_predators) { //If we saw a stranger predator, increment the number of times we've seen them. 
			ticks_seeing_stranger_predators += 1; 
		}
		//Finish average positions. 
		if(same_color_count > 0.0) {
			same_color_com_x /= same_color_count; 
			same_color_com_y /= same_color_count; 
		}
		if(chase_color_count > 0.0) {
			chase_color_com_x /= chase_color_count; 
			chase_color_com_y /= chase_color_count; 
		}
		if(flee_color_count > 0.0) {
			flee_color_com_x /= flee_color_count; 
			flee_color_com_y /= flee_color_count; 
		}

		//Update relevenat conditions. 
		docluster = same_color_count > 0.1; //Can this agent attempt to cluster? 
		dochase = chase_color_count > 0.1; //Can this agent attempt to chase? 
		doflee = flee_color_count > 0.1; /* && dist < r*flee_radius_scale;*/ //Can this agent attempt to flee? 
	}

	//Execute these actions before prioritised rules. 
	void pre_rules(std::vector<agent*> a) {
		//Compute all relevant centres of mass. 
		compute_centres_of_mass(a); 
		//Handle ambient motion (avoiding walls / jittering). 
		if(fabs(x - grx) < wr) {
			const_dx += -wall; //Right wall. 
		} else if(fabs(x - -grx) < wr) {
			const_dx += wall; //Left wall. 
		} else if(fabs(y - gry) < wr) {
			const_dy += -wall; //Bottom wall. 
		} else if(fabs(y - -gry) < wr) {
			const_dy += wall; //Top wall. 
		} else {
			//Not near to a wall. 
			if(in_goal) { //AN: Possible solution of wandering out? 
				const_dx += rng_r_fast() * goaljitter; 
				const_dy += rng_r_fast() * goaljitter; 
			} else {
				const_dx += rng_r_fast() * jitter; 
				const_dy += rng_r_fast() * jitter; 
			}
		}
		//Determine if the goal/antigoal is being approached. 
		if(knowsgoal()) {
			double goal_dist_last = sqrt((last_x - last_goal_x)*(last_x - last_goal_x) + (last_y - last_goal_y)*(last_y - last_goal_y)); 
			double goal_dist_curr = sqrt((x - last_goal_x)*(x - last_goal_x) + (y - last_goal_y)*(y - last_goal_y)); 
			approaching_goal = goal_dist_curr < goal_dist_last; 
		}
		if(knowsantigoal()) {
			double goal_dist_last = sqrt((last_x - last_antigoal_x)*(last_x - last_antigoal_x) + (last_y - last_antigoal_y)*(last_y - last_antigoal_y)); 
			double goal_dist_curr = sqrt((x - last_antigoal_x)*(x - last_antigoal_x) + (y - last_antigoal_y)*(y - last_antigoal_y)); 
			approaching_antigoal = goal_dist_curr < goal_dist_last; 
		}
		//Periodically put a stopper on the conserved velocity when in the goal to help prevent wandering out. 
		if(knowsgoal() && t % 100 == 0) {
			const_dx *= 0.9; 
			const_dy *= 0.9; 
		}
		last_x = x; //Record position before motion for calculation of "approaching_goal". 
		last_y = y; 
		//Motion gets reset to the preserved value (from last tick) each tick. 
		dx = const_dx; 
		dy = const_dy; 
	}
	//Execute these actions after prioritised rules. 
	void post_rules(std::vector<agent*> a) {
		//Always cluster, always chase prey, always flee from predators. 
		//AN: This in theory also implements the "pushing" necessity we talked about. 
		if(docluster && shouldcluster) accel(same_color_com_x, same_color_com_y, clust); 
		if(is_clustered && is_predator && dochase) accel(chase_color_com_x, chase_color_com_y, chase); 
		if(is_clustered && is_prey && doflee) accel(flee_color_com_x, flee_color_com_y, -flee); 
		//Ensure preserved velocity doesn't get too high. 
		double speed = sqrt(const_dx*const_dx + const_dy*const_dy); 
		if(speed > const_cap) {
			//Create unit vector. 
			const_dx /= speed; 
			const_dy /= speed; 
			//Multiply by speed cap. 
			const_dx *= const_cap; 
			const_dy *= const_cap; 
		}
		//Ensure total velocity doesn't get too high. 
		speed = sqrt(dx*dx + dy*dy); 
		if(speed > cap) {
			//Create unit vector. 
			dx /= speed; 
			dy /= speed; 
			//Multiply by speed cap. 
			dx *= cap; 
			dy *= cap; 
		}
	}
	//Replace named booleans with their corresponding truth-values. 
	std::string parse_rule_bools(std::string rb) {
		auto bool_to_str = [](bool b) -> std::string {
			return (b) ? "true" : "false" ;
		}; 
		//Hardcoded list of bools. 
		rb = ktw::substitute(rb, "KNOWS_GOAL", bool_to_str(knowsgoal())); 
		rb = ktw::substitute(rb, "IS_CLUSTERED", bool_to_str(isclustered())); 
		rb = ktw::substitute(rb, "PUSHING", bool_to_str(dochase || (doflee && !ispredator()))); 
		rb = ktw::substitute(rb, "APPROACHING_GOAL", bool_to_str(approaching_goal)); 
		rb = ktw::substitute(rb, "APPROACHING_ANTIGOAL", bool_to_str(approaching_antigoal)); 
		rb = ktw::substitute(rb, "KNOWS_ANTIGOAL", bool_to_str(knowsantigoal())); 
		rb = ktw::substitute(rb, "FLEEING", bool_to_str(doflee)); 
		rb = ktw::substitute(rb, "ISSEENPREDPUSHING", bool_to_str(is_seen_pred_pushing)); 
		return rb; 
	}
	//Perform the behaviour of a single named action. 
	//AN: Add actions for predators to keep prey away from goal, and for prey to try to get to the goal. 
	void parse_rule_action(std::string ra) {
		//Hardcoded list of actions. 
		if(ra == "IDLE"); 
		if(ra == "START_CLUSTERING") shouldcluster = true; 
		if(ra == "STOP_CLUSTERING") shouldcluster = false; 
		if(ra == "SEEK_GOAL") { //Manoeuver towards the last-known goal. 
			if(doflee) {
				accel(last_goal_x, last_goal_y, goal, goal_cap_fleeing); //Seek goal more weakly if being chased. 
			} else {
				accel(last_goal_x, last_goal_y, goal, goal_cap); 
			}
		}
		if(ra == "GUARD_GOAL") { //Perform goal-guarding behaviour. 
			double dist = sqrt((x - last_goal_x)*(x - last_goal_x) + (y - last_goal_y)*(y - last_goal_y)); 
			if(dist > guard_dist) {
				accel(last_goal_x, last_goal_y, guard, guard_cap); 
			} else if(dist < guard_dist) {
				accel(last_goal_x, last_goal_y, -guard, guard_cap); 
			} else {
				//If at the correct distance, accelerate to circle around the goal. 
				double perp_x = (y - last_goal_y); 
				double perp_y = -(x - last_goal_x); 
				const_accel(perp_x, perp_y, 2.0*guard, guard_cap); 
			}
		}
		if(ra == "ROTATE_CLOCKWISE") { //Direct forward velocity to its relative right. 
			double mag = sqrt(dx*dx + dy*dy); 
			double sdx = dx/mag, sdy = dy/mag; //Unit velocity vector. 
			accel(sdy, -sdx, rotate); 
		}
		if(ra == "ROTATE_COUNTERCLOCKWISE") { //Direct forward velocity to its relative left. 
			double mag = -sqrt(dx*dx + dy*dy); 
			double sdx = dx/mag, sdy = dy/mag; //Unit velocity vector. 
			accel(sdy, -sdx, rotate); 
		}
		if(ra == "FLEE_ANTIGOAL") { //Manoeuver away from the last-known antigoal. 
			if(doflee) {
				accel(last_antigoal_x, last_antigoal_y, -antigoal, antigoal_cap_fleeing); 
			} else {
				accel(last_antigoal_x, last_antigoal_y, -antigoal, antigoal_cap); 
			}
		}
		if(ra == "KILL_PREY") { //Attempt to manoeuver prey being chased towards the antigoal to kill them. 
			accel(last_antigoal_x, last_antigoal_y, goal, goal_cap); //AN: Should only be used while chasing. 
		}
		if(ra == "HEAD_OFF_PREY") {
			double midpoint_prey_goal_x = (chase_color_com_x + last_goal_x) / 2; 
			double midpoint_prey_goal_y = (chase_color_com_y + last_goal_y) / 2; 
			accel(midpoint_prey_goal_x, midpoint_prey_goal_y, goal, goal_cap); 
		}
		if(ra == "SIDESTEP") {
			accel(last_goal_y, -last_goal_x, goal, goal_cap); 
		}
		if(ra == "FLEE_PREDATORS") { //AN: This is placed here (apparantly redundantly) to allow predators to avoid other predators, if desired. 
			if(doflee) accel(flee_color_com_x, flee_color_com_y, -flee); 
		}
		/*
			Possible explanation of the anomaly goes as follows. 
			The only tweak I would have made to the code in between the old behavior and the new, betterer behavior was to add stuff
			for "SEEK_PREDATORS". It's possible that it somehow always fires, which would have flown under the radar in the past. 
			Find out if it does indeed always fire or if the issue lies elsewhere. 
			I refuse to accept that new data until we sort this out. 

			Consider printing out the rules when they are loaded in to verify they are being loaded correctly. 

			Consider running an older version of the code. 

			Verify that the anomly is not just caused by the addition of the 6th win condition. 
		*/
		if(ra == "SEEK_PREDATORS") { //AN: What we want is for the predators to stop seeking each other once their stranger tick limit is exceeded. 
			//Note: Good chance that the incrementing of ticks_... is happening once per tick per stranger predator seen. 
			if(doflee && ticks_seeing_stranger_predators < predator_stranger_tick_limit) { //If 'doflee' and we are not set to disengage. 
				double dist = ktw::distance(x, y, flee_color_com_x, flee_color_com_y); 
				if(dist > lowagent_radius*0.5) { //Keep predators slightly apart, to preserve property that they are still isolated. 
					accel(flee_color_com_x, flee_color_com_y, flee); 
				} else {
					accel(flee_color_com_x, flee_color_com_y, -flee); 
				}
			}
		}
	}
	//Execute rules encoded in "loaded_config". 
	//AN: The parsing of the RPN logic has turned out to be quite slow, can we preprocess the rules into numbers to hasten it? 
	void prioritised_rules(std::vector<agent*> a) {
		for(size_t i = 0; i < loaded_config.size(); i += 2) {
			if(loaded_config[i][0] != ' ') { //If it doesn't start with a space, it is a rule. 
				std::string tmp = loaded_config[i]; 
				//This is a low-level agent, so only execute low-level rules. 
				if(tmp.find_first_of(low_level_indicator) == 0) {
					tmp.erase(0, low_level_indicator.size()); 
				} else {
					continue; 
				}
				bool b = ktw::parse_rpn_logic(parse_rule_bools(tmp)); //AN: THIS OPERATION IS SLOW, IT IS IMPACTING PERFORMANCE GREATLY, FIX? 
				if(b) { //If this rule is found to be true, perform its actions and don't do any more rules. 
					tallies[i / 2] += 1; //Tally this rules occurance. 
					std::string w = loaded_config[i + 1]; 

					acting = w; 

					w.erase(w.begin()); 
					while(w.size() > 0) {
						size_t tmp = w.find_first_of(','); 
						parse_rule_action(w.substr(0, tmp)); //Perform the relevant action. 
						if(tmp != std::string::npos) {
							w.erase(0, tmp + 1); 
						} else {
							w = ""; 
						}
					}
					break; 
				}
			}
		}
	}
	//Evaluate rule.cfg codes for each combination of predator and prey. 
	void parse_predprey_state(std::string s) {
		if(s == "PREDATOR") {
			is_predator = true; 
			is_prey = false; 
		} else if(s == "PREY") {
			is_predator = false; 
			is_prey = true; 
		} else if(s == "PREDPREY") {
			is_predator = true; 
			is_prey = true; 
		} else if(s == "BYSTANDER") {
			is_predator = false; 
			is_prey = false; 
		}
	}

public: 
	//Constructor. 
	cluster_agent(double x0, double y0, double dx0, double dy0, double r0, std::vector<std::string> loaded_config0) : agent(x0,y0,dx0,dy0,r0,sf::Color::White) {
		//Determine who to be, who to chase, and who to flee. 
		loaded_config0[0].erase(loaded_config0[0].begin()); 
		std::vector<std::string> who = ktw::sv(loaded_config0[0], ","); 
		color = parse_color(who[0]); 
		parse_predprey_state(who[1]); 
		loaded_config0.erase(loaded_config0.begin()); 
		loaded_config = loaded_config0; 
		tallies = std::vector<unsigned>(loaded_config.size() / 2); 
	}
//State-conditionals of this agent. 
	//This kind of agent is never a goal. 
	virtual bool isgoal() { return false; }
	//This kind of agent is never an antigoal. 
	virtual bool isantigoal() { return false; }
	//This kind of agent only knows of the goal if it has been encountered. 
	virtual bool knowsgoal() { return !std::isnan(last_goal_x) && !std::isnan(last_goal_y); }
	//This kind of agent only knows of the antigoal if it has been encountered. 
	virtual bool knowsantigoal() { return !std::isnan(last_antigoal_x) && !std::isnan(last_antigoal_y); }
	//"in_goal" is computed in "rule()". 
	virtual bool ingoal() { return in_goal; }
	//If this agent is a predator, this can be true if it is pursuing prey. 
	virtual bool ispushing() { return is_seen_pred_pushing; }
	//Determine which agents get chased and which chase. 
	virtual bool isprey() { return is_prey; }
	virtual bool ispredator() { return is_predator; }
//Behaviour of this agent. 
	//Utility method to accelerate this particle towards (x,y), scaled by s and capped by c. 
	virtual void accel(double x1, double y1, double s, double c = 0.0, bool unit = false) {
		double ax = (x1 - x) * s; 
		double ay = (y1 - y) * s; 
		std::vector<double> a = {(x1 - x), (y1 - y)}; 
		if(c > 0.0) a = veccap(a, c); 
		if(unit) a = ktw::hat(a); 
		a[0] *= s; 
		a[1] *= s; 
		dx += a[0]; 
		dy += a[1]; 
	}
	virtual void const_accel(double x1, double y1, double s, double c = 0.0, bool unit = false) {
		double ax = (x1 - x) * s; 
		double ay = (y1 - y) * s; 
		std::vector<double> a = {(x1 - x), (y1 - y)}; 
		if(c > 0.0) a = veccap(a, c); 
		if(unit) a = ktw::hat(a); 
		a[0] *= s; 
		a[1] *= s; 
		const_dx += a[0]; 
		const_dy += a[1]; 
	}
	//This agent has the ability to alter its movement by a capped-sum of various "seeking velocities". 
	virtual void rule(std::vector<agent*> a) {
		pre_rules(a); //Before rules, always do the following actions. 
		prioritised_rules(a); //Execute prioritised rules. 
		post_rules(a); //After rules, always do the following actions. 
	}
//Drawing of this agent. 
	//Draw this particle's radius of influence. 
	virtual void draw_1(sf::RenderWindow* w, double cx, double cy, double s) {
		if(isclustered()) {
			draw_circle(s*x + cx, s*y + cy, s*r, sf::Color(color.r / 4, color.g / 4, color.b / 4, 64)); 
			//draw_circle(s*x + cx, s*y + cy, s*r*flee_radius_scale, sf::Color(color.r / 4, color.g / 4, color.b / 4, 170)); 
		} else {
			draw_circle(s*x + cx, s*y + cy, s*r, sf::Color(32, 32, 32, 64)); 
		}
	}
	//Draw this particle. 
	virtual void draw_2(sf::RenderWindow* w, double cx, double cy, double s) {
		if(draw_extras) {
			//draw_arrow(s*x + cx, s*y + cy, s*(x + 4.0*dx) + cx, s*(y + 4.0*dy) + cy, color); 
			if(docluster) draw_arrow(s*x + cx, s*y + cy, s*same_color_com_x + cx, s*same_color_com_y + cy, color); 
			if(dochase) draw_arrow(s*x + cx, s*y + cy, s*chase_color_com_x + cx, s*chase_color_com_y + cy, sf::Color(255,255,255)); 
			if(doflee) draw_arrow(s*x + cx, s*y + cy, s*flee_color_com_x + cx, s*flee_color_com_y + cy, sf::Color(64,64,64)); 
			if(knowsgoal()) draw_arrow(s*x + cx, s*y + cy, s*last_goal_x + cx, s*last_goal_y + cy, sf::Color(255,255,255)); 
			if(knowsantigoal()) draw_arrow(s*x + cx, s*y + cy, s*last_antigoal_x + cx, s*last_antigoal_y + cy, sf::Color(255,255,255)); 
			draw_string(acting, s*x + cx, s*y + cy, color); 
		}
		draw_circle(s*x + cx, s*y + cy, lowagent_size, color); 
	}
}; 

/*
	High-level predator and/or prey agent. 

	AN: Augment agents so that predators know when they are at the guard radius, use "knowsgoal"? 

	AUTHOR: Kyle T. Wylie
	EST: 3 July 2020
*/
class highlevel_agent : public agent {
private: 
	bool first_tick = true; //Is this agent currently on it's first tick?
	double const_dx = dx, const_dy = dy; //Preserved velocity component for each tick. 
	sf::Color chase_color, flee_color; //Colours to chase and flee. 
	double last_goal_x = NAN, last_goal_y = NAN; //Last known position of goal region. 
	double last_antigoal_x = NAN, last_antigoal_y = NAN; //Last known position of antigoal region. 
	bool in_goal = false; //Is this agent currently in the goal region. 
	std::string current_rule_state = ""; //Which prioritised rule is currently active on this agent. 
	std::vector<std::string> loaded_config; //Prioritised rules for this agent. 
	std::vector<unsigned> tallies; //Tallies of rule-occurance for this agent alone. 

	std::string acting; //Which actions is this agent currently performing. 

	//Variables relating to centres of mass of other particles. 
	double chase_color_com_x = 0.0, chase_color_com_y = 0.0; //Prey. 
	double chase_color_count = 0.0; 
	double flee_color_com_x = 0.0, flee_color_com_y = 0.0; //Predators. 
	double flee_color_count = 0.0; 

	//Misc. movement control booleans. 
	bool dochase, doflee; 
	bool approaching_goal, approaching_antigoal; //Is this agent getting closer to the goal (only calculable if position is known). 
	bool is_seen_pred_pushing; //Has this agent seen a predator who is pushing a prey? 
	double last_x, last_y; //Coordinate position last tick (for computing approaching_goal). 

	//For predators: how long has this predator been seeing predators of a different color? 
	unsigned ticks_seeing_stranger_predators = 0; 

	//Booleans which define if this agent is a predator or is prey (or both, or neither). 
	bool is_predator, is_prey; 

	//Compute average position of neighbours same, chase, and flee colors. 
	void compute_centres_of_mass(std::vector<agent*> a) {
		/*
		same_color_com_x = 0.0, same_color_com_y = 0.0; 
		same_color_count = 0.0; 
		*/
		chase_color_com_x = 0.0, chase_color_com_y = 0.0; 
		chase_color_count = 0.0; 
		flee_color_com_x = 0.0, flee_color_com_y = 0.0; 
		flee_color_count = 0.0; 
		in_goal = false; 
		is_seen_pred_pushing = false; 
		double dist; 
		bool saw_stranger_predators = false; //Were any "stranger" predators spotted this tick. 
		for(size_t i = 0; i < a.size(); i++) {
			if(a[i]->isgoal()) { //If a goal region is found, record it's position. 
				dist = sqrt((x - a[i]->getx())*(x - a[i]->getx()) + (y - a[i]->gety())*(y - a[i]->gety())); 
				if(is_prey && goalsensor_works) { //Prey only records it's own goal. 
					if(a[i]->getcolor() == color) { //This is done rather than '&&' to force all goals to be caught here. 
						last_goal_x = a[i]->getx(); //AN: Alter such that agents are in the goal if in the goal's radius. 
						last_goal_y = a[i]->gety(); 
						in_goal = dist < goal_radius; //Currently in the goal region. 
					}
				} else if(is_predator) { //Predators record the first goal they encounter. 
					last_goal_x = a[i]->getx(); 
					last_goal_y = a[i]->gety(); 
					in_goal = dist < goal_radius; //Currently in the goal region. 
				}
			} else if(a[i]->isantigoal()) { //If an antigoal region is found, record it's position. 
				if(is_prey) {
					if(a[i]->getcolor() == color) { //...iff it is the dangerous colour. 
						last_antigoal_x = a[i]->getx(); 
						last_antigoal_y = a[i]->gety(); 
					}
				} else if(is_predator) {
					last_antigoal_x = a[i]->getx(); //Predators remember all antigoals. 
					last_antigoal_y = a[i]->gety(); 
				}
			} else if(a[i]->getcolor() == color) { //Otherwise, compute centres of mass. 
				/*
				same_color_count += 1.0; 
				same_color_com_x += a[i]->getx(); 
				same_color_com_y += a[i]->gety(); 
				*/
			} else if(!a[i]->isclustered()) { //Otherwise, if not clustered, disregard. 
				continue; 
			} else if(a[i]->isprey()) {
				chase_color_count += 1.0; 
				chase_color_com_x += a[i]->getx(); 
				chase_color_com_y += a[i]->gety(); 
				if(this->ispredator()) { //If we are a predator, we have seen prey so do not disengage from friendly predators. 
					ticks_seeing_stranger_predators = 0; 
				}
			} else if(a[i]->ispredator()) {
				if(a[i]->ispushing()) is_seen_pred_pushing = true; 
				flee_color_count += 1.0; 
				flee_color_com_x += a[i]->getx(); 
				flee_color_com_y += a[i]->gety(); 
				if(this->ispredator()) { //If we are a predator, we are seeing foreign predators so count ticks having seen them. 
					saw_stranger_predators = true; 
				}
			}
		}
		if(saw_stranger_predators) { //If we saw a stranger predator, increment the number of times we've seen them. 
			ticks_seeing_stranger_predators += 1; 
		}
		//Finish average positions. 
		/*
		if(same_color_count > 0.0) {
			same_color_com_x /= same_color_count; 
			same_color_com_y /= same_color_count; 
		}
		*/
		if(chase_color_count > 0.0) {
			chase_color_com_x /= chase_color_count; 
			chase_color_com_y /= chase_color_count; 
		}
		if(flee_color_count > 0.0) {
			flee_color_com_x /= flee_color_count; 
			flee_color_com_y /= flee_color_count; 
		}

		//Update relevenat conditions. 
		//docluster = same_color_count > 0.1; //Can this agent attempt to cluster? 
		dochase = chase_color_count > 0.1; //Can this agent attempt to chase? 
		doflee = flee_color_count > 0.1; /* && dist < r*flee_radius_scale;*/ //Can this agent attempt to flee? 
	}

	//Execute these actions before prioritised rules. 
	void pre_rules(std::vector<agent*> a) {
		//Compute all relevant centres of mass. 
		compute_centres_of_mass(a); 
		//Handle ambient motion (avoiding walls / jittering). 
		if(fabs(x - grx) < wr) {
			const_dx += -wall; //Right wall. 
		} else if(fabs(x - -grx) < wr) {
			const_dx += wall; //Left wall. 
		} else if(fabs(y - gry) < wr) {
			const_dy += -wall; //Bottom wall. 
		} else if(fabs(y - -gry) < wr) {
			const_dy += wall; //Top wall. 
		} else {
			//Not near to a wall. 
			if(in_goal) { //AN: Possible solution of wandering out? 
				const_dx += rng_r_fast() * goaljitter; 
				const_dy += rng_r_fast() * goaljitter; 
			} else {
				const_dx += rng_r_fast() * jitter; 
				const_dy += rng_r_fast() * jitter; 
			}
		}
		//Determine if the goal is being approached. 
		if(knowsgoal()) {
			double goal_dist_last = sqrt((last_x - last_goal_x)*(last_x - last_goal_x) + (last_y - last_goal_y)*(last_y - last_goal_y)); 
			double goal_dist_curr = sqrt((x - last_goal_x)*(x - last_goal_x) + (y - last_goal_y)*(y - last_goal_y)); 
			approaching_goal = goal_dist_curr < goal_dist_last; 
		}
		if(knowsantigoal()) {
			double goal_dist_last = sqrt((last_x - last_antigoal_x)*(last_x - last_antigoal_x) + (last_y - last_antigoal_y)*(last_y - last_antigoal_y)); 
			double goal_dist_curr = sqrt((x - last_antigoal_x)*(x - last_antigoal_x) + (y - last_antigoal_y)*(y - last_antigoal_y)); 
			approaching_antigoal = goal_dist_curr < goal_dist_last; 
		}
		//Periodically put a stopper on the conserved velocity when in the goal to help prevent wandering out. 
		if(knowsgoal() && t % 100 == 0) {
			const_dx *= 0.9; 
			const_dy *= 0.9; 
			//std::cout << const_dx << ", " << const_dy << std::endl; 
		}
		//Record position before motion for calculation of "approaching_goal". 
		last_x = x; 
		last_y = y; 
		//Motion gets reset to the preserved value (from last tick) each tick. 
		dx = const_dx; 
		dy = const_dy; 
	}
	//Execute these actions after prioritised rules. 
	void post_rules(std::vector<agent*> a) {
		//Always chase prey, always flee from predators. 
		//AN: This in theory also implements the "pushing" necessity we talked about. 
		if(is_predator && dochase) accel(chase_color_com_x, chase_color_com_y, chase); 
		if(is_prey && doflee) accel(flee_color_com_x, flee_color_com_y, -flee); 
		//Ensure preserved velocity doesn't get too high. 
		double speed = sqrt(const_dx*const_dx + const_dy*const_dy); 
		if(speed > const_cap) {
			//Create unit vector. 
			const_dx /= speed; 
			const_dy /= speed; 
			//Multiply by speed cap. 
			const_dx *= const_cap; 
			const_dy *= const_cap; 
		}
		//Ensure total velocity doesn't get too high. 
		speed = sqrt(dx*dx + dy*dy); 
		if(speed > cap) {
			//Create unit vector. 
			dx /= speed; 
			dy /= speed; 
			//Multiply by speed cap. 
			dx *= cap; 
			dy *= cap; 
		}
	}
	//Replace named booleans with their corresponding truth-values. 
	std::string parse_rule_bools(std::string rb) {
		auto bool_to_str = [](bool b) -> std::string {
			return (b) ? "true" : "false" ;
		}; 
		//Hardcoded list of bools. 
		rb = ktw::substitute(rb, "TRUE", bool_to_str(true)); 
		rb = ktw::substitute(rb, "FALSE", bool_to_str(false)); 
		rb = ktw::substitute(rb, "KNOWS_GOAL", bool_to_str(knowsgoal())); 
		rb = ktw::substitute(rb, "PUSHING", bool_to_str(dochase)); 
		rb = ktw::substitute(rb, "APPROACHING_GOAL", bool_to_str(approaching_goal)); 
		rb = ktw::substitute(rb, "APPROACHING_ANTIGOAL", bool_to_str(approaching_antigoal)); 
		rb = ktw::substitute(rb, "KNOWS_ANTIGOAL", bool_to_str(knowsantigoal())); 
		rb = ktw::substitute(rb, "FLEEING", bool_to_str(doflee)); 
		rb = ktw::substitute(rb, "ISSEENPREDPUSHING", bool_to_str(is_seen_pred_pushing)); 
		return rb; 
	}
	//Perform the behaviour of a single named action. 
	//AN: Add actions for predators to keep prey away from goal, and for prey to try to get to the goal. 
	void parse_rule_action(std::string ra) {
		//Hardcoded list of actions. 
		if(ra == "IDLE"); 
		if(ra == "SEEK_GOAL") { //Manoeuver towards the last-known goal. 
			if(doflee) {
				accel(last_goal_x, last_goal_y, goal, goal_cap_fleeing); //Seek goal more weakly if being chased. 
			} else {
				accel(last_goal_x, last_goal_y, goal, goal_cap); 
			}
		}
		if(ra == "GUARD_GOAL") { //Perform goal-guarding behaviour. 
			double dist = sqrt((x - last_goal_x)*(x - last_goal_x) + (y - last_goal_y)*(y - last_goal_y)); 
			if(dist > guard_dist) {
				accel(last_goal_x, last_goal_y, guard, guard_cap); 
			} else if(dist < guard_dist) {
				accel(last_goal_x, last_goal_y, -guard, guard_cap); 
			} else {
				//Scaledown jitter. 
				const_dx *= goaljitter; 
				const_dy *= goaljitter; 
				//If at the correct distance, accelerate to circle around the goal. 
				if(!dochase) {
					double perp_x = (y - last_goal_y); 
					double perp_y = -(x - last_goal_x); 
					const_accel(perp_x, perp_y, guard, guard_cap); 
				}
			}
		}
		if(ra == "ROTATE_CLOCKWISE") { //Direct forward velocity to its relative right. 
			double mag = sqrt(dx*dx + dy*dy); 
			double sdx = dx/mag, sdy = dy/mag; //Unit velocity vector. 
			accel(sdy, -sdx, rotate); 
		}
		if(ra == "ROTATE_COUNTERCLOCKWISE") { //Direct forward velocity to its relative left. 
			double mag = -sqrt(dx*dx + dy*dy); 
			double sdx = dx/mag, sdy = dy/mag; //Unit velocity vector. 
			accel(sdy, -sdx, rotate); 
		}
		if(ra == "FLEE_ANTIGOAL") { //Manoeuver away from the last-known antigoal. 
			if(doflee) {
				accel(last_antigoal_x, last_antigoal_y, -antigoal, antigoal_cap_fleeing); 
			} else {
				accel(last_antigoal_x, last_antigoal_y, -antigoal, antigoal_cap); 
			}
		}
		if(ra == "KILL_PREY") { //Attempt to manoeuver prey being chased towards the antigoal to kill them. 
			accel(last_antigoal_x, last_antigoal_y, goal, goal_cap); //AN: Should only be used while chasing. 
		}
		if(ra == "HEAD_OFF_PREY") {
			double midpoint_prey_goal_x = (chase_color_com_x + last_goal_x) / 2; 
			double midpoint_prey_goal_y = (chase_color_com_y + last_goal_y) / 2; 
			accel(midpoint_prey_goal_x, midpoint_prey_goal_y, goal, goal_cap); 
		}
		if(ra == "SIDESTEP") {
			accel(last_goal_y, -last_goal_x, goal, goal_cap); 
		}
		if(ra == "FLEE_PREDATORS") { //AN: This is placed here (apparantly redundantly) to allow predators to avoid other predators, if desired. 
			if(doflee) accel(flee_color_com_x, flee_color_com_y, -flee); 
		}
		if(ra == "SEEK_PREDATORS") { //AN: What we want is for the predators to stop seeking each other once their stranger tick limit is exceeded. 
			//Note: Good chance that the incrementing of ticks_... is happening once per tick per stranger predator seen. 
			if(doflee && ticks_seeing_stranger_predators < predator_stranger_tick_limit) { //If 'doflee' and we are not set to disengage. 
				double dist = ktw::distance(x, y, flee_color_com_x, flee_color_com_y); 
				if(dist > lowagent_radius*0.5) { //Keep predators slightly apart, to preserve property that they are still isolated. 
					accel(flee_color_com_x, flee_color_com_y, flee); 
				} else {
					accel(flee_color_com_x, flee_color_com_y, -flee); 
				}
			}
		}
	}
	//Execute rules encoded in "loaded_config". 
	//AN: The parsing of the RPN logic has turned out to be quite slow, can we preprocess the rules into numbers to hasten it? 
	void prioritised_rules(std::vector<agent*> a) {
		for(size_t i = 0; i < loaded_config.size(); i += 2) {
			if(loaded_config[i][0] != ' ') { //If it doesn't start with a space, it is a rule. 
				//This is a high-level agent, so only execute high-level rules. 
				std::string tmp = loaded_config[i]; 
				if(tmp.find_first_of(high_level_indicator) == 0) {
					tmp.erase(0, high_level_indicator.size()); 
				} else {
					continue; 
				}
				bool b = ktw::parse_rpn_logic(parse_rule_bools(tmp)); //AN: THIS OPERATION IS SLOW, IT IS IMPACTING PERFORMANCE GREATLY, FIX? 
				if(b) { //If this rule is found to be true, perform its actions and don't do any more rules. 
					tallies[i / 2] += 1; //Tally this rules occurance. 
					std::string w = loaded_config[i + 1]; 

					acting = w; 

					w.erase(w.begin()); 
					while(w.size() > 0) {
						size_t tmp = w.find_first_of(','); 
						parse_rule_action(w.substr(0, tmp)); //Perform the relevant action. 
						if(tmp != std::string::npos) {
							w.erase(0, tmp + 1); 
						} else {
							w = ""; 
						}
					}
					break; 
				}
			}
		}
	}
	//Evaluate rule.cfg codes for each combination of predator and prey. 
	void parse_predprey_state(std::string s) {
		if(s == "PREDATOR") {
			is_predator = true; 
			is_prey = false; 
		} else if(s == "PREY") {
			is_predator = false; 
			is_prey = true; 
		} else if(s == "PREDPREY") {
			is_predator = true; 
			is_prey = true; 
		} else if(s == "BYSTANDER") {
			is_predator = false; 
			is_prey = false; 
		}
	}

public: 
	//Constructor. 
	highlevel_agent(double x0, double y0, double dx0, double dy0, double r0, std::vector<std::string> loaded_config0) : agent(x0,y0,dx0,dy0,r0,sf::Color::White) {
		//Determine who to be, who to chase, and who to flee. 
		loaded_config0[0].erase(loaded_config0[0].begin()); 
		std::vector<std::string> who = ktw::sv(loaded_config0[0], ","); 
		color = parse_color(who[0]); 
		parse_predprey_state(who[1]); 
		loaded_config0.erase(loaded_config0.begin()); 
		loaded_config = loaded_config0; 
		tallies = std::vector<unsigned>(loaded_config.size() / 2); 
	}
//State-conditionals of this agent. 
	//This kind of agent is never a goal. 
	virtual bool isgoal() { return false; }
	//This kind of agent is never an antigoal. 
	virtual bool isantigoal() { return false; }
	//High-level agents are always "clustered". 
	virtual bool isclustered() { return true; } //AN: Does it being a goal actually matter? 
	//This kind of agent only knows of the goal if it has been encountered. 
	virtual bool knowsgoal() { return !std::isnan(last_goal_x) && !std::isnan(last_goal_y); }
	//This kind of agent only knows of the antigoal if it has been encountered. 
	virtual bool knowsantigoal() { return !std::isnan(last_antigoal_x) && !std::isnan(last_antigoal_y); }
	//"in_goal" is computed in "rule()". 
	virtual bool ingoal() { return in_goal; }
	//If this agent is a predator, this can be true if it is pursuing prey. 
	virtual bool ispushing() { return is_seen_pred_pushing; }
	//Determine which agents get chased and which chase. 
	virtual bool isprey() { return is_prey; }
	virtual bool ispredator() { return is_predator; }
//Behaviour of this agent. 
	//Utility method to accelerate this particle towards (x,y), scaled by s and capped by c. 
	virtual void accel(double x1, double y1, double s, double c = 0.0, bool unit = false) {
		double ax = (x1 - x) * s; 
		double ay = (y1 - y) * s; 
		std::vector<double> a = {(x1 - x), (y1 - y)}; 
		if(c > 0.0) a = veccap(a, c); 
		if(unit) a = ktw::hat(a); 
		a[0] *= s; 
		a[1] *= s; 
		dx += a[0]; 
		dy += a[1]; 
	}
	virtual void const_accel(double x1, double y1, double s, double c = 0.0, bool unit = false) {
		double ax = (x1 - x) * s; 
		double ay = (y1 - y) * s; 
		std::vector<double> a = {(x1 - x), (y1 - y)}; 
		if(c > 0.0) a = veccap(a, c); 
		if(unit) a = ktw::hat(a); 
		a[0] *= s; 
		a[1] *= s; 
		const_dx += a[0]; 
		const_dy += a[1]; 
	}
	//This agent has the ability to alter its movement by a capped-sum of various "seeking velocities". 
	virtual void rule(std::vector<agent*> a) {
		pre_rules(a); //Before rules, always do the following actions. 
		prioritised_rules(a); //Execute prioritised rules. 
		post_rules(a); //After rules, always do the following actions. 
	}
//Drawing of this agent. 
	//Draw this particle's radius of influence. 
	virtual void draw_1(sf::RenderWindow* w, double cx, double cy, double s) {
		draw_circle(s*x + cx, s*y + cy, s*r, sf::Color(color.r / 4, color.g / 4, color.b / 4, 64)); 
		//draw_circle(s*x + cx, s*y + cy, s*r*flee_radius_scale, sf::Color(color.r / 4, color.g / 4, color.b / 4, 128)); 
	}
	//Draw this particle. 
	virtual void draw_2(sf::RenderWindow* w, double cx, double cy, double s) {
		if(draw_extras) {
			//draw_arrow(s*x + cx, s*y + cy, s*(x + 4.0*dx) + cx, s*(y + 4.0*dy) + cy, color); 
			if(dochase) draw_arrow(s*x + cx, s*y + cy, s*chase_color_com_x + cx, s*chase_color_com_y + cy, sf::Color(255,255,255)); 
			if(doflee) draw_arrow(s*x + cx, s*y + cy, s*flee_color_com_x + cx, s*flee_color_com_y + cy, sf::Color(64,64,64)); 
			if(knowsgoal()) draw_arrow(s*x + cx, s*y + cy, s*last_goal_x + cx, s*last_goal_y + cy, sf::Color(255,255,255)); 
			if(knowsantigoal()) draw_arrow(s*x + cx, s*y + cy, s*last_antigoal_x + cx, s*last_antigoal_y + cy, sf::Color(255,255,255)); 
			draw_string(acting, s*x + cx, s*y + cy, color); 
			draw_string(ktw::str(const_dx) + ", " + ktw::str(const_dy), s*x + cx, 20 + s*y + cy, color); 
		}
		draw_circle(s*x + cx, s*y + cy, highagent_size, color); 
	}
}; 

#endif