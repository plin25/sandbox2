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
	
	int line_number = 0;
	
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
			line_number++;
			if ( trim(data_line).empty() )
				continue;
			int addr, val;
			istringstream data_str(data_line);
			// This is possibly a bad programming practice, to assign/mutate
			// variables in an if statement. Oh well. I use throughout.
			if ( ! ( data_str >> addr >> val ) ) {
				cerr << "Error: Data file format error (line " << line_number << ").\n";
				return 2;
			}
			if ( mem_oob(addr) ) {
				cerr << "Error: Address out of bounds in Data file (line " << line_number << ").\n";
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
	
	line_number = 0;
	
	string prog_line;
	bool exit_flag = false;
	
	while ( !exit_flag ) {
		if ( ! getline( prog_file, prog_line ) ) {
			cerr << "Error: Program File read error.\n";
			return 1;
		}
		line_number++;
		if ( trim(prog_line).empty() )
			continue;
		istringstream prog_str(prog_line);
		string command;
		prog_str >> command;
		
		// Arithmetic
		if ( command == "ADD" || command == "SUB" ) {
			int target, source_1, source_2;
			if ( ! ( prog_str >> target >> source_1 >> source_2 ) ) {
				cerr << "Error: Program file format error (arithmetic, line " << line_number << ").\n";
				return 2;
			}
			if ( ! ( process(target) && process(source_1) && process(source_2) ) ) {
				cerr << "Error: Address out of bounds (line " << line_number << ").\n";
				return 3;
			}
			if ( command == "ADD" )
				(*memory)[target] = (*memory)[source_1] + (*memory)[source_2];
			else
				(*memory)[target] = (*memory)[source_1] - (*memory)[source_2];
		
		// Copying
		} else if ( command == "COPY" ) {
			int target, source;
			if ( ! ( prog_str >> target >> source ) ) {
				cerr << "Error: Program file format error (copy, line " << line_number << ").\n";
				return 2;
			}
			if ( ! ( process(target) && process(source) ) ) {
				cerr << "Error: Address out of bounds (line " << line_number << ").\n";
				return 3;
			}
			(*memory)[target] = (*memory)[source];
		
		// Setting
		} else if ( command == "SET" ) {
			int target, num;
			if ( ! ( prog_str >> target >> num ) ) {
				cerr << "Error: Program file format error (set, line " << line_number << ").\n";
				return 2;
			}
			if ( ! process(target) ) {
				cerr << "Error: Address out of bounds (line " << line_number << ").\n";
				return 3;
			}
			(*memory)[target] = num;
		
		// Branching
		} else if ( command == "BEQ" || command == "BNE" || command == "BG" ||
					command == "BGE" || command == "BL" || command == "BLE" ) {
			int label, source_1, source_2;
			if ( ! ( prog_str >> label >> source_1 >> source_2 ) ) {
				cerr << "Error: Program file format error (branching, line " << line_number << ").\n";
				return 2;
			}
			if ( ! ( process(source_1) && process(source_2) ) ) {
				cerr << "Error: Address out of bounds (line " << line_number << ").\n";
				return 3;
			}
			bool flag = false;
			if ( command == "BEQ" )
				flag = (*memory)[source_1] == (*memory)[source_2];
			else if ( command == "BNE" )
				flag = (*memory)[source_1] != (*memory)[source_2];
			else if ( command == "BG" )
				flag = (*memory)[source_1] > (*memory)[source_2];
			else if ( command == "BGE" )
				flag = (*memory)[source_1] >= (*memory)[source_2];
			else if ( command == "BL" )
				flag = (*memory)[source_1] < (*memory)[source_2];
			else if ( command == "BLE" )
				flag = (*memory)[source_1] <= (*memory)[source_2];
			
			if ( flag ) {
				// Seek to the beginning of the file and scan
				// until the specified label is found
				prog_file.clear();
				prog_file.seekg( 0, ios::beg );
				line_number = 0;
				
				string read_line;
				bool read_success;
				while ( (read_success = getline( prog_file, read_line ) ) ) {
					line_number++;
					if ( trim(read_line).empty() )
						continue;
					istringstream read_str(read_line);
					string read_command;
					int read_label;
					if ( ! ( read_str >> read_command ) ) {
						cerr << "Error: Program file format error (line " << line_number << ").\n";
						return 2;
					}
					if ( read_command == "LABEL" ) {
						if ( ! ( read_str >> read_label ) ) {
							cerr << "Error: Program file format error (label, line " << line_number << ").\n";
							return 2;
						}
						if ( read_label == label )
							break;
					}
				}
				if ( !read_success ) {
					cerr << "Error: Program file format error: label " << label << " not found.\n";
					return 2;

				}
			}
		
		// Set Label
		} else if ( command == "LABEL" ) {
			// Do nothing. This will be taken care of in branching.
		
		// Display
		} else if ( command == "DISP" ) {
			int source;
			if ( ! ( prog_str >> source ) ) {
				cerr << "Error: Program file format error (display, line " << line_number << ").\n";
				return 2;
			}
			if ( ! process(source) ) {
				cerr << "Error: Address out of bounds (line " << line_number << ").\n";
				return 3;
			}
			cout << (*memory)[source] << "\n";
		
		// Exit
		} else if ( command == "EXIT" ) {
			int num;
			if ( ! ( prog_str >> num ) ) {
				cerr << "Error: Program file format error (exit, line " << line_number << ").\n";
				return 2;
			}
			if ( num != 0 ) {
				cout << "Return code: " << num << "\n";
			}
			exit_flag = true;
		
		// Comments/Empty strings
		} else if ( command == "//" || command.empty() ) {
			// Do nothing
		
		} else {
			cerr << "Error: Invalid syntax in Program File: " << command << " (line " << line_number << ").\n";
			return 4;
		}
	}
	
	prog_file.close();
	
	return 0;
}
