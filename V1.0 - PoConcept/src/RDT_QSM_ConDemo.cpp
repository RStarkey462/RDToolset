// QSM.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <string>
#include <vector>
#include <iostream>
#include <sstream>

class RDT_QSM {

	class location {
	private:
		std::string name;
		int id = 0;
		bool used = false;
	public:
		location(int iid, std::string nam) { id = iid; name = nam; }
		void SetName(std::string nam) { name = nam; }
		void SetUse(bool state) { used = state; }

		std::string GetName() { return name; }
		int GetID() { return id; }
		bool GetUsed() { return used; }
	};
	class event {
		int id = 0;
		std::vector<int> locationIds;
		std::string name;
		bool used = false;
		//Event
			//Location
			//Sublocation (specific, one-slot)
			//Sublocation 2-n
			//Alternative (tied event)
			//Is this optional?
			// 
			// Prerequisite (Prior event)
	public:
		event(int iid) { id = iid; used = false; }
		event(int iid, std::string nam) { id = iid; used = false; name = nam; }

		void ClearLocations() { locationIds.clear(); }
		void AddLocation(int location) {
			locationIds.push_back(location);
		}
		std::vector<int> GetLocations() { return locationIds; }
		std::string GetName() { return name; }
		bool GetUsed() { return used; }
		int GetID() { return id; }

		void SetUsed() { used = true; }
	};

	class pairing {
	public:
		event eventId;
		location locationId;

	};



	// Start with events with only 1 option. (no alt) (no optional)
	// Assign them to their slots.

	// Search event alternatives, remove option no longer valid.

	// Check all for conflict (CAN FAIL HERE - INFORM USER OF FIX)
	// 
	// LOOP {




	// Prioritise events with only one slot left.
	// update and repeat.
	// 
	// if all multiple slots
		// pick at random
		// SET BRANCH LOCATION.

	// If prerequisite fulfilled, add new event to eventlist.


	// if conflict found (after branch set)
		// Return to last branch, attempt new random.
			//If all randoms attempted, Return to higher branch.
				// If no higher branches, FAIL and REPORT.

	//Repeat }

	// Otherwise, a successful random puzzle has been generated.
	// Inform developer of high-traffic location subareas. (Suggest adding extra slot.)
	std::vector<event> eventList;
	std::vector<location> locationList;

	std::vector<std::vector<pairing>> successfulFind;

	int foundAnswers = 0;
	int recursionCount = 0;
	const int recursionLimit = 1000;
	std::string outMessage = "";

	char RecursiveCheck(std::vector<location> locations, std::vector<event> events, std::vector<pairing> pairlist) { return RecursiveCheck(locations, events, pairlist, 0); }
	char RecursiveCheck(std::vector<location> locations, std::vector<event> events, std::vector<pairing> pairlist, int random) {
		if (recursionCount > recursionLimit) { return 1; }
		recursionCount++;

		{
			if (locations.size() < events.size()) { return 1; }

			bool stillSpace = false;
			for (int i = 0; i < events.size(); i++) {
				if (!events.at(i).GetUsed()) { stillSpace = true; break; }
			}
			if (!stillSpace) {
				successfulFind.push_back(pairlist);
				foundAnswers++;
				return 0;
			}
		}

		for (int x = 0; x < events.size(); x++) {
			if (!events.at(x).GetUsed()) {
				std::vector<int> possible = events.at(x).GetLocations();

				for (int y = 0; y < possible.size(); y++) {

					if (!locations.at(possible.at(y)).GetUsed()) {
						//Valid location. Start recursion tree.

						std::vector<event> newEvents = events;
						newEvents.at(x).SetUsed();
						std::vector<location> newLocations = locations;
						newLocations.at(possible.at(y)).SetUse(true);
						std::vector<pairing> newPairs = pairlist;
						newPairs.push_back({ eventList.at(x),locationList.at( locations.at(possible.at(y)).GetID() ) });

						char result = RecursiveCheck(newLocations, newEvents, newPairs);
						switch (result) {
						case 0: if (!findAll) { return 0; } break; //Only one solution needed
						case 1: return 1;	//RecursionLimitReached
						}
					}

				}
			}

		}
		return 2;
		// 0 = Successful Map.
		// 1 = Hit recursion Limit
		// 2 = No valid solutions
	}


public:

	bool findAll = false;


	void AddEvent(std::string id, std::vector<int> locals) {
		//Name ID Locations
		event ev = event(eventList.size(), id);
		for (int i = 0; i < locals.size(); i++) {
			ev.AddLocation(locals.at(i));
		}
		eventList.push_back(ev);
	}

	void AddLocation(std::string id) {
		//Name ID
		location lc = location(locationList.size(), id);
		locationList.push_back(lc);
	}


	bool Generate() {

		// Initial test
		if (locationList.size() < eventList.size()) { outMessage = " Exception_QSM: InvalidLocations! Less available spawn-positions than events, consider adding more!"; return false; }
		{
			bool valid = true;
			for (int i = 0; i < eventList.size(); i++) {
				if (eventList.at(i).GetLocations().size() == 0) { valid = false; break; }
			}
			if (!valid) { outMessage = " Exception_QSM: InvalidEvents! One or more provided events had no possible spawning locations!"; return false; }
		}

		// Prepare
		recursionCount = 0;
		foundAnswers = 0;

		std::vector<pairing> pair;
		int out = RecursiveCheck(locationList, eventList, pair, 0);
		if (foundAnswers == 0) {
			switch (out) {
			case 0: outMessage = "";break;
			case 1: outMessage = " Exception_QSM: RecursionLimit reached! Either increase this limit or consider splitting the amount of events into multiple QSM requests."; break;
			case 2: outMessage = " Exception_QSM: Invalid Graph! No valid solutions found. Consider adding more possible locations."; break;
			}
			return false;	// Valid configuration found.
		}
		else { return true; }
	}






	std::string FinalStatsDetailed() {
		std::stringstream str;

		str << "QSM-Stats (Detailed)" << '\n' << '\n';
		str << "Found Solutions: " << foundAnswers;
		if (!findAll) { str << " (Short-Search)"; }
		else { str << " (Full-Search)"; }
		str << '\n';
		str << " Recursions: ";
		str << recursionCount;
		str << " / ";
		str << recursionLimit;
		str << '\n';
		str << "Generation Message: ";
		str << outMessage;
		str << '\n';
		str << '\n';
		if (foundAnswers > 0) {
			for (int i = 0; i < successfulFind.at(0).size(); i++) {
				str << "Pair " << i;
				str << '\n' << " : e-";
				str << successfulFind.at(0).at(i).eventId.GetID();
				str << " At l-" << successfulFind.at(0).at(i).locationId.GetID();
				str << '\n';
				str << "e: '" << successfulFind.at(0).at(i).eventId.GetName();
				str << "' At l: '" << successfulFind.at(0).at(i).locationId.GetName();
				str << "'" << '\n' << '\n';
			}
		}
		return str.str();
	}


};

int main() {
	RDT_QSM qsm;
	qsm.findAll = false;
	std::vector<int> locals;

	qsm.AddLocation("Location1_Werehouse");
	qsm.AddLocation("Location2_Cafe");
	qsm.AddLocation("Location3_Office");
	qsm.AddLocation("Location4_BowlingAlley");
	qsm.AddLocation("Location5_SwimPool");
	qsm.AddLocation("Location6_Library");

	locals.push_back(0);
	qsm.AddEvent("event1- Picky", locals);

	locals.clear();
	locals.push_back(1);
	locals.push_back(2);
	qsm.AddEvent("event2- Competing_A", locals);

	locals.clear();
	locals.push_back(1);
	locals.push_back(2);
	qsm.AddEvent("event3- Competing_B", locals);

	locals.clear();
	locals.push_back(0);
	locals.push_back(1);
	locals.push_back(4);
	qsm.AddEvent("event4- Selective", locals);

	locals.clear();
	locals.push_back(0);
	locals.push_back(1);
	locals.push_back(2);
	locals.push_back(3);
	locals.push_back(4);
	qsm.AddEvent("event5- Promiscuous", locals);


	qsm.Generate();
	std::cout << qsm.FinalStatsDetailed();
}