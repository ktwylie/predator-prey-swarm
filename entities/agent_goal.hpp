#ifndef AGENT_GOAL_HPP
#define AGENT_GOAL_HPP

/*
	Utility "agent" that does not move, but defines a goal region for one colour of cluster_agents. 
	This is a bit of hack to get goal regions, but it's not totally awful. 

	AUTHOR: Kyle T. Wylie
	EST: 27 May 2020
*/
class goal_agent : public agent {
public: 
	//Constructor. 
	goal_agent(double x0, double y0, double r0, sf::Color color0) : agent(x0,y0,0,0,r0,color0) {}
	//This utility-agent is always the goal. 
	virtual bool isgoal() { return true; }
	//...and never the antigoal. 
	virtual bool isantigoal() { return false; }
	//Since this agent is the goal, it trivially knows of the goal. 
	virtual bool knowsgoal() { return false; }
	//Since this agent is the goal, it trivially knows of the antigoal. 
	virtual bool knowsantigoal() { return false; }
	//Since this agent is the goal, it is trivially in the goal. 
	virtual bool ingoal() { return true; }
	//Since this agent is a goal, it cannot be pushing. 
	virtual bool ispushing() { return false; }
	//Goals are neither predators nor prey. 
	virtual bool isprey() { return false; }
	virtual bool ispredator() { return false; }
	//This agent has no behavior to speak of. 
	virtual void rule(std::vector<agent*> a) {}
	//Draw this particle's radius of influence. 
	virtual void draw_1(sf::RenderWindow* w, double cx, double cy, double s) {
		draw_circle(s*x + cx, s*y + cy, s*r, /*sf::Color(color.r / 2, color.g / 2, color.b / 2, 32)*/sf::Color(0,0,0,0), color); 
	}
	//Draw this particle. 
	virtual void draw_2(sf::RenderWindow* w, double cx, double cy, double s) {
		//draw_circle(s*x + cx, s*y + cy, s/2.0, color); 
	}
}; 

/*
	Utility "agent" that does not move, but defines an antigoal region for one colour of cluster_agents. 
	AN: NEED TO FULLY IMPLEMENT, JUST A PLACEHOLDER; MAY NEED TO MODIFY 'AGENT' WITH MORE VIRTUALS. 

	AUTHOR: Kyle T. Wylie
	EST: 23 June 2020
*/
class antigoal_agent : public agent {
public: 
	//Constructor. 
	antigoal_agent(double x0, double y0, double r0, sf::Color color0) : agent(x0,y0,0,0,r0,color0) {}
	//This utility-agent is never the goal. 
	virtual bool isgoal() { return false; }
	//...and always the antigoal. 
	virtual bool isantigoal() { return true; }
	//Since this agent is the goal, it trivially knows of the goal. 
	virtual bool knowsgoal() { return false; }
	//Since this agent is the goal, it trivially knows of the antigoal. 
	virtual bool knowsantigoal() { return true; }
	//Since this agent is the goal, it is trivially in the goal. 
	virtual bool ingoal() { return true; }
	//Since this agent is a goal, it cannot be pushing. 
	virtual bool ispushing() { return false; }
	//Goals are neither predators nor prey. 
	virtual bool isprey() { return false; }
	virtual bool ispredator() { return false; }
	//This agent, as an antigoal, deletes prey that comes within range of it. 
	virtual void rule(std::vector<agent*> a) {
		for(size_t i = 0; i < a.size(); i++) if(a[i]->isprey() && a[i]->getcolor() == color) a[i]->flagforremoval(); 
	}
	//Draw this particle's radius of influence. 
	virtual void draw_1(sf::RenderWindow* w, double cx, double cy, double s) {
		draw_circle(s*x + cx, s*y + cy, s*r, sf::Color(color.r / 2, color.g / 2, color.b / 2, 64), sf::Color(0,0,0,0)); 
	}
	//Draw this particle. 
	virtual void draw_2(sf::RenderWindow* w, double cx, double cy, double s) {
		//draw_circle(s*x + cx, s*y + cy, s/2.0, color); 
	}
}; 

#endif