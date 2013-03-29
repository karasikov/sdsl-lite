/*
 * This program outputs the structure of an index.
 */
#include <sdsl/suffix_arrays.hpp>
#include <string>

#include <cstdlib>

using namespace sdsl;
using namespace std;

int main(int argc, char* argv[])
{
    char* filename;
    if (argc < 2)	{
        cout << "./" << argv[0] << " index_file " << endl;
        return 1;
    }
    CSA_TYPE csa;
    load_from_file(csa, string(argv[1]) + "." + string(SUF));
    write_structure<JSON_FORMAT>(csa, cout);
}