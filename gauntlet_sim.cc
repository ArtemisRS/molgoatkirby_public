#include <iostream>
#include <fstream>
#include <iomanip>
#include <math.h>
#include <time.h>
#include <stdlib.h>
#include <string>
#include <algorithm>
using namespace std;
#include <random>

bool splash = false;
static bool corrupt = true;

int counts = 0;
random_device rd;
//mt19937 gen(rd());
mt19937 gen(time(NULL));
uniform_int_distribution<int> brimDist(1,100);

int getRandom(int from, int to) {
	uniform_int_distribution<int> distribution(from,to);
	return distribution(gen);
}

int calcAD(int equip, int stat, double boost) {
	return (int)((equip+64)*(stat+8)*boost);
}


int hit(int atk, int def, int max) {
	double atkRoll = getRandom(0,atk);
	double defRoll = getRandom(0,def);
	if ( atkRoll > defRoll ) {
		splash = false;
		return getRandom(0,max);
	} else {
		splash = true;
		return 0;
	}
}

struct gear {
	//range
	int accCbow1 = 0; int maxCbow1 = 0;
	int accCbow2 = 0; int maxCbow2 = 0;
	int accCbow3 = 0; int maxCbow3 = 0;

	//melee
	int accPunchNaked   = 0; int maxPunchNaked   = 0;
	int accPunchArmour    = 0; int maxPunchArmour    = 0;
	int accScepter    = 0; int maxScepter    = 0;
	int accChally1 = 0; int maxChally1 = 0;
	int accChally2 = 0; int maxChally2 = 0;
	int accChally3 = 0; int maxChally3 = 0;

	//mage
	int accCstaff1 = 0; int maxCstaff1 = 0;
	int accCstaff2 = 0; int maxCstaff2 = 0;
	int accCstaff3 = 0; int maxCstaff3 = 0;

	int hp = 99;
	int food = 7;
	bool alive = true;
	int redempsLeft = 3; // 1 for regular, 4 for corrupted
	int redempThresh = 13; // if above this hp, go for redemp, if below tick eat
	int hitDelay = 1;

	int auguryDef;
	int rigourDef;
	int curMageDef;
	int curRangeDef;
	int incomingHitDelay;
	int incomingDmg;

	int magicACC = 0; int magicMAX = 0;
	int meleeACC = 0; int meleeMAX = 0;
	int rangeACC = 0; int rangeMAX = 0;

	gear() {
		if (corrupt) {
			redempsLeft = 3;
			redempThresh = 13;
			auguryDef = calcAD(166,99*1.25,1);
                        //TODO: this appears to be broken
			rigourDef = calcAD(166,99*(1.25*0.3 +0.7),1);
		} else 	{
			redempsLeft = 1;
			redempThresh = 12;
			auguryDef = calcAD(0,99*1.25,1);
                        //TODO: this appears to be broken
			rigourDef = calcAD(0,99*(1.25*0.3 +0.7),1);
		}
                //TODO: this appears very broken
		curMageDef = rigourDef; curMageDef = auguryDef;
		curRangeDef = auguryDef;
		incomingHitDelay = -1;
		incomingDmg = 0;
	}

	void init(int a,int a2,int b,int b2,int c,int c2, int d, int d2, int e, int e2) {	
		accPunchNaked = c; maxPunchNaked = c2;
	}

	// resetStats, food, probably uneccessary
	void resetStats(int ifood) {
		hp = 99;
		food = ifood;
		alive = true;
		if (corrupt) redempsLeft = 3; else redempsLeft = 1;
		hitDelay = 1;
		incomingHitDelay = -1;
		incomingDmg = 0;
	}

	void updateDelay() {
		hitDelay--;
		incomingHitDelay--;
	}

	void addHit(int dmg, int projectileDelay) {
		incomingHitDelay = 1;
		if (dmg < hp) incomingDmg = dmg;
		else incomingDmg = hp;
	}

	void handleHealing(bool nados) {
		if (incomingHitDelay == 0) {
			if (hp <= redempThresh && food > 0) {
				food--;
				hp += 20;
				hitDelay += 3;
				hp -= incomingDmg;
			} else {
				hp -= incomingDmg;
				if (hp > 0 && hp < 10 && redempsLeft > 0) {
					hp += 24;
					redempsLeft--;
				}
			}
			if (hp < 1) alive = false;
		}
	}

	void setWeapons(int magicWep, int rangeWep, int meleeWep) {
		switch (magicWep) {
			case 1: magicACC = accCstaff1; magicMAX = maxCstaff1; break;
			case 2: magicACC = accCstaff2; magicMAX = maxCstaff2; break;
			case 3: magicACC = accCstaff3; magicMAX = maxCstaff3; break;
			default: throw "wtf unknown weapon selected";
		}
		switch (rangeWep) {
			case 1: rangeACC = accCbow1; rangeMAX = maxCbow1; break;
			case 2: rangeACC = accCbow2; rangeMAX = maxCbow2; break;
			case 3: rangeACC = accCbow3; rangeMAX = maxCbow3; break;
			default: throw "wtf unknown weapon selected";
		}
		switch (meleeWep) {
			case 1: meleeACC = accChally1; meleeMAX = maxChally1; break;
			case 2: meleeACC = accChally2; meleeMAX = maxChally2; break;
			case 3: meleeACC = accChally3; meleeMAX = maxChally3; break;
			case 4: meleeACC = accPunchNaked; meleeMAX = maxPunchNaked; break;
			case 5: meleeACC = accPunchArmour; meleeMAX = maxPunchArmour; break;
			case 6: meleeACC = accScepter; meleeMAX = maxScepter; break;
			default: throw "wtf unknown weapon selected";
		}

	}


};

struct npcBoss {
	//comment out unused stats to avoid calculations

	//stats
	int baseHp = 600;	int curHp  = baseHp; //600 for regular, 1000 for corrupted
	int baseAtk = 240;	int curAtk = baseAtk;
	int baseStr = 240;	int curStr = baseStr;
	int baseDef = 240;	int curDef = baseDef;
	int baseMag = 240;	int curMag = baseMag;
	int baseRng = 240;	int curRng = baseRng;

	//equipment offence bonuses
	int equipMeleeAtk = 76; //-76 regular, 90 corrupted
	int equipMagicAtk = 76;
	int equipRangeAtk = 76;

	//equipment defence bonuses
	int equipStabDef  = 20;
	int equipSlashDef = 20;
	int equipCrushDef = 20;
	int equipMagicDef = 20;
	int equipRangeDef = 20;

	//accuracy roll calcs
	int meleeAtk = calcAD(equipMeleeAtk,curAtk+1,1);
	int magicAtk = calcAD(equipMagicAtk,curMag+1,1);
	int rangeAtk = calcAD(equipRangeAtk,curRng+1,1);

	//defence roll calcs
	int stabDef  = calcAD(equipStabDef,curDef+1,1);
	int slashDef = calcAD(equipSlashDef,curDef+1,1);
	int crushDef = calcAD(equipCrushDef,curDef+1,1);
	int magicDef = calcAD(equipMagicDef,curMag+1,1);
	int rangeDef = calcAD(equipRangeDef,curDef+1,1);

	//max hit
	// 13 corrupted, 12 regular
	int bossMax = 12;

	//tracking variables	
	int lastHit = 0;
	int hitCount = 0;
	int hitDelay = 3;
	int ticks = 0;
	int rubyThreshold = 250;
	bool redempNado = true;
	bool redemp = true;
	bool nado = false;
	int nadoDelay = 6*5; //seems to be 4-8 usually, but seen as high as 10
	bool playerAlive = true;
	int prayerStart = 0;
	string prayer = "test";
	int prayerCount = 0;

	npcBoss() {
		if (corrupt) {
			baseHp = 1000; curHp = baseHp;
			equipMeleeAtk = 90;
			equipMagicAtk = 90;
			equipRangeAtk = 90;
			meleeAtk = calcAD(equipMeleeAtk,curAtk+1,1);
			magicAtk = calcAD(equipMagicAtk,curMag+1,1);
			rangeAtk = calcAD(equipRangeAtk,curRng+1,1);
			bossMax = 13;
		}
	}

	void hitBoss(int accuracy, int max, string style) {
		int defenceValue = 0;
		if (style == "stab") defenceValue = stabDef;
		else if(style == "slash") defenceValue = slashDef;
		else if(style == "crush") defenceValue = crushDef;
		else if(style == "magic") defenceValue = magicDef;
		else if(style == "range") defenceValue = rangeDef;
		else if(style == "melee") defenceValue = crushDef; // assuming all are the same
		else throw "Unknown style for hitBoss";
                int ahit = hit(accuracy, defenceValue, max);
                //cout << "  <pla> " << style << " atk: " << ahit << endl;
		curHp -= ahit;
	}

	void randomizePrayer() {
		prayerStart = getRandom(1,3);
		if (prayerStart == 1) prayer = "magic";
		else if (prayerStart == 2) prayer = "range";
		else prayer = "melee";
	}

	void updateDelay(){
		hitDelay--;
		nadoDelay--;
		if (nadoDelay == 7*5) nado = false;
                //cout << "  [hun] hd: " << hitDelay << ", nd: " << nadoDelay << endl;
	}

	void hitPlayer(gear& myGear) {
		if (nadoDelay <= 0) {
			nado = true;
			nadoDelay = 12*5;
                        //cout << "  [hun] sending nado" << endl;
		} else if (hitCount%8 < 4) {
			myGear.addHit(hit(rangeAtk,myGear.curRangeDef,bossMax),1);
                        //cout << "  [hun] range hit: " << myGear.incomingDmg << endl;
			//hitDelay = 4;
		} else {
			myGear.addHit(hit(magicAtk,myGear.curMageDef,bossMax),1);
                        //cout << "  [hun] mage hit: " << myGear.incomingDmg << endl;
			//hitDelay = 4;
		}
		hitCount++;
	}

	//range is the weak weapon
	void handleAttacking5to1range(gear& myGear) {
		//player attack
		if (myGear.hitDelay == 0) {
			myGear.hitDelay = 4;
			prayerCount++;
			if (prayer == "magic") {
				if (prayerCount == 6) {
					hitBoss(myGear.rangeACC, myGear.rangeMAX, "range");
					prayer = "range";
					prayerCount = 0;
				} else {
					hitBoss(myGear.rangeACC, myGear.rangeMAX,"range");
				}
			} else if (prayer == "range") {
				if (prayerCount == 6) {
					hitBoss(myGear.meleeACC, myGear.meleeMAX, "melee");
					prayer = "melee";
					prayerCount = 0;
				} else {
					hitBoss(myGear.magicACC, myGear.magicMAX,"magic");
				}
			} else if (prayer == "melee") {
				if (prayerCount == 6) {
					hitBoss(myGear.rangeACC, myGear.rangeMAX, "range");
					prayer = "range";
					prayerCount = 0;
				} else {
					hitBoss(myGear.magicACC, myGear.magicMAX,"magic");
				}
			}
		}

		//boss attack
		if (hitDelay == 0) {
			hitDelay = 5;
			hitPlayer(myGear);
		}

	}

	//mage is the weak weapon
	void handleAttacking5to1mage(gear& myGear) {
		//player attack
		if (myGear.hitDelay == 0) {
			myGear.hitDelay = 4;
			prayerCount++;
			if (prayer == "magic") {
				if (prayerCount == 6) {
					hitBoss(myGear.meleeACC, myGear.meleeMAX, "melee");
					prayer = "melee";
					prayerCount = 0;
				} else {
					hitBoss(myGear.rangeACC, myGear.rangeMAX,"range");
				}
			} else if (prayer == "range") {
				if (prayerCount == 6) {
					hitBoss(myGear.magicACC, myGear.magicMAX, "magic");
					prayer = "magic";
					prayerCount = 0;
				} else {
					hitBoss(myGear.magicACC, myGear.magicMAX,"magic");
				}
			} else if (prayer == "melee") {
				if (prayerCount == 6) {
					hitBoss(myGear.magicACC, myGear.magicMAX, "magic");
					prayer = "magic";
					prayerCount = 0;
				} else {
					hitBoss(myGear.rangeACC, myGear.rangeMAX,"range");
				}
			}
		}

		//boss attack
		if (hitDelay == 0) {
			hitDelay = 5;
			hitPlayer(myGear);
		}

	}

	void handleAttackingt2t3staffhally(gear& myGear) {
		//player attack
		if (myGear.hitDelay == 0) {
			myGear.hitDelay = 4;
			prayerCount++;
			if (prayer == "magic") {
				if (prayerCount == 6) {
					hitBoss(myGear.meleeACC, myGear.meleeMAX, "melee");
					prayer = "melee";
					prayerCount = 0;
				} else {
					hitBoss(myGear.meleeACC, myGear.meleeMAX,"melee");
				}
			} else if (prayer == "range") {
				if (prayerCount == 6) {
					hitBoss(myGear.magicACC, myGear.magicMAX, "magic");
					prayer = "magic";
					prayerCount = 0;
				} else {
					hitBoss(myGear.meleeACC, myGear.meleeMAX,"melee");
				}
			} else if (prayer == "melee") {
				if (prayerCount == 6) {
					hitBoss(myGear.magicACC, myGear.magicMAX, "magic");
					prayer = "magic";
					prayerCount = 0;
				} else {
					hitBoss(myGear.magicACC, myGear.magicMAX,"magic");
				}
			}
		}

		//boss attack
		if (hitDelay == 0) {
			hitDelay = 5;
			hitPlayer(myGear);
		}

	}

	void handleAttackingdoublet3staffbow(gear& myGear) {
		//player attack
		if (myGear.hitDelay == 0) {
			myGear.hitDelay = 4;
			prayerCount++;
			if (prayer == "magic") {
				if (prayerCount == 6) {
					hitBoss(myGear.rangeACC, myGear.rangeMAX, "range");
					prayer = "range";
					prayerCount = 0;
				} else {
					hitBoss(myGear.rangeACC, myGear.rangeMAX,"range");
				}
			} else if (prayer == "range") {
				if (prayerCount == 6) {
					hitBoss(myGear.magicACC, myGear.magicMAX, "magic");
					prayer = "magic";
					prayerCount = 0;
				} else {
					hitBoss(myGear.magicACC, myGear.magicMAX,"magic");
				}
			} else if (prayer == "melee") {
				if (prayerCount == 6) {
					hitBoss(myGear.magicACC, myGear.magicMAX, "magic");
					prayer = "magic";
					prayerCount = 0;
				} else {
					hitBoss(myGear.rangeACC, myGear.rangeMAX,"range");
				}
			}
		}

		//boss attack
		if (hitDelay == 0) {
			hitDelay = 5;
			hitPlayer(myGear);
		}

	}

	void chooseMethod(gear& myGear, string method) {
		if (method == "5to1range") handleAttacking5to1range(myGear);
		else if (method == "5to1mage") handleAttacking5to1mage(myGear);
		else if (method == "t2t3staffhally") handleAttackingt2t3staffhally(myGear);
		else if (method == "doublet3staffbow") handleAttackingdoublet3staffbow(myGear);
		else throw "wrong method chosen";
	}



};

npcBoss fightBoss(gear myGear, int tickLoss, string method){
	// boss starts on tick 3, I attack on tick 1. 
	// if boss in corner, I start tyick 1, boss starts tick 7
	npcBoss boss;
	boss.randomizePrayer();
	myGear.hitDelay += tickLoss;
	//boss.ticks = 787 enter, 1090 hitsplat ko = 303 ticks, but 307 ticks on vod, so 4 tick death animation
	//add 4 ticks at end then, and fire initial hit on tick 1

	if (method == "5to1range") {
		while (boss.curHp > 0 && myGear.alive) {
			boss.ticks++;
			boss.updateDelay();
			myGear.updateDelay();
			myGear.handleHealing(boss.nado);
			//boss.chooseMethod(myGear, "5to1range"); //5to1range, 5to1mage, t2t3staffhally, doublet3staffbow
			boss.handleAttacking5to1range(myGear);
		}
	}
	else if (method == "5to1mage") {
		while (boss.curHp > 0 && myGear.alive) {
			boss.ticks++;
			boss.updateDelay();
			myGear.updateDelay();
			myGear.handleHealing(boss.nado);
			boss.handleAttacking5to1mage(myGear);
		}
	}
	else if (method == "t2t3staffhally") {
		while (boss.curHp > 0 && myGear.alive) {
			boss.ticks++;
			boss.updateDelay();
			myGear.updateDelay();
			myGear.handleHealing(boss.nado);
			boss.handleAttackingt2t3staffhally(myGear);
		}
	}
	else if (method == "doublet3staffbow") {
		while (boss.curHp > 0 && myGear.alive) {
			boss.ticks++;
                        //cout << "t" << boss.ticks << " - HHP: " << boss.curHp << " - PHP: " << myGear.hp << endl;
			boss.updateDelay();
			myGear.updateDelay();
			myGear.handleHealing(boss.nado);
			boss.handleAttackingdoublet3staffbow(myGear);
		}
	}
	else throw "wrong method chosen";

	boss.playerAlive = myGear.alive;
	if (boss.curHp <= 0) boss.ticks += 4 + 2; // animation delay + hit delay on last hit

	return boss;

}


string ticksToTime(int ticks) {
	int seconds = ticks*0.6;
	string time = to_string(seconds/60);
	time += ":";
	seconds = seconds%60;
	if (seconds < 10) time += "0";
	time += to_string(seconds%60);
	return time;
}



int main() {
	npcBoss result;
	int tickLoss;
	//player stats

	//range
	int cbow1A = 72;	int cbow1Max = 21;
	int cbow2A = 118;	int cbow2Max = 31;
	int cbow3A = 172; 	int cbow3Max = 41;

	//melee
	int punch_nakedA   = 0; 		int punch_nakedMax   = 13; //punch naked
	int punch_armourA    = 16;		int punch_armourMax    = 13; //punch armour
	int scepterA    = 0;		int scepterMax    = 17; //scepter naked
	int chally1A = 68;		int chally1Max = 22;
	int chally2A = 114;		int chally2Max = 31;
	int chally3A = 166;		int chally3Max = 42;

	//mage
	int cstaff1A = 84;	int cstaff1Max = 23;
	int cstaff2A = 128;	int cstaff2Max = 31;
	int cstaff3A = 184;	int cstaff3Max = 39;

	if (corrupt) {
		cbow1A += 16; cbow2A += 16; cbow3A += 16;
		chally1A += 16; chally2A += 16; chally3A += 16;
		cstaff1A += 16; cstaff2A += 16; cstaff3A += 16;
	}

	//range accuracy rolls
	int accCbow1 = calcAD(cbow1A,99*1.2,1);
	int accCbow2 = calcAD(cbow2A,99*1.2,1);
	int accCbow3 = calcAD(cbow3A,99*1.2,1);

	//melee accuracy rolls
	int accPunchNaked   = calcAD(punch_nakedA,  99*1.2,1);
	int accPunchArmour    = calcAD(punch_armourA,   99*1.2,1);
	int accScepter    = calcAD(scepterA,   99*1.2,1);
	int accChally1 = calcAD(chally1A,99*1.2,1);
	int accChally2 = calcAD(chally2A,99*1.2,1);
	int accChally3 = calcAD(chally3A,99*1.2,1);


	//mage accuracy rolls
	int accCstaff1 = calcAD(cstaff1A,99*1.25+3,1);
	int accCstaff2 = calcAD(cstaff2A,99*1.25+3,1);
	int accCstaff3 = calcAD(cstaff3A,99*1.25+3,1);

	cout << "Starting Program..." << endl;
	gear normalGear;

	//initialize range gear
	normalGear.accCbow1   = accCbow1;	normalGear.maxCbow1 = cbow1Max;
	normalGear.accCbow2   = accCbow2;	normalGear.maxCbow2 = cbow2Max;
	normalGear.accCbow3   = accCbow3;	normalGear.maxCbow3 = cbow3Max;

	//initialize melee gear
	normalGear.accPunchNaked   = accPunchNaked;	normalGear.maxPunchNaked = punch_nakedMax;
	normalGear.accPunchArmour    = accPunchArmour; 	normalGear.maxPunchArmour = punch_armourMax;
	normalGear.accScepter    = accScepter; 	normalGear.maxScepter = scepterMax;
	normalGear.accChally1 = accChally1;	normalGear.maxChally1 = chally1Max;
	normalGear.accChally2 = accChally2;	normalGear.maxChally2 = chally2Max;
	normalGear.accChally3 = accChally3;	normalGear.maxChally3 = chally3Max;

	//initialize mage gear
	normalGear.accCstaff1 = accCstaff1; normalGear.maxCstaff1 = cstaff1Max;
	normalGear.accCstaff2 = accCstaff2; normalGear.maxCstaff2 = cstaff2Max;
	normalGear.accCstaff3 = accCstaff3; normalGear.maxCstaff3 = cstaff3Max;

	//initialize other gear
	int punch = 4;
	if (corrupt) punch = 5;
	if (corrupt) tickLoss = 10;
	else tickLoss = 5;
	normalGear.setWeapons(1,3,punch); //magic, range, melee, 4 naked punch, 5 armour punch, 6 naked sceptre

	//tracking variables
	/* USE THIS IF YOU"RE OK WItH A MAX TEST SIZE OF 20,000.
	int staff3bow3[13][20000] = {0}; 	int hally3staff2[13][20000] = {0};
	int staff3bow2[13][20000] = {0}; 	int bow3staff2[13][20000] = {0};
	int staff3bow1[13][20000] = {0};	int bow3staff1[13][20000] = {0};
	*/

	//use this for bigger than 20,000 test size
	int** staff3bow3 = new int*[13];	int** hally3staff2 = new int*[13];
	int** staff3bow2 = new int*[13];	int** bow3staff2 = new int*[13];
	int** staff3bow1 = new int*[13];	int** bow3staff1 = new int*[13];
	for (int i = 0; i < 13; i++) {
		staff3bow3[i] = new int[200000];	hally3staff2[i] = new int[200000];
		staff3bow2[i] = new int[200000];	bow3staff2[i] = new int[200000];
		staff3bow1[i] = new int[200000];	bow3staff1[i] = new int[200000];
	}

	double survivalRate = 0; double speedRate = 0;

	string fileName = "gauntletData.csv";
	if (corrupt) fileName = "gauntletCorruptData.csv";
	
	try {
		
		ofstream resultFile;
		resultFile.open(fileName);
		
		int loops = 200000;
		int totalWins = 0;
		int sub2 = 0;
		cout << "food " << normalGear.food << ", defence " << normalGear.curMageDef << "," << normalGear.curRangeDef << endl;
		cout << "staff accuracies: " << normalGear.accCstaff3 << "," << normalGear.accCstaff2 << "," << normalGear.accCstaff1 << endl;
		cout << "staff maxes: " << normalGear.maxCstaff3 << "," << normalGear.maxCstaff2 << "," << normalGear.maxCstaff1 << endl;
		cout << "bow accuracies: " << normalGear.accCbow3 << "," << normalGear.accCbow2 << "," << normalGear.accCbow1 << endl;
		cout << "bow maxes: " << normalGear.maxCbow3 << "," << normalGear.maxCbow2 << "," << normalGear.maxCbow1 << endl;
		normalGear.setWeapons(3,2,punch);
		cout << "3 staff: " << normalGear.magicACC << "," << normalGear.magicMAX << ". 2 bow: " << normalGear.rangeACC << "," << normalGear.rangeMAX << ". punch: " << normalGear.meleeACC << "," << normalGear.meleeMAX << endl;
		normalGear.setWeapons(1,3,punch);
		cout << "1 staff: " << normalGear.magicACC << "," << normalGear.magicMAX << ". 3 bow: " << normalGear.rangeACC << "," << normalGear.rangeMAX << ". punch: " << normalGear.meleeACC << "," << normalGear.meleeMAX << endl;
		/* regular single attempt
		for (int y = 0; y < loops; y++) {
			result = fightBoss(normalGear, tickLoss, "doublet3staffbow");
			if (result.curHp <= 0 && result.playerAlive) totalWins++;
			if (result.ticks <= 200 && result.playerAlive) sub2++;
		}	*/
		bool quickTest = false; //Use this to skip most of the tests, and just do the speedrun at end
		// get data for all 6 methods, for 0-12 food
		for (int x = 0; x <= 12; x++) {
			if (quickTest) x = 7;
			normalGear.food = x;
			
			normalGear.setWeapons(3,3,punch);
			for (int y = 0; y < loops; y++) {
				result = fightBoss(normalGear, tickLoss, "doublet3staffbow");
				if (result.curHp <= 0 && result.playerAlive) staff3bow3[x][y] = result.ticks; 
				else staff3bow3[x][y] = 10000; }	
			
			normalGear.setWeapons(2,1,3);
			for (int y = 0; y < loops; y++) {
				result = fightBoss(normalGear, tickLoss, "t2t3staffhally");
				if (result.curHp <= 0 && result.playerAlive) hally3staff2[x][y] = result.ticks; 
				else hally3staff2[x][y] = 10000; }
			

			normalGear.setWeapons(3,2,punch);
			for (int y = 0; y < loops; y++) {
				result = fightBoss(normalGear, tickLoss, "5to1range");
				if (result.curHp <= 0 && result.playerAlive) staff3bow2[x][y] = result.ticks; 
				else staff3bow2[x][y] = 10000; }
			

			normalGear.setWeapons(3,1,punch);
			for (int y = 0; y < loops; y++) {
				result = fightBoss(normalGear, tickLoss, "5to1range");
				if (result.curHp <= 0 && result.playerAlive) staff3bow1[x][y] = result.ticks; 
				else staff3bow1[x][y] = 10000; }
			

			normalGear.setWeapons(2,3,punch);
			for (int y = 0; y < loops; y++) {
				result = fightBoss(normalGear, tickLoss, "5to1mage");
				if (result.curHp <= 0 && result.playerAlive) bow3staff2[x][y] = result.ticks; 
				else bow3staff2[x][y] = 10000; }
			

			normalGear.setWeapons(1,3,punch);
			for (int y = 0; y < loops; y++) {
				result = fightBoss(normalGear, tickLoss, "5to1mage");
				if (result.curHp <= 0 && result.playerAlive) bow3staff1[x][y] = result.ticks; 
				else bow3staff1[x][y] = 10000; }

			if (quickTest) x = 12;
		}

		string subText1 = "Sub 2:00 Chance,";
		string subText2 = "Sub 2:15 Chance,";
		string subText3 = "Sub 2:30 Chance,";
		int subTick1 = 200; int subTick2 = 225; int subTick3 = 250;

		if (corrupt) {
			subText1 = "Sub 3:15 Chance,";
			subText2 = "Sub 3:30 Chance,";
			subText3 = "Sub 3:45 Chance,";
			subTick1 = 325; subTick2 = 350; subTick3 = 375;
		}

		if (quickTest) goto quickStuff; //quickTest skip stuff we don't need

		//Title for columns 0...12
		resultFile << ",";
		for (int i = 0; i <= 12; i++) resultFile << i << ",";
		resultFile << "\n";
		//Data for 0...12 stuff
		resultFile << "Staff 3 Bow 3,";
		for (int x = 0; x <= 12; x++) {
			for (int y = 0; y < loops; y++) {
				if (staff3bow3[x][y] < 10000) survivalRate++;
			}
			survivalRate = 100*survivalRate/loops;
			resultFile << survivalRate << ",";
			survivalRate = 0;
		}
		resultFile << "\n";

		resultFile << "Hally 3 Staff 2,";
		for (int x = 0; x <= 12; x++) {
			for (int y = 0; y < loops; y++) {
				if (hally3staff2[x][y] < 10000) survivalRate++;
			}
			survivalRate = 100*survivalRate/loops;
			resultFile << survivalRate << ",";
			survivalRate = 0;
		}
		resultFile << "\n";

		resultFile << "Staff 3 Bow 2,";
		for (int x = 0; x <= 12; x++) {
			for (int y = 0; y < loops; y++) {
				if (staff3bow2[x][y] < 10000) survivalRate++;
			}
			survivalRate = 100*survivalRate/loops;
			resultFile << survivalRate << ",";
			survivalRate = 0;
		}
		resultFile << "\n";

		resultFile << "Staff 3 Bow 1,";
		for (int x = 0; x <= 12; x++) {
			for (int y = 0; y < loops; y++) {
				if (staff3bow1[x][y] < 10000) survivalRate++;
			}
			survivalRate = 100*survivalRate/loops;
			resultFile << survivalRate << ",";
			survivalRate = 0;
		}
		resultFile << "\n";

		resultFile << "Bow 3 Staff 2,";
		for (int x = 0; x <= 12; x++) {
			for (int y = 0; y < loops; y++) {
				if (bow3staff2[x][y] < 10000) survivalRate++;
			}
			survivalRate = 100*survivalRate/loops;
			resultFile << survivalRate << ",";
			survivalRate = 0;
		}
		resultFile << "\n";

		resultFile << "Bow 3 Staff 1,";
		for (int x = 0; x <= 12; x++) {
			for (int y = 0; y < loops; y++) {
				if (bow3staff1[x][y] < 10000) survivalRate++;
			}
			survivalRate = 100*survivalRate/loops;
			resultFile << survivalRate << ",";
			survivalRate = 0;
		}		
		resultFile << "\n";









		resultFile << ",";
		for (int i = 0; i <= 12; i++) resultFile << i << ",";
		resultFile << "\n";

		resultFile << "Survival Rate,";
		for (int x = 0; x <= 12; x++) {
			for (int y = 0; y < loops; y++) {
				if (bow3staff1[x][y] < 10000) survivalRate++;
			}
			survivalRate = 100*survivalRate/loops;
			resultFile << survivalRate << ",";
			survivalRate = 0;
		}		
		resultFile << "\n";
	
		resultFile << subText1;
		for (int x = 0; x <= 12; x++) {
			for (int y = 0; y < loops; y++) {
				if (bow3staff1[x][y] < subTick1) survivalRate++;
			}
			survivalRate = 100*survivalRate/loops;
			resultFile << survivalRate << ",";
			survivalRate = 0;
		}		
		resultFile << "\n";	

		resultFile << subText2;
		for (int x = 0; x <= 12; x++) {
			for (int y = 0; y < loops; y++) {
				if (bow3staff1[x][y] < subTick2) survivalRate++;
			}
			survivalRate = 100*survivalRate/loops;
			resultFile << survivalRate << ",";
			survivalRate = 0;
		}		
		resultFile << "\n";	

		resultFile << subText3;
		for (int x = 0; x <= 12; x++) {
			for (int y = 0; y < loops; y++) {
				if (bow3staff1[x][y] < subTick3) survivalRate++;
			}
			survivalRate = 100*survivalRate/loops;
			resultFile << survivalRate << ",";
			survivalRate = 0;
		}		
		resultFile << "\n";	







		quickStuff:

		int lowerTickLimit = 135; int upperTickLimit = 285;
		if (corrupt) {
			lowerTickLimit = 250;
			upperTickLimit = 400;
		}

		resultFile << ",";
		for (int i = lowerTickLimit; i <= upperTickLimit; i += 10) resultFile << ticksToTime(i) << ",";
		resultFile << "\n";

		resultFile << "Staff 3 Bow 3,";
		for (int x = lowerTickLimit; x <= upperTickLimit; x += 10) {
			for (int y = 0; y < loops; y++) {
				if (staff3bow3[7][y] < x) survivalRate++;
			}
			survivalRate = 100*survivalRate/loops;
			resultFile << survivalRate << ",";
			survivalRate = 0;
		}		
		resultFile << "\n";	

		resultFile << "Hally 3 Staff 2,";
		for (int x = lowerTickLimit; x <= upperTickLimit; x += 10) {
			for (int y = 0; y < loops; y++) {
				if (hally3staff2[7][y] < x) survivalRate++;
			}
			survivalRate = 100*survivalRate/loops;
			resultFile << survivalRate << ",";
			survivalRate = 0;
		}		
		resultFile << "\n";	

		resultFile << "Staff 3 Bow 2,";
		for (int x = lowerTickLimit; x <= upperTickLimit; x += 10) {
			for (int y = 0; y < loops; y++) {
				if (staff3bow2[7][y] < x) survivalRate++;
			}
			survivalRate = 100*survivalRate/loops;
			resultFile << survivalRate << ",";
			survivalRate = 0;
		}		
		resultFile << "\n";	

		resultFile << "Staff 3 Bow 1,";
		for (int x = lowerTickLimit; x <= upperTickLimit; x += 10) {
			for (int y = 0; y < loops; y++) {
				if (staff3bow1[7][y] < x) survivalRate++;
			}
			survivalRate = 100*survivalRate/loops;
			resultFile << survivalRate << ",";
			survivalRate = 0;
		}		
		resultFile << "\n";	

		resultFile << "Bow 3 Staff 2,";
		for (int x = lowerTickLimit; x <= upperTickLimit; x += 10) {
			for (int y = 0; y < loops; y++) {
				if (bow3staff2[7][y] < x) survivalRate++;
			}
			survivalRate = 100*survivalRate/loops;
			resultFile << survivalRate << ",";
			survivalRate = 0;
		}		
		resultFile << "\n";	

		resultFile << "Bow 3 Staff 1,";
		for (int x = lowerTickLimit; x <= upperTickLimit; x += 10) {
			for (int y = 0; y < loops; y++) {
				if (bow3staff1[7][y] < x) survivalRate++;
			}
			survivalRate = 100*survivalRate/loops;
			resultFile << survivalRate << ",";
			survivalRate = 0;
		}		
		resultFile << "\n";	


		cout << loops << " boss fights." << endl << endl;
		cout << totalWins << " total wins out of " << loops << " fights. " << 100*(double)totalWins/loops << "% win rate." << endl;
		cout << sub2 << " sub 2 runs out of " << loops << " fights. " << 100*(double)sub2/loops << "% win rate." << endl;
		cout << "1 in " << (double)loops / totalWins << " chance of winning." << endl;
		//cout << subTest << subTestText << 100*(double)(subTest)/loops << "%." << endl;
		cout << endl;

		resultFile.close();
		


		
	}
	catch ( ... ) {
		cout << "Ending Program" << endl;
	}
	cout << "done program lol" << endl;
}
