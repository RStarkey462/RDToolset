// APR-DEMO.cpp : This file contains the 'main' function. Program execution begins and ends there.
//



#include <iostream>
#include <sstream>
#include <iostream>
#include <vector>


class RDT_PWG {
public:

	class SectorElement;

	///			OVERWRITE CLASSES	------------------------------------------------------------------------------------

	virtual class PWGCntl {
	public:
		int areaWidth = 1;				/// Dimensions (X) of the room. Reccomended 1-4.
		int areaLength = 1;				/// Dimensions (Y) of the room. Reccomended 1-4.
		int entryOffset = 5;			/// How far should the area-entrance be from the bottom left corner?
		int noOfRooms = 10;				/// How many rooms the algorithm should attempt to place before stopping.
		//bool preferEnds = true;			/// The Algorithm will prefer to place new rooms at corridor dead-ends.
		int ppAddedRooms = 0;			/// Post-Processing effect: Add rooms to existing corridors to fill gaps. (No extra corridors generated)

		virtual void ConsoleWrite(std::string errordesc) {
			/// <summary>
			///  Overwrite this to allow RDT-APR Library to give error messages to your engine console.
			/// </summary>
		}
		virtual int CallRandom() {	///
			/// <summary>
			///  !! Overwrite this to allow RDT-APR to get random numbers.
			/// </summary>
			/// <para>
			/// Please overwrite this function as a priority, otherwise output will be consistent.
			/// Should give a random integer 0-9. 
			/// Example: In UnityEngine - Random.Range(0,10)
			/// </para>
			return 1;
		}

		PWGCntl() { areaWidth = 5; areaLength = 5; entryOffset = 1; noOfRooms = 3; }
		std::vector<SectorElement> AllRoomsList;

	};

	virtual class SectorElement {
	public:
		int Width = 1;				/// Dimensions (X) of the room. Reccomended 1-4.
		int Length = 1;				/// Dimensions (Y) of the room. Reccomended 1-4.
		int EntryOffset = 1;		/// Distance between corner and entrance, used for setting corridor state. [-1 to disable]
		int EntryOffsetb = -1;		/// Distance between corner and entrance, used for setting corridor state. [-1 to disable]

		bool Repeatable = false;	/// Should this room be repeatable within the final sequence?
		bool Priority = false;		/// This room will be forced to spawn. (Cannot guarantee in smaller spaces)

		virtual void Place(int posx, int posy, short facing) {
			/// <summary>
			///  Overwrite this to place your assets into the world!
			/// </summary>
			/// <param name="posx"> X position of the asset </param>
			/// <param name="posy"> Y position of the asset </param>
			/// <param name="rot"> Rotation of said asset. 0-3. Placing left corner of the door. </param>

		}

	};
	virtual class CorridorElement {
	public:
		bool doorNorth = false;
		bool doorSouth = false;
		bool doorEast = false;
		bool doorWest = false;

		bool corridorNorth = false;
		bool corridorSouth = false;
		bool corridorEast = false;
		bool corridorWest = false;

		virtual void Place(int posx, int posy) {
			/// <summary>
			///  Overwrite this to place your assets into the world!
			/// </summary>
			/// <param name="posx"> X position of the asset </param>
			/// <param name="posy"> Y position of the asset </param>
			/// <param name="rot"> Rotation of said asset. 0-3. Placing left corner of the door. </param>

		}

	};


	RDT_PWG() {
		PWGCntl cntl; PWGControl = cntl;
	}


	RDT_PWG::PWGCntl PWGControl;
	int GetRandom() {
		return PWGControl.CallRandom();
	}


	class Coordinate {
	public:
		int x = 0;
		int y = 0;
		Coordinate(int X, int Y) { x = X; y = Y; }
		Coordinate() { x = 0; y = 0; }
	};

	class internalTilegrid {
	private:
		PWGCntl cntl;



		class placableSector {
		public:
			SectorElement asset;
			Coordinate local;
			short facing;

			placableSector(Coordinate xy) { local = xy; facing = 0; }
		};
		class placableCorridor {
		public:
			static CorridorElement asset;
			Coordinate local;
			bool cNorth = false;
			bool cSouth = false;
			bool cWest = false;
			bool cEast = false;

			bool dNorth = false;
			bool dSouth = false;
			bool dWest = false;
			bool dEast = false;

			placableCorridor(Coordinate xy) { local = xy; }
		};






		int dimX = 10;
		int dimY = 10;
		int entryOffset = 5;	/// How far should the area-entrance be from the bottom left corner?
		int roomCount = 10;		/// How many rooms the algorithm should attempt to place before stopping.
		//bool preferEnds = true;	/// The Algorithm will prefer to place new rooms at corridor dead-ends.

		std::vector<std::vector<char>> Grid;	///
		std::vector<SectorElement> AllRoomsList;
		CorridorElement corridorAssets;

		std::vector<placableSector> roomSpawnObject;
		std::vector<placableCorridor> corrSpawnObject;








		Coordinate TranslateOrientation(Coordinate origin, Coordinate offset, int orientation) {
			Coordinate output = origin;
			switch (orientation) {
			case 0: {
				output = { origin.x + offset.x, origin.y + offset.y };
			} break;
			case 1: {
				output = { origin.x + offset.y, origin.y - offset.x };
			} break;
			case 2: {
				output = { origin.x - offset.x, origin.y - offset.y };
			} break;
			case 3: {
				output = { origin.x - offset.y, origin.y + offset.x };
			} break;
			default: break;
			}
			return output;
		}
		bool CheckArea(Coordinate corner1, Coordinate corner2, bool corFails) {

			if (corner1.x < 0 || corner1.x > dimX || corner1.y < 0 || corner1.y > dimY) { return false; }
			if (corner2.x < 0 || corner2.x > dimX || corner2.y < 0 || corner2.y > dimY) { return false; }


			//Normalise corner (bottom left + top right.)
			if (corner1.x > corner2.x) {
				int tmp = corner1.x; corner1.x = corner2.x; corner2.x = tmp;
			}
			if (corner1.y > corner2.y) {
				int tmp = corner1.y; corner1.y = corner2.y; corner2.y = tmp;
			}

			if (corner1.x < 1 || corner1.x >= dimX) { return false; }
			if (corner1.y < 1 || corner1.y >= dimY) { return false; }
			if (corner2.x < 1 || corner2.x >= dimX) { return false; }
			if (corner2.y < 1 || corner2.y >= dimY) { return false; }

			//Check
			bool foundObstacle = false;
			bool foundCorridor = false;
			//		u - Unknown		b - Blocker/Border		c - Corridorpiece		r - Roomspawner		f - room(Filler)



			bool corridorNorth = false;
			bool corridorSouth = false;
			bool corridorEast = false;
			bool corridorWest = false;


			for (int i = corner1.x; i <= corner2.x; i++) {	// Scan area
				for (int y = corner1.y; y <= corner2.y; y++) {
					if (Grid.at(i).at(y) == 'f' || Grid.at(i).at(y) == 'r') { foundObstacle = true; return false; }
				}
			}

			//Check boundries
			for (int x = corner1.x; x < corner2.x; x++) {
				if (Grid.at(x).at(corner1.y) == 'c') { corridorSouth = true; }
				if (Grid.at(x).at(corner2.y) == 'c') { corridorNorth = true; }
			}
			for (int y = corner1.y; y < corner2.y; y++) {
				if (Grid.at(corner1.x).at(y) == 'c') { corridorEast = true; }
				if (Grid.at(corner2.x).at(y) == 'c') { corridorWest = true; }
			}

			if (corridorSouth && corridorNorth) { foundObstacle = true; return false; }
			if (corridorEast && corridorWest) { foundObstacle = true; return false; }



			if (foundObstacle) { return false; }
			if (corFails && foundCorridor) { return false; }
			return true;

		}
		void FillArea(Coordinate corner1, Coordinate corner2, char fill) {
			if (corner1.x < 0 || corner1.x > dimX) { return; }
			if (corner1.y < 0 || corner1.y > dimY) { return; }
			if (corner2.x < 0 || corner2.x > dimX) { return; }
			if (corner2.y < 0 || corner2.y > dimY) { return; }

			if (corner1.x > corner2.x) { int tmp = corner1.x; corner1.x = corner2.x; corner2.x = tmp; }
			if (corner1.y > corner2.y) { int tmp = corner1.y; corner1.y = corner2.y; corner2.y = tmp; }

			for (int ix = corner1.x; ix <= corner2.x; ix++) {	// Check North+South peri
				for (int iy = corner1.y; iy <= corner2.y; iy++) {
					Grid.at(ix).at(iy) = fill;
				}
			}
		}

		void InitialiseGrid() {
			if (entryOffset < 1 || entryOffset > dimX - 1) { entryOffset = dimX / 2; }
			if (dimX < 1) { dimX = 5; }
			if (dimY < 1) { dimY = 5; }

			//Initialize tilegrid
			std::vector<std::vector<char>> gridData;
			{
				std::vector<char> unassigned;	//Fill grid with empty data.
				for (int i = 0; i < dimY; i++) { unassigned.push_back('u'); }

				for (int i = 0; i < dimX; i++) { gridData.push_back(unassigned); }


				//Set boundries
				for (int i = 0; i < dimY; i++) { gridData.at(0).at(i) = 'b'; }
				for (int i = 0; i < dimX; i++) { gridData.at(i).at(0) = 'b'; }
				for (int i = 0; i < dimY; i++) { gridData.at(dimX - 1).at(i) = 'b'; }
				for (int i = 0; i < dimX; i++) { gridData.at(i).at(dimY - 1) = 'b'; }
			}

			//SetStarttile
			gridData.at(entryOffset).at(0) = 'c';
			// TILE TYPES	-
			//		u - Unknown		b - Blocker/Border		c - Corridorpiece		r - Roomspawner		f - room(Filler)
			Grid = gridData;

		}
		std::vector<int> SelectRoomlist() {		///OBSOLETE - Developer uploads order for better control.
			{
				//GRAB Rooms
				//Organise into priority list and non-p
				//Assemble a menu
				std::vector<int> highPRooms;
				std::vector<int> lowPRooms;


				for (int p = 0; p < AllRoomsList.size(); p++) {
					if (AllRoomsList.at(p).Priority) {
						highPRooms.push_back(p);
					}
					else { lowPRooms.push_back(p); }
				}

				if (highPRooms.size() > roomCount) { roomCount = (int)highPRooms.size(); return highPRooms; }
				// roomCount-priority = non-p leeway.
				std::vector<int> finallist;
				for (int i = 0; i < highPRooms.size(); i++) { finallist.push_back(highPRooms.at(i)); }
				for (int i = 0; i < roomCount - highPRooms.size(); i++) { finallist.push_back(lowPRooms.at(((cntl.CallRandom() + 1) * lowPRooms.size()) / 10)); }

				//SHUFFLE
				return finallist;

			}
		}

		bool GenerateGrid() {
			if (entryOffset < 1 || entryOffset > dimX - 1) { entryOffset = dimX / 2; }
			if (dimX < 1 || dimY < 1) { return false; }

			InitialiseGrid();

			std::vector<Coordinate> corPiece;
			std::vector<Coordinate> corEnds;
			corEnds.push_back({ entryOffset,1 });

			int runCount = 0;
			int placedRooms = 0;


			int failCount = 0;

			//SELECT ROOM LIST

			std::vector<int> rmls = SelectRoomlist();

			for (int r = 0; r < AllRoomsList.size(); r++) {
				if (runCount >= roomCount) { break; }
				if (failCount == 10) { break; }
				runCount++;

				SectorElement room = AllRoomsList.at(r);

				/*	STEPS -----
				 * 1) Select room
				 * 2) Find a corridorpiece to extend off.
				 * 3) Check room can be placed
				 * 4) Select an orientation.
				 * 5) Place and update map.
				 * 6) Check if conditions filled.
				 */

				 // <2 + 3>	Corridor and placement	---

				Coordinate nextTileCandidate = { -1, -1 };
				Coordinate cn = { -1,-1 };

				for (int i = 0; i < corEnds.size(); i++) {
					int index = i + ((cntl.CallRandom() * corEnds.size()) / 10) % (corEnds.size());
					bool success = false;

					//Test tile
					bool test = false;
					Coordinate cn1 = corEnds.at(i);
					Coordinate cn2 = { 0,0 };

					//Check spawn orientations
					cn2 = { (cn1.x + room.Width + 1), (cn1.y + room.Length + 1) };
					if (CheckArea(cn1, cn2, false)) { cn = cn2; nextTileCandidate = cn1; break; }
					cn2 = { (cn1.x + room.Width + 1), (cn1.y - room.Length - 1) };
					if (CheckArea(cn1, cn2, false)) { cn = cn2; nextTileCandidate = cn1; break; }
					cn2 = { (cn1.x - room.Width - 1), (cn1.y + room.Length + 1) };
					if (CheckArea(cn1, cn2, false)) { cn = cn2; nextTileCandidate = cn1; break; }
					cn2 = { (cn1.x - room.Width - 1), (cn1.y - room.Length - 1) };
					if (CheckArea(cn1, cn2, false)) { cn = cn2; nextTileCandidate = cn1; break; }


					if (success) { nextTileCandidate = corEnds.at(index); break; }
				}

				if (nextTileCandidate.x == -1) {	// Still no valid placement. Try another room.
					if (room.Priority) { failCount++; break; }
					else { continue; }
				}

				// <4 + 5>	Room placement ----

				//Prefer continuation
					// Check selected tile orientation -> mark straight path as intended
					// % Chance, dev option (preferibly high
				//Other axis = 50/50 OR 80/20 away from centre.

				{
					Coordinate cn1 = { (nextTileCandidate.x + room.Width + 1), (nextTileCandidate.y + room.Length + 1) };
					Coordinate cn2 = { (nextTileCandidate.x + room.Width + 1), (nextTileCandidate.y - room.Length - 1) };
					Coordinate cn3 = { (nextTileCandidate.x - room.Width - 1), (nextTileCandidate.y + room.Length + 1) };
					Coordinate cn4 = { (nextTileCandidate.x - room.Width - 1), (nextTileCandidate.y - room.Length - 1) };

					if (CheckArea(nextTileCandidate, cn1, false)) { cn = cn1; }
					else if (CheckArea(nextTileCandidate, cn2, false)) { cn = cn2; }
					else if (CheckArea(nextTileCandidate, cn3, false)) { cn = cn3; }
					else if (CheckArea(nextTileCandidate, cn4, false)) { cn = cn4; }
				}

				short orientation = -1;
				if (nextTileCandidate.x < cn.x) {	//West
					if (nextTileCandidate.y < cn.y) { orientation = 0; }	//North
					else { orientation = 1; }	//South
				}
				else {	//East
					if (nextTileCandidate.y < cn.y) { orientation = 2; }	//North
					else { orientation = 3; }	//Southh
				}

				// 3 Orientations | Place room: NorthEast, NorthWest, SouthEast	
				Coordinate offset = { room.Width + 1, room.Length + 1 };



				// NEW ROOM SYSTEM


				// We have other corner, and tile.
				// Tile = guarantee corridor on connecting corner
				// Bottom left = 3 possible -> ES, EN, SW

				//orientation = 0 (up right) 1 (down right) 2 (down left) 3 (up left)
				bool north = false;
				bool east = false;
				Coordinate rmcorner1 = nextTileCandidate;
				Coordinate rmcorner2 = cn;
				if (rmcorner1.x > rmcorner2.x) { int tmp = rmcorner1.x; rmcorner1.x = rmcorner2.x; rmcorner2.x = tmp; }
				if (rmcorner1.y > rmcorner2.y) { int tmp = rmcorner1.y; rmcorner1.y = rmcorner2.y; rmcorner2.y = tmp; }

				{	//Continue existing corridor pieces / without overwriting.

					bool existingNorth = false;
					bool existingSouth = false;
					bool existingEast = false;
					bool existingWest = false;
					//Check boundries
					for (int x = rmcorner1.x; x < rmcorner2.x; x++) {
						if (Grid.at(x).at(rmcorner1.y) == 'c') { existingSouth = true; }
						if (Grid.at(x).at(rmcorner2.y) == 'c') { existingNorth = true; }
					}
					for (int y = rmcorner1.y + 1; y < rmcorner2.y; y++) {
						if (Grid.at(rmcorner1.x).at(y) == 'c') { existingEast = true; }
						if (Grid.at(rmcorner2.x).at(y) == 'c') { existingWest = true; }
					}
					if (existingNorth) { north = true; }
					else if (existingSouth) { north = false; }
					else { if (cntl.CallRandom() < 5) { north++; } }

					if (existingEast) { east = true; }
					else if (existingWest) { east = false; }
					else { if (cntl.CallRandom() < 5) { east++; } }


					if (orientation == 0 && (north && east)) {
						if (existingNorth || existingSouth) { east = false; }
						else { north = false; }
					}
					if (orientation == 1 && (!north && east)) {
						if (existingNorth || existingSouth) { east = false; }
						else { north = true; }
					}
					if (orientation == 2 && (!north && !east)) {
						if (existingNorth || existingSouth) { east = true; }
						else { north = true; }
					}
					if (orientation == 3 && (north && !east)) {
						if (existingNorth || existingSouth) { east = true; }
						else { north = false; }
					}
				}


				//Orientation : 0 bottomL, 1 topL, 2 topR, 3 bottomR



				Coordinate markedRoomTile = { -1,-1 };
				FillArea(nextTileCandidate, cn, 'c');
				//std::cout << GridReadout();


				short orient = 0;
				Coordinate newTileChecker1 = { 0,0 }; Coordinate newTileChecker2 = { 0,0 }; Coordinate newTileChecker3 = { 0,0 };
				if (north && east) {
					FillArea(rmcorner1, { rmcorner2.x - 1, rmcorner2.y - 1 }, 'f');
					markedRoomTile = { rmcorner2.x - 1, rmcorner2.y - 1 };
					newTileChecker1 = { rmcorner1.x - 1, rmcorner2.y };
					newTileChecker2 = { rmcorner2.x, rmcorner1.y - 1 };
					newTileChecker3 = { rmcorner2.x, rmcorner2.y + 1 };
					orient = 2;
				}

				if (north && !east) {
					FillArea({ rmcorner1.x + 1, rmcorner1.y }, { rmcorner2.x,	   rmcorner2.y - 1 }, 'f');
					markedRoomTile = { rmcorner1.x + 1, rmcorner2.y - 1 };
					newTileChecker1 = { rmcorner1.x, rmcorner1.y - 1 };
					newTileChecker2 = { rmcorner2.x + 1, rmcorner2.y };
					newTileChecker3 = { rmcorner1.x, rmcorner2.y + 1 };
					orient = 1;
				}


				if (!north && east) {
					FillArea({ rmcorner1.x, rmcorner1.y + 1 }, { rmcorner2.x - 1, rmcorner2.y }, 'f');
					markedRoomTile = { rmcorner2.x - 1, rmcorner1.y + 1 };
					newTileChecker1 = { rmcorner2.x, rmcorner2.y + 1 };
					newTileChecker2 = { rmcorner1.x - 1, rmcorner1.y };
					newTileChecker3 = { rmcorner2.x, rmcorner1.y - 1 };
					orient = 3;
				}


				if (!north && !east) {
					FillArea({ rmcorner1.x + 1, rmcorner1.y + 1 }, { rmcorner2.x    , rmcorner2.y }, 'f');
					markedRoomTile = { rmcorner1.x + 1, rmcorner1.y + 1 };
					newTileChecker1 = { rmcorner1.x, rmcorner2.y + 1 };
					newTileChecker2 = { rmcorner2.x + 1, rmcorner1.y };
					newTileChecker3 = { rmcorner1.x, rmcorner1.y - 1 };
					orient = 0;
				}

				if (!(newTileChecker1.x < 0 || newTileChecker1.x > dimX || newTileChecker1.y < 0 || newTileChecker1.y > dimY)) {
					corEnds.push_back(newTileChecker1);
				}
				if (!(newTileChecker2.x < 0 || newTileChecker2.x > dimX || newTileChecker2.y < 0 || newTileChecker2.y > dimY)) {
					corEnds.push_back(newTileChecker2);
				}
				if (!(newTileChecker3.x < 0 || newTileChecker3.x > dimX || newTileChecker3.y < 0 || newTileChecker3.y > dimY)) {
					corEnds.push_back(newTileChecker3);
				}


				if (!(markedRoomTile.x < 0 || markedRoomTile.x > dimX || markedRoomTile.y < 0 || markedRoomTile.y > dimY)) {
					Grid.at(markedRoomTile.x).at(markedRoomTile.y) = 'r';
					placableSector rlocal = { markedRoomTile };
					rlocal.facing = orient;
					rlocal.asset = room;
					roomSpawnObject.push_back(rlocal);
				}

				std::cout << GridReadout();
			}











			//Create corridors (SpawnObjects)


			for (int x = 1; x < dimX - 1; x++) {
				for (int y = 1; y < dimY - 1; y++) {
					if (Grid.at(x).at(y) == 'c') {
						placableCorridor corrlocal = { {x,y} };
						if (Grid.at(x).at(y + 1) == 'c') { corrlocal.cNorth = true; }
						if (Grid.at(x).at(y - 1) == 'c') { corrlocal.cSouth = true; }
						if (Grid.at(x + 1).at(y) == 'c') { corrlocal.cEast = true; }
						if (Grid.at(x - 1).at(y) == 'c') { corrlocal.cWest = true; }
						corrSpawnObject.push_back(corrlocal);
					}
				}

			}
			for (int c = 1; c < roomSpawnObject.size(); c++) {
				Coordinate tmpLocal = roomSpawnObject.at(c).local;
				TranslateOrientation(tmpLocal, { roomSpawnObject.at(c).asset.EntryOffset,0 }, roomSpawnObject.at(c).facing);
				if (roomSpawnObject.at(c).asset.EntryOffsetb != -1) {
					Coordinate place1 = TranslateOrientation(tmpLocal, { roomSpawnObject.at(c).asset.EntryOffset,-1 }, roomSpawnObject.at(c).facing);
					Coordinate place2 = TranslateOrientation(tmpLocal, { -1, roomSpawnObject.at(c).asset.EntryOffsetb }, roomSpawnObject.at(c).facing);
					int relevantObject1 = -1; int relevantObject2 = -1;

					for (int i = 0; i < corrSpawnObject.size(); i++) {
						if ((place1.x == corrSpawnObject.at(i).local.x) && (place1.y == corrSpawnObject.at(i).local.y)) { relevantObject1 = i; }
						if ((place2.x == corrSpawnObject.at(i).local.x) && (place2.y == corrSpawnObject.at(i).local.y)) { relevantObject2 = i; }
					}
					if (relevantObject1 == -1) { break; }

					switch (roomSpawnObject.at(c).facing) {
					case 0:
						corrSpawnObject.at(relevantObject1).dNorth = true;
						if (relevantObject2 != -1) { corrSpawnObject.at(relevantObject2).dEast = true; }
						break;
					case 1:
						corrSpawnObject.at(relevantObject1).dEast = true;
						if (relevantObject2 != -1) { corrSpawnObject.at(relevantObject2).dEast = true; }
						break;
					case 2:
						corrSpawnObject.at(relevantObject1).dSouth = true;
						if (relevantObject2 != -1) { corrSpawnObject.at(relevantObject2).dEast = true; }
						break;
					case 3:
						corrSpawnObject.at(relevantObject1).dWest = true;
						if (relevantObject2 != -1) { corrSpawnObject.at(relevantObject2).dEast = true; }
						break;
					}

				}
				else {

				}


			}


		}



	public:

		internalTilegrid(int width, int length, PWGCntl pw) { dimX = width; dimY = length; entryOffset = 5; roomCount = 10; cntl = pw; }
		internalTilegrid(int width, int length, int noOfRooms, PWGCntl pw) { dimX = width; dimY = length; entryOffset = 5; roomCount = noOfRooms; cntl = pw; }


		void Generate() { GenerateGrid(); }
		void Clear() {
			InitialiseGrid();
		}
		void PlaceAll() {

		}

		void AddSectorE(SectorElement newelement) {
			AllRoomsList.push_back(newelement);
		}
		void SetCorriE(CorridorElement incorr) {
			corridorAssets = incorr;
		}
		std::string GridReadout() {
			std::stringstream stream;
			for (int x = 0; x < dimX; x++) {
				stream << "\n";
				for (int y = 0; y < dimY; y++) {
					switch (Grid.at(x).at(y)) {
					case 'u': stream << ". "; break;
					case 'b': stream << "# "; break;
					case 'f': stream << "+ "; break;
					case 'r': stream << "R "; break;
					case 'c': stream << "@ "; break;
					default:  stream << "? "; break;
					}
				}
			}
			return stream.str();

		}





	};













};





int main()
{
	std::cout << "Hello World!\n";


	RDT_PWG pwg;
	RDT_PWG::PWGCntl control;
	pwg.PWGControl = control;
	pwg.PWGControl.areaLength = 20;
	pwg.PWGControl.areaWidth = 20;
	pwg.PWGControl.noOfRooms = 4;

	RDT_PWG::internalTilegrid wg = { 20,20,3, control };
	wg.AddSectorE({});
	RDT_PWG::SectorElement room;
	room.Length = 4;
	room.Width = 4;
	room.Priority = true;
	room.EntryOffset = 1;

	wg.AddSectorE(room);

	room.Length = 1;
	room.Width = 1;
	room.Priority = true;
	room.EntryOffset = 0;

	wg.AddSectorE(room);
	room.Length = 2;
	room.Width = 2;
	room.Priority = true;
	room.EntryOffset = 1;

	wg.AddSectorE(room);

	wg.AddSectorE(room);
	room.Length = 5;
	room.Width = 3;
	room.Priority = true;
	room.EntryOffset = 1;

	wg.AddSectorE(room);
	room.Length = 1;
	room.Width = 1;
	room.Priority = true;
	room.EntryOffset = 1;

	wg.AddSectorE(room);
	room.Length = 6;
	room.Width = 6;
	room.Priority = true;
	room.EntryOffset = 1;

	wg.AddSectorE(room);

	wg.Generate();

	std::cout << wg.GridReadout();





}

