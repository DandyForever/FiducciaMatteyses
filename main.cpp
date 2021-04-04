#include "fiducciaMatteyses.h"

using namespace std;

int main(int argc, char* argv[])
{
    if (argc < 2) {
        cout << "File not found" << endl;
        return -1;
    }
    int tolerance_percentage = 0;
    if (argc == 3) {
        tolerance_percentage = atoi (argv[2]);
    }
    size_t start_t = clock();
    string in_file_name = string(argv[1]);
    string out_file_name = in_file_name + ".part.2";
    string log_file_name = in_file_name + ".log";
    ofstream logger;
    logger.open(log_file_name);
    logger << "Algorithm Feduccie Mattheyses" << endl;
    Hypergraph hg(in_file_name, logger);
    Partitions partitions(hg, true, tolerance_percentage);
    size_t epoch = FM(partitions);
    size_t end_t = clock();
    size_t exec_t = (end_t - start_t) / 1000;
    partitions.dump(out_file_name);
    logger << "Total execution time is " << exec_t / 60 << " minutes " << exec_t % 60 << " seconds" << endl;
    logger.close();
    cout << "Test name: " << in_file_name <<
            " Num of vertecies " << hg.vert_num <<
            " Num of edges " << hg.edge_num <<
            " balance " << partitions.balance <<
            " cut " << partitions.solution_cost <<
            " time " << exec_t <<
            " #iters " << epoch << endl;
    return 0;
}

