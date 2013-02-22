#include <unordered_map>
#include <iostream>
#include <list>
#include <queue>
#include <sstream>
#include <fstream>
#include <string>
#include <memory>
#include <functional>


namespace YodleJugglers{

	typedef struct {
		double H;
		double E;
		double P;
	} Aspect;

	typedef struct Circuit {
		const std::string Name;
		const Aspect aspect;
		Circuit(const std::string& Name, const Aspect& aspect):Name(Name),aspect(aspect){}
	} Circuit; 

	typedef struct _Juggler{
		const std::string Name;
		const Aspect aspect;
		std::vector<std::shared_ptr<Circuit> > preferences;
		std::size_t nextPref; // we are going to need the list of preferences during saving results
		_Juggler(const std::string& Name, const Aspect& aspect):Name(Name), aspect(aspect) {}
	} _Juggler;

	typedef std::shared_ptr<_Juggler> Juggler;

	struct comparator{
		// custom comparator for priority_queue
		explicit comparator(std::shared_ptr<Circuit> circuit):circuit(circuit) {}
		comparator(){}

		static double score(std::shared_ptr<Circuit> circuit, const Juggler& juggler){
			return juggler->aspect.E*circuit->aspect.E+juggler->aspect.H*circuit->aspect.H+
				juggler->aspect.P*circuit->aspect.P;
		}

		bool operator()(const Juggler& juggler1, const Juggler& juggler2){
			return score(circuit,juggler1)>score(circuit,juggler2);
		}

	private:
		std::shared_ptr<Circuit> circuit;
	};

	template <typename T,typename C,typename Comparator>
	class limited_priority_queue : public std::priority_queue<T,C,Comparator> {
	// priority_queue doesn't provide access to its comparator and underlying container
	// but they are protected members
	// Only interface is extended so no harm done
	public:
		explicit limited_priority_queue(const Comparator& C):priority_queue(C){}
		limited_priority_queue():priority_queue(){}

		const T pushOver(const T& x){
			const T t(top());
			//comp is protected member of priority_queue
			if (comp(x,t)){
				pop();
				push(x);
				return t;
			}
			return x;
		}

		// c is protected underlying container of priority_queue
		typename C::const_iterator begin(){
			return c.begin();
		}

		typename C::const_iterator end(){
			return c.end();
		}

	};

	typedef limited_priority_queue<Juggler,std::vector<Juggler>,comparator> Team;

	class JuggleReader{
		static Aspect words2Aspect(std::vector<std::string>::const_iterator first){
			// read three strings from vector
			// convert to them to numerical values
			auto convert=[&]()->double{
				std::stringstream ss((*first).substr(2,(*first).size()));
				double d;
				ss>>d;
				if (!ss) 
					throw "File has wrong format.";
				return d;
			};

			Aspect _aspect={-1,-1,-1};
			try{
				_aspect.H=convert();
				++first;
				_aspect.E=convert();
				++first;
				_aspect.P=convert();
			} catch (const char * exception) {
				throw exception;
			}
			return _aspect;
		}

	public:

		static std::vector<std::string> readFile(const std::string& fName, std::list<Juggler>& jugglers, 
			std::unordered_map<std::string,Team>& Teams){
				std::fstream file(fName);
				if (!file.is_open())
					throw "Could not open input file.";

				// read file line by line
				std::string line;

				std::vector<std::string> content;
				// for mapping jugglers preferences to circuit pointers
				std::unordered_map<std::string, std::shared_ptr<Circuit> > Circuits;

				std::string tmp;
				auto splitString=[&](const std::string& line, char delim, std::vector<std::string>& words){
					std::istringstream ss(line);
					while (std::getline(ss,tmp,delim)) 
						words.push_back(tmp);		
				};

				std::vector<std::string> words;
				while (std::getline(file, line)){
					if (line.empty()) continue;
					splitString(line,' ',words);
					if (words[0]=="C"){
						// Circuit
						// e.g. C C0 H:7 E:7 P:10
						if (words.size()<5)
							throw "Unrecognized format of Circuit record.";

						std::string CircuitName(words[1]);
						Aspect _asp=words2Aspect(words.begin()+2);
						std::shared_ptr<Circuit> circuit(new Circuit(CircuitName,_asp));
						Circuits[CircuitName]= circuit;
						comparator Comparator(circuit);
						Team team(Comparator);
						Teams[CircuitName]=team;
						words.clear();
					} else if (words[0]=="J"){
						// Juggler
						// e.g. J J0 H:3 E:9 P:2 C2,C0,C1
						if (words.size()<6)
							throw "Unrecognized format of Juggler record.";

						std::string tmplocal(words.back());
						words.pop_back();
						splitString(tmplocal,',',words); // now last word is split into several
						// words[2]="H:3"
						// words[3]="E:9"
						// words[4]="P:2"
						
						Aspect _asp=words2Aspect(words.begin()+2);
						Juggler _jug(new _Juggler(words[1],_asp));

						for (auto i=words.begin()+5;i!=words.end();++i)
							_jug->preferences.push_back(Circuits[*i]);
						_jug->nextPref=0;

						jugglers.push_back(_jug);

						words.clear();
					} else {
						throw "Unrecognized file format.";
					}
					content.push_back(line);

				}
				return content;
		}

		static void WriteFile(const std::string& fname, const std::unordered_map<std::string, Team>& Teams){
			///*One line per circuit assignment. Each line should contain the circuit name followed by the
			// juggler name, followed by that juggler�s circuits in order of preference and the match score
			// for that circuit. The line should include all jugglers matched to the circuit. 

			//The example below is a valid assignment
			//C2 J6 C2:128 C1:31 C0:188, J3 C2:120 C0:171 C1:31, J10 C0:120 C2:86 C1:21, J0 C2:83 C0:104 C1:17
			//C1 J9 C1:23 C2:86 C0:94, J8 C1:21 C0:100 C2:80, J7 C2:75 C1:20 C0:106, J1 C0:119 C2:74 C1:18
			//C0 J5 C0:161 C2:112 C1:26, J11 C0:154 C1:27 C2:108, J2 C0:128 C2:68 C1:18, J4 C0:122 C2:106 C1:23*/

			std::ofstream file(fname);
			if (!file.is_open())
				throw "Could not open output file.";

			auto printJuggler=[&](const Juggler& juggler){
				// print juggler name, followed by that juggler�s circuits in order 
				// of preference and the match score for that circuit.
				file<<" "<<juggler->Name;
				if (!juggler->preferences.empty()){
					for (std::size_t i = 0; i < juggler->preferences.size(); ++i){
						auto circuit=juggler->preferences[i];
						file<<" "<<circuit->Name<<":"<<comparator::score(circuit,juggler);
					}
				}
			};

			for (auto teamRecord : Teams){
				//team.first - name of circuit
				std::string circuitName(teamRecord.first);
				file<<circuitName;
				Team& team(teamRecord.second);
				if (!team.empty()){
					printJuggler(*(team.begin()));
					auto nxtJuggler(team.begin()+1);
					while (nxtJuggler!=team.end()){
						file<<",";
						printJuggler(*(nxtJuggler++));
					}
				}
				file<<"\n";
			}

		}

	};


	class Solution{

	public:

		Solution():Jugglers(), Teams(), nonFull() {}

		void ReadFile(const std::string& fname){
			JuggleReader::readFile(fname,Jugglers,Teams);
			for (auto t:Teams)
				nonFull.push_front(t.first);
			if (Teams.empty())
				throw "File doesn't list any circuits.";
			fullTeamSize=Jugglers.size()/Teams.size();
		}

		//std::vector<std::shared_ptr<Circuit> > Circuits;

		std::vector<std::string> TeamMembers(const std::string& CircuitName){
			// Utility method to return row of the assignment table
			// Used to output team corresponding to circuit C1970 

			Team& team(Teams[CircuitName]);
			std::vector<std::string> _teamMembers;
			for (auto juggler : team){
				_teamMembers.push_back(juggler->Name);
			}
			return _teamMembers;
		}

		void Assign(){
			// Assign jugglers to circuits
			while (!Jugglers.empty()){
				Juggler juggler(Jugglers.front());
				//remove juggler from the list
				//we'll put him back if he can't get into his current favourite team
				Jugglers.pop_front(); 

				if (juggler->nextPref==juggler->preferences.size()){
					// Can happen if juggler's preferences list does not include all the teams

					// we tried to put juggler to every team in his preference list
					// put him on first team in non-full list

					// unlikely scenario, so just linear search for next non full team
					while (!nonFull.empty())
						if (Teams[nonFull.front()].size()>=fullTeamSize)
							nonFull.pop_front();
						else
							break;
					if (nonFull.empty())
						throw "ERROR::Unassigned juggler, Code Bug!";

					Team& team(Teams[nonFull.front()]);
					team.push(juggler);
				} else {
					//	try to assign juggler to next highest priority circuit
					std::shared_ptr<Circuit> topPriorityCircuit(juggler->preferences[juggler->nextPref++]);
					Team& team(Teams[topPriorityCircuit->Name]);

					if (team.size()<fullTeamSize) {
						//	if there are spots on the corresponding team, put him in the team 		
						team.push(juggler);
					} else {
						//	if team is full, compare him with the worst juggler on the team
						//  if he is better fit for the team then worst juggler currently on a team,
						//  replace
						//  Whichever juggler ends up out of the team goes back into list
						Jugglers.push_front(team.pushOver(juggler));			
					}

				}
			}
		}

		void Save(const std::string& fname){
			JuggleReader::WriteFile(fname,Teams);
		}

	private:

		std::list<Juggler> Jugglers;
		std::unordered_map<std::string,Team> Teams;
		std::list<std::string> nonFull;
		std::size_t fullTeamSize;

	};

}


int main(int argc, char * args[]){
	std::string fname;
	if (argc>1){
		fname=args[1];
	} else {
		fname="jugglefest.txt";
	}

	std::string saveTo;
	if (argc>2){
		saveTo=args[2];
	} else {
		std::istringstream fss(fname);
		std::string fname_stripped;
		std::getline(fss,fname_stripped,'.');
		saveTo=fname_stripped+"_solution.txt";
	}

	YodleJugglers::Solution solution;
	try{
		solution.ReadFile(fname);
		solution.Assign();
		solution.Save(saveTo);
	} catch (const char * exception) {
		std::cerr << exception << std::endl; 
		return 1;
	}

	std::string TeamName("C1970");
	auto teamMembers=solution.TeamMembers(TeamName);

	if (!teamMembers.empty()){
		int Sum(0);
		std::cout<<"Team "<<TeamName<<" : ";
		std::for_each(teamMembers.begin(),teamMembers.end(),[&](std::string jName){
			std::istringstream ID(jName.substr(1,jName.length()));
			std::cout<<jName<<" ";
			int _num;
			ID>>_num;
			Sum+=_num;
		});
		std::cout<<"\n";
		std::cout<<"Sum of names is "<<Sum<<std::endl;
	} 

	return 0;
}
