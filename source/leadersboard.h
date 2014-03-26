
#include <string>
#include <vector>

#define LEADERSBOARD_NUM 10

class LeadersboardScore {
 public:
  LeadersboardScore(std::string _name, int _level, int _sec, bool _my);
  std::string name;
  int pos;
  int level;
  int sec;
  bool my;
};

class Leadersboard {
 public:
	// return the instance
	static Leadersboard* getInstance();
	
	// constructor and destructor
	Leadersboard();
	~Leadersboard();
	static void shutdown();

	//leadersboard list
	int updateCurrentScore(); // (PP) check for position
	LeadersboardScore getMyLeadersboardScore(); // (PP) get current position
	LeadersboardScore getLeadersboardScore(int p); // (PP) get p position
	void resetLeadersboard();

	// load
	void load();
	// update
	void update();
	// if available on this platform
	bool isAvailable();

 private:
	bool _leadersboard;
 private:
	static Leadersboard* instance;
};
