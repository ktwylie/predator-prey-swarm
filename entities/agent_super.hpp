#ifndef AGENT_SUPER_HPP
#define AGENT_SUPER_HPP

/*
	Generic agent for the summer swarm research project (2020) with Dr. Palmer. 

	AUTHOR: Kyle T. Wylie
	EST: 15 May 2020
*/
class agent {
protected: 
//Private member fields. 
	double x, y; //Position. 
	double dx, dy; //Velocity. 
	sf::Color color; //Color of this agent. 
	double r; //Radius of effect. 
	bool is_clustered; //Is this agent currently in a cluster? 
	bool flagged_for_removal = false; //Is this agent queued to be deleted? 
//Private member functions. 
	//Returns the sign of x. 
	int sign(int x) { return (x < 0) ? -1 : 1; }
	//Get agents within the radius of this one and determine if this agent is currently clustered. 
	//AN: Should the construction of this method change based on the shape of the sensing region? 
	std::vector<agent*> nbors_in_radius(std::vector<agent*> nbors, size_t self_i) {
		std::vector<agent*> v; 
		unsigned same_count = 0; 
		double dist; 
		for(size_t i = 0; i < nbors.size(); i++) {
			if(i == self_i) continue; 
			std::vector<double> mp = {x,y}, tp = {nbors[i]->getx(),nbors[i]->gety()}; 
			dist = ktw::distance(mp, tp); 
			bool is_any_goal = nbors[i]->isgoal() || nbors[i]->isantigoal(); 
			if(is_any_goal) {
				if(dist < nbors[i]->getr() + r) {
					v.push_back(nbors[i]); 
				}
			} else {
				 if(dist < r) {
				 	v.push_back(nbors[i]); 
					if(color == nbors[i]->getcolor() && !nbors[i]->isgoal() && !nbors[i]->isantigoal()) same_count++; 
				 }
			}
		}
		is_clustered = same_count >= clust_density; //Record if this agent is currently clustered.  
		return v; 
	}
public: 
//Constructors & destructors. 
	agent(double x0, double y0, double dx0, double dy0, double r0, sf::Color color0) {
		x = x0; y = y0; dx = dx0; dy = dy0; color = color0; r = r0; 
	}
//Methods. 
	double getx() { return x; }
	double gety() { return y; }
	double getr() { return r; }
	sf::Color getcolor() { return color; }

	//Returns true if and only if the subclass is a goal/antigoal agent. 
	virtual bool isgoal() = 0; 
	virtual bool isantigoal() = 0; 

	//Returns true if and only if this agent is currently clustered. 
	virtual bool isclustered() { return is_clustered && !isgoal(); } //AN: Does it being a goal actually matter? 

	//Returns true if and only if the subclass knows the location of the goal. 
	virtual bool knowsgoal() = 0; 

	//Returns true if and only if the subclass knows the location of the antigoal. 
	virtual bool knowsantigoal() = 0; 

	//Returns true if and only if the subclsss is in the vicinity of the goal. 
	virtual bool ingoal() = 0; 

	//Returns true if and only if the subclass is a predator who is pushing a prey. 
	virtual bool ispushing() = 0; 

	//Returns true if and only if the subclass falls into the respective category. 
	virtual bool isprey() = 0; 
	virtual bool ispredator() = 0; 

	//Utility methods to accelerate this particle towards (x,y), scaled by s and capped by c. 
	virtual void accel(double x1, double y1, double s, double c = 0.0, bool unit = false) {}
	virtual void const_accel(double x1, double y1, double s, double c = 0.0, bool unit = false) {}

	//Returns true if and only if the agent is flagged for removal. 
	bool isflaggedforremoval() { return flagged_for_removal; }
	void flagforremoval() { flagged_for_removal = true; }

	//Abstract method that dictates behaviour of this agent based on neighbours. 
	virtual void rule(std::vector<agent*> a) = 0; 

	//Apply physics to this particle. 
	void tick(std::vector<agent*> nbors, size_t self_i) {
		rule(nbors_in_radius(nbors, self_i)); 
	}
	//Move this particle at the conclusion of each tick. 
	void move() {
		x += dx; 
		y += dy; 
		if(x < -grx) x = -grx; 
		if(x > grx) x = grx; 
		if(y < -gry) y = -gry; 
		if(y > gry) y = gry; 
	}
	//Draw this particle's radius of influence. 
	virtual void draw_1(sf::RenderWindow* w, double cx, double cy, double s) = 0; 
	//Draw this particle. 
	virtual void draw_2(sf::RenderWindow* w, double cx, double cy, double s) = 0; 
}; 

#endif