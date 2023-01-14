//============================================================================
// Name        : GenAlgoJTARs.cpp
// Author      : Jason Noble
// Version     : 0.1
// Copyright   :
// Description : Genetic Algorithm optimization of JTAR assignments to aviation assets.
//============================================================================

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <random>
#include <array>
#include <ctime>
#include <chrono>
#include <vector>
#include <functional>

const int num_organisms = 50;
const int num_genes = 10; //same as # of aircraft sections
const int num_tgts = 10;
const float mutationrate = 0.01;
int organism = 0;
int gene = 0;
int costconstant = 0; //constant added to -(cost) = total value of all targets. effectively sets a fitness floor of 0

std::array<std::array<int, num_genes>, num_organisms> currentGeneration = {0};
std::array<std::array<int, num_genes>, num_organisms> nextGeneration = {0};

std::random_device rd;
std::mt19937 gen(rd());
std::uniform_real_distribution<> dis(0, 1);
std::uniform_int_distribution<> tgtdis(0, (num_tgts-1));
std::uniform_int_distribution<> genedis(1, (num_genes-2));
std::uniform_int_distribution<> tgttypepick(0, 4);
std::uniform_int_distribution<> tgtqty(1, 15);

struct ac_section{
	std::string num_msn; //ATO MSN number
	int pos_lat, pos_long; //latitude and longitude, decimal
	int speed; // in km/h
	int sk_vehstat; //stored kills vs stationary vehicles or aircraft on the deck
	int sk_vehmov; //stored kills vs mobile vehicles
	int sk_pers; //stored kills vs infantry/personnel
	int sk_iads; //stored kills vs air-defense assets/radar sites
	ac_section(std::string num_msn, int pos_lat, int pos_long, int speed, int sk_vehstat, int sk_vehmov, int sk_pers, int sk_iads)
		: num_msn(num_msn), pos_lat(pos_lat), pos_long(pos_long), speed(speed), sk_vehstat(sk_vehstat), sk_vehmov(sk_vehmov), sk_pers(sk_pers), sk_iads(sk_iads) {}
	ac_section(): num_msn(""), pos_lat(0), pos_long(0), speed(0), sk_vehstat(0), sk_vehmov(0), sk_pers(0), sk_iads(0) {}
};

struct target{
	int num_jtar; //JTAR number, odd numbers only
	int pos_lat, pos_long; //latitude and longitude, decimal
	int hp_acdeck; //"hit points" of aircraft on the deck
	int hp_vehstat; //"hit points" of stationary vehicles or aircraft, stationary
	int hp_vehmov; //"hit points" of vehicles or aircraft, moving
	int hp_pers; //"hit points" of infantry/personnel
	int hp_iads; //"hit points" of air-defense assets/radar sites
	target(int num_jtar, int pos_lat, int pos_long, int hp_acdeck, int hp_vehstat, int hp_vehmov, int hp_pers, int hp_iads)
		: num_jtar(num_jtar), pos_lat(pos_lat), pos_long(pos_long), hp_acdeck(hp_acdeck), hp_vehstat(hp_vehstat), hp_vehmov(hp_vehmov), hp_pers(hp_pers), hp_iads(hp_iads) {}
	target(): num_jtar(0), pos_lat(0), pos_long(0), hp_acdeck(0), hp_vehstat(0), hp_vehmov(0), hp_pers(0), hp_iads(0) {}
};

struct orgfitness{
	int cost;
	float fitness;
	float normfitness; //organism's fitness value normalized as a fraction of 1
	float accumfitness; //accumulated fitness
	int index; //saves the original organism's position in the array
};
bool elitefit(const orgfitness& lhs, const orgfitness& rhs){
	return lhs.fitness < rhs.fitness;
}
bool desSort(const orgfitness& lhs, const orgfitness& rhs){
	return lhs.fitness > rhs.fitness;
}
std::vector<orgfitness> fitvector(num_organisms);


ac_section Rainbow01("0001",0,0,280,4,4,4,0);
ac_section Leopard01("0002",0,0,900,12,0,12,0);
ac_section Shark01("0003",0,0,1380,8,0,8,0);
ac_section Flanker01("0004",0,0,1380,8,8,8,12);
ac_section Qiang01("0005",0,0,1380,4,4,4,4);
ac_section Rainbow02("0006",0,0,280,4,4,4,0);
ac_section Leopard03("0007",0,0,900,12,0,12,0);
ac_section Shark03("0008",0,0,1380,8,0,8,0);
ac_section Flanker03("0009",0,0,1380,8,8,8,12);
ac_section Qiang03("0010",0,0,1380,4,4,4,4);
std::vector<ac_section> ATO = {Rainbow01, Leopard01, Shark01, Flanker01, Qiang01, Rainbow02, Leopard03, Shark03, Flanker03, Qiang03};
//std::vector<ac_section> ATO;

target JTAR1(1,0,0,6,0,0,0,0);
target JTAR3(3,0,0,0,0,0,0,4);
target JTAR5(5,0,0,0,0,0,0,2);
target JTAR7(7,0,0,0,0,10,0,0);
target JTAR9(9,0,0,0,0,0,15,0);
target JTAR11(11,0,0,0,20,0,0,0);
target JTAR13(13,0,0,10,0,0,0,0);
target JTAR15(15,0,0,0,0,0,15,0);
target JTAR17(17,0,0,8,0,0,0,0);
target JTAR19(19,0,0,0,20,0,0,0);
std::vector<target> JTARS = {JTAR1, JTAR3, JTAR5, JTAR7, JTAR9, JTAR11, JTAR13, JTAR15, JTAR17, JTAR19};
//std::vector<target> JTARS;

void ATOgen(int i){
	ATO.push_back(ac_section());
	int t = tgttypepick(gen);

	if(t == 0){
		ATO[i] = Rainbow01;
	}
	else if(t == 1){
		ATO[i] = Leopard01;
	}
	else if(t == 2){
		ATO[i] = Shark01;
	}
	else if(t == 3){
		ATO[i] = Flanker01;
	}
	else if(t == 4){
		ATO[i] = Qiang01;
	}
	ATO[i].num_msn = std::to_string(i);
}
void JTARsgen(int h){
	JTARS.push_back(target());
	int t = tgttypepick(gen);

	JTARS[h].num_jtar = h;
	if(t == 0){
		JTARS[h].hp_acdeck = tgtqty(gen);
	}
	else if(t == 1){
		JTARS[h].hp_vehstat = tgtqty(gen);
	}
	else if(t == 2){
		JTARS[h].hp_vehmov = tgtqty(gen);
	}
	else if(t == 3){
		JTARS[h].hp_pers = tgtqty(gen);
	}
	else if(t == 4){
		JTARS[h].hp_iads = tgtqty(gen);
	}
}

void InitializeOrganisms(){
	//init 1st generation with randomly-distributed target assignments
	for(organism = 0; organism < num_organisms; organism++){
		for(gene = 0; gene < num_genes; gene++){
			currentGeneration[organism][gene] = tgtdis(gen);
		}
	}

	for(int j = 0; j < num_genes; j++){
		costconstant += (JTARS[j].hp_acdeck * 3);
		costconstant += (JTARS[j].hp_vehstat * 1.5);
		costconstant += (JTARS[j].hp_vehmov * 2);
		costconstant += JTARS[j].hp_pers;
		costconstant += (JTARS[j].hp_iads * 4);
	}
	std::cout << "Cost constant is " << costconstant << std::endl;
}

std::vector<target> EvaluateACKills(std::vector<target> survivors, int ac, int i){
	int tgt = currentGeneration[i][ac]; //pull tgt # from current organism's current aircraft

	if (survivors[tgt].hp_acdeck > 0 && survivors[tgt].hp_acdeck >= ATO[ac].sk_vehstat)
		survivors[tgt].hp_acdeck -= ATO[ac].sk_vehstat;
	if (survivors[tgt].hp_vehstat > 0 && survivors[tgt].hp_vehstat >= ATO[ac].sk_vehstat)
		survivors[tgt].hp_vehstat -= ATO[ac].sk_vehstat;
	if(survivors[tgt].hp_vehstat <0)
		survivors[tgt].hp_vehstat = 0;
	if (survivors[tgt].hp_vehmov > 0 && survivors[tgt].hp_vehmov >= ATO[ac].sk_vehmov)
		survivors[tgt].hp_vehmov -= ATO[ac].sk_vehmov;
	if(survivors[tgt].hp_vehmov <0)
		survivors[tgt].hp_vehmov = 0;
	if (survivors[tgt].hp_pers > 0 && survivors[tgt].hp_pers >= ATO[ac].sk_pers)
		survivors[tgt].hp_pers -= ATO[ac].sk_pers;
	if(survivors[tgt].hp_pers <0)
		survivors[tgt].hp_pers = 0;
	if (survivors[tgt].hp_iads > 0 && survivors[tgt].hp_iads >= ATO[ac].sk_iads)
		survivors[tgt].hp_iads -= ATO[ac].sk_iads;
	if(survivors[tgt].hp_iads <0)
		survivors[tgt].hp_iads = 0;

	return survivors;
}

//calc undestroyed assets
//undestroyed assets = sum TGThp - ATOsk for all TGT types
//costs is *2 for station veh/ac, *2.5 for mobile, *4 for IADS
//unexpended ordnance cost?

void EvaluateOrganismCost(int i){
	//copy JTARS to survivor tally for this organism
	//std::array<target, num_tgts> survivors = JTARS;
	std::vector<target> survivors = JTARS;

	int ac = 0;
	int cost = 0; //total value of undestroyed assets

	for(int l = 0; l < gene; l++)
		std::cout << " " << currentGeneration[i][l];
	std::cout << std::endl;
	for(ac = 0; ac < num_genes; ac++){
		survivors = EvaluateACKills(survivors, ac, i);
	}

	//add weighted values of undestroyed targets to cost
	for(int j = 0; j < num_genes; j++){
		cost += (survivors[j].hp_acdeck * 3);
		cost += (survivors[j].hp_vehstat * 1.5);
		cost += (survivors[j].hp_vehmov * 2);
		cost += survivors[j].hp_pers;
		cost += (survivors[j].hp_iads * 4);
		//std::cout << "Cost for JTAR" << JTARS[j].num_jtar << " is" << cost << std::endl;
	}

	fitvector[i].cost = cost;
	fitvector[i].fitness = -fitvector[i].cost + costconstant; //formerly 200;
	std::cout << "Fitness for Organism" << i << " is" << fitvector[i].fitness << std::endl;
}

void GenerationCost()
{
	for(int i = 0; i < num_organisms; i++){
		EvaluateOrganismCost(i);
	}
}

void ProduceNextGen(){

	bool par1sel = false;
	bool par2sel = false;

	int totalfitness = 0; //sum of all fitness values in generation
	int par1x = -1; //index value for parent 1
	int par2x = -1; //index value for parent 2
	int elitex = 0; //index value for the elite-selected organism
	float mrand = 0; //mutation randomizer
	float rvalue = 0; //normally-distributed number between 0-1, probability a fitness is selected

	//copy indexes
	for(int h = 0; h < num_organisms; h++){
		fitvector[h].index = h;
	}

	//elite select the #1 most fit
	auto elitepos = std::distance(fitvector.begin(), std::max_element(fitvector.begin(), fitvector.end(), elitefit));
	elitex = fitvector[elitepos].index;
	nextGeneration[0] = currentGeneration[elitex];

	std::cout << "Elite organism is: ";
	for(int h = 0; h < num_genes; h++)
		std::cout << nextGeneration[0][h] << " ";
	std::cout << std::endl;

	//calc total fitness
	for(int h = 0; h < num_organisms; h++){
		totalfitness += fitvector[h].fitness;
	}

	//normalize fitness
	for(int h = 0; h < num_organisms; h++){
		fitvector[h].normfitness = float(fitvector[h].fitness) / totalfitness;
	}

	//sort by descending fitness
	std::sort(fitvector.begin(),fitvector.end(), desSort);

	//roulette wheel select the rest
	//calc accumulated fitness
	std::cout << "Accumulated Fitness ";
	for(int h = 0; h < num_organisms; h++){
		fitvector[h].accumfitness = fitvector[h].normfitness + fitvector[h-1].accumfitness;
		std::cout << fitvector[h].accumfitness << " ";
	}
	std::cout << std::endl;

	for(int h = 1; h < num_organisms; h++){
		//compare accumulated fitness to rvalue here until 2 unique parents selected
		rvalue = dis(gen);

		for(int i = 0; par2sel == false; i++){

			if(rvalue < fitvector[i].accumfitness && par1sel == false){
				par1sel = true;
				par1x = i;
			}
			else if(rvalue < fitvector[i].accumfitness && par2sel == false){
				par2sel = true;
				par2x = i;
			}

			if(par1x == i or par2x == i){
				rvalue = dis(gen);
			}
			if(i==num_organisms)
				i=0; //continues the loop in case the end of the gene is reached but 2 parents aren't selected yet
		}

		//convert from accumulated fitness index to generation index
		par1x = fitvector[par1x].index;
		par2x = fitvector[par2x].index;

		int genecpypt = genedis(gen); //random point on genome to execute the copy

		//crossover copies the genome, checking for mutations with each gene
		for(int k=0; k < genecpypt; k++){
			mrand = dis(gen);
			if(mrand <= mutationrate){
				nextGeneration[h][k] = tgtdis(gen);
				std::cout << "***MUTATION***" << std::endl;
			}
			else
				nextGeneration[h][k] = currentGeneration[par1x][k];
		}
		for(int l=genecpypt; l < num_genes; l++){
			mrand = dis(gen);
			if(mrand <= mutationrate){
				nextGeneration[h][l] = tgtdis(gen);
				std::cout << "***MUTATION***" << std::endl;
			}
			else
				nextGeneration[h][l] = currentGeneration[par2x][l];
		}
		std::cout << "Parents" << par1x << "and" << par2x << " produced:";
		for(int m = 0; m < num_genes; m++){
			std::cout << nextGeneration[h][m];
		}
		std::cout << std::endl;
		par1sel = false;
		par2sel = false;
	}
}

void bestOrganism(std::ofstream& out, int num_generations){
	int bestfit, genedex;
	GenerationCost();
	bestfit = std::distance(fitvector.begin(), std::max_element(fitvector.begin(), fitvector.end(), elitefit));
	std::cout << "Maximum fitness rating is " << fitvector[bestfit].fitness
			<< " for an efficiency of " << ( float(fitvector[bestfit].fitness / costconstant) * 100)
			<< "%" << std::endl;
	std::cout << "Aircraft-target assignments are " << std::endl;
	for(int i = 0; i < num_genes; i++){
		genedex = currentGeneration[bestfit][i];
		std::cout << "MSN#" << ATO[i].num_msn << ": " << JTARS[genedex].num_jtar << std::endl;
	}

	out.open("log.txt", std::ios::app);
	out << num_organisms << "," << num_generations << "," << fitvector[bestfit].fitness
			<< "," << ( float(fitvector[bestfit].fitness / costconstant) * 100) << ",";

}

void lifecycle(int num_generations){

	for(int i = 0; i < num_generations; i++){
		GenerationCost();
		ProduceNextGen();
		currentGeneration = nextGeneration;
		nextGeneration = {0};
	}
}

int main() {

	int num_generations[10] = {10,20,50,100,500,1000,2000,3000,4000,5000}; //number of generations to reproduce
	std::ofstream out("log.txt");
	out << "Generation Size,# of Generations,maximum fitness,efficiency%,time" << std::endl;
	out.close();

	/*std::string userinput = " ";
	std::cout << "Enter # of generations";
	std::getline(std::cin, userinput);
	std::stringstream userstream(userinput);
	userstream >> num_generations;
	std::cout << "Will execute with " << num_generations << " generations." << std::endl; */

	/*for(int h = 0; h < num_tgts; h++){
		JTARsgen(h);
	}
	for(int i = 0; i < num_genes; i++){
		ATOgen(i);
	}*/
	InitializeOrganisms();
	for(int h : num_generations){
		std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
		lifecycle(h);
		bestOrganism(out, h);
		std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
		std::cout << "Targeting assignment took " << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count()
					<< "us.\n";
		out << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << std::endl;
		out.close();
	}

	return 0;
}
