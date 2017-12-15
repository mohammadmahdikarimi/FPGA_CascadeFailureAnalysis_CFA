//*********************************************************************************************
//*  Project : FPGA Functional Safety Analysis
//*  Function : 
//*********************************************************************************************
//*  Author : Mohammad Mahdi Karimi
//*  Email : mmkarimi@aggies.ncat.edu
//*********************************************************************************************
//*  Copyright 2017 by kVA
//********************************************************************************************* 
/*............including headers..............*/
#include <iostream>   /*for input and output*/
#include <fstream>     /*for file input and output*/
#include <vector>     /*for array-like features*/
#include <string>
#include <cstdlib>
#include <sstream>
#include <algorithm>
#include <math.h>
using namespace std;

//string xdl_fn = "c17.xdl";
//string vhd_fn = "c17_timesim.vhd";
//string xdl_rep_fn = "c17_XDL_report.txt";
//string vhd_rep_fn = "c17_VHDL_report.txt";
//string traverse_rep_fn = "c17_Traverse_report.txt";
//ofstream trav_data(traverse_rep_fn);

string xdl_fn = "b13.xdl";
string vhd_fn = "b13_timesim.vhd";
string xdl_rep_fn = "b13_XDL_report.txt";
string vhd_rep_fn = "b13_VHDL_report.txt";
string traverse_rep_fn = "b13_Traverse_report.txt";
ofstream trav_data(traverse_rep_fn);

string debugg = "debugg.txt";
ofstream debug_code(debugg);
//========== Structures ========== 
struct T_IID {
	string pout_name;
	vector<int> tile;
	vector<int> out;
	double p = 0;
};

struct N_ID {
	string name;
	int id;
};

struct Inp {
	//# every input has the information of what is its name, and to where it has been connected. also we initialize the probability to -1
	string name;
	N_ID wired_out;
	N_ID wired_tile;
	double p = 0;
	int visited = 50;// for the traverse to see if we have visited this input already change to 0!
};

struct InstInp_Slice {
	//# this has the structure of SLICEL blocks. we added this to make the inputs sorted.
	int a1 = 0, a2 = 1, a3 = 2, a4 = 3, a5 = 4, a6 = 5, b1 = 6, b2 = 7, b3 = 8, b4 = 9, b5 = 10, b6 = 11, c1 = 12, c2 = 13, c3 = 14, c4 = 15, c5 = 16, c6 = 17, d1 = 18, d2 = 19, d3 = 20, d4 = 21, d5 = 22, d6 = 23, ax = 24, bx = 25, cx = 26, dx = 27, clk = 28, ce = 29, sr = 30, cin = 31;
	Inp a1_0, a2_1, a3_2, a4_3, a5_4, a6_5, b1_6, b2_7, b3_8, b4_9, b5_10, b6_11, c1_12, c2_13, c3_14, c4_15, c5_16, c6_17, d1_18, d2_19, d3_20, d4_21, d5_22, d6_23, AX_24, BX_25, CX_26, DX_27, CLK_28, CE_29, SR_30, CIN_31;
	vector<Inp> inpv{ a1_0, a2_1, a3_2, a4_3, a5_4, a6_5, b1_6, b2_7, b3_8, b4_9, b5_10, b6_11, c1_12, c2_13, c3_14, c4_15, c5_16, c6_17, d1_18, d2_19, d3_20, d4_21, d5_22, d6_23, AX_24, BX_25, CX_26, DX_27, CLK_28, CE_29, SR_30, CIN_31 };
	vector<int> inpvecList;
};

struct Out {
	string name;
	double p = 0;
	int visited = 50;// chage to 0
	int loop = 0;
	vector<T_IID> paths;
};

struct CO {
	double p = 0;
};

struct double_data {
	string data1;
	string data2;
};

struct LUT {
	string l5_name, l5_func, l5_init;
	string l6_name, l6_func, l6_init;
};

struct CFG {
	string cnf;
};

struct lut1 {
	string name, init;
};

struct vhdlExplore_out {
	vector<lut1> lut_info;
	vector<string> lut_name_list;
};

struct SLICEOut {
	vector<string> out_set_name = { "AQ","A","AMUX", "BQ","B","BMUX", "CQ","C","CMUX", "DQ","D","DMUX", "COUT" };
	vector<string> out_set_mux = { "FFMUX","O6","OUTMUX", "FFMUX","O6","OUTMUX","FFMUX","O6","OUTMUX","FFMUX","O6","OUTMUX", "COUT" };
	vector<int> out_set_case = { 1,2,3,1,2,3,1,2,3,1,2,3,4 };
	vector<int> out_set_lut = { 0,0,0,1,1,1,2,2,2,3,3,3,3 };
};

vector<T_IID> OutStream_lists;

//========== Line Info Extracting Functions ==========
double_data line_info(string line) {
	size_t pos1, pos2, pos3;
	double_data out;
	pos1 = (line.find(" \"")) + 2;
	pos2 = (line.find("\" ", pos1));
	out.data1 = line.substr(pos1, pos2 - pos1);

	pos3 = (line.find(" ,", pos2));
	out.data2 = line.substr(pos2 + 2, pos3 - pos2 - 2);
	return out;
}

vector<string> line_info_inst(string line) {
	size_t pos1, pos2, pos3;
	vector<string> s;
	pos3 = line.find("\"");
	pos2 = line.find("\"", pos3 + 1);
	s.push_back(line.substr(pos3 + 1, pos2 - pos3 - 1));
	//out_data << line.substr(pos5 + 6, pos2 - pos5 - 6) << endl;// inst name

	pos3 = line.find("\"", pos2 + 1);
	pos2 = line.find("\"", pos3 + 1);
	s.push_back(line.substr(pos3 + 1, pos2 - pos3 - 1));
	//out_data << line.substr(pos3 + 1, pos2 - pos3 - 1) << endl; //inst type

	pos3 = line.find("placed ", pos2 + 1) + 6;
	pos2 = line.find(" ", pos3 + 1);
	s.push_back(line.substr(pos3 + 1, pos2 - pos3 - 1));
	//out_data << line.substr(pos3 + 1, pos2 - pos3 - 1) << endl; //inst pos tile

	pos3 = pos2;
	pos2 = line.find(" ", pos3 + 1);
	s.push_back(line.substr(pos3 + 1, pos2 - pos3 - 1));

	return s;
	//out_data << pos3 << " : " << pos2 - 1 << " " << line.substr(pos3 + 1, pos2 - pos3 - 1) << endl; //inst pos site
}
//========== Instance Class ==========
class inst {
	//# Class of instances with their important properties.
public:
	string name, type;
	string  pos_tile, pos_site;
	LUT lut_a, lut_b, lut_c, lut_d;
	vector<LUT> luti;

	CFG acy0, bcy0, ccy0, dcy0; //ACY0,...
	vector<CFG> xcy0;
	CFG affmux, bffmux, cffmux, dffmux; //AFFMUX,...
	vector<CFG> xffmux;
	CFG aoutmux, boutmux, coutmux, doutmux; //AOUTMUX,...
	vector<CFG> xoutmux;
	CFG aused, bused, cused, dused; //AUSED,... this should be related to the AOUTMUX will see
	vector<CFG> xused;
	CFG precyinit;
	CO coinit, co0, co1, co2, co3;
	//vector<CFG> coi;


	vector<Inp> inp_other;//we might have several inputs for an other type instance
	InstInp_Slice inp_slice;//we have one input set for a SLICEL machine that contains all the input sets
	vector<Out> outputs;

	void set_name(string s) { name = s; };
	void set_type(string s) { type = s; };
	void set_pos(string s_tile, string s_site) { pos_tile = s_tile; pos_site = s_site; };
	void set_lut_a(LUT l) { lut_a = l; };
	void set_lut_b(LUT l) { lut_b = l; };
	void set_lut_c(LUT l) { lut_c = l; };
	void set_lut_d(LUT l) { lut_d = l; };
	void set_input(string, string, int, string, int);
	void set_config();
	void inst::set_output(Out);
	string disp_info();
};

void inst::set_output(Out n) {
	//# This funtion sets the output name and initializez the probability to -1
	///N_ID out1; out1.name = n; out1.id = v;
	outputs.push_back(n);
}

void inst::set_input(string N, string T_n, int T_i, string O_n, int O_i) {
	//# This function sets the input as sorted. 
	//# it checks if the instance is SLICE it positions the input in order also sets the other values as the linked output to each input
	//# the destination output has the information of tile number(instance number), tile name, output name and output number
	N_ID O, T; O.name = O_n; O.id = O_i; T.name = T_n; T.id = T_i;
	Inp inp1; inp1.name = N; inp1.wired_tile = T; inp1.wired_out = O;
	//InstInp_Slice inp_slice;
	// Check the inp name and assign it to the appropriate position
	if (type == "SLICEL") {
		if (N.size() == 3) {//CIN,CLK
			if (N == "CIN") {
				inp_slice.inpv[inp_slice.cin] = inp1;
				inp_slice.inpvecList.push_back(inp_slice.cin);
			}

			else if (N == "CLK") {
				//inp_slice.CLK = inp1;
				inp_slice.inpv[inp_slice.clk] = inp1;
				inp_slice.inpvecList.push_back(inp_slice.clk);
			}
		}
		else if (N[1] == 'E' || N[1] == 'R' || N[1] == 'X') {//CE,SR,AX,BX,..
			if (N == "CE") {
				inp_slice.inpv[inp_slice.ce] = inp1;
				inp_slice.inpvecList.push_back(inp_slice.ce);
				//inp_slice.CE = inp1;
			}
			else if (N == "SR") {
				inp_slice.inpv[inp_slice.sr] = inp1;
				inp_slice.inpvecList.push_back(inp_slice.sr);
				//inp_slice.SR = inp1;
			}
			else if (N == "AX") {
				inp_slice.inpv[inp_slice.ax] = inp1;
				inp_slice.inpvecList.push_back(inp_slice.ax);
				//inp_slice.AX = inp1;
			}
			else if (N == "BX") {
				inp_slice.inpv[inp_slice.bx] = inp1;
				inp_slice.inpvecList.push_back(inp_slice.bx);
				//inp_slice.BX = inp1;
			}
			else if (N == "CX") {
				inp_slice.inpv[inp_slice.cx] = inp1;
				inp_slice.inpvecList.push_back(inp_slice.cx);
				//inp_slice.CX = inp1;
			}
			else if (N == "DX") {
				inp_slice.inpv[inp_slice.dx] = inp1;
				inp_slice.inpvecList.push_back(inp_slice.dx);
				//inp_slice.DX = inp1;
			}
		}
		else {//A1,B1,...
			int num;
			if (N[1] == '1') {
				num = 1;
			}
			else if (N[1] == '2') {
				num = 2;
			}
			else if (N[1] == '3') {
				num = 3;
			}
			else if (N[1] == '4') {
				num = 4;
			}
			else if (N[1] == '5') {
				num = 5;
			}
			else if (N[1] == '6') {
				num = 6;
			}

			if (N[0] == 'A') {
				inp_slice.inpv[inp_slice.a1 + num - 1] = inp1;
				inp_slice.inpvecList.push_back(inp_slice.a1 + num - 1);
				//inp_slice.A[num]= inp1;
			}

			else if (N[0] == 'B') {
				inp_slice.inpv[inp_slice.b1 + num - 1] = inp1;
				inp_slice.inpvecList.push_back(inp_slice.b1 + num - 1);
				//inp_slice.B[num] = inp1;
			}
			else if (N[0] == 'C') {
				inp_slice.inpv[inp_slice.c1 + num - 1] = inp1;
				inp_slice.inpvecList.push_back(inp_slice.c1 + num - 1);
				//inp_slice.C[num] = inp1;
			}
			else if (N[0] == 'D') {
				inp_slice.inpv[inp_slice.d1 + num - 1] = inp1;
				inp_slice.inpvecList.push_back(inp_slice.d1 + num - 1);
				//inp_slice.D[num] = inp1;
			}
		}
	}
	else
	{
		inp_other.push_back(inp1);
	}

}

string inst::disp_info() {
	//# This function displays the important information of the block of instance. specially the SLICELs
	string s = "name = " + name + " type:<" + type + "> position <" + pos_tile + "> <" + pos_site + ">";
	if (type == "SLICEL") {
		s = s + "\n--------" + "\nLUT Information:";
		if (lut_a.l5_name != "#OFF")
			s = s + "\n  " + "LUTA5 (" + lut_a.l5_name + ") O5=" + lut_a.l5_func + " init=0X" + lut_a.l5_init;
		if (lut_a.l6_name != "#OFF")
			s = s + "\n  " + "LUTA6 (" + lut_a.l6_name + ") O6=" + lut_a.l6_func + " init=0X" + lut_a.l6_init;
		if (lut_b.l5_name != "#OFF")
			s = s + "\n  " + "LUTB5 (" + lut_b.l5_name + ") O5=" + lut_b.l5_func + " init=0X" + lut_b.l5_init;
		if (lut_b.l6_name != "#OFF")
			s = s + "\n  " + "LUTB6 (" + lut_b.l6_name + ") O6=" + lut_b.l6_func + " init=0X" + lut_b.l6_init;
		if (lut_c.l5_name != "#OFF")
			s = s + "\n  " + "LUTC5 (" + lut_c.l5_name + ") O5=" + lut_c.l5_func + " init=0X" + lut_c.l5_init;
		if (lut_c.l6_name != "#OFF")
			s = s + "\n  " + "LUTC6 (" + lut_c.l6_name + ") O6=" + lut_c.l6_func + " init=0X" + lut_c.l6_init;
		if (lut_d.l5_name != "#OFF")
			s = s + "\n  " + "LUTD5 (" + lut_d.l5_name + ") O5=" + lut_d.l5_func + " init=0X" + lut_d.l5_init;
		if (lut_d.l6_name != "#OFF")
			s = s + "\n  " + "LUTD6 (" + lut_d.l6_name + ") O6=" + lut_d.l6_func + " init=0X" + lut_d.l6_init;

		s = s + "\n  " + "cfg:  ACY0:  " + acy0.cnf + " \tAFFMUX:  " + affmux.cnf + "\tAOUTMUX:  " + aoutmux.cnf + "\tAUSED:  " + aused.cnf;
		s = s + "\n  " + "cfg:  BCY0:  " + bcy0.cnf + " \tBFFMUX:  " + bffmux.cnf + "\tBOUTMUX:  " + boutmux.cnf + "\tBUSED:  " + bused.cnf;
		s = s + "\n  " + "cfg:  CCY0:  " + ccy0.cnf + " \tCFFMUX:  " + cffmux.cnf + "\tCOUTMUX:  " + coutmux.cnf + "\tCUSED:  " + cused.cnf;
		s = s + "\n  " + "cfg:  DCY0:  " + dcy0.cnf + " \tDFFMUX:  " + dffmux.cnf + "\tDOUTMUX:  " + doutmux.cnf + "\tDUSED:  " + dused.cnf;
		s = s + "\n  " + "cfg:  PRECYINIT:  " + precyinit.cnf;

	}
	s = s + "\noutput [" + to_string(outputs.size()) + "]";
	if (to_string(outputs.size()) != "0") {
		s = s + "\n  ";
		for (int i = 0; i < outputs.size(); i++)
			s = s + " " + to_string(i) + "- " + outputs[i].name + " , ";
	}
	if (type == "SLICEL") {// for SLICEL we have inp_slice holding the information

		sort(inp_slice.inpvecList.begin(), inp_slice.inpvecList.end());
		s = s + "\ninput [" + to_string(inp_slice.inpvecList.size()) + "]";
		if (to_string(inp_slice.inpvecList.size()) != "0") {
			s = s + "\n  ";
			for (int i = 0; i < inp_slice.inpvecList.size(); i++)
				s = s + " " + to_string(i) + "- " + inp_slice.inpv[inp_slice.inpvecList[i]].name + " , ";
			//s = s + "\ninput Wired to: Instance (ID)[outport (ID)]";
			s = s + "\n  ";
			for (int i = 0; i < inp_slice.inpvecList.size(); i++)
				s = s + " " + to_string(i) + "- " + inp_slice.inpv[inp_slice.inpvecList[i]].wired_tile.name + " (" + to_string(inp_slice.inpv[inp_slice.inpvecList[i]].wired_tile.id) + ")" + "[" + inp_slice.inpv[inp_slice.inpvecList[i]].wired_out.name + " (" + to_string(inp_slice.inpv[inp_slice.inpvecList[i]].wired_out.id) + ")" + "]" + " , ";
		}
	}
	else {
		s = s + "\ninput [" + to_string(inp_other.size()) + "]";
		if (to_string(inp_other.size()) != "0") {
			s = s + "\n  ";
			for (int i = 0; i < inp_other.size(); i++)
				s = s + " " + to_string(i) + "- " + inp_other[i].name + " , ";
			//s = s + "\ninput Wired to: Instance (ID)[outport (ID)]";
			s = s + "\n  ";
			for (int i = 0; i < inp_other.size(); i++)
				s = s + " " + to_string(i) + "- " + inp_other[i].wired_tile.name + " (" + to_string(inp_other[i].wired_tile.id) + ")" + "[" + inp_other[i].wired_out.name + " (" + to_string(inp_other[i].wired_out.id) + ")" + "]" + " , ";
		}
	}
	//cout << s << endl;
	return s;
}

void inst::set_config() {
	luti = { lut_a, lut_b, lut_c, lut_d };
	xcy0 = { acy0, bcy0, ccy0, dcy0 };
	xffmux = { affmux,bffmux,cffmux,dffmux };
	xoutmux = { aoutmux,boutmux,coutmux,doutmux };
	xused = { aused,bused,cused,dused };

}

//========== Global Variables ==========
vector<string> inst_name_list;
vector<inst> inst_class_list;

//========== VHDL Exploring Function ==========
vhdlExplore_out vhdlExplore() {
	ifstream vhdlFile;
	vhdlFile.open(vhd_fn);
	ofstream vhdl_data(vhd_rep_fn);
	string search1 = "X_LUT";
	string search2 = ");";
	string line;
	lut1 lut_info;

	vhdlExplore_out out;
	vector<string> lut_name_list;
	string lut_name;
	size_t pos1, pos2, pos3, pos4;
	int counter = 0;

	while (vhdlFile.good())
	{
		counter++;
		getline(vhdlFile, line);
		pos1 = line.find(search1);

		//extract the LUT name
		if (pos1 != string::npos) {
			pos2 = line.find("\\");
			if (pos2 != string::npos) {
				pos3 = line.find("\\", pos2 + 1);
				lut_info.name = line.substr(pos2 + 1, pos3 - pos2 - 1);
			}
			else {
				pos2 = line.find(" ");
				pos3 = line.find(" ", pos2 + 1);
				while (pos3 == pos2 + 1) {
					pos2 = pos3;
					pos3 = line.find(" ", pos2 + 1);
				}
				lut_info.name = line.substr(pos2 + 1, pos3 - pos2 - 1);
			}
			//vhdl_data << lut_info.name << endl;

			size_t pos5 = line.find(search2); //untill it finish 
			while (vhdlFile.good() && pos5 == string::npos) {
				counter++;
				getline(vhdlFile, line);
				pos5 = line.find(search2);
				pos4 = line.find("INIT => X");
				//extract the LUT init
				if (pos4 != string::npos) {
					pos2 = pos4 + 9;
					pos3 = line.find("\"", pos2 + 1);
					lut_info.init = line.substr(pos2 + 1, pos3 - pos2 - 1);
					vhdl_data << lut_info.name << " : " << lut_info.init << endl;
					out.lut_info.push_back(lut_info);
					out.lut_name_list.push_back(lut_info.name);
				}
			}
		}

	}
	return out;
}

//========== LUT Functions ==========
vector<unsigned> lut_HexText2Binary(string s) {
	//# LUT:0X0000000000040044 this code will translate the string value for the hex LUT to vector of the corresponding binary value
	//# We use the resulting vector to calculate the probability of signal to be one
	//# we can multiply the ones and zeros to activate or disactivate a calculation
	//# input to this function is a string LUT.

	string s1 = s.substr(0, 8);//s1 calculates the LUT5 functions where we have 32 bit output/ or first half of the LUT6
	stringstream ss1;
	ss1 << hex << s1;
	unsigned n1;
	ss1 >> n1;
	double c = 0;
	vector<unsigned int> b;
	if (std::size(s) > 8)
		b = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	else
		b = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	vector<unsigned int>b2;
	int i1 = 0;
	while (i1<32) {
		b2.push_back(n1 & 0x1);
		c += (n1 & 0x1);
		n1 >>= 1;
		i1++;
	}
	for (unsigned int i = 0; i < size(b2); i++) {
		b[31 - i] = b2[i];//this way the number 1 is positioned in the right binary place. as n1 is not 32 bit necessary.
	}
	if (std::size(s) > 8) {
		vector<unsigned int>b2;
		string s2 = s.substr(8, 8);//s2 calculate the second half of the LUT6
		stringstream ss2;
		ss2 << hex << s2;
		unsigned n2;
		ss2 >> n2;
		while (n2) {
			b2.push_back(n2 & 0x1);
			c += (n2 & 0x1);
			n2 >>= 1;
			i1++;
		}
		for (unsigned int i = 0; i < size(b2); i++) {
			b[63 - i] = b2[i];
		}
	}
	return b;
}

vector<double> lut_ProbabilityCalc(vector<unsigned> b, vector<double>p1, string lut_type) {
	//p is the probability of P(I[x]=1)
	//# this function is called by lut_ProbTop and calculated the probability of every output in every row of the LUT
	//# it takes the LUT INIT value in the vector from lut_HexText2Binary and input probabilities and the LUT type from the TOP module
	//# it sends back probability of each row in the LUT

	int j = 0;
	vector<double>p = p1;
	vector<double> po;
	unsigned int test = 0x00000;//this is just to say where we are for COUT
	string sout;
	sout = sout + "LUT INIT: ";
	for (int i = 0; i < size(b); i++)
		sout = sout + to_string(b[i]);
	if (lut_type == "LUT5") {
		sout = sout + "\n#" + ": \t" + "A5" + "| " + "A4" + "| " + "A3" + "| " + "A2" + "| " + "A1" + "||   " + "INIT" + " \t" + "Po" + "\n" + "----------------------------------------\n";
		for (int i5 = 0; i5 < 2; i5++) {
			p[4] = 1 - p[4];
			for (int i4 = 0; i4 < 2; i4++) {
				p[3] = 1 - p[3];
				for (int i3 = 0; i3 < 2; i3++) {
					p[2] = 1 - p[2];
					for (int i2 = 0; i2 < 2; i2++) {
						p[1] = 1 - p[1];
						for (int i1 = 0; i1 < 2; i1++) {
							p[0] = 1 - p[0];//probability of P(I0=0 or 1) same order of 0/1
							po.push_back((p[0] * p[1] * p[2] * p[3] * p[4]));
							sout = sout + to_string(test) + ": \t" + to_string(i5) + " | " + to_string(i4) + " | " + to_string(i3) + " | " + to_string(i2) + " | " + to_string(i1) + " ||   " + to_string(b[j]) + " \t" + to_string(po[j]) + "\n";
							test++;
							j++;
						}
					}
				}
			}
		}
	}
	else if (lut_type == "LUT6") {
		for (int i6 = 0; i6 < 2; i6++) {
			p[5] = 1 - p[5];
			for (int i5 = 0; i5 < 2; i5++) {
				p[4] = 1 - p[4];
				for (int i4 = 0; i4 < 2; i4++) {
					p[3] = 1 - p[3];
					for (int i3 = 0; i3 < 2; i3++) {
						p[2] = 1 - p[2];
						for (int i2 = 0; i2 < 2; i2++) {
							p[1] = 1 - p[1];
							for (int i1 = 0; i1 < 2; i1++) {
								p[0] = 1 - p[0];//probability of P(I0=0 or 1) same order of 0/1
								po.push_back((p[0] * p[1] * p[2] * p[3] * p[4] * p[5]));
								sout = sout + to_string(test) + ":\t" + to_string(i6) + "  " + to_string(i5) + "  " + to_string(i4) + "  " + to_string(i3) + "  " + to_string(i2) + "  " + to_string(i1) + " \t" + to_string(po[j]) + "\n";
								test++;
								j++;
							}
						}
					}
				}
			}
		}
	}
	//cout << sout;
	return po;
}

double lut_Prob_Top(string s, vector<double>p) {
	//# takes the LUT INIT in string and probability value of the inputs to be 1
	//# returns the output probability to be one for the LUT
	string type, sout;
	if (s.size() > 8)
		type = "LUT6";
	else
		type = "LUT5";
	vector<unsigned int> b = lut_HexText2Binary(s);
	vector<double> po = lut_ProbabilityCalc(b, p, type);
	double lut_p = 0;
	for (int i = 0; i < size(po); i++) {
		sout = sout + to_string(i) + ":\t" + to_string(po[i]) + "  x  " + to_string(b[i]) + " = \t" + to_string(po[i] * b[i]) + "\n";
		lut_p += po[i] * b[i];
	}
	sout += "\t\tp(LUT) =\t" + to_string(lut_p) + "\n";
	//cout << sout;
	return lut_p;
}

//========== Traversing Function ==========
double Traverse(int tloc, int oloc, T_IID o_stream) {
	//# This is a recursive code that starts from a given primary output coming from Traverse Top module and reaches to the inputs
	//# input to this file is the next output that is connected to the current input.
	//# it gets the tloc which is the instance id and the oloc which is the output id in that instance. 
	//# it also gets the o_stream to keep track of top primary output, previous outputs that have visited, their tile id (Instance ID) and the ouput ID in that tile.
	//# //o_stream.p holds the value of probability of the wired output, which then need to be assigned to the input.p as well
	//PART 1: Detect Loops here

	for (int i = 0; i < o_stream.tile.size(); i++) {
		if (o_stream.tile[i] == tloc) {
			if (o_stream.out[i] == oloc) {//we again at the same instance and same output meaning loop here
				o_stream.p = .5;// its a 50% chance to be one for now!!
				inst_class_list[tloc].outputs[oloc].paths.push_back(o_stream);

				inst_class_list[tloc].outputs[oloc].loop++;
				inst_class_list[tloc].outputs[oloc].visited = 50;
				inst_class_list[tloc].outputs[oloc].p = o_stream.p;
				return o_stream.p;
			}
		}
	}
	
	inst_class_list[tloc].outputs[oloc].paths.push_back(o_stream);

	//This is Disactivated as I want to know how each and every output would be connected to the primary outputs. so I set visited==2 not 1
	if (inst_class_list[tloc].outputs[oloc].visited == 30) {// Here we get the value of the output that has already calculated.
		o_stream.p = inst_class_list[tloc].outputs[oloc].p;
		return o_stream.p;
	}


	// PART 2: Add current state to the visited list
	o_stream.tile.push_back(tloc); o_stream.out.push_back(oloc);

	//inst_class_list[tloc].outputs[oloc].visited = 50; // I took this out I think this will include the loops that will make my calculations incorrect.

	//PART 3 : Detect Inputs	
	if (inst_class_list[tloc].type == "IOB" && inst_class_list[tloc].outputs.size() == 1) {//if its Input Block
		debug_code << "\nwe at INPUT";
		//cout << "\nwe at INPUT";
		o_stream.p = inst_class_list[tloc].outputs[oloc].p;//we have to take the value of p and propagate it. I changed this from 0 to oloc, i think its the same
														   /*
														   //Display Information From HERE
														   string sloop1 = "\n=============== 1) We at Input ===============\n";
														   sloop1 += "\n\t\t1) Lookin At: ";
														   string sloop2 = "\n\t\t1) Lookin At: ";
														   for (int i = 0; i < o_stream.tile.size(); i++) {
														   sloop1 = sloop1 + to_string(o_stream.tile[i]) + "_" + to_string(o_stream.out[i]) + " >> ";
														   sloop2 = sloop2 + inst_class_list[o_stream.tile[i]].name + " _ " + inst_class_list[o_stream.tile[i]].outputs[o_stream.out[i]].name + " >> ";
														   }
														   sloop1 = sloop1 + "P: " + to_string(o_stream.p);
														   sloop2 = sloop2 + "P: " + to_string(o_stream.p);
														   trav_data << sloop1 << sloop2 << endl;
														   //TO HERE
														   */
		OutStream_lists.push_back(o_stream);
		return o_stream.p;
	}

	//PART 4 : Detect TIEOFF
	else if (inst_class_list[tloc].type == "TIEOFF") {
		debug_code << "\nwe at TIEOFF";
		//cout << "\nwe at TIEOFF";

		string output_name = inst_class_list[tloc].outputs[oloc].name;
		if (output_name == "HARD0") {
			o_stream.p = 0; //probability of the signal to be one
			inst_class_list[tloc].outputs[oloc].visited = 50;
			inst_class_list[tloc].outputs[oloc].p = o_stream.p;
			OutStream_lists.push_back(o_stream);
			return o_stream.p;
		}
		else if (output_name == "HARD1" || output_name == "KEEP1") {
			o_stream.p = 1; //probability of the signal to be one
			inst_class_list[tloc].outputs[oloc].visited = 50;
			inst_class_list[tloc].outputs[oloc].p = o_stream.p;
			OutStream_lists.push_back(o_stream);
			return o_stream.p;
		}
	}

	//PART 5: Detect LUT and SLICELs
	else if (inst_class_list[tloc].type == "SLICEL") {
		//Here we Found a LUT!!
		//Identify the output and classify it with A,B,.. and Pick the right LUT
		SLICEOut outset; // we use this to find the output type
		int out_id = -1;// we use this ID to find which output are we dealing with according to the outset package
		string output_name = inst_class_list[tloc].outputs[oloc].name;

		//DISPLAY HERE
		debug_code << endl << tloc << "_" << oloc << ":" << output_name << " is: ";
		//cout << endl << tloc << "_" << oloc << ":" << output_name << " is: ";

		InstInp_Slice Input_S = inst_class_list[tloc].inp_slice;
		//string cfg_xffmux="#OFF"; //cfg_xffmux can be {AX,BX,CX,DX} , O5, O6, CY, XOR
		string precyinit = inst_class_list[tloc].precyinit.cnf;
		int i = 0;
		while (out_id == -1) {

			if (outset.out_set_name[i] == output_name) {
				out_id = i;
			}
			i++;
		}

		debug_code << outset.out_set_name[out_id] << " " << outset.out_set_case[out_id] << " " << outset.out_set_mux[out_id] << " " << outset.out_set_lut[out_id];
		//cout << outset.out_set_name[out_id] << " " << outset.out_set_case[out_id] << " " << outset.out_set_mux[out_id] << " " << outset.out_set_lut[out_id];

		int lut_num = outset.out_set_lut[out_id];// this will be 0, 1, 2, 3

		int case_this = outset.out_set_case[out_id];//it has the case id. meaning the 1)AQ,.. 2)A,... 3)AMUX,.. 4)COUT

													//===================================
													//Here we check for different types of outputs in SLICEL, OUTMUX,FFMUX,O6,COUT
		switch (case_this) {
			//=============================================
			// AQ,BQ,..

		case 1: {
			string cfg_xffmux = inst_class_list[tloc].xffmux[outset.out_set_lut[out_id]].cnf;
			// 1,2) cfg xFFMUX::CY/XOR
			debug_code << " " << cfg_xffmux << " ";
			//cout << " " << cfg_xffmux << " ";
			if (cfg_xffmux == "CY" || cfg_xffmux == "XOR") {
				//PART 4.1.1.1: Calculate the COinit or CO-1. Check PRECYINIT::0/1/AX/#OFF
				int count2it = 0;//We start from the base of the carry out and reach to the current COi that we are calculating.
				double answer2it_cy;//this will hold the final answer
				double answer2it_xor;//this will hold the final answer

									 //=========================================================
									 // CO calculation STARTS HERE
				if (count2it <= lut_num) {
					count2it++;
					//=========================================================
					// 1) COinit calculation
					if (precyinit == "#OFF") {
						if (Input_S.inpv[Input_S.cin].visited == 50) {
							o_stream.p = Traverse(Input_S.inpv[Input_S.cin].wired_tile.id, Input_S.inpv[Input_S.cin].wired_out.id, o_stream);
							// when we get to this line we have return back from the recursion so :1)we met the input 2)we met other blocks 3)we hit a loop
							inst_class_list[tloc].inp_slice.inpv[Input_S.cin].visited = 50;
							inst_class_list[tloc].inp_slice.inpv[Input_S.cin].p = o_stream.p;
							inst_class_list[tloc].coinit.p = o_stream.p;
						}
						else {
							inst_class_list[tloc].coinit.p = inst_class_list[tloc].inp_slice.inpv[Input_S.cin].p;// the pin was already visited and we just get the probability for calculations
						}
					}
					else if (precyinit == "0") {
						inst_class_list[tloc].coinit.p = 0;
					}
					else if (precyinit == "1") {
						inst_class_list[tloc].coinit.p = 1;
					}
					else if (precyinit == "AX") {
						if (Input_S.inpv[Input_S.ax].visited == 50) {
							o_stream.p = Traverse(Input_S.inpv[Input_S.ax].wired_tile.id, Input_S.inpv[Input_S.ax].wired_out.id, o_stream);
							inst_class_list[tloc].inp_slice.inpv[Input_S.ax].visited = 50;
							inst_class_list[tloc].inp_slice.inpv[Input_S.ax].p = o_stream.p;
							inst_class_list[tloc].coinit.p = o_stream.p;
						}
						else {
							inst_class_list[tloc].coinit.p = inst_class_list[tloc].inp_slice.inpv[Input_S.ax].p;// the pin was already visited and we just get the probability for calculations
						}
					}
					//=========================================================
					// 2) CO0 calculation
					double propagate, generate, co_in;// probability of propagate and generate signals in carry lookeahead
					co_in = inst_class_list[tloc].coinit.p;
					//PART: Check the ACY0
					//Calculate P(generate) 
					if (inst_class_list[tloc].acy0.cnf == "AX") {
						if (Input_S.inpv[Input_S.ax].visited == 50) {
							o_stream.p = Traverse(Input_S.inpv[Input_S.ax].wired_tile.id, Input_S.inpv[Input_S.ax].wired_out.id, o_stream);
							inst_class_list[tloc].inp_slice.inpv[Input_S.ax].visited = 50;
							inst_class_list[tloc].inp_slice.inpv[Input_S.ax].p = o_stream.p;
							generate = o_stream.p;
						}
						else {
							generate = inst_class_list[tloc].inp_slice.inpv[Input_S.ax].p;// the pin was already visited and we just get the probability for calculations
						}
					}
					else if (inst_class_list[tloc].acy0.cnf == "O5") {//LUTA
						vector<int> input_seq;
						for (int i2 = 0; i2 < Input_S.inpvecList.size(); i2++) {//check whick of the inputs exist int the list. inpvecList has the input IDs that were used
							if (Input_S.inpvecList[i2] >= Input_S.a1 && Input_S.inpvecList[i2] <= Input_S.a5) {//if the ID is in the selecting range we pick it
								int i3 = Input_S.inpvecList[i2];
								input_seq.push_back(i3);
								//input_seq.push_back(i2);
								if (Input_S.inpv[i3].visited == 50) {//check if it was visited before
									o_stream.p = Traverse(Input_S.inpv[i3].wired_tile.id, Input_S.inpv[i3].wired_out.id, o_stream);
									// when we get to this line we have return back from the recursion so :1)we met the input 2)we met other blocks 3)we hit a loop
									inst_class_list[tloc].inp_slice.inpv[i3].visited = 50;//When we calculated the probability of the input, we set the input as visited in this for loop.
									inst_class_list[tloc].inp_slice.inpv[i3].p = o_stream.p;
								}
							}
						}

						//out_set_lut[out_id] * 6 + 5;
						vector<double>p = {};
						for (int i = Input_S.a1; i <= Input_S.a5; i++) {//for all the inputs of the LUT5 we get their probability
							p.push_back(inst_class_list[tloc].inp_slice.inpv[i].p);
						}
						generate = lut_Prob_Top(inst_class_list[tloc].lut_a.l5_init, p);//+++++++XXX
					}

					//Calculate the P(propagate). the O6 is in the select line
					vector<int> input_seq;
					for (int i2 = 0; i2 < Input_S.inpvecList.size(); i2++) {//check whick of the inputs exist int the list. inpvecList has the input IDs that were used
						if (Input_S.inpvecList[i2] >= Input_S.a1 && Input_S.inpvecList[i2] <= Input_S.a6) {//if the ID is in the selecting range we pick it
							int i3 = Input_S.inpvecList[i2];
							input_seq.push_back(i3);
							//input_seq.push_back(i2);
							if (Input_S.inpv[i3].visited == 50) {//check if it was visited before
								o_stream.p = Traverse(Input_S.inpv[i3].wired_tile.id, Input_S.inpv[i3].wired_out.id, o_stream);
								// when we get to this line we have return back from the recursion so :1)we met the input 2)we met other blocks 3)we hit a loop
								inst_class_list[tloc].inp_slice.inpv[i3].visited = 50;//When we calculated the probability of the input, we set the input as visited in this for loop.
								inst_class_list[tloc].inp_slice.inpv[i3].p = o_stream.p;
							}
						}
					}

					//out_set_lut[out_id] * 6 + 5;
					vector<double>p = {};
					for (int i = Input_S.a1; i <= Input_S.a6; i++) {//for all the inputs of the LUT6 we get their probability
						p.push_back(inst_class_list[tloc].inp_slice.inpv[i].p);
					}
					propagate = lut_Prob_Top(inst_class_list[tloc].lut_a.l6_init, p);

					inst_class_list[tloc].co0.p = ((1 - propagate)*generate) + (propagate*co_in);

					// Upto here we have calculated the P(CO0)
					answer2it_cy = inst_class_list[tloc].co0.p;
					answer2it_xor = ((1 - propagate)*co_in) + (propagate*(1 - co_in));
				}
				//=========================================================
				// 3) CO1 calculation
				if (count2it <= lut_num) {
					count2it++;
					double propagate = 1, generate = 1, co_in = 1;// probability of propagate and generate signals in carry lookeahead
					co_in = inst_class_list[tloc].co0.p;
					//PART: Check the BCY0
					//Calculate P(generate) 
					if (inst_class_list[tloc].bcy0.cnf == "BX") {
						if (Input_S.inpv[Input_S.bx].visited == 50) {
							o_stream.p = Traverse(Input_S.inpv[Input_S.bx].wired_tile.id, Input_S.inpv[Input_S.bx].wired_out.id, o_stream);
							inst_class_list[tloc].inp_slice.inpv[Input_S.bx].visited = 50;
							inst_class_list[tloc].inp_slice.inpv[Input_S.bx].p = o_stream.p;
							generate = o_stream.p;
						}
						else {
							generate = inst_class_list[tloc].inp_slice.inpv[Input_S.bx].p;// the pin was already visited and we just get the probability for calculations
						}
					}
					else if (inst_class_list[tloc].bcy0.cnf == "O5") {//LUTB
						vector<int> input_seq;
						for (int i2 = 0; i2 < Input_S.inpvecList.size(); i2++) {//check whick of the inputs exist int the list. inpvecList has the input IDs that were used
							if (Input_S.inpvecList[i2] >= Input_S.b1 && Input_S.inpvecList[i2] <= Input_S.b5) {//if the ID is in the selecting range we pick it
								int i3 = Input_S.inpvecList[i2];
								input_seq.push_back(i3);
								//input_seq.push_back(i2);
								if (Input_S.inpv[i3].visited == 50) {//check if it was visited before
									o_stream.p = Traverse(Input_S.inpv[i3].wired_tile.id, Input_S.inpv[i3].wired_out.id, o_stream);
									// when we get to this line we have return back from the recursion so :1)we met the input 2)we met other blocks 3)we hit a loop
									inst_class_list[tloc].inp_slice.inpv[i3].visited = 50;//When we calculated the probability of the input, we set the input as visited in this for loop.
									inst_class_list[tloc].inp_slice.inpv[i3].p = o_stream.p;
								}
							}
						}

						//out_set_lut[out_id] * 6 + 5;
						vector<double>p = {};
						for (int i = Input_S.b1; i <= Input_S.b5; i++) {//for all the inputs of the LUT5 we get their probability
							p.push_back(inst_class_list[tloc].inp_slice.inpv[i].p);
						}
						generate = lut_Prob_Top(inst_class_list[tloc].lut_b.l5_init, p);
					}

					//Calculate the P(propagate). the O6 is in the select line
					vector<int> input_seq;
					for (int i2 = 0; i2 < Input_S.inpvecList.size(); i2++) {//check whick of the inputs exist int the list. inpvecList has the input IDs that were used
						if (Input_S.inpvecList[i2] >= Input_S.b1 && Input_S.inpvecList[i2] <= Input_S.b6) {//if the ID is in the selecting range we pick it
							int i3 = Input_S.inpvecList[i2];
							input_seq.push_back(i3);
							//input_seq.push_back(i2);
							if (Input_S.inpv[i3].visited == 50) {//check if it was visited before
								o_stream.p = Traverse(Input_S.inpv[i3].wired_tile.id, Input_S.inpv[i3].wired_out.id, o_stream);
								// when we get to this line we have return back from the recursion so :1)we met the input 2)we met other blocks 3)we hit a loop
								inst_class_list[tloc].inp_slice.inpv[i3].visited = 50;//When we calculated the probability of the input, we set the input as visited in this for loop.
								inst_class_list[tloc].inp_slice.inpv[i3].p = o_stream.p;
							}
						}
					}

					//out_set_lut[out_id] * 6 + 5;
					vector<double>p = {};
					for (int i = Input_S.b1; i <= Input_S.b6; i++) {//for all the inputs of the LUT6 we get their probability
						p.push_back(inst_class_list[tloc].inp_slice.inpv[i].p);
					}
					propagate = lut_Prob_Top(inst_class_list[tloc].lut_b.l6_init, p);

					inst_class_list[tloc].co1.p = ((1 - propagate)*generate) + (propagate*co_in);

					// Upto here we have calculated the P(CO1)
					answer2it_cy = inst_class_list[tloc].co1.p;
					answer2it_xor = ((1 - propagate)*co_in) + (propagate*(1 - co_in));
				}
				//=========================================================
				// 4) CO2 calculation
				if (count2it <= lut_num) {
					count2it++;
					double propagate, generate, co_in;// probability of propagate and generate signals in carry lookeahead
					co_in = inst_class_list[tloc].co1.p;
					//PART: Check the CCY0
					//Calculate P(generate) 
					if (inst_class_list[tloc].ccy0.cnf == "CX") {
						if (Input_S.inpv[Input_S.cx].visited == 50) {
							o_stream.p = Traverse(Input_S.inpv[Input_S.cx].wired_tile.id, Input_S.inpv[Input_S.cx].wired_out.id, o_stream);
							inst_class_list[tloc].inp_slice.inpv[Input_S.cx].visited = 50;
							inst_class_list[tloc].inp_slice.inpv[Input_S.cx].p = o_stream.p;
							generate = o_stream.p;
						}
						else {
							generate = inst_class_list[tloc].inp_slice.inpv[Input_S.cx].p;// the pin was already visited and we just get the probability for calculations
						}
					}
					else if (inst_class_list[tloc].ccy0.cnf == "O5") {//LUTB
						vector<int> input_seq;
						for (int i2 = 0; i2 < Input_S.inpvecList.size(); i2++) {//check whick of the inputs exist int the list. inpvecList has the input IDs that were used
							if (Input_S.inpvecList[i2] >= Input_S.c1 && Input_S.inpvecList[i2] <= Input_S.c5) {//if the ID is in the selecting range we pick it
								int i3 = Input_S.inpvecList[i2];
								input_seq.push_back(i3);
								//input_seq.push_back(i2);
								if (Input_S.inpv[i3].visited == 50) {//check if it was visited before
									o_stream.p = Traverse(Input_S.inpv[i3].wired_tile.id, Input_S.inpv[i3].wired_out.id, o_stream);
									// when we get to this line we have return back from the recursion so :1)we met the input 2)we met other blocks 3)we hit a loop
									inst_class_list[tloc].inp_slice.inpv[i3].visited = 50;//When we calculated the probability of the input, we set the input as visited in this for loop.
									inst_class_list[tloc].inp_slice.inpv[i3].p = o_stream.p;
								}
							}
						}

						//out_set_lut[out_id] * 6 + 5;
						vector<double>p = {};
						for (int i = Input_S.c1; i <= Input_S.c5; i++) {//for all the inputs of the LUT5 we get their probability
							p.push_back(inst_class_list[tloc].inp_slice.inpv[i].p);
						}
						generate = lut_Prob_Top(inst_class_list[tloc].lut_c.l5_init, p);
					}

					//Calculate the P(propagate). the O6 is in the select line
					vector<int> input_seq;
					for (int i2 = 0; i2 < Input_S.inpvecList.size(); i2++) {//check whick of the inputs exist int the list. inpvecList has the input IDs that were used
						if (Input_S.inpvecList[i2] >= Input_S.c1 && Input_S.inpvecList[i2] <= Input_S.c6) {//if the ID is in the selecting range we pick it
							int i3 = Input_S.inpvecList[i2];
							input_seq.push_back(i3);
							//input_seq.push_back(i2);
							if (Input_S.inpv[i3].visited == 50) {//check if it was visited before
								o_stream.p = Traverse(Input_S.inpv[i3].wired_tile.id, Input_S.inpv[i3].wired_out.id, o_stream);
								// when we get to this line we have return back from the recursion so :1)we met the input 2)we met other blocks 3)we hit a loop
								inst_class_list[tloc].inp_slice.inpv[i3].visited = 50;//When we calculated the probability of the input, we set the input as visited in this for loop.
								inst_class_list[tloc].inp_slice.inpv[i3].p = o_stream.p;
							}
						}
					}

					//out_set_lut[out_id] * 6 + 5;
					vector<double>p = {};
					for (int i = Input_S.c1; i <= Input_S.c6; i++) {//for all the inputs of the LUT6 we get their probability
						p.push_back(inst_class_list[tloc].inp_slice.inpv[i].p);
					}
					propagate = lut_Prob_Top(inst_class_list[tloc].lut_c.l6_init, p);

					inst_class_list[tloc].co2.p = ((1 - propagate)*generate) + (propagate*co_in);

					// Upto here we have calculated the P(C2)
					answer2it_cy = inst_class_list[tloc].co2.p;
					answer2it_xor = ((1 - propagate)*co_in) + (propagate*(1 - co_in));
				}
				//=========================================================
				// 5) CO3 calculation
				if (count2it <= lut_num) {
					count2it++;
					double propagate, generate, co_in;// probability of propagate and generate signals in carry lookeahead
					co_in = inst_class_list[tloc].co2.p;
					//PART: Check the DCY0
					//Calculate P(generate) 
					if (inst_class_list[tloc].dcy0.cnf == "DX") {
						if (Input_S.inpv[Input_S.dx].visited == 50) {
							o_stream.p = Traverse(Input_S.inpv[Input_S.dx].wired_tile.id, Input_S.inpv[Input_S.dx].wired_out.id, o_stream);
							inst_class_list[tloc].inp_slice.inpv[Input_S.dx].visited = 50;
							inst_class_list[tloc].inp_slice.inpv[Input_S.dx].p = o_stream.p;
							generate = o_stream.p;
						}
						else {
							generate = inst_class_list[tloc].inp_slice.inpv[Input_S.dx].p;// the pin was already visited and we just get the probability for calculations
						}
					}
					else if (inst_class_list[tloc].dcy0.cnf == "O5") {//LUTB
						vector<int> input_seq;
						for (int i2 = 0; i2 < Input_S.inpvecList.size(); i2++) {//check whick of the inputs exist int the list. inpvecList has the input IDs that were used
							if (Input_S.inpvecList[i2] >= Input_S.d1 && Input_S.inpvecList[i2] <= Input_S.d5) {//if the ID is in the selecting range we pick it
								int i3 = Input_S.inpvecList[i2];
								input_seq.push_back(i3);
								//input_seq.push_back(i2);
								if (Input_S.inpv[i3].visited == 50) {//check if it was visited before
									o_stream.p = Traverse(Input_S.inpv[i3].wired_tile.id, Input_S.inpv[i3].wired_out.id, o_stream);
									// when we get to this line we have return back from the recursion so :1)we met the input 2)we met other blocks 3)we hit a loop
									inst_class_list[tloc].inp_slice.inpv[i3].visited = 50;//When we calculated the probability of the input, we set the input as visited in this for loop.
									inst_class_list[tloc].inp_slice.inpv[i3].p = o_stream.p;
								}
							}
						}

						//out_set_lut[out_id] * 6 + 5;
						vector<double>p = {};
						for (int i = Input_S.d1; i <= Input_S.d5; i++) {//for all the inputs of the LUT5 we get their probability
							p.push_back(inst_class_list[tloc].inp_slice.inpv[i].p);
						}
						generate = lut_Prob_Top(inst_class_list[tloc].lut_d.l5_init, p);
					}

					//Calculate the P(propagate). the O6 is in the select line
					vector<int> input_seq;
					for (int i2 = 0; i2 < Input_S.inpvecList.size(); i2++) {//check whick of the inputs exist int the list. inpvecList has the input IDs that were used
						if (Input_S.inpvecList[i2] >= Input_S.d1 && Input_S.inpvecList[i2] <= Input_S.d6) {//if the ID is in the selecting range we pick it
							int i3 = Input_S.inpvecList[i2];
							input_seq.push_back(i3);
							//input_seq.push_back(i2);
							if (Input_S.inpv[i3].visited == 50) {//check if it was visited before
								o_stream.p = Traverse(Input_S.inpv[i3].wired_tile.id, Input_S.inpv[i3].wired_out.id, o_stream);
								// when we get to this line we have return back from the recursion so :1)we met the input 2)we met other blocks 3)we hit a loop
								inst_class_list[tloc].inp_slice.inpv[i3].visited = 50;//When we calculated the probability of the input, we set the input as visited in this for loop.
								inst_class_list[tloc].inp_slice.inpv[i3].p = o_stream.p;
							}
						}
					}

					//out_set_lut[out_id] * 6 + 5;
					vector<double>p = {};
					for (int i = Input_S.d1; i <= Input_S.d6; i++) {//for all the inputs of the LUT6 we get their probability
						p.push_back(inst_class_list[tloc].inp_slice.inpv[i].p);
					}
					propagate = lut_Prob_Top(inst_class_list[tloc].lut_d.l6_init, p);

					inst_class_list[tloc].co3.p = ((1 - propagate)*generate) + (propagate*co_in);

					// Upto here we have calculated the P(CO3)
					answer2it_cy = inst_class_list[tloc].co3.p;
					answer2it_xor = ((1 - propagate)*co_in) + (propagate*(1 - co_in));

					//Setting the Probability of the output COUT
					for (int i = 0; i < inst_class_list[tloc].outputs.size(); i++) {
						if (inst_class_list[tloc].outputs[i].name == "COUT") {
							inst_class_list[tloc].outputs[i].visited = 50;
							inst_class_list[tloc].outputs[i].p = inst_class_list[tloc].co3.p;
							o_stream.tile.push_back(tloc); o_stream.out.push_back(i);//this output is dependent and its probability has calculated.
						}
					}
				}

				//answer2it has the right value for the probability
				if (cfg_xffmux == "CY")
					o_stream.p = answer2it_cy;
				else if (cfg_xffmux == "XOR")
					o_stream.p = answer2it_xor;

				inst_class_list[tloc].outputs[oloc].visited = 50;
				inst_class_list[tloc].outputs[oloc].p = o_stream.p;
				return o_stream.p;
			}

			//3) cfg xFFMUX::xX
			//ax = 24, bx = 25, cx = 26, dx = 27
			if (cfg_xffmux == "AX" || cfg_xffmux == "BX" || cfg_xffmux == "CX" || cfg_xffmux == "DX") {
				int xX_loc = 24 + outset.out_set_lut[out_id]; //this will pick the input to be ax or .. based on the LUT number
				if (Input_S.inpv[xX_loc].visited == 50) {
					o_stream.p = Traverse(Input_S.inpv[xX_loc].wired_tile.id, Input_S.inpv[xX_loc].wired_out.id, o_stream);

					inst_class_list[tloc].inp_slice.inpv[xX_loc].visited = 50;
					inst_class_list[tloc].inp_slice.inpv[xX_loc].p = o_stream.p;//o_stream.p holds the value of probability of the wired output
					inst_class_list[tloc].outputs[oloc].visited = 50;
					inst_class_list[tloc].outputs[oloc].p = o_stream.p;
					return o_stream.p;
				}
				else {
					o_stream.p = Input_S.inpv[xX_loc].p;
					inst_class_list[tloc].outputs[oloc].visited = 50;
					inst_class_list[tloc].outputs[oloc].p = o_stream.p;
					return o_stream.p;
				}
			}

			//4,5) cfg xFFMUX::O5/O6
			if (cfg_xffmux == "O5" || cfg_xffmux == "O6") {

				string current_lut;
				vector<int> i_list = { 0,0 }; //list of defult positions to look for available input
				i_list[0] = outset.out_set_lut[out_id] * 6;
				if (cfg_xffmux == "O5") {
					i_list[1] = outset.out_set_lut[out_id] * 6 + 5;
					current_lut = inst_class_list[tloc].luti[outset.out_set_lut[out_id]].l5_init;
				}
				else if (cfg_xffmux == "O6") {
					i_list[1] = outset.out_set_lut[out_id] * 6 + 5;
					current_lut = inst_class_list[tloc].luti[outset.out_set_lut[out_id]].l6_init;
				}

				vector<int> input_seq;
				for (int i2 = 0; i2 < Input_S.inpvecList.size(); i2++) {//check whick of the inputs exist int the list. inpvecList has the input IDs that were used
					if (Input_S.inpvecList[i2] >= i_list[0] && Input_S.inpvecList[i2] <= i_list[1]) {//if the ID is in the selecting range we pick it
						int i3 = Input_S.inpvecList[i2];
						input_seq.push_back(i3);
						//input_seq.push_back(i2);
						if (Input_S.inpv[i3].visited == 50) {//check if it was visited before
							o_stream.p = Traverse(Input_S.inpv[i3].wired_tile.id, Input_S.inpv[i3].wired_out.id, o_stream);
							// when we get to this line we have return back from the recursion so :1)we met the input 2)we met other blocks 3)we hit a loop
							inst_class_list[tloc].inp_slice.inpv[i3].visited = 50;//When we calculated the probability of the input, we set the input as visited in this for loop.
							inst_class_list[tloc].inp_slice.inpv[i3].p = o_stream.p;
						}
						// we later take care of the already visited ones right before the return
					}
				}

				//out_set_lut[out_id] * 6 + 5;
				vector<double>p = {};
				for (int i = i_list[0]; i <= i_list[1]; i++) {//for all the inputs of the LUT5 we get their probability there are some that were not even included. they have P=0
					p.push_back(inst_class_list[tloc].inp_slice.inpv[i].p);
				}

				o_stream.p = lut_Prob_Top(current_lut, p); //current_lut holds the lut info relating to the current output.
				inst_class_list[tloc].outputs[oloc].visited = 50;
				inst_class_list[tloc].outputs[oloc].p = o_stream.p;
				return o_stream.p;
			}
		}

				//=============================================
				//A,B,... we look at the O6 
		case 2: {
			if (inst_class_list[tloc].xused[outset.out_set_lut[out_id]].cnf == "0") {

				vector<int> i_list = { 0,0 }; //list of defult positions to look for available input
				i_list[0] = outset.out_set_lut[out_id] * 6;
				i_list[1] = outset.out_set_lut[out_id] * 6 + 5;
				string current_lut = inst_class_list[tloc].luti[outset.out_set_lut[out_id]].l6_init;
				vector<int> input_seq;
				//we should not look at the i2 we should look at the inpvec list(i2)
				for (int i2 = 0; i2 < Input_S.inpvecList.size(); i2++) {//check whick of the inputs exist int the list. inpvecList has the input IDs that were used
					if (Input_S.inpvecList[i2] >= i_list[0] && Input_S.inpvecList[i2] <= i_list[1]) {//if the ID is in the selecting range we pick it
						int i3 = Input_S.inpvecList[i2];
						input_seq.push_back(i3);
						if (Input_S.inpv[i3].visited == 50) {//check if it was visited before
							o_stream.p = Traverse(Input_S.inpv[i3].wired_tile.id, Input_S.inpv[i3].wired_out.id, o_stream);
							// when we get to this line we have return back from the recursion so :1)we met the input 2)we met other blocks 3)we hit a loop
							inst_class_list[tloc].inp_slice.inpv[i3].visited = 50;//When we calculated the probability of the input, we set the input as visited in this for loop.
							inst_class_list[tloc].inp_slice.inpv[i3].p = o_stream.p;
						}
					}
				}

				//out_set_lut[out_id] * 6 + 5;
				vector<double>p = {};
				for (int i = i_list[0]; i <= i_list[1]; i++) {//for all the inputs of the LUT5 we get their probability there are some that were not even included. they have P=0
					p.push_back(inst_class_list[tloc].inp_slice.inpv[i].p);
				}

				o_stream.p = lut_Prob_Top(current_lut, p); //current_lut holds the lut info relating to the current output.
				inst_class_list[tloc].outputs[oloc].visited = 50;
				inst_class_list[tloc].outputs[oloc].p = o_stream.p;
				return o_stream.p;
			}

		}

				//=============================================
				//AMUX,BMUX,..

		case 3: {
			string cfg_xoutmux = inst_class_list[tloc].xoutmux[outset.out_set_lut[out_id]].cnf;//here we pick which
																							   // 1,2) cfg xFFMUX::CY/XOR
			if (cfg_xoutmux == "CY" || cfg_xoutmux == "XOR") {
				//PART 4.1.1.1: Calculate the COinit or CO-1. Check PRECYINIT::0/1/AX/#OFF
				int count2it = 0;//We start from the base of the carry out and reach to the current COi that we are calculating.
				double answer2it_cy;//this will hold the final answer
				double answer2it_xor;//this will hold the final answer

									 //=========================================================
									 // CO calculation STARTS HERE
				if (count2it <= lut_num) {
					count2it++;
					//=========================================================
					// 1) COinit calculation
					if (precyinit == "#OFF") {
						if (Input_S.inpv[Input_S.cin].visited == 50) {
							o_stream.p = Traverse(Input_S.inpv[Input_S.cin].wired_tile.id, Input_S.inpv[Input_S.cin].wired_out.id, o_stream);
							// when we get to this line we have return back from the recursion so :1)we met the input 2)we met other blocks 3)we hit a loop
							inst_class_list[tloc].inp_slice.inpv[Input_S.cin].visited = 50;
							inst_class_list[tloc].inp_slice.inpv[Input_S.cin].p = o_stream.p;
							inst_class_list[tloc].coinit.p = o_stream.p;
						}
						else {
							inst_class_list[tloc].coinit.p = inst_class_list[tloc].inp_slice.inpv[Input_S.cin].p;// the pin was already visited and we just get the probability for calculations
						}
					}
					else if (precyinit == "0") {
						inst_class_list[tloc].coinit.p = 0;
					}
					else if (precyinit == "1") {
						inst_class_list[tloc].coinit.p = 1;
					}
					else if (precyinit == "AX") {
						if (Input_S.inpv[Input_S.ax].visited == 50) {
							o_stream.p = Traverse(Input_S.inpv[Input_S.ax].wired_tile.id, Input_S.inpv[Input_S.ax].wired_out.id, o_stream);
							inst_class_list[tloc].inp_slice.inpv[Input_S.ax].visited = 50;
							inst_class_list[tloc].inp_slice.inpv[Input_S.ax].p = o_stream.p;
							inst_class_list[tloc].coinit.p = o_stream.p;
						}
						else {
							inst_class_list[tloc].coinit.p = inst_class_list[tloc].inp_slice.inpv[Input_S.ax].p;// the pin was already visited and we just get the probability for calculations
						}
					}
					//=========================================================
					// 2) CO0 calculation
					double propagate, generate, co_in;// probability of propagate and generate signals in carry lookeahead
					co_in = inst_class_list[tloc].coinit.p;
					//PART: Check the ACY0
					//Calculate P(generate) 
					if (inst_class_list[tloc].acy0.cnf == "AX") {
						if (Input_S.inpv[Input_S.ax].visited == 50) {
							o_stream.p = Traverse(Input_S.inpv[Input_S.ax].wired_tile.id, Input_S.inpv[Input_S.ax].wired_out.id, o_stream);
							inst_class_list[tloc].inp_slice.inpv[Input_S.ax].visited = 50;
							inst_class_list[tloc].inp_slice.inpv[Input_S.ax].p = o_stream.p;
							generate = o_stream.p;
						}
						else {
							generate = inst_class_list[tloc].inp_slice.inpv[Input_S.ax].p;// the pin was already visited and we just get the probability for calculations
						}
					}
					else if (inst_class_list[tloc].acy0.cnf == "O5") {//LUTA
						vector<int> input_seq;
						for (int i2 = 0; i2 < Input_S.inpvecList.size(); i2++) {//check whick of the inputs exist int the list. inpvecList has the input IDs that were used
							if (Input_S.inpvecList[i2] >= Input_S.a1 && Input_S.inpvecList[i2] <= Input_S.a5) {//if the ID is in the selecting range we pick it
								int i3 = Input_S.inpvecList[i2];
								input_seq.push_back(i3);
								//input_seq.push_back(i2);
								if (Input_S.inpv[i3].visited == 50) {//check if it was visited before
									o_stream.p = Traverse(Input_S.inpv[i3].wired_tile.id, Input_S.inpv[i3].wired_out.id, o_stream);
									// when we get to this line we have return back from the recursion so :1)we met the input 2)we met other blocks 3)we hit a loop
									inst_class_list[tloc].inp_slice.inpv[i3].visited = 50;//When we calculated the probability of the input, we set the input as visited in this for loop.
									inst_class_list[tloc].inp_slice.inpv[i3].p = o_stream.p;
								}
							}
						}

						//out_set_lut[out_id] * 6 + 5;
						vector<double>p = {};
						for (int i = Input_S.a1; i <= Input_S.a5; i++) {//for all the inputs of the LUT5 we get their probability
							p.push_back(inst_class_list[tloc].inp_slice.inpv[i].p);
						}
						generate = lut_Prob_Top(inst_class_list[tloc].lut_a.l5_init, p);
					}

					//Calculate the P(propagate). the O6 is in the select line
					vector<int> input_seq;
					for (int i2 = 0; i2 < Input_S.inpvecList.size(); i2++) {//check whick of the inputs exist int the list. inpvecList has the input IDs that were used
						if (Input_S.inpvecList[i2] >= Input_S.a1 && Input_S.inpvecList[i2] <= Input_S.a6) {//if the ID is in the selecting range we pick it
							int i3 = Input_S.inpvecList[i2];
							input_seq.push_back(i3);
							//input_seq.push_back(i2);
							if (Input_S.inpv[i3].visited == 50) {//check if it was visited before
								o_stream.p = Traverse(Input_S.inpv[i3].wired_tile.id, Input_S.inpv[i3].wired_out.id, o_stream);
								// when we get to this line we have return back from the recursion so :1)we met the input 2)we met other blocks 3)we hit a loop
								inst_class_list[tloc].inp_slice.inpv[i3].visited = 50;//When we calculated the probability of the input, we set the input as visited in this for loop.
								inst_class_list[tloc].inp_slice.inpv[i3].p = o_stream.p;
							}
						}
					}

					//out_set_lut[out_id] * 6 + 5;
					vector<double>p = {};
					for (int i = Input_S.a1; i <= Input_S.a6; i++) {//for all the inputs of the LUT6 we get their probability
						p.push_back(inst_class_list[tloc].inp_slice.inpv[i].p);
					}
					propagate = lut_Prob_Top(inst_class_list[tloc].lut_a.l6_init, p);

					inst_class_list[tloc].co0.p = ((1 - propagate)*generate) + (propagate*co_in);

					// Upto here we have calculated the P(CO0)
					answer2it_cy = inst_class_list[tloc].co0.p;
					answer2it_xor = ((1 - propagate)*co_in) + (propagate*(1 - co_in));
				}
				//=========================================================
				// 3) CO1 calculation
				if (count2it <= lut_num) {
					count2it++;
					double propagate, generate, co_in;// probability of propagate and generate signals in carry lookeahead
					co_in = inst_class_list[tloc].co0.p;
					//PART: Check the BCY0
					//Calculate P(generate) 
					if (inst_class_list[tloc].bcy0.cnf == "BX") {
						if (Input_S.inpv[Input_S.bx].visited == 50) {
							o_stream.p = Traverse(Input_S.inpv[Input_S.bx].wired_tile.id, Input_S.inpv[Input_S.bx].wired_out.id, o_stream);
							inst_class_list[tloc].inp_slice.inpv[Input_S.bx].visited = 50;
							inst_class_list[tloc].inp_slice.inpv[Input_S.bx].p = o_stream.p;
							generate = o_stream.p;
						}
						else {
							generate = inst_class_list[tloc].inp_slice.inpv[Input_S.bx].p;// the pin was already visited and we just get the probability for calculations
						}
					}
					else if (inst_class_list[tloc].bcy0.cnf == "O5") {//LUTB
						vector<int> input_seq;
						for (int i2 = 0; i2 < Input_S.inpvecList.size(); i2++) {//check whick of the inputs exist int the list. inpvecList has the input IDs that were used
							if (Input_S.inpvecList[i2] >= Input_S.b1 && Input_S.inpvecList[i2] <= Input_S.b5) {//if the ID is in the selecting range we pick it
								int i3 = Input_S.inpvecList[i2];
								input_seq.push_back(i3);
								//input_seq.push_back(i2);
								if (Input_S.inpv[i3].visited == 50) {//check if it was visited before
									o_stream.p = Traverse(Input_S.inpv[i3].wired_tile.id, Input_S.inpv[i3].wired_out.id, o_stream);
									// when we get to this line we have return back from the recursion so :1)we met the input 2)we met other blocks 3)we hit a loop
									inst_class_list[tloc].inp_slice.inpv[i3].visited = 50;//When we calculated the probability of the input, we set the input as visited in this for loop.
									inst_class_list[tloc].inp_slice.inpv[i3].p = o_stream.p;
								}
							}
						}

						//out_set_lut[out_id] * 6 + 5;
						vector<double>p = {};
						for (int i = Input_S.b1; i <= Input_S.b5; i++) {//for all the inputs of the LUT5 we get their probability
							p.push_back(inst_class_list[tloc].inp_slice.inpv[i].p);
						}
						generate = lut_Prob_Top(inst_class_list[tloc].lut_b.l5_init, p);
					}

					//Calculate the P(propagate). the O6 is in the select line
					vector<int> input_seq;
					for (int i2 = 0; i2 < Input_S.inpvecList.size(); i2++) {//check whick of the inputs exist int the list. inpvecList has the input IDs that were used
						if (Input_S.inpvecList[i2] >= Input_S.b1 && Input_S.inpvecList[i2] <= Input_S.b6) {//if the ID is in the selecting range we pick it
							int i3 = Input_S.inpvecList[i2];
							input_seq.push_back(i3);
							//input_seq.push_back(i2);
							if (Input_S.inpv[i3].visited == 50) {//check if it was visited before
								o_stream.p = Traverse(Input_S.inpv[i3].wired_tile.id, Input_S.inpv[i3].wired_out.id, o_stream);
								// when we get to this line we have return back from the recursion so :1)we met the input 2)we met other blocks 3)we hit a loop
								inst_class_list[tloc].inp_slice.inpv[i3].visited = 50;//When we calculated the probability of the input, we set the input as visited in this for loop.
								inst_class_list[tloc].inp_slice.inpv[i3].p = o_stream.p;
							}
						}
					}

					//out_set_lut[out_id] * 6 + 5;
					vector<double>p = {};
					for (int i = Input_S.b1; i <= Input_S.b6; i++) {//for all the inputs of the LUT6 we get their probability
						p.push_back(inst_class_list[tloc].inp_slice.inpv[i].p);
					}
					propagate = lut_Prob_Top(inst_class_list[tloc].lut_b.l6_init, p);

					inst_class_list[tloc].co1.p = ((1 - propagate)*generate) + (propagate*co_in);

					// Upto here we have calculated the P(CO1)
					answer2it_cy = inst_class_list[tloc].co1.p;
					answer2it_xor = ((1 - propagate)*co_in) + (propagate*(1 - co_in));
				}
				//=========================================================
				// 4) CO2 calculation
				if (count2it <= lut_num) {
					count2it++;
					double propagate, generate, co_in;// probability of propagate and generate signals in carry lookeahead
					co_in = inst_class_list[tloc].co1.p;
					//PART: Check the CCY0
					//Calculate P(generate) 
					if (inst_class_list[tloc].ccy0.cnf == "CX") {
						if (Input_S.inpv[Input_S.cx].visited == 50) {
							o_stream.p = Traverse(Input_S.inpv[Input_S.cx].wired_tile.id, Input_S.inpv[Input_S.cx].wired_out.id, o_stream);
							inst_class_list[tloc].inp_slice.inpv[Input_S.cx].visited = 50;
							inst_class_list[tloc].inp_slice.inpv[Input_S.cx].p = o_stream.p;
							generate = o_stream.p;
						}
						else {
							generate = inst_class_list[tloc].inp_slice.inpv[Input_S.cx].p;// the pin was already visited and we just get the probability for calculations
						}
					}
					else if (inst_class_list[tloc].ccy0.cnf == "O5") {//LUTB
						vector<int> input_seq;
						for (int i2 = 0; i2 < Input_S.inpvecList.size(); i2++) {//check whick of the inputs exist int the list. inpvecList has the input IDs that were used
							if (Input_S.inpvecList[i2] >= Input_S.c1 && Input_S.inpvecList[i2] <= Input_S.c5) {//if the ID is in the selecting range we pick it
								int i3 = Input_S.inpvecList[i2];
								input_seq.push_back(i3);
								//input_seq.push_back(i2);
								if (Input_S.inpv[i3].visited == 50) {//check if it was visited before
									o_stream.p = Traverse(Input_S.inpv[i3].wired_tile.id, Input_S.inpv[i3].wired_out.id, o_stream);
									// when we get to this line we have return back from the recursion so :1)we met the input 2)we met other blocks 3)we hit a loop
									inst_class_list[tloc].inp_slice.inpv[i3].visited = 50;//When we calculated the probability of the input, we set the input as visited in this for loop.
									inst_class_list[tloc].inp_slice.inpv[i3].p = o_stream.p;
								}
							}
						}

						//out_set_lut[out_id] * 6 + 5;
						vector<double>p = {};
						for (int i = Input_S.c1; i <= Input_S.c5; i++) {//for all the inputs of the LUT5 we get their probability
							p.push_back(inst_class_list[tloc].inp_slice.inpv[i].p);
						}
						generate = lut_Prob_Top(inst_class_list[tloc].lut_c.l5_init, p);
					}

					//Calculate the P(propagate). the O6 is in the select line
					vector<int> input_seq;
					for (int i2 = 0; i2 < Input_S.inpvecList.size(); i2++) {//check whick of the inputs exist int the list. inpvecList has the input IDs that were used
						if (Input_S.inpvecList[i2] >= Input_S.c1 && Input_S.inpvecList[i2] <= Input_S.c6) {//if the ID is in the selecting range we pick it
							int i3 = Input_S.inpvecList[i2];
							input_seq.push_back(i3);
							//input_seq.push_back(i2);
							if (Input_S.inpv[i3].visited == 50) {//check if it was visited before
								o_stream.p = Traverse(Input_S.inpv[i3].wired_tile.id, Input_S.inpv[i3].wired_out.id, o_stream);
								// when we get to this line we have return back from the recursion so :1)we met the input 2)we met other blocks 3)we hit a loop
								inst_class_list[tloc].inp_slice.inpv[i3].visited = 50;//When we calculated the probability of the input, we set the input as visited in this for loop.
								inst_class_list[tloc].inp_slice.inpv[i3].p = o_stream.p;
							}
						}
					}

					//out_set_lut[out_id] * 6 + 5;
					vector<double>p = {};
					for (int i = Input_S.c1; i <= Input_S.c6; i++) {//for all the inputs of the LUT6 we get their probability
						p.push_back(inst_class_list[tloc].inp_slice.inpv[i].p);
					}
					propagate = lut_Prob_Top(inst_class_list[tloc].lut_c.l6_init, p);

					inst_class_list[tloc].co2.p = ((1 - propagate)*generate) + (propagate*co_in);

					// Upto here we have calculated the P(C2)
					answer2it_cy = inst_class_list[tloc].co2.p;
					answer2it_xor = ((1 - propagate)*co_in) + (propagate*(1 - co_in));
				}
				//=========================================================
				// 5) CO3 calculation
				if (count2it <= lut_num) {
					count2it++;
					double propagate, generate, co_in;// probability of propagate and generate signals in carry lookeahead
					co_in = inst_class_list[tloc].co2.p;
					//PART: Check the DCY0
					//Calculate P(generate) 
					if (inst_class_list[tloc].dcy0.cnf == "DX") {
						if (Input_S.inpv[Input_S.dx].visited == 50) {
							o_stream.p = Traverse(Input_S.inpv[Input_S.dx].wired_tile.id, Input_S.inpv[Input_S.dx].wired_out.id, o_stream);
							inst_class_list[tloc].inp_slice.inpv[Input_S.dx].visited = 50;
							inst_class_list[tloc].inp_slice.inpv[Input_S.dx].p = o_stream.p;
							generate = o_stream.p;
						}
						else {
							generate = inst_class_list[tloc].inp_slice.inpv[Input_S.dx].p;// the pin was already visited and we just get the probability for calculations
						}
					}
					else if (inst_class_list[tloc].dcy0.cnf == "O5") {//LUTB
						vector<int> input_seq;
						for (int i2 = 0; i2 < Input_S.inpvecList.size(); i2++) {//check whick of the inputs exist int the list. inpvecList has the input IDs that were used
							if (Input_S.inpvecList[i2] >= Input_S.d1 && Input_S.inpvecList[i2] <= Input_S.d5) {//if the ID is in the selecting range we pick it
								int i3 = Input_S.inpvecList[i2];
								input_seq.push_back(i3);
								//input_seq.push_back(i2);
								if (Input_S.inpv[i3].visited == 50) {//check if it was visited before
									o_stream.p = Traverse(Input_S.inpv[i3].wired_tile.id, Input_S.inpv[i3].wired_out.id, o_stream);
									// when we get to this line we have return back from the recursion so :1)we met the input 2)we met other blocks 3)we hit a loop
									inst_class_list[tloc].inp_slice.inpv[i3].visited = 50;//When we calculated the probability of the input, we set the input as visited in this for loop.
									inst_class_list[tloc].inp_slice.inpv[i3].p = o_stream.p;
								}
							}
						}

						//out_set_lut[out_id] * 6 + 5;
						vector<double>p = {};
						for (int i = Input_S.d1; i <= Input_S.d5; i++) {//for all the inputs of the LUT5 we get their probability
							p.push_back(inst_class_list[tloc].inp_slice.inpv[i].p);
						}
						generate = lut_Prob_Top(inst_class_list[tloc].lut_d.l5_init, p);
					}

					//Calculate the P(propagate). the O6 is in the select line
					vector<int> input_seq;
					for (int i2 = 0; i2 < Input_S.inpvecList.size(); i2++) {//check whick of the inputs exist int the list. inpvecList has the input IDs that were used
						if (Input_S.inpvecList[i2] >= Input_S.d1 && Input_S.inpvecList[i2] <= Input_S.d6) {//if the ID is in the selecting range we pick it
							int i3 = Input_S.inpvecList[i2];
							input_seq.push_back(i3);
							//input_seq.push_back(i2);
							if (Input_S.inpv[i3].visited == 50) {//check if it was visited before
								o_stream.p = Traverse(Input_S.inpv[i3].wired_tile.id, Input_S.inpv[i3].wired_out.id, o_stream);
								// when we get to this line we have return back from the recursion so :1)we met the input 2)we met other blocks 3)we hit a loop
								inst_class_list[tloc].inp_slice.inpv[i3].visited = 50;//When we calculated the probability of the input, we set the input as visited in this for loop.
								inst_class_list[tloc].inp_slice.inpv[i3].p = o_stream.p;
							}
						}
					}

					//out_set_lut[out_id] * 6 + 5;
					vector<double>p = {};
					for (int i = Input_S.d1; i <= Input_S.d6; i++) {//for all the inputs of the LUT6 we get their probability
						p.push_back(inst_class_list[tloc].inp_slice.inpv[i].p);
					}
					propagate = lut_Prob_Top(inst_class_list[tloc].lut_d.l6_init, p);

					inst_class_list[tloc].co3.p = ((1 - propagate)*generate) + (propagate*co_in);

					// Upto here we have calculated the P(CO3)
					answer2it_cy = inst_class_list[tloc].co3.p;
					answer2it_xor = ((1 - propagate)*co_in) + (propagate*(1 - co_in));

					//Setting the Probability of the output COUT
					for (int i = 0; i < inst_class_list[tloc].outputs.size(); i++) {
						if (inst_class_list[tloc].outputs[i].name == "COUT") {
							inst_class_list[tloc].outputs[i].visited = 50;
							inst_class_list[tloc].outputs[i].p = inst_class_list[tloc].co3.p;
							o_stream.tile.push_back(tloc); o_stream.out.push_back(i);//this output is dependent and its probability has calculated.
						}
					}
				}

				//answer2it has the right value for the probability
				if (cfg_xoutmux == "CY")
					o_stream.p = answer2it_cy;
				else if (cfg_xoutmux == "XOR")
					o_stream.p = answer2it_xor;

				inst_class_list[tloc].outputs[oloc].visited = 50;
				inst_class_list[tloc].outputs[oloc].p = o_stream.p;
				return o_stream.p;
			}

			//3) cfg xFFMUX::xX
			//ax = 24, bx = 25, cx = 26, dx = 27
			if (cfg_xoutmux == "AX" || cfg_xoutmux == "BX" || cfg_xoutmux == "CX" || cfg_xoutmux == "DX") {
				int xX_loc = 24 + outset.out_set_lut[out_id]; //this will pick the input to be ax or .. based on the LUT number
				if (Input_S.inpv[xX_loc].visited == 50) {
					o_stream.p = Traverse(Input_S.inpv[xX_loc].wired_tile.id, Input_S.inpv[xX_loc].wired_out.id, o_stream);

					inst_class_list[tloc].inp_slice.inpv[xX_loc].visited = 50;
					inst_class_list[tloc].inp_slice.inpv[xX_loc].p = o_stream.p;//o_stream.p holds the value of probability of the wired output
					inst_class_list[tloc].outputs[oloc].visited = 50;
					inst_class_list[tloc].outputs[oloc].p = o_stream.p;
					return o_stream.p;
				}
				else {
					o_stream.p = Input_S.inpv[xX_loc].p;
					inst_class_list[tloc].outputs[oloc].visited = 50;
					inst_class_list[tloc].outputs[oloc].p = o_stream.p;
					return o_stream.p;
				}
			}

			//4,5) cfg xFFMUX::O5/O6
			if (cfg_xoutmux == "O5" || cfg_xoutmux == "O6") {

				string current_lut;
				vector<int> i_list = { 0,0 }; //list of defult positions to look for available input
				i_list[0] = outset.out_set_lut[out_id] * 6;
				if (cfg_xoutmux == "O5") {
					i_list[1] = outset.out_set_lut[out_id] * 6 + 5;
					current_lut = inst_class_list[tloc].luti[outset.out_set_lut[out_id]].l5_init;
				}
				else if (cfg_xoutmux == "O6") {
					i_list[1] = outset.out_set_lut[out_id] * 6 + 5;
					current_lut = inst_class_list[tloc].luti[outset.out_set_lut[out_id]].l6_init;
				}

				vector<int> input_seq;
				for (int i2 = 0; i2 < Input_S.inpvecList.size(); i2++) {//check whick of the inputs exist int the list. inpvecList has the input IDs that were used
					if (Input_S.inpvecList[i2] >= i_list[0] && Input_S.inpvecList[i2] <= i_list[1]) {//if the ID is in the selecting range we pick it
						int i3 = Input_S.inpvecList[i2];
						input_seq.push_back(i3);
						//input_seq.push_back(i2);
						if (Input_S.inpv[i3].visited == 50) {//check if it was visited before
							o_stream.p = Traverse(Input_S.inpv[i3].wired_tile.id, Input_S.inpv[i3].wired_out.id, o_stream);
							// when we get to this line we have return back from the recursion so :1)we met the input 2)we met other blocks 3)we hit a loop
							inst_class_list[tloc].inp_slice.inpv[i3].visited = 50;//When we calculated the probability of the input, we set the input as visited in this for loop.
							inst_class_list[tloc].inp_slice.inpv[i3].p = o_stream.p;
						}
						// we later take care of the already visited ones right before the return
					}
				}

				//out_set_lut[out_id] * 6 + 5;
				vector<double>p = {};
				for (int i = i_list[0]; i <= i_list[1]; i++) {//for all the inputs of the LUT5 we get their probability there are some that were not even included. they have P=0
					p.push_back(inst_class_list[tloc].inp_slice.inpv[i].p);
				}

				o_stream.p = lut_Prob_Top(current_lut, p); //current_lut holds the lut info relating to the current output.
				inst_class_list[tloc].outputs[oloc].visited = 50;
				inst_class_list[tloc].outputs[oloc].p = o_stream.p;
				return o_stream.p;
			}
		}

				//=============================================
				//COUT

		case 4: {

			//PART 4.1.1.1: Calculate the COinit or CO-1. Check PRECYINIT::0/1/AX/#OFF
			int count2it = 0;//We start from the base of the carry out and reach to the current COi that we are calculating.
			double answer2it_cy;//this will hold the final answer

								//This time we have to go to the end of the line from the begining
								//=========================================================
								// CO calculation STARTS HERE
			if (count2it <= 3) {
				count2it++;
				//=========================================================
				// 1) COinit calculation
				if (precyinit == "#OFF") {
					if (Input_S.inpv[Input_S.cin].visited == 50) {
						o_stream.p = Traverse(Input_S.inpv[Input_S.cin].wired_tile.id, Input_S.inpv[Input_S.cin].wired_out.id, o_stream);
						// when we get to this line we have return back from the recursion so :1)we met the input 2)we met other blocks 3)we hit a loop
						inst_class_list[tloc].inp_slice.inpv[Input_S.cin].visited = 50;
						inst_class_list[tloc].inp_slice.inpv[Input_S.cin].p = o_stream.p;
						inst_class_list[tloc].coinit.p = o_stream.p;
					}
					else {
						inst_class_list[tloc].coinit.p = inst_class_list[tloc].inp_slice.inpv[Input_S.cin].p;// the pin was already visited and we just get the probability for calculations
					}
				}
				else if (precyinit == "0") {
					inst_class_list[tloc].coinit.p = 0;
				}
				else if (precyinit == "1") {
					inst_class_list[tloc].coinit.p = 1;
				}
				else if (precyinit == "AX") {
					if (Input_S.inpv[Input_S.ax].visited == 50) {
						o_stream.p = Traverse(Input_S.inpv[Input_S.ax].wired_tile.id, Input_S.inpv[Input_S.ax].wired_out.id, o_stream);
						inst_class_list[tloc].inp_slice.inpv[Input_S.ax].visited = 50;
						inst_class_list[tloc].inp_slice.inpv[Input_S.ax].p = o_stream.p;
						inst_class_list[tloc].coinit.p = o_stream.p;
					}
					else {
						inst_class_list[tloc].coinit.p = inst_class_list[tloc].inp_slice.inpv[Input_S.ax].p;// the pin was already visited and we just get the probability for calculations
					}
				}
				//=========================================================
				// 2) CO0 calculation
				double propagate, generate, co_in;// probability of propagate and generate signals in carry lookeahead
				co_in = inst_class_list[tloc].coinit.p;
				//PART: Check the ACY0
				//Calculate P(generate) 
				if (inst_class_list[tloc].acy0.cnf == "AX") {
					if (Input_S.inpv[Input_S.ax].visited == 50) {
						o_stream.p = Traverse(Input_S.inpv[Input_S.ax].wired_tile.id, Input_S.inpv[Input_S.ax].wired_out.id, o_stream);
						inst_class_list[tloc].inp_slice.inpv[Input_S.ax].visited = 50;
						inst_class_list[tloc].inp_slice.inpv[Input_S.ax].p = o_stream.p;
						generate = o_stream.p;
					}
					else {
						generate = inst_class_list[tloc].inp_slice.inpv[Input_S.ax].p;// the pin was already visited and we just get the probability for calculations
					}
				}
				else if (inst_class_list[tloc].acy0.cnf == "O5") {//LUTA
					vector<int> input_seq;
					for (int i2 = 0; i2 < Input_S.inpvecList.size(); i2++) {//check whick of the inputs exist int the list. inpvecList has the input IDs that were used
						if (Input_S.inpvecList[i2] >= Input_S.a1 && Input_S.inpvecList[i2] <= Input_S.a5) {//if the ID is in the selecting range we pick it
							int i3 = Input_S.inpvecList[i2];
							input_seq.push_back(i3);
							//input_seq.push_back(i2);
							if (Input_S.inpv[i3].visited == 50) {//check if it was visited before
								o_stream.p = Traverse(Input_S.inpv[i3].wired_tile.id, Input_S.inpv[i3].wired_out.id, o_stream);
								// when we get to this line we have return back from the recursion so :1)we met the input 2)we met other blocks 3)we hit a loop
								inst_class_list[tloc].inp_slice.inpv[i3].visited = 50;//When we calculated the probability of the input, we set the input as visited in this for loop.
								inst_class_list[tloc].inp_slice.inpv[i3].p = o_stream.p;
							}
						}
					}

					//out_set_lut[out_id] * 6 + 5;
					vector<double>p = {};
					for (int i = Input_S.a1; i <= Input_S.a5; i++) {//for all the inputs of the LUT5 we get their probability
						p.push_back(inst_class_list[tloc].inp_slice.inpv[i].p);
					}
					generate = lut_Prob_Top(inst_class_list[tloc].lut_a.l5_init, p);
				}

				//Calculate the P(propagate). the O6 is in the select line
				vector<int> input_seq;
				for (int i2 = 0; i2 < Input_S.inpvecList.size(); i2++) {//check whick of the inputs exist int the list. inpvecList has the input IDs that were used
					if (Input_S.inpvecList[i2] >= Input_S.a1 && Input_S.inpvecList[i2] <= Input_S.a6) {//if the ID is in the selecting range we pick it
						int i3 = Input_S.inpvecList[i2];
						input_seq.push_back(i3);
						//input_seq.push_back(i2);
						if (Input_S.inpv[i3].visited == 50) {//check if it was visited before
							o_stream.p = Traverse(Input_S.inpv[i3].wired_tile.id, Input_S.inpv[i3].wired_out.id, o_stream);
							// when we get to this line we have return back from the recursion so :1)we met the input 2)we met other blocks 3)we hit a loop
							inst_class_list[tloc].inp_slice.inpv[i3].visited = 50;//When we calculated the probability of the input, we set the input as visited in this for loop.
							inst_class_list[tloc].inp_slice.inpv[i3].p = o_stream.p;
						}
					}
				}

				//out_set_lut[out_id] * 6 + 5;
				vector<double>p = {};
				for (int i = Input_S.a1; i <= Input_S.a6; i++) {//for all the inputs of the LUT6 we get their probability
					p.push_back(inst_class_list[tloc].inp_slice.inpv[i].p);
				}
				propagate = lut_Prob_Top(inst_class_list[tloc].lut_a.l6_init, p);

				inst_class_list[tloc].co0.p = ((1 - propagate)*generate) + (propagate*co_in);

				// Upto here we have calculated the P(CO0)
				answer2it_cy = inst_class_list[tloc].co0.p;
			}
			//=========================================================
			// 3) CO1 calculation
			if (count2it <= 3) {
				count2it++;
				double propagate, generate, co_in;// probability of propagate and generate signals in carry lookeahead
				co_in = inst_class_list[tloc].co0.p;
				//PART: Check the BCY0
				//Calculate P(generate) 
				if (inst_class_list[tloc].bcy0.cnf == "BX") {
					if (Input_S.inpv[Input_S.bx].visited == 50) {
						o_stream.p = Traverse(Input_S.inpv[Input_S.bx].wired_tile.id, Input_S.inpv[Input_S.bx].wired_out.id, o_stream);
						inst_class_list[tloc].inp_slice.inpv[Input_S.bx].visited = 50;
						inst_class_list[tloc].inp_slice.inpv[Input_S.bx].p = o_stream.p;
						generate = o_stream.p;
					}
					else {
						generate = inst_class_list[tloc].inp_slice.inpv[Input_S.bx].p;// the pin was already visited and we just get the probability for calculations
					}
				}
				else if (inst_class_list[tloc].bcy0.cnf == "O5") {//LUTB
					vector<int> input_seq;
					for (int i2 = 0; i2 < Input_S.inpvecList.size(); i2++) {//check whick of the inputs exist int the list. inpvecList has the input IDs that were used
						if (Input_S.inpvecList[i2] >= Input_S.b1 && Input_S.inpvecList[i2] <= Input_S.b5) {//if the ID is in the selecting range we pick it
							int i3 = Input_S.inpvecList[i2];
							input_seq.push_back(i3);
							//input_seq.push_back(i2);
							if (Input_S.inpv[i3].visited == 50) {//check if it was visited before
								o_stream.p = Traverse(Input_S.inpv[i3].wired_tile.id, Input_S.inpv[i3].wired_out.id, o_stream);
								// when we get to this line we have return back from the recursion so :1)we met the input 2)we met other blocks 3)we hit a loop
								inst_class_list[tloc].inp_slice.inpv[i3].visited = 50;//When we calculated the probability of the input, we set the input as visited in this for loop.
								inst_class_list[tloc].inp_slice.inpv[i3].p = o_stream.p;
							}
						}
					}

					//out_set_lut[out_id] * 6 + 5;
					vector<double>p = {};
					for (int i = Input_S.b1; i <= Input_S.b5; i++) {//for all the inputs of the LUT5 we get their probability
						p.push_back(inst_class_list[tloc].inp_slice.inpv[i].p);
					}
					generate = lut_Prob_Top(inst_class_list[tloc].lut_b.l5_init, p);
				}

				//Calculate the P(propagate). the O6 is in the select line
				vector<int> input_seq;
				for (int i2 = 0; i2 < Input_S.inpvecList.size(); i2++) {//check whick of the inputs exist int the list. inpvecList has the input IDs that were used
					if (Input_S.inpvecList[i2] >= Input_S.b1 && Input_S.inpvecList[i2] <= Input_S.b6) {//if the ID is in the selecting range we pick it
						int i3 = Input_S.inpvecList[i2];
						input_seq.push_back(i3);
						//input_seq.push_back(i2);
						if (Input_S.inpv[i3].visited == 50) {//check if it was visited before
							o_stream.p = Traverse(Input_S.inpv[i3].wired_tile.id, Input_S.inpv[i3].wired_out.id, o_stream);
							// when we get to this line we have return back from the recursion so :1)we met the input 2)we met other blocks 3)we hit a loop
							inst_class_list[tloc].inp_slice.inpv[i3].visited = 50;//When we calculated the probability of the input, we set the input as visited in this for loop.
							inst_class_list[tloc].inp_slice.inpv[i3].p = o_stream.p;
						}
					}
				}

				//out_set_lut[out_id] * 6 + 5;
				vector<double>p = {};
				for (int i = Input_S.b1; i <= Input_S.b6; i++) {//for all the inputs of the LUT6 we get their probability
					p.push_back(inst_class_list[tloc].inp_slice.inpv[i].p);
				}
				propagate = lut_Prob_Top(inst_class_list[tloc].lut_b.l6_init, p);

				inst_class_list[tloc].co1.p = ((1 - propagate)*generate) + (propagate*co_in);

				// Upto here we have calculated the P(CO1)
				answer2it_cy = inst_class_list[tloc].co1.p;
			}
			//=========================================================
			// 4) CO2 calculation
			if (count2it <= 3) {
				count2it++;
				double propagate, generate, co_in;// probability of propagate and generate signals in carry lookeahead
				co_in = inst_class_list[tloc].co1.p;
				//PART: Check the CCY0
				//Calculate P(generate) 
				if (inst_class_list[tloc].ccy0.cnf == "CX") {
					if (Input_S.inpv[Input_S.cx].visited == 50) {
						o_stream.p = Traverse(Input_S.inpv[Input_S.cx].wired_tile.id, Input_S.inpv[Input_S.cx].wired_out.id, o_stream);
						inst_class_list[tloc].inp_slice.inpv[Input_S.cx].visited = 50;
						inst_class_list[tloc].inp_slice.inpv[Input_S.cx].p = o_stream.p;
						generate = o_stream.p;
					}
					else {
						generate = inst_class_list[tloc].inp_slice.inpv[Input_S.cx].p;// the pin was already visited and we just get the probability for calculations
					}
				}
				else if (inst_class_list[tloc].ccy0.cnf == "O5") {//LUTB
					vector<int> input_seq;
					for (int i2 = 0; i2 < Input_S.inpvecList.size(); i2++) {//check whick of the inputs exist int the list. inpvecList has the input IDs that were used
						if (Input_S.inpvecList[i2] >= Input_S.c1 && Input_S.inpvecList[i2] <= Input_S.c5) {//if the ID is in the selecting range we pick it
							int i3 = Input_S.inpvecList[i2];
							input_seq.push_back(i3);
							//input_seq.push_back(i2);
							if (Input_S.inpv[i3].visited == 50) {//check if it was visited before
								o_stream.p = Traverse(Input_S.inpv[i3].wired_tile.id, Input_S.inpv[i3].wired_out.id, o_stream);
								// when we get to this line we have return back from the recursion so :1)we met the input 2)we met other blocks 3)we hit a loop
								inst_class_list[tloc].inp_slice.inpv[i3].visited = 50;//When we calculated the probability of the input, we set the input as visited in this for loop.
								inst_class_list[tloc].inp_slice.inpv[i3].p = o_stream.p;
							}
						}
					}

					//out_set_lut[out_id] * 6 + 5;
					vector<double>p = {};
					for (int i = Input_S.c1; i <= Input_S.c5; i++) {//for all the inputs of the LUT5 we get their probability
						p.push_back(inst_class_list[tloc].inp_slice.inpv[i].p);
					}
					generate = lut_Prob_Top(inst_class_list[tloc].lut_c.l5_init, p);
				}

				//Calculate the P(propagate). the O6 is in the select line
				vector<int> input_seq;
				for (int i2 = 0; i2 < Input_S.inpvecList.size(); i2++) {//check whick of the inputs exist int the list. inpvecList has the input IDs that were used
					if (Input_S.inpvecList[i2] >= Input_S.c1 && Input_S.inpvecList[i2] <= Input_S.c6) {//if the ID is in the selecting range we pick it
						int i3 = Input_S.inpvecList[i2];
						input_seq.push_back(i3);
						//input_seq.push_back(i2);
						if (Input_S.inpv[i3].visited == 50) {//check if it was visited before
							o_stream.p = Traverse(Input_S.inpv[i3].wired_tile.id, Input_S.inpv[i3].wired_out.id, o_stream);
							// when we get to this line we have return back from the recursion so :1)we met the input 2)we met other blocks 3)we hit a loop
							inst_class_list[tloc].inp_slice.inpv[i3].visited = 50;//When we calculated the probability of the input, we set the input as visited in this for loop.
							inst_class_list[tloc].inp_slice.inpv[i3].p = o_stream.p;
						}
					}
				}

				//out_set_lut[out_id] * 6 + 5;
				vector<double>p = {};
				for (int i = Input_S.c1; i <= Input_S.c6; i++) {//for all the inputs of the LUT6 we get their probability
					p.push_back(inst_class_list[tloc].inp_slice.inpv[i].p);
				}
				propagate = lut_Prob_Top(inst_class_list[tloc].lut_c.l6_init, p);

				inst_class_list[tloc].co2.p = ((1 - propagate)*generate) + (propagate*co_in);

				// Upto here we have calculated the P(C2)
				answer2it_cy = inst_class_list[tloc].co2.p;
			}
			//=========================================================
			// 5) CO3 calculation
			if (count2it <= 3) {
				count2it++;
				double propagate, generate, co_in;// probability of propagate and generate signals in carry lookeahead
				co_in = inst_class_list[tloc].co2.p;
				//PART: Check the DCY0
				//Calculate P(generate) 
				if (inst_class_list[tloc].dcy0.cnf == "DX") {
					if (Input_S.inpv[Input_S.dx].visited == 50) {
						o_stream.p = Traverse(Input_S.inpv[Input_S.dx].wired_tile.id, Input_S.inpv[Input_S.dx].wired_out.id, o_stream);
						inst_class_list[tloc].inp_slice.inpv[Input_S.dx].visited = 50;
						inst_class_list[tloc].inp_slice.inpv[Input_S.dx].p = o_stream.p;
						generate = o_stream.p;
					}
					else {
						generate = inst_class_list[tloc].inp_slice.inpv[Input_S.dx].p;// the pin was already visited and we just get the probability for calculations
					}
				}
				else if (inst_class_list[tloc].dcy0.cnf == "O5") {//LUTB
					vector<int> input_seq;
					for (int i2 = 0; i2 < Input_S.inpvecList.size(); i2++) {//check whick of the inputs exist int the list. inpvecList has the input IDs that were used
						if (Input_S.inpvecList[i2] >= Input_S.d1 && Input_S.inpvecList[i2] <= Input_S.d5) {//if the ID is in the selecting range we pick it
							int i3 = Input_S.inpvecList[i2];
							input_seq.push_back(i3);
							//input_seq.push_back(i2);
							if (Input_S.inpv[i3].visited == 50) {//check if it was visited before
								o_stream.p = Traverse(Input_S.inpv[i3].wired_tile.id, Input_S.inpv[i3].wired_out.id, o_stream);
								// when we get to this line we have return back from the recursion so :1)we met the input 2)we met other blocks 3)we hit a loop
								inst_class_list[tloc].inp_slice.inpv[i3].visited = 50;//When we calculated the probability of the input, we set the input as visited in this for loop.
								inst_class_list[tloc].inp_slice.inpv[i3].p = o_stream.p;
							}
						}
					}

					//out_set_lut[out_id] * 6 + 5;
					vector<double>p = {};
					for (int i = Input_S.d1; i <= Input_S.d5; i++) {//for all the inputs of the LUT5 we get their probability
						p.push_back(inst_class_list[tloc].inp_slice.inpv[i].p);
					}
					generate = lut_Prob_Top(inst_class_list[tloc].lut_d.l5_init, p);
				}

				//Calculate the P(propagate). the O6 is in the select line
				vector<int> input_seq;
				for (int i2 = 0; i2 < Input_S.inpvecList.size(); i2++) {//check whick of the inputs exist int the list. inpvecList has the input IDs that were used
					if (Input_S.inpvecList[i2] >= Input_S.d1 && Input_S.inpvecList[i2] <= Input_S.d6) {//if the ID is in the selecting range we pick it
						int i3 = Input_S.inpvecList[i2];
						input_seq.push_back(i3);
						//input_seq.push_back(i2);
						if (Input_S.inpv[i3].visited == 50) {//check if it was visited before
							o_stream.p = Traverse(Input_S.inpv[i3].wired_tile.id, Input_S.inpv[i3].wired_out.id, o_stream);
							// when we get to this line we have return back from the recursion so :1)we met the input 2)we met other blocks 3)we hit a loop
							inst_class_list[tloc].inp_slice.inpv[i3].visited = 50;//When we calculated the probability of the input, we set the input as visited in this for loop.
							inst_class_list[tloc].inp_slice.inpv[i3].p = o_stream.p;
						}
					}
				}

				//out_set_lut[out_id] * 6 + 5;
				vector<double>p = {};
				for (int i = Input_S.d1; i <= Input_S.d6; i++) {//for all the inputs of the LUT6 we get their probability
					p.push_back(inst_class_list[tloc].inp_slice.inpv[i].p);
				}
				propagate = lut_Prob_Top(inst_class_list[tloc].lut_d.l6_init, p);

				inst_class_list[tloc].co3.p = ((1 - propagate)*generate) + (propagate*co_in);

				// Upto here we have calculated the P(CO3)
				answer2it_cy = inst_class_list[tloc].co3.p;

				//Setting the Probability of the output COUT
				for (int i = 0; i < inst_class_list[tloc].outputs.size(); i++) {
					if (inst_class_list[tloc].outputs[i].name == "COUT") {
						inst_class_list[tloc].outputs[i].visited = 50;
						inst_class_list[tloc].outputs[i].p = inst_class_list[tloc].co3.p;
						o_stream.tile.push_back(tloc); o_stream.out.push_back(i);//this output is dependent and its probability has calculated.
					}
				}
			}

			//answer2it has the right value for the probability
			o_stream.p = answer2it_cy;// CO3 = COUT

			inst_class_list[tloc].outputs[oloc].visited = 50;
			inst_class_list[tloc].outputs[oloc].p = o_stream.p;
			return o_stream.p;
		}
		default: {
			cout << "error here" << endl;
			break;
		}
		}

	}

	// detect Other Blocks that are not Inputs or SLICELs--TIOF(KEEP0,KEEP1)

	else {

		o_stream.p = 0.53;// its a 50% chance to be one for now!!

						  /*
						  //Display Information From HERE
						  string sloop1 = "\n=============== 2) ||| Blocked by other Blocks of type <" + inst_class_list[tloc].type + "> ===============\n";
						  sloop1 += "\n\t\t2) Lookin At: ";
						  string sloop2 = "\n\t\t2) Lookin At: ";
						  for (int i = 0; i < o_stream.tile.size(); i++) {
						  sloop1 = sloop1 + to_string(o_stream.tile[i]) + "_" + to_string(o_stream.out[i]) + " >> ";
						  sloop2 = sloop2 + inst_class_list[o_stream.tile[i]].name + " _ " + inst_class_list[o_stream.tile[i]].outputs[o_stream.out[i]].name + " >> ";
						  }
						  sloop1 = sloop1 + "P: " + to_string(o_stream.p);
						  sloop2 = sloop2 + "P: " + to_string(o_stream.p);
						  trav_data << sloop1 << sloop2 << endl;
						  //To HERE
						  */
		inst_class_list[tloc].outputs[oloc].visited = 50;
		inst_class_list[tloc].outputs[oloc].p = o_stream.p;
		return o_stream.p;
	}
}

void TraverseTop() {
	//# this traverse to the inputs of the FPGA.
	//# it figure outs the relationship between the I/O in blocks and between blocks
	//# The probability in the inputs will be changed through this function
	//# input to this function is the global variable inst_class_list which holds all the used instance blocks in the FPGA
	string sout2;
	for (int i = 0; i < inst_class_list.size(); i++) {//find the IOB blocks
		if (inst_class_list[i].type == "IOB" && inst_class_list[i].inp_other.size() == 1) {
			T_IID o_stream; o_stream.pout_name = inst_class_list[i].name;
			//vector<int> visited_tiles, visited_outputs;
			//visited_tiles.push_back(list[i].inputs[0].wired_tile.id);
			//visited_outputs.push_back(list[i].inputs[0].wired_out.id);
			sout2 +=  "\n=============== startign from: " + inst_class_list[i].name + "===============\n\t\t==============================\n";
			o_stream.p = Traverse(inst_class_list[i].inp_other[0].wired_tile.id, inst_class_list[i].inp_other[0].wired_out.id, o_stream);
			sout2 += to_string(o_stream.out.size()) + " _ " + to_string(o_stream.p) + "\n";
		}
	}
	string sout="START\n";
	for (int i = 0; i < OutStream_lists.size(); i++) {
		sout += "From out:" + inst_class_list[OutStream_lists[i].tile[0]].name + " to inp:" + inst_class_list[OutStream_lists[i].tile[OutStream_lists[i].tile.size() - 1]].name + "\n";

		sout += "From out:" + inst_class_list[OutStream_lists[i].tile[0]].outputs[OutStream_lists[i].out[0]].name + " to inp:" + inst_class_list[OutStream_lists[i].tile[OutStream_lists[i].tile.size() - 1]].outputs[OutStream_lists[i].out[OutStream_lists[i].tile.size() - 1]].name + "\n";
		for (int j = 0; j < OutStream_lists[i].out.size(); j++) {
			sout += "-> " + to_string(OutStream_lists[i].tile[j]) + "_" + to_string(OutStream_lists[i].out[j]);
		}
		sout += "\n";
	}
	trav_data << sout << sout2;
}

//========== Main Function ==========
int main()
{
	//=================================
	// Section One: scanning the xdl file to get the data structure ready 
	//=================================
	string search1, search2, search3, search4;
	ifstream xdlFile;
	ifstream vhdlFile;
	string line;
	int counter = 0;
	string s;

	xdlFile.open(xdl_fn);
	vhdlFile.open(vhd_fn);
	ofstream out_data(xdl_rep_fn);
	ofstream trav_data(traverse_rep_fn);
	//vector<string> inst_name_list;
	//vector<inst> inst_class_list;

	search1 = "net \"";
	search2 = "outpin \"";
	search3 = "inpin \"";
	search4 = "inst \"";

	// explore the VHDL file to get the LUT information
	vhdlExplore_out vhdl_out = vhdlExplore();
	//

	vector<string> search5 = {
		" A5LUT:", ":" ,"O5=", " ", " A6LUT:", ":" ,"O6=", " ", "ACY0::", " ", "AFFMUX::", " ", "AOUTMUX::", " ", "AUSED::", " ",
		" B5LUT:", ":" ,"O5=", " ", " B6LUT:", ":" ,"O6=", " ", "BCY0::", " ", "BFFMUX::", " ", "BOUTMUX::", " ", "BUSED::", " ",
		" C5LUT:", ":" ,"O5=", " ", " C6LUT:", ":" ,"O6=", " ", "CCY0::", " ", "CFFMUX::", " ", "COUTMUX::", " ", "CUSED::", " ",
		" D5LUT:", ":" ,"O5=", " ", " D6LUT:", ":" ,"O6=", " ", "DCY0::", " ", "DFFMUX::", " ", "DOUTMUX::", " ", "DUSED::", " " };
	vector<string> search6 = { "PRECYINIT::"," " };
	size_t pos1, pos2, pos3, pos4, pos5;
	while (xdlFile.good())
	{
		counter++;
		getline(xdlFile, line); // get line from file

		pos1 = line.find(search1); // search for net
		pos5 = line.find(search4); // search for inst
		if (pos5 != string::npos) {
			vector<string> inf_line_inst_vec = line_info_inst(line);
			inst_name_list.push_back(inf_line_inst_vec[0]);
			inst *inst1 = new inst;
			inst1->set_name(inf_line_inst_vec[0]);//inst name
			inst1->set_type(inf_line_inst_vec[1]);//inst type
			inst1->set_pos(inf_line_inst_vec[2], inf_line_inst_vec[3]);//inst position {placed <tile> <site>}


			if (inf_line_inst_vec[1] == "SLICEL") {
				LUT luta, lutb, lutc, lutd;

				CFG acfg, bcfg, ccfg, dcfg; //ACY0,...

				vector<LUT> vlut = { luta, lutb, lutc, lutd };
				vector<CFG> xcy0 = { acfg, bcfg, ccfg, dcfg };
				vector<CFG> xffmux = { acfg, bcfg, ccfg, dcfg };
				vector<CFG> xoutmux = { acfg, bcfg, ccfg, dcfg };
				vector<CFG> xused = { acfg, bcfg, ccfg, dcfg };
				CFG precyinit;

				pos4 = line.find(";");
				int i1 = 0;//search itterator 
				int i2 = 0;// LUT itterator
				vector<string>::iterator find_id;

				while (i2<4) {// Find all the 4 LUTs
							  //LUT5
					pos2 = line.find(search5[0 + i1]);	// find lut5 name
					while (pos2 == string::npos) {
						counter++;
						getline(xdlFile, line); // get line from file
						pos2 = line.find(search5[0 + i1]);
					}
					if (pos2 != string::npos) { //if its in this line
						pos2 = pos2 + 6;
						pos3 = line.find(search5[1 + i1], pos2 + 1);//find the end of the name
						if (pos3 != pos2 + 1) {//heck if the lut was used by checking the name
							vlut[i2].l5_name = line.substr(pos2 + 1, pos3 - pos2 - 1);
						}
						else
							vlut[i2].l5_name = "#OFF";
					}

					if (vlut[i2].l5_name != "#OFF") { //if the LUT was active check for the function
						find_id = std::find(vhdl_out.lut_name_list.begin(), vhdl_out.lut_name_list.end(), vlut[i2].l5_name);
						int pos = find_id - vhdl_out.lut_name_list.begin();
						//out_data << "CHECK:: From VHDL: " << vhdl_out.lut_info[pos].name << " From XDL: " << vlut[i2].l5_name<<endl;
						vlut[i2].l5_init = vhdl_out.lut_info[pos].init;

						pos2 = line.find(search5[2 + i1]); // find lut5 function
						while (pos2 == string::npos) {
							counter++;
							getline(xdlFile, line); // get line from file
							pos2 = line.find(search5[2 + i1]); // find lut5 function							
						}
						if (pos2 != string::npos) {// if the string is in this line	
							pos2 = pos2 + 2; //O5 two characters
							pos3 = line.find(search5[3 + i1], pos2 + 1);//get the value
							vlut[i2].l5_func = line.substr(pos2 + 1, pos3 - pos2 - 1);
						}
					}
					//LUT6
					pos2 = line.find(search5[4 + i1]);	// find lut6 name
					while (pos2 == string::npos) {
						counter++;
						getline(xdlFile, line); // get line from file
						pos2 = line.find(search5[4 + i1]); // find lut6 name						
					}
					if (pos2 != string::npos) { //if its in this line
						pos2 = pos2 + 6;
						pos3 = line.find(search5[5 + i1], pos2 + 1);//find the end of the name
						if (pos3 != pos2 + 1) {//heck if the lut5 was used by checking the name
							vlut[i2].l6_name = line.substr(pos2 + 1, pos3 - pos2 - 1);
						}
						else
							vlut[i2].l6_name = "#OFF";
					}
					if (vlut[i2].l6_name != "#OFF") {
						find_id = std::find(vhdl_out.lut_name_list.begin(), vhdl_out.lut_name_list.end(), vlut[i2].l6_name);
						int pos = find_id - vhdl_out.lut_name_list.begin();
						//out_data << "CHECK:: From VHDL: " << vhdl_out.lut_info[pos].name << " From XDL: " << vlut[i2].l6_name<<endl;
						vlut[i2].l6_init = vhdl_out.lut_info[pos].init;

						pos2 = line.find(search5[6 + i1]); // find lut6 function
						while (pos2 == string::npos) {
							counter++;
							getline(xdlFile, line); // get line from file
							pos2 = line.find(search5[6 + i1]); // find lut6 function							
						}
						if (pos2 != string::npos) {// if the string is in this line	
							pos2 = pos2 + 2; //O6 two characters
							pos3 = line.find(search5[7 + i1], pos2 + 1);//get the value
							vlut[i2].l6_func = line.substr(pos2 + 1, pos3 - pos2 - 1);
						}
					}
					//xCY0
					pos2 = line.find(search5[8 + i1]);	// find xCY0 cfg
					while (pos2 == string::npos) {// search untill it finds xCY0
						counter++;
						getline(xdlFile, line); // get line from file
						pos2 = line.find(search5[8 + i1]); // find xCY0::						
					}
					if (pos2 != string::npos) { //if its in this line
						pos2 = pos2 + size(search5[8 + i1]) - 1; //xCY0:: characters
						pos3 = line.find(search5[9 + i1], pos2 + 1);//get the value
						xcy0[i2].cnf = line.substr(pos2 + 1, pos3 - pos2 - 1);
					}
					//xFFMUX
					pos2 = line.find(search5[10 + i1]);	// find lut6 name
					while (pos2 == string::npos) {
						counter++;
						getline(xdlFile, line); // get line from file
						pos2 = line.find(search5[10 + i1]); // find xFFMUX::						
					}
					if (pos2 != string::npos) { //if its in this line
						pos2 = pos2 + size(search5[10 + i1]) - 1; //xFFMUX:: eight characters
						pos3 = line.find(search5[11 + i1], pos2 + 1);//get the value
						xffmux[i2].cnf = line.substr(pos2 + 1, pos3 - pos2 - 1);
					}
					//xOUTMUX
					pos2 = line.find(search5[12 + i1]);	// find xOUTMUX cfg
					while (pos2 == string::npos) {
						counter++;
						getline(xdlFile, line); // get line from file
						pos2 = line.find(search5[12 + i1]); // find xOUTMUX::						
					}
					if (pos2 != string::npos) { //if its in this line
						pos2 = pos2 + size(search5[12 + i1]) - 1; //xOUTMUX:: eight characters
						pos3 = line.find(search5[13 + i1], pos2 + 1);//get the value
						xoutmux[i2].cnf = line.substr(pos2 + 1, pos3 - pos2 - 1);
					}
					//xUSED
					pos2 = line.find(search5[14 + i1]);	// find xUSED cfg
					while (pos2 == string::npos) {
						counter++;
						getline(xdlFile, line); // get line from file
						pos2 = line.find(search5[14 + i1]); // find xUSED::						
					}
					if (pos2 != string::npos) { //if its in this line
						pos2 = pos2 + size(search5[14 + i1]) - 1; //xUSED:: eight characters
						pos3 = line.find(search5[15 + i1], pos2 + 1);//get the value
						xused[i2].cnf = line.substr(pos2 + 1, pos3 - pos2 - 1);
					}

					i1 = i1 + search5.size() / 4;
					i2++;
				}
				//PRECYINIT
				pos2 = line.find(search6[0]);	// find PRECYINIT cfg
				while (pos2 == string::npos) {
					counter++;
					getline(xdlFile, line); // get line from file
					pos2 = line.find(search6[0]); // find PRECYINIT::						
				}
				if (pos2 != string::npos) { //if its in this line
					pos2 = pos2 + size(search6[0]) - 1; //PRECYINIT:: eight characters
					pos3 = line.find(search6[1], pos2 + 1);//get the value
					precyinit.cnf = line.substr(pos2 + 1, pos3 - pos2 - 1);
				}

				//out_data << "LUTA5 (" << vlut[0].l5_name << ") O5=" << vlut[0].l5_func << " init=0X" << vlut[0].l5_init << endl;
				//out_data << "LUTA6 (" << vlut[0].l6_name << ") O6=" << vlut[0].l6_func << " init=0X" << vlut[0].l6_init << endl;
				//out_data << "LUTB5 (" << vlut[1].l5_name << ") O5=" << vlut[1].l5_func << " init=0X" << vlut[1].l5_init << endl;
				//out_data << "LUTB6 (" << vlut[1].l6_name << ") O6=" << vlut[1].l6_func << " init=0X" << vlut[1].l6_init << endl;
				//out_data << "LUTC5 (" << vlut[2].l5_name << ") O5=" << vlut[2].l5_func << " init=0X" << vlut[2].l5_init << endl;
				//out_data << "LUTC6 (" << vlut[2].l6_name << ") O6=" << vlut[2].l6_func << " init=0X" << vlut[2].l6_init << endl;
				//out_data << "LUTD5 (" << vlut[3].l5_name << ") O5=" << vlut[3].l5_func << " init=0X" << vlut[3].l5_init << endl;
				//out_data << "LUTD6 (" << vlut[3].l6_name << ") O6=" << vlut[3].l6_func << " init=0X" << vlut[3].l6_init << endl;
				//out_data << "AFFMUX (" << xffmux[0].cnf << endl;
				//out_data << "BFFMUX (" << xffmux[1].cnf << endl;
				//out_data << "CFFMUX (" << xffmux[2].cnf << endl;
				//out_data << "DFFMUX (" << xffmux[3].cnf << endl;
				//out_data << "====================" << endl;


				//Set Configuration information here
				inst1->set_lut_a(vlut[0]); inst1->set_lut_b(vlut[1]); inst1->set_lut_c(vlut[2]); inst1->set_lut_d(vlut[3]);
				inst1->acy0 = xcy0[0]; inst1->bcy0 = xcy0[1]; inst1->ccy0 = xcy0[2]; inst1->dcy0 = xcy0[3];
				inst1->affmux = xffmux[0]; inst1->bffmux = xffmux[1]; inst1->cffmux = xffmux[2]; inst1->dffmux = xffmux[3];
				inst1->aoutmux = xoutmux[0]; inst1->boutmux = xoutmux[1]; inst1->coutmux = xoutmux[2]; inst1->doutmux = xoutmux[3];
				inst1->aused = xused[0]; inst1->bused = xused[1]; inst1->cused = xused[2]; inst1->dused = xused[3];
				inst1->precyinit = precyinit;
				inst1->set_config();
			}


			inst_class_list.push_back(*inst1);
		}

		if (pos1 != string::npos) {
			counter++;
			getline(xdlFile, line);
			pos2 = line.find(search2);//search for outpin
			pos3 = line.find(search3);//search for intpin
			pos4 = line.find(";");
			double_data outpin;
			int pos_oi = 0; int pos_ov = 0;//output associated inst location and output location in the vector in the inst
			while (pos4 == string::npos && xdlFile.good()) {
				//if it finds the output
				vector<string>::iterator find_id;

				// Note: if there is an inpin there will be an outpin connected to it
				if (pos2 != string::npos) {// if we find the outpin line
										   //get the info from the line
					outpin = line_info(line);
					//locating the outpin associated inst in the vector list					
					find_id = std::find(inst_name_list.begin(), inst_name_list.end(), outpin.data1);
					if (find_id != inst_name_list.end()) {
						pos_oi = find_id - inst_name_list.begin();//output instance location in the vector
						pos_ov = inst_class_list[pos_oi].outputs.size();//where will be this output sit in the output vector in the inst
						Out out1; out1.name = outpin.data2;
						inst_class_list[pos_oi].set_output(out1);
					}
					else {//error Missing Instance!!
						printf("Missing Instance");
						exit(EXIT_FAILURE);
					}
				}
				if (pos3 != string::npos) {// if it finds the inpin line
										   //get the info from the line
					double_data inpin = line_info(line);
					//locating the inpin associated inst in the vector list
					find_id = std::find(inst_name_list.begin(), inst_name_list.end(), inpin.data1);
					if (find_id != inst_name_list.end()) {
						int pos_i = find_id - inst_name_list.begin();
						inst_class_list[pos_i].set_input(inpin.data2, outpin.data1, pos_oi, outpin.data2, pos_ov);

					}
					else {//error Missing Instance!!
						printf("Missing Instance");
						exit(EXIT_FAILURE);
					}
				}
				counter++;
				getline(xdlFile, line);
				pos2 = line.find(search2);
				pos3 = line.find(search3);
				pos4 = line.find(";");

			}
		}
	}

	//=================================
	// Section Two: Traversing Through the data to find the I/O connection.
	//=================================

	//Set the probability to the inputs

	for (int i = 0; i < inst_class_list.size(); i++) {//find the IOB blocks
		if (inst_class_list[i].type == "IOB" && inst_class_list[i].outputs.size() == 1) {
			inst_class_list[i].outputs[0].p = 0.5;
		}
	}

	TraverseTop();
	trav_data.close();
	//string sout;
	out_data << "\n# =======================================================\n" <<
		"# Instanses I/O Connection Information:\n" <<
		"# =======================================================\n" << "\ninput Wired to: Instance (ID)[outport (ID)]\n";
	for (int tloc = 0; tloc < inst_class_list.size(); tloc++) {
		out_data << "\n==== " << to_string(tloc) << " ====\n" << inst_class_list[tloc].disp_info();// << "\n========\n";
		
		//sout += "\n";
	}

	string trav_out = "outputs.txt";
	ofstream trav_outs(trav_out);

	for (int tloc = 0; tloc < inst_class_list.size(); tloc++) {
		for (int oloc = 0; oloc < inst_class_list[tloc].outputs.size(); oloc++) {
			trav_outs << "\nFrom output: " << inst_class_list[tloc].outputs[oloc].name << " at inst: " << inst_class_list[tloc].name;
			for (int ploc = 0; ploc < inst_class_list[tloc].outputs[oloc].paths.size(); ploc++) {
				trav_outs << "\n\tpath " << to_string(ploc) << ":  \t";
				for (int pt = 0; pt < inst_class_list[tloc].outputs[oloc].paths[ploc].out.size(); pt++) {
					trav_outs << "-> " << to_string(inst_class_list[tloc].outputs[oloc].paths[ploc].tile[pt]) << "_" << to_string(inst_class_list[tloc].outputs[oloc].paths[ploc].out[pt]);
					//trav_data << sout;
				}
			}
			trav_outs << "\n";
		}
	}
	trav_outs.close();
	out_data.close();
		//system("PAUSE");
}


