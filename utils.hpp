#ifndef UTILS_HPP
#define UTILS_HPP

//World constants.
const unsigned width = 1600; 
const unsigned height = 1200; 
const unsigned fontsize = 15; 
const double framedelay = 1.0L/60.0L; 
const double tickdelay = 0.01; 
const double fps_calc_delay = 1.0L; //Delay between sampling FPS (seconds). 

//Window variables. 
sf::Font font; //Global font. 
sf::Text text; //Global string (for convenience). 
sf::RenderWindow* mw; //Global pointer to primary window. 
unsigned long long frames_since_last = 0; //Frames since last fps-check. 
unsigned long long ticks_since_last = 0; //Ticks since last tps-check. 
unsigned long long t = 0; //Total number of ticks elapsed. 
unsigned long long f = 0; //Total number of frames elapsed. 
double fps = 0.0; //FPS calculated. 
double tps = 0.0; //TPS calculated. 

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//Draw a string at the specified coordinates with the specified colour. 
template <typename T> void draw_string(T in, double x, double y, sf::Color c, sf::RenderWindow* w = mw) {
	text.setString(ktw::str(in)); 
	text.setFillColor(c); 
	text.setPosition(x, y); 
	w->draw(text); 
}

//Draw a line between specified coordinates with specified colours. 
void draw_line(double x1, double y1, double x2, double y2, sf::Color c, sf::RenderWindow* w = mw) {
	sf::Vertex line[] = {
		sf::Vertex(sf::Vector2f(x1, y1), c), 
		sf::Vertex(sf::Vector2f(x2, y2), c), 
	}; 
	w->draw(line, 2, sf::Lines); 
}

//Draw an arrow from (x1,y1) to (x2,y2). 
void draw_arrow(double x1, double y1, double x2, double y2, sf::Color c, double h_size = 10, double h_theta = ktw::pi/6.0, sf::RenderWindow* w = mw) {
	draw_line(x1, y1, x2, y2, c, w); 
	double th = atan2(y2 - y1, x2 - x1); 
	draw_line(x2, y2, x2 - h_size*cos(-th - h_theta), y2 + h_size*sin(-th - h_theta), c, w); 
	draw_line(x2, y2, x2 - h_size*cos(-th + h_theta), y2 + h_size*sin(-th + h_theta), c, w); 
}

//Draw a cross ('+') at the specified coordinates with the specified colour. 
void draw_cross(double x, double y, double r, sf::Color c, sf::RenderWindow* w = mw) { 
	draw_line(x - r, y    , x + r, y    , c, w); 
	draw_line(x    , y - r, x    , y + r, c, w); 
}

//Draw an 'x' at the specified coordinates with the specified colour. 
void draw_x(double x, double y, double r, sf::Color c, sf::RenderWindow* w = mw) {
	draw_line(x - r, y - r, x + r, y + r, c, w); 
	draw_line(x - r, y + r, x + r, y - r, c, w); 
}

//Draw a circle at the specified coordinates. 
void draw_circle(double x, double y, double r, sf::Color fill, sf::Color line = sf::Color(0,0,0,0), size_t points = 50, sf::RenderWindow* w = mw) {
	sf::CircleShape circ; 
	circ.setPointCount(points); 
	circ.setRadius(r); 
	circ.setPosition(x - r, y - r); 
	circ.setFillColor(fill); 
	circ.setOutlineColor(line); 
	circ.setOutlineThickness(1.0f); 
	w->draw(circ); 
}

//Draw a rectangle at the specified coordinates. 
void draw_rect(double x1, double y1, double x2, double y2, sf::Color c, sf::RenderWindow* w = mw) {
	sf::RectangleShape r; 
	r.setFillColor(c); 
	r.setPosition(x1, y1); 
	r.setSize({fabsf(x2 - x1), fabsf(y2 - y1)}); 
	w->draw(r); 
}

//Plot a bar-graph of a set of data. 
//AN: Add other kinds of plot to this template. 
template <typename T> void draw_bargraph(std::vector<T> v, double x, double y, double w0, double h0, unsigned ticks, sf::Color c, sf::RenderWindow* w = mw) {
	T max_val = ktw::max_val(v); 
	sf::RectangleShape bar; 
	bar.setFillColor(c); 
	float bar_width = w0 / v.size(); 
	for(size_t i = 0; i < v.size(); i++) {
		bar.setPosition(x + bar_width*i, y); 
		bar.setSize({bar_width, (float) (-h0 * v[i] / max_val)}); 
		w->draw(bar); 
	}
	draw_string(max_val, x + w0, y - h0, c); 
	draw_string((T) 0, x + w0, y, c); 
}

//Mouse coordinate utility functions. 
int mx(sf::RenderWindow* w = mw) { return sf::Mouse::getPosition(*w).x; }
int my(sf::RenderWindow* w = mw) { return sf::Mouse::getPosition(*w).y; }

//Faster version of real number function from LLCAPRNG. 
double rng_r_fast() {
	return ((rand() % 2 == 0) ? -1 : 1) * (double) rand() / (RAND_MAX); 
}

//Generate a random colour. 
sf::Color rand_col(bool rand_transparency = false) {
	return sf::Color(rng.next<unsigned char>(), rng.next<unsigned char>(), rng.next<unsigned char>(), (rand_transparency) ? rng.next<unsigned char>() : 255); 
}

//Locate occurance of "pat" in "str", DEPRECATE. 
//AN: Needed since std::string::find seems to be innacurate sometimes, investigate and attempt to deprecate. 
size_t find(std::string str, std::string pat) {
	for(size_t i = 0; i < str.size(); i++) {
		bool correct = true; 
		for(size_t j = 0; j < pat.size() && i+j < str.size(); j++) {
			if(str[i + j] != pat[j]) correct = false; 
		}
		if(correct) return i; 
	}
	return std::string::npos; 
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
	Basic clickable button. 
	Procedure template "(std::function<void(void)>)[](void)->void{ ... }". 
	
	AUTHOR: Kyle T. Wylie
	EST: 24 March 2020
*/
class button {
private: 
//Private member fields. 
	int x, y; //Position of this button. 
	unsigned w, h; //Dimensions of this button. 
	sf::Color ci, ch, cc; //Color when idle, being hovered over, and when clicked. 
	std::string text; //Text to display on this button. 
	sf::Color ti, th, tc; //(Text) Color when idle, being hovered over, and when clicked. 
	std::function<void(void)> onclick; //Procedure upon being clicked. 
	bool justfired; //Was this button just clicked? 
	unsigned clicks; //How many times has this button been clicked? 
public: 
	//Constructor. 
	button(int ix, int iy, unsigned width, unsigned height, std::string itext, std::array<sf::Color,3> bcol, std::array<sf::Color,3> tcol, std::function<void(void)> ionclick) {
		x = ix; y = iy; w = width; h = height; text = itext; ci = bcol[0]; ch = bcol[1]; cc = bcol[2]; ti = tcol[0]; th = tcol[1]; tc = tcol[2]; onclick = ionclick; 
		justfired = false; 
		clicks = 0; 
	}
	//Draw this button. 
	void draw(sf::RenderWindow* w) {
		sf::RectangleShape brect; 
		brect.setSize(sf::Vector2f(this->w, this->h)); 
		brect.setPosition(x, y); 
		//Set rectangle's colour. 
		sf::Color bcol, tcol; 
		if(hover(w)) {
			if(sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) { //AN: RMB? 
				bcol = cc; 
				tcol = tc; 
			} else {
				bcol = ch; 
				tcol = th; 
			}
		} else {
			bcol = ci; 
			tcol = ti; 
		}
		brect.setFillColor(bcol); 
		w->draw(brect); //Draw rectangle. 
		draw_string(text, x, y, tcol, w); //Draw text. 
	}
	//Returns true if the mouse is hovering over this button. 
	bool hover(sf::RenderWindow* w) {
		return (mx(w) > x && mx(w) < x+(int)this->w) && (my(w) > y && my(w) < y+(int)this->h); 
	}
	//Perform this button's click procedure if it is clicked. 
	void click(sf::RenderWindow* w) {
		if(hover(w) && sf::Mouse::isButtonPressed(sf::Mouse::Button::Left) && !justfired) {
			onclick(); 
			justfired = true; //Ensure holding mouse doesn't fire 'onclick' more than once. 
			clicks++; 
		} else if(!sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) {
			justfired = false; 
		}
	}
	//Retrieve number of times this button has been clicked. 
	unsigned getclicks() { return clicks; }
}; 
std::vector<button> buttons; //Global list of buttons. 

#endif