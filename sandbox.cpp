#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
// #include <vector>
#include <map>
#include <cmath>

using namespace std;

map<int,int> * memory;

bool mem_oob( int addr ) {
	// This is arbitrary. Perhaps it would be better
	// to make it system dependent?
	return ( addr < 1 || addr > 1048576 );
}

bool process( int & addr ) {
	if ( mem_oob(abs(addr)) )
		return false;
	if ( addr < 0 )
		addr = (*memory)[-addr];
	if ( mem_oob(addr) )
		return false;
	return true;
}

// From: http://stackoverflow.com/questions/1798112/removing-leading-and-trailing-spaces-from-a-string
string trim( const string & str, const string & whitespace = " \t\r" ) {
    size_t strBegin = str.find_first_not_of(whitespace);
    if (strBegin == string::npos)
        return ""; // no content

    size_t strEnd = str.find_last_not_of(whitespace);
    size_t strRange = strEnd - strBegin + 1;

    return str.substr(strBegin, strRange);
}

int main( int argc, char** argv ) {
	if ( argc < 2 ) {
		cerr << "Error: No program file specified.\n";
		return 1;
	}
	
	// Read Program File ======================================================
	ifstream prog_file;
	prog_file.open(argv[1]);
	if ( ! prog_file.is_open() ) {
		cerr << "Error: Program file could not be opened.\n";
		return 1;
	}
	
	memory = new map<int,int>;
	
	// Read Data File =========================================================
	if ( argc > 2 ) {
		ifstream data_file;
		data_file.open( argv[2] );
		if ( ! data_file.is_open() ) {
			cerr << "Error: Data file could not be opened.\n";
			return 1;
		}
		
		string data_line;
		
		while ( getline( data_file, data_line ) ) {
			if ( data_line[data_line.size()-1] == '\r' )
				data_line.resize(data_line.size()-1);
			if ( data_line.empty() )
				continue;
			int addr, val;
			istringstream data_str(data_line);
			if ( ! ( data_str >> addr >> val ) ) {
				cerr << "Error: Data file format error.\n";
				return 2;
			}
			if ( mem_oob(addr) ) {
				cerr << "Error: Address out of bounds.\n";
				return 3;
			}
			(*memory)[addr] = val;
		}
		data_file.close();
	}
	
	// Run Program ============================================================
	
	// Reset to the beginning of the file
	prog_file.clear();
	prog_file.seekg( 0, ios::beg );
		
	string prog_line;

	bool exit_flag = false;
	
	while ( !exit_flag ) {
		if ( ! getline( prog_file, prog_line ) ) {
			cerr << "Error: Program File read error.\n";
			return 1;
		}
		if ( trim(prog_line).empty() )
			continue;
		istringstream prog_str(prog_line);
		string command;
		prog_str >> command;
		if ( command == "ADD" || command == "SUB" ) {
			int target, source_1, source_2;
			if ( ! ( prog_str >> target >> source_1 >> source_2 ) ) {
				cerr << "Error: Program file format error 1.\n";
				return 2;
			}
			if ( ! ( process(target) && process(source_1) && process(source_2) ) ) {
				cerr << "Error: Address out of bounds.\n";
				return 3;
			}
			if ( command == "ADD" )
				(*memory)[target] = (*memory)[source_1] + (*memory)[source_2];
			else
				(*memory)[target] = (*memory)[source_1] - (*memory)[source_2];
		} else if ( command == "COPY" ) {
			int target, source;
			if ( ! ( prog_str >> target >> source ) ) {
				cerr << "Error: Program file format error 2.\n";
				return 2;
			}
			if ( ! ( process(target) && process(source) ) ) {
				cerr << "Error: Address out of bounds.\n";
				return 3;
			}
			(*memory)[target] = (*memory)[source];
		} else if ( command == "SET" ) {
			int target, num;
			if ( ! ( prog_str >> target >> num ) ) {
				cerr << "Error: Program file format error 3.\n";
				return 2;
			}
			if ( ! process(target) ) {
				cerr << "Error: Address out of bounds.\n";
				return 3;
			}

			(*memory)[target] = num;
		} else if ( command == "BEQ" || command == "BNE" || command == "BG" ||
					command == "BGE" || command == "BL" || command == "BLE" ) {
			int label, source_1, source_2;
			if ( ! ( prog_str >> label >> source_1 >> source_2 ) ) {
				cerr << "Error: Program file format error 4.\n";
				return 2;
			}
			if ( ! ( process(source_1) && process(source_2) ) ) {
				cerr << "Error: Address out of bounds.\n";
				return 3;
			}
			bool flag = false;
			if ( command == "BEQ" ) {
				flag = (*memory)[source_1] == (*memory)[source_2];
			} else if ( command == "BNE" ) {
				flag = (*memory)[source_1] != (*memory)[source_2];
			} else if ( command == "BG" ) {
				flag = (*memory)[source_1] > (*memory)[source_2];
			} else if ( command == "BGE" ) {
				flag = (*memory)[source_1] >= (*memory)[source_2];
			} else if ( command == "BL" ) {
				flag = (*memory)[source_1] < (*memory)[source_2];
			} else if ( command == "BLE" ) {
				flag = (*memory)[source_1] <= (*memory)[source_2];
			}
			if ( flag ) {
				prog_file.clear();
				prog_file.seekg( 0, ios::beg );
				
				string read_line;
				while ( getline( prog_file, read_line ) ) {
					if ( trim(read_line).empty() )
						continue;
					istringstream read_str(read_line);
					string read_command;
					int read_label;
					if ( ! ( read_str >> read_command ) ) {
						cerr << "Error: Program file format error 5.\n";
						return 2;
					}
					if ( read_command == "LABEL" ) {
						if ( ! ( read_str >> read_label ) ) {
							cerr << "Error: Program file format error 6.\n";
							return 2;
						}
						if ( read_label == label ) {
							break;
						}
					}
				}
			}
		} else if ( command == "LABEL" ) {
			// Do nothing. This will be taken care of in branching.
		} else if ( command == "DISP" ) {
			int source;
			if ( ! ( prog_str >> source ) ) {
				cerr << "Error: Program file format error 8.\n";
				return 2;
			}
			if ( ! process(source) ) {
				cerr << "Error: Address out of bounds.\n";
				return 3;
			}
			cout << (*memory)[source] << "\n";
		} else if ( command == "EXIT" ) {
			int num;
			if ( ! ( prog_str >> num ) ) {
				cerr << "Error: Program file format error 9.\n";
				return 2;
			}
			if ( num != 0 ) {
				cout << "Return code: " << num << "\n";
			}
			exit_flag = true;
		} else if ( command == "//" || command.empty() ) {
			// Do nothing
		} else {
			cerr << "Error: Invalid syntax in Program File: " << command << ".\n";
			return 4;
		}
	}
	
	prog_file.close();
	
	return 0;
}
